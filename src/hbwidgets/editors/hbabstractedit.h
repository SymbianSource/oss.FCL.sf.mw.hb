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

#ifndef HBABSTRACTEDIT_H
#define HBABSTRACTEDIT_H

#include <hbglobal.h>
#include <hbnamespace.h>
#include <hbwidget.h>
#include <hbsmileytheme.h>
#include <hbformatdialog.h>

#include <QTextDocument>
#include <QTextCursor>
#include <QAbstractTextDocumentLayout>

class HbEditItem;
class HbAbstractEditPrivate;
class HbScrollArea;
class HbValidator;
class HbMenu;

class HB_WIDGETS_EXPORT HbAbstractEdit: public HbWidget {

    Q_OBJECT

    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition)
    Q_PROPERTY(bool acceptableInput READ hasAcceptableInput)
    Q_PROPERTY(Qt::Alignment alignment WRITE setAlignment READ alignment)
    Q_PROPERTY(bool smileysEnabled READ isSmileysEnabled WRITE setSmileysEnabled)
    Q_PROPERTY(Hb::TextCursorVisibility cursorVisibility READ cursorVisibility WRITE setCursorVisibility)
    Q_PROPERTY(Hb::TextContextMenuFlags contextMenuFlags
               READ contextMenuFlags
               WRITE setContextMenuFlags)
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText)
    Q_PROPERTY(bool selectionControlEnabled READ isSelectionControlEnabled WRITE setSelectionControlEnabled)

public:
    virtual ~HbAbstractEdit();

    bool isReadOnly() const;
    void setReadOnly(bool value);

    int cursorPosition() const;

    Hb::TextCursorVisibility cursorVisibility() const;
    void setCursorVisibility(Hb::TextCursorVisibility value);

    HbValidator *validator() const;
    void setValidator(HbValidator *validator);
    bool hasAcceptableInput() const;

    QGraphicsItem *primitive(HbStyle::Primitive primitive) const;

    bool event(QEvent* event);

    enum { Type = Hb::ItemType_AbstractEdit };
    int type() const;

    void setAlignment(Qt::Alignment alignment); 
    Qt::Alignment alignment() const;

    Hb::TextContextMenuFlags contextMenuFlags() const;
    void setContextMenuFlags(Hb::TextContextMenuFlags flags);
    void setContextMenuFlag(Hb::TextContextMenuFlag flag);
    void clearContextMenuFlag(Hb::TextContextMenuFlag flag);

    QString placeholderText() const;
    void setPlaceholderText(const QString &text);

    QString anchorAt(const QPointF &pos) const;
    QString anchorAt(int pos) const;

    void setSmileyTheme(const HbSmileyTheme& theme);
    HbSmileyTheme smileyTheme() const;
    HbSmileyTheme defaultSmileyTheme() const;
    void resetSmileyTheme();

    bool isSmileysEnabled() const;
    void setSmileysEnabled(bool enabled);

    void setFormatDialog(HbFormatDialogPointer dialog);

    QChar characterAt(int pos) const;

    void setSelectionControlEnabled(bool enabled);
    bool isSelectionControlEnabled() const;

signals:
    void contentsChanged();
    void cursorPositionChanged(int oldPos, int newPos);
    void selectionChanged(const QTextCursor &oldCursor, const QTextCursor& newCursor);
    void aboutToShowContextMenu(HbMenu *contextMenu, const QPointF &pos);
    void anchorTapped(const QString &anchor);

public slots:
    void updatePrimitives();
    virtual void setCursorPosition(int pos, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
    void cut();
    void copy();
    void paste();
    void selectClickedWord();
    void selectAll();
    void deselect();
    void format();

protected:
    explicit HbAbstractEdit(QGraphicsItem *parent = 0);
    HbAbstractEdit(HbAbstractEditPrivate &dd, QGraphicsItem *parent);

    void drawContents(QPainter *painter, const QStyleOptionGraphicsItem &option);

    void resizeEvent(QGraphicsSceneResizeEvent *event);

    void inputMethodEvent(QInputMethodEvent *event);
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);

    void changeEvent(QEvent *event);
    void hideEvent(QHideEvent *event);

    void setDocument(QTextDocument *document);
    QTextDocument *document() const;

    void setTextCursor(const QTextCursor &cursor);
    QTextCursor textCursor() const;

    QString toPlainText() const;
    QString toHtml() const;

    void moveCursor(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

    QRectF blockBoundingRect(const QTextBlock &block) const;

    bool isScrollable() const;
    void setScrollable(bool value);

    HbScrollArea *scrollArea() const;

    virtual HbMenu* createContextMenu();
    void showContextMenu(QPointF position = QPointF(0,0));
    virtual QMimeData *createMimeDataFromSelection() const;
    QString filterInputText(const QString &text);
    virtual bool canInsertFromMimeData(const QMimeData *source) const;
    virtual void insertFromMimeData(const QMimeData *source);

    QRectF rectForPosition(int position, QTextLine::Edge edge = QTextLine::Leading) const;

    void updateGeometry();

    void polish( HbStyleParameters& params );
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void gestureEvent(QGestureEvent* e);

protected slots:
    void setPlainText(const QString &text);
    void setHtml(const QString &text);

    virtual void documentLayoutChanged();
    virtual void documentSizeChanged(const QSizeF &size);
    virtual void blockCountChanged(int newBlockCount);

    void timerEvent(QTimerEvent *e);

private:
    Q_DISABLE_COPY(HbAbstractEdit)
    Q_DECLARE_PRIVATE_D(d_ptr, HbAbstractEdit)

    Q_PRIVATE_SLOT(d_func(), void _q_updateRequest(QRectF))
    Q_PRIVATE_SLOT(d_func(), void _q_updateBlock(QTextBlock))
    Q_PRIVATE_SLOT(d_func(), void _q_contentsChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_contentsChange(int, int, int))
    Q_PRIVATE_SLOT(d_func(), void _q_selectionChanged(const QTextCursor&, const QTextCursor&))
    Q_PRIVATE_SLOT(d_func(), void _q_scrollStarted())
    Q_PRIVATE_SLOT(d_func(), void _q_scrollEnded())


    friend class HbEditItem;
    friend class HbEditorFocusObject;
    friend class HbSelectionControl;
    friend class HbSelectionControlPrivate;
    friend class HbFormatDialog;
    friend class HbFormatDialogPrivate;
};

#endif // HBABSTRACTEDIT_H
