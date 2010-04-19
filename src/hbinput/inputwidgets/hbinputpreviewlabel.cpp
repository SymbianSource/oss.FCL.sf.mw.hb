/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbInput module of the UI Extensions for Mobile.
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
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <hbcolorscheme.h>
#include "hbinputpreviewlabel.h"
#include "hbfontspec.h"

/*!
    \fn void HbPreviewLabel::selected()

    This signal is emitted when there is a mouse press event.
*/

/*!
Constructor.
@param previewSymbols The string of the label and parent Graphics Item.
*/
HbPreviewLabel::HbPreviewLabel(QString previewSymbols, QGraphicsItem *parent)
    :HbWidget(parent),
    mTextItem(0)
{
        mTextItem = static_cast<HbTextItem*>(this->style()->createPrimitive(HbStyle::P_Label_text, this));
        mTextItem->setText(previewSymbols);
        mTextItem->setAlignment(Qt::AlignCenter);
        mTextItem->setFontSpec(HbFontSpec(HbFontSpec::Primary));
}

/*!
Destroy the object
*/
HbPreviewLabel::~HbPreviewLabel()
{
}

/*!
sets geometry of textItem
@param itemsize the Geometry of Button.
*/
void HbPreviewLabel::setTextGeometry(qreal width, qreal height)
{
	QColor color = HbColorScheme::color("qtc_editor_normal");
    QRectF rect;
    rect.setWidth(width/2);
    rect.setHeight(height);
    if (color.isValid()) {
        mTextItem->setTextColor(color);
    }
    mTextItem->setGeometry(rect);
}

/*!
This function handles the mouse press event.
@param event The mouse events in the graphics view framework.
*/
void HbPreviewLabel::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    Q_UNUSED(event);
    emit showAccentedPreview(mTextItem->text(),sceneBoundingRect());
}

/*!
This function handles the mouse move event.
@param event The mouse events in the graphics view framework.

In case of a key/touch movement will activate a mousePressEvent on another Label
and the next Label is set as the grabber item.
*/
void HbPreviewLabel::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    bool transfered = false;
    if (!isUnderMouse()) {
        // get the list of item's at current mouse position
        QList<QGraphicsItem *> list = scene()->items(event->scenePos());
        for (int i =0; i < list.count(); i++) {
            // let's check if we have HbPreviewLabel
            HbPreviewLabel *label  = hbpreviewlabel_cast(list.at(i));
            if (label  && label->isEnabled() && (label->parent() == parent())) {
                // release old label
                ungrabMouse();
                event->setButton(Qt::LeftButton);
                // now call the mousepressEvent function of the Label under mouse
                QGraphicsSceneMouseEvent pressEvent;
                pressEvent.setButton(Qt::LeftButton);
                label->mousePressEvent(&pressEvent);
                // now to make label under cursor to get mouse events we have to manually make that Label
                // a mouse grabber item. after this Label will start recieving the press movements.
                label->grabMouse();
                transfered = true;
                break;
            }
        }
        QRectF rect = boundingRect();
        QPointF pos = event->pos();
        if (!transfered && (pos.y() < 0  ||pos.y() > rect.height() || pos.x() > rect.width() || pos.x() < 0) ) {
            emit hideAccentedPreview();
        }
    }
    if (!transfered) {
        if (isUnderMouse()) {
            QGraphicsSceneMouseEvent pressEvent;
            pressEvent.setButton(Qt::LeftButton);
            HbPreviewLabel::mousePressEvent(&pressEvent);
        }
    }
}

/*!
This function handles the mouse release event.
@param event The mouse events in the graphics view framework.

emits the signal selected to input the corrosponding character mapped to the key whenever
a key is released and afterwards hides the PreviewPopup by emiitting hidePreview
*/
void HbPreviewLabel::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    Q_UNUSED(event);
    if (isUnderMouse()) {
        emit selected();
    }
    emit hidePreview();
    ungrabMouse();
}

// End Of File
