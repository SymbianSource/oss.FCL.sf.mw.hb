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

#ifndef HBABSTRACTEDIT_P_H
#define HBABSTRACTEDIT_P_H

//#define HB_DEBUG_EDITOR_DRAW_RECTS
//#define HB_DEBUG_EDITOR_HANDLES

#include "hbglobal.h"
#include "hbstyle.h"
#include "hbwidget_p.h"
#include "hbscrollarea.h"
#include "hbscrollarea_p.h"
#include "hbabstractedit.h"
#include "hbvalidator.h"
#include "hbformatdialog.h"

#include <QEvent>
#include <QTextCursor>
#include <QBasicTimer>
#include <QTextDocumentFragment>
#include <QPainter>
#include <QDebug>

class QEvent;
class QGraphicsSceneMouseEvent;
class QTextDocument;
class HbWidget;
class HbScrollArea;
class HbValidator;
class HbSelectionControl;
class HbSmileyEngine;
class HbFormatDialog;

class HbAbstractEditMimeData : public QMimeData
{
public:
    inline HbAbstractEditMimeData(const QTextDocumentFragment &aFragment) : fragment(aFragment) {}

    virtual QStringList formats() const;
protected:
    virtual QVariant retrieveData(const QString &mimeType, QVariant::Type type) const;
private:
    void setup() const;

    mutable QTextDocumentFragment fragment;
};

class HB_AUTOTEST_EXPORT HbAbstractEditPrivate: public HbWidgetPrivate {
    Q_DECLARE_PUBLIC(HbAbstractEdit)

    typedef HbValidator::ChangeSource CursorChange;

public:
    HbAbstractEditPrivate ();
    virtual ~HbAbstractEditPrivate ();

    void init();
    virtual void updatePaletteFromTheme();

    void acceptKeyPressEvent(QKeyEvent *event);
public:
    void removeCurrentDocument();
    void connectToNewDocument(QTextDocument *newDoc);
    void setContent(Qt::TextFormat format = Qt::RichText, const QString &text = QString());
    void ensurePositionVisible(int position);
    void ensureCursorVisible();    
    bool setFocusToAnchor(const QTextCursor &newCursor);
    bool setFocusToNextOrPreviousAnchor(bool next);
    bool findNextPrevAnchor(const QTextCursor& from, bool next, QTextCursor& newAnchor);
    void setCursorPosition(int pos, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
    bool cursorMoveKeyEvent(QKeyEvent *e);
    void repaintOldAndNewSelection(const QTextCursor &oldSelection);
    void updateCurrentCharFormat();
    QRectF cursorRectPlusUnicodeDirectionMarkers(int position) const;
    void setBlinkingCursorEnabled(bool enable);
    void repaintCursor();
    void setTextInteractionFlags(Qt::TextInteractionFlags flags);
    void cursorChanged(CursorChange origin);
    void selectionChanged(bool forceEmitSelectionChanged = false);
    QAbstractTextDocumentLayout::PaintContext getPaintContext() const;
    int hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const;
    QRectF cursorRect(const QTextCursor &cursor) const;
    QRectF cursorRect() const;
    QRectF selectionRect(const QTextCursor &cursor) const;
    QRectF selectionRect() const;
    QRectF rectForPositionInCanvasCoords(int position, QTextLine::Edge edge) const;
    QRectF viewPortRect() const;
    QValidator::State validateContent(int position, int charsRemoved, int charsAdded);
    void initValidator();
    bool undo();
    virtual bool canPaste() const;
    virtual bool canCopy() const;
    virtual bool canCut() const;
    virtual void prepDocForPaste();
    bool canFormat() const;
    virtual bool isCursorVisible() const;

    void longTapGesture(const QPointF &point);
    void tapGesture(const QPointF &point);
    void gestureReceived();

    void sendInputPanelEvent(QEvent::Type type);
    void openInputPanel();
    void closeInputPanel();
    void minimizeInputPanel();

    int contentLength() const;
    bool hasAcceptableInput() const;
    void sendMouseEventToInputContext(const QPointF &tapPos) const;
    virtual void updateEditingSize();
    void drawSelectionEdges(QPainter *painter, QAbstractTextDocumentLayout::PaintContext);
    HbSmileyEngine* smileyEngineInstance() const;

    virtual void drawContentBackground(QPainter *painter,
                                       const QStyleOptionGraphicsItem &option) const;

    void updatePlaceholderDocProperties();
    void filterInputText(QString &text) const;

    void _q_updateRequest(QRectF rect);
    void _q_updateBlock(QTextBlock block);
    void _q_contentsChanged();
    void _q_contentsChange(int position, int charsRemoved, int charsAdded);
    void _q_selectionChanged();
    void _q_scrollStarted();
    void _q_scrollEnded();
    static Qt::Alignment alignmentFromString(const QString &text);

    void validateAndCorrect();

    QTextDocument *doc;
    QTextDocument *placeholderDoc;

    int previousCursorAnchor;
    int previousCursorPosition;
    QTextCursor cursor;
    QTextCursor selectionCursor;

    QPointer<HbValidator> validator;
    bool imEditInProgress;
    int imPosition;
    int imAdded;
    int imRemoved;

    Qt::TextInteractionFlags interactionFlags;
    QPointF tapPosition;
    bool cursorOn;

    QTextCharFormat lastCharFormat;
    QBasicTimer cursorBlinkTimer;

    int preeditCursor;
    bool preeditCursorVisible; // used to hide the cursor in the preedit area
    bool apiCursorVisible;

    HbWidget *canvas;
    HbScrollArea *scrollArea;

    bool scrollable;
    bool hadSelectionOnMousePress;

    HbSelectionControl *selectionControl;

    bool acceptSignalContentsChange;
    bool acceptSignalContentsChanged;

    int validRevision;

    bool wasGesture;

    Hb::TextContextMenuFlags contextMenuShownOn;
    bool smileysEnabled;  
    mutable HbSmileyEngine *smileyEngine;

    HbFormatDialogPointer formatDialog;
    QTextCursor nextCharCursor;

    bool updatePrimitivesInProgress;

private:
    static HbAbstractEditPrivate *d_ptr(HbAbstractEdit *edit) {
        Q_ASSERT(edit);
        return edit->d_func();
    }
    friend class HbFormatDialog;
    friend class HbFormatDialogPrivate;
    // To be able to unit test private features
    friend class TestHbAbstractEdit;
    friend class TestHbLineEdit;
    friend class HbSelectionControl;
    friend class HbSelectionControlPrivate;
};

#endif // HBABSTRACTEDIT_P_H
