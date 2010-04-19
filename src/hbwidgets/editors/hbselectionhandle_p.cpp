/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbWidgets module of the UI Extensions for Mobile.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at developer.feedback@nokia.com.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Hb API.  It exists purely as an
// implementation detail.  This file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "hbselectionhandle_p.h"
#include "hbstyleoption.h"

#include "hbicon.h"
#include "hbmenu.h"
#include "hbaction.h"

#include "hbinstance.h"

#include <QtDebug>

#include <QTextCursor>
#include <QTextDocument>
#include <QTextBlock>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QPointF>
#include <QPainter>
#include <QApplication>
#include <QClipboard>

const QSizeF HandleSize = QSizeF(10, 10);
const QRectF MouseTreshold = QRectF(-5, -5, 5, 5);
const QPointF MenuOffset = QPointF(10, 50);

HbSelectionHandle::HbSelectionHandle (HandleType type, HbTextControl *control, QGraphicsItem *parent) :
    QGraphicsItem(parent), mControl(control), mType(type)
{
    hide(); // defaults to hidden

    if (mType == Cursor) {
        mIcon = HbIcon("qtg_graf_editor_handle_end.svg");
    } else {
        mIcon = HbIcon("qtg_graf_editor_handle_begin.svg");
    }

    connect(mControl, SIGNAL(selectionChanged()), this, SLOT(cursorChanged()));

    cursorChanged();
}

void HbSelectionHandle::paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    painter->save();
    mIcon.setSize(HandleSize);
    mIcon.paint(painter, boundingRect());
    painter->restore();
}

QRectF HbSelectionHandle::boundingRect() const
{
    return QRectF(QPointF(0, 0), HandleSize);
}

void HbSelectionHandle::mouseMoveEvent (QGraphicsSceneMouseEvent *event)
{
    float x1 = event->scenePos().x();
    float x2 = mMousePos.x();

    if (qAbs(x1 - x2) > HandleSize.width()) {
        mMousePos = event->scenePos();
        if (x1 > x2) {
            movePosition(QTextCursor::NextCharacter);
        } else if(x1 < x2) {
            movePosition(QTextCursor::PreviousCharacter);
        }
    }

    float y1 = event->scenePos().y();
    float y2 = mMousePos.y();

    if (qAbs(y1 - y2) > HandleSize.height()) {
        mMousePos = event->scenePos();
        if (y1 > y2) {
            movePosition(QTextCursor::Down);
        } else if(y1 < y2) {
            movePosition(QTextCursor::Up);
        }
    }
}

void HbSelectionHandle::movePosition(QTextCursor::MoveOperation op)
{
    if (mType == Cursor) {
        QTextCursor cursor = mControl->textCursor();
        cursor.movePosition(op, QTextCursor::KeepAnchor);
        mControl->setTextCursor(cursor);
    } else {
        QTextCursor cursor = mControl->textCursor();
        QTextCursor c = QTextCursor(mControl->document());
        c.setPosition(cursor.position());
        c.setPosition(cursor.anchor(), QTextCursor::KeepAnchor);
        c.movePosition(op, QTextCursor::KeepAnchor);
        cursor.setPosition(c.position());
        cursor.setPosition(c.anchor(), QTextCursor::KeepAnchor);
        mControl->setTextCursor(cursor);
    }
}

void HbSelectionHandle::mousePressEvent (QGraphicsSceneMouseEvent *event)
{
    mMousePressPos = mMousePos = event->scenePos();
    event->accept();
}

void HbSelectionHandle::mouseReleaseEvent (QGraphicsSceneMouseEvent *event)
{
    event->accept();
    if (MouseTreshold.contains(mMousePressPos - event->scenePos())) {
        showMenu(event->scenePos() + MenuOffset);
    }
}

void HbSelectionHandle::cursorChanged ()
{
    QRectF r;
    QPointF p;

    int c = mControl->textCursor().position();
    int a = mControl->textCursor().anchor();

    if (mType == Cursor) {
        r = rectForPosition(c);
        p = r.topRight();
        if (c < a) {
            p.rx() -= HandleSize.width();
        }
    } else {
        r = rectForPosition(mControl->textCursor().anchor());
        p = r.topLeft();
        if (c >= a) {
            p.rx() -= HandleSize.width();
        }
    }

    if (c >= a) {
        mIcon.setMirroringMode(HbIcon::Prevented);
    } else {
        mIcon.setMirroringMode(HbIcon::Forced);
    }

    if (QApplication::layoutDirection() == Qt::RightToLeft) {
        p.rx() = parentItem()->boundingRect().right() - p.x();
    }
    setPos(p);
}

void HbSelectionHandle::showMenu(QPointF position)
{
    HbMenu *menu = new HbMenu();
    HbMenuItem *mi;

    mi = menu->addAction("Cut");
    connect(mi->action(), SIGNAL(triggered()), SLOT(cut()));

    mi = menu->addAction("Copy");
    connect(mi->action(), SIGNAL(triggered()), SLOT(copy()));

    mi = menu->addAction("Paste");
    connect(mi->action(), SIGNAL(triggered()), SLOT(paste()));

    menu->setMenuType(HbMenu::MenuContext);
    menu->exec(position);
}

void HbSelectionHandle::copy ()
{
   QString selectedText = mControl->textCursor().selectedText();
#ifndef QT_NO_CLIPBOARD
   QClipboard *clipboard = QApplication::clipboard();
   clipboard->setText(selectedText);
#else
   mClipboard = selectedText;
#endif
}

void HbSelectionHandle::cut ()
{
    copy();
    mControl->textCursor().removeSelectedText();
}

void HbSelectionHandle::paste ()
{
#ifndef QT_NO_CLIPBOARD
   QClipboard *clipboard = QApplication::clipboard();
   QString clipText = clipboard->text();
#else
   QString clipText = mClipboard;
#endif
   mControl->textCursor().removeSelectedText();
   mControl->textCursor().insertText(clipText);
}

#include <QAbstractTextDocumentLayout>
#include <QTextLayout>
#include <QTextLine>

QRectF HbSelectionHandle::rectForPosition(int position) const
{
    const QTextBlock block = mControl->document()->findBlock(position);

    if (!block.isValid())
        return QRectF();

    const QTextLayout *layout = block.layout();

    const QPointF layoutPos = blockBoundingRect(block).topLeft();

    int relativePos = position - block.position();

    QTextLine line = layout->lineForTextPosition(relativePos);

    QRectF r;
    qreal w = 1;

    if (line.isValid()) {
        qreal x = line.cursorToX(relativePos);
        r = QRectF(layoutPos.x() + x, layoutPos.y() + line.y(),
                   w, line.height());
    } else {
        r = QRectF(layoutPos.x(), layoutPos.y(), w, HandleSize.height()); // #### correct height
    }

    return r;
}

QRectF HbSelectionHandle::blockBoundingRect(const QTextBlock &block) const
{
    return mControl->document()->documentLayout()->blockBoundingRect(block);
}
