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

#include "hbabstractedit.h"
#include "hbabstractedit_p.h"

#include "hbvalidator.h"
#include "hbstyle.h"
#include "hbstyleoption.h"
#include "hbwidget.h"
#include "hbscrollarea.h"
#include "hbevent.h"
#include <hbwidgetfeedback.h>
#include "hbmenu.h"
#include "hbaction.h"
#include "hbselectioncontrol_p.h"
#include "hbmeshlayout_p.h"
#include "hbsmileyengine.h"
#include "hbinputeditorinterface.h"

#include <QApplication>
#include "hbpopup.h"
#include "hbformatdialog.h"
#include <QTextList>
#include <QFontMetrics>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QClipboard>
#include <QInputContext>

/*!
    \class HbAbstractEdit
    \brief HbAbstractEdit is a base class for all text editors.
    @proto
    @hbwidgets

    This is not mentioned as a class to be used directly. All editors inherit this and
    basic functionality is in this class.

    It uses \ref QTextDocument internally to handle the content. And in some editors the document is also exposed to direct access and manipulation.

    \section functionality Functionality this class provides

    \li Editor content: The editor is set read-only with method \ref setReadOnly. Content changes emit a \ref contentsChanged signal.

    \li Cursor position and visibility:  Cursor position can be set with slot \ref setCursorPosition and 
    read with method \ref cursorPosition(). It is also a property. Cursor position changes emit a \ref cursorPositionChanged signal. 
    Cursor visibility can be controlled with \ref setCursorVisibility.

    \li Editing: There is a group of useful methods for editing the content. Clipboard functionality can be found 
    in \ref cut, \ref copy and \paste.  Finishing editing emits a \ref editingFinished signal and text changes 
    emit a \ref textChanged signal. 

    \li Selection handling: Whenever there is a interactive selection special handles are shown which can be used to manipulate the selection. 
    Selection changes emit a \ref selectionChanged signal.

    \li Anchor support: Anchors are strings associated to some a content in editor. There are two \ref anchorAt methods to access the anchors. 
    When user invokes an anchor it will open a context menu that may contain operations related to the anchor item (e.g. creating calls 
    for phone numbers, opening a web browser for web-addresses, etc.). This is always application specific functionality. 
    Editor itself does not create any anchors by examining the content.

    There is also protected API in HbAbstractEdit. Some of these API methods can be exposed as public in more specific editor widgets.

    \li Editor content:  Both rich text and plain text are supported in HbAbstractEdit. Depending on the actual 
    editor widget only one or the other type might be supported. Plain text content is set with 
    method \ref setPlainText. Method \ref toPlainText returns the content converted to plain text. 

    \li Focus control: Editor is responsble for controlling the input panel visibility. In some situations special editor may need to also handle this, but in general it is always handled by HbAbstractEdit. 
    Input panel is closed when editor loses focus.

    \li Cursor control: A convenience method \ref moveCursor can also be used to move the cursor.

    \li Scrolling: When e.g. the size of the content exceeds the size of editor, method \ref setScrollable
    can be used to make the editor scrollable. Method isScrollable tells whether the editor is scrollable or not. 
    Method scrollArea returns the used scroll area widget. Scrolling is enabled by default.

    \li Input validation: The validator for restricting the content input is set with method \ref setValidator. Only \ref HbValidator
    based validators can be used.

    \section composition Widget composition

    Editor widget consists of these sub-items visible in the picture. 
    In this example editor shows only one visible line of the content.

    \image html editor_composition.jpg

    The size of the canvas is always same as the size of the document, and the width of the scroll area 
    is same as the width of the document. The editor margins are defined in the CSS.

*/

/*!
    \fn void HbAbstractEdit::contentsChanged()

    This signal is emitted whenever the text changes. The text argument is the new text.
    This signal is also emitted when the text is changed programmatically, for example,
    by calling setPlainText().
*/

/*!
    \fn void HbAbstractEdit::selectionChanged(const QTextCursor &oldCursor, const QTextCursor &newCursor)

    This signal is emitted whenever the selection changes. */

/*!
    \fn void HbAbstractEdit::cursorPositionChanged(int oldPos, int newPos)

    This signal is emitted when cursor position has been changed.
*/

/*!
    \fn QString HbAbstractEdit::toPlainText() const

    Returns the contents as plain text.

    \sa QTextDocument::toPlainText()
*/

#define Hb_Invalid_Position QPointF(-1.0, -1.0)

/*!
    Constructs a HbAbstractEdit with parent \a parent.
 */
HbAbstractEdit::HbAbstractEdit(QGraphicsItem *parent) :
    HbWidget(*new HbAbstractEditPrivate, parent)
{
    Q_D(HbAbstractEdit);
    d->q_ptr = this;

    d->init();
}

/*!
    Constructs a HbAbstractEdit with abstract edit private object \a dd and parent \a parent.
 */
HbAbstractEdit::HbAbstractEdit(HbAbstractEditPrivate &dd, QGraphicsItem *parent) :
    HbWidget(dd, parent)
{
    Q_D(HbAbstractEdit);
    d->q_ptr = this;

    d->init();
}

/*!
    Destructor.
*/
HbAbstractEdit::~HbAbstractEdit()
{
}

/*!
    \reimp
*/
void HbAbstractEdit::resizeEvent(QGraphicsSceneResizeEvent *event)
{    
    HbWidget::resizeEvent(event);
    updatePrimitives();
}

/*!
    \reimp
*/
bool HbAbstractEdit::event(QEvent* event)
{
    if(event->type() ==  QEvent::DynamicPropertyChange) {
        const QString p = static_cast<QDynamicPropertyChangeEvent *>(event)->propertyName();
        if(p == "SmileyIcon") {
            Q_D(HbAbstractEdit);
            QString iconName = property("SmileyIcon").toString();

            HbSmileyTheme smileyTheme = d->smileyEngineInstance()->theme();

            QString pattern = smileyTheme.patterns(iconName).first();

            QTextCursor cursor = textCursor();
            int oldPos = cursor.position();

            cursor.insertText(pattern);
            setTextCursor(cursor);

            QTextCursor matchCursor(document());
            matchCursor.setPosition(oldPos);
            d->smileyEngineInstance()->insertSmiley(matchCursor);

            d->cursorChanged(HbValidator::CursorChangeFromContentUpdate);
        }
    }
    return HbWidget::event(event);
}

/*!
    \reimp
*/
bool HbAbstractEdit::eventFilter(QObject *obj, QEvent *e)
{
    Q_D(HbAbstractEdit);

    if (obj == d->scrollArea
        && (e->type() == QEvent::GraphicsSceneMousePress
        || e->type() == QEvent::GraphicsSceneMouseMove
        || e->type() == QEvent::GraphicsSceneMouseRelease)) {

        // map e->pos to the editor's coordinate system
        QGraphicsSceneMouseEvent *event = static_cast<QGraphicsSceneMouseEvent*>(e);
        QPointF oldPos = event->pos();
        event->setPos(mapFromItem(d->scrollArea,oldPos));
        bool ret = sceneEvent(e);
        event->setPos(oldPos);
        return ret;
    }

    return false;
}

QVariant HbAbstractEdit::inputMethodQuery (Qt::InputMethodQuery query) const
{
    Q_D(const HbAbstractEdit);

    QTextBlock block = d->cursor.block();
    switch(query) {
    case Qt::ImMicroFocus:
        return d->cursorRect();
    case Qt::ImFont:
        return QVariant(d->cursor.charFormat().font());
    case Qt::ImCursorPosition:
        return QVariant(d->cursor.position() - block.position());
    case Qt::ImSurroundingText:
        return QVariant(block.text());
    case Qt::ImCurrentSelection:
        return QVariant(d->cursor.selectedText());
    default:
        return QVariant();
    }
}

void HbAbstractEdit::inputMethodEvent (QInputMethodEvent *e)
{
    Q_D(HbAbstractEdit);
    
    
    if (!(d->interactionFlags & Qt::TextEditable) || d->cursor.isNull()) {
        e->ignore();
        return;
    }

    QTextBlock block = d->cursor.block();
    QTextLayout *layout = block.layout();

    bool isGettingInput = !e->commitString().isEmpty()
            || e->preeditString() != layout->preeditAreaText()
            || e->replacementLength() > 0;

    d->cursor.beginEditBlock();

    if (isGettingInput) {
        if (!d->imEditInProgress) {
            d->imEditInProgress = true;
            d->imPosition = d->selectionCursor.selectionStart();
            d->imRemoved = d->selectionCursor.selectionEnd() - d->selectionCursor.selectionStart();
        }

        d->selectionCursor.removeSelectedText();
    }

    // insert commit string
    if (!e->commitString().isEmpty() || e->replacementLength()) {
        QTextCursor c = d->cursor;
        c.setPosition(c.position() + e->replacementStart());
        c.setPosition(c.position() + e->replacementLength(), QTextCursor::KeepAnchor);
        QString commitString = e->commitString();
        c.insertText(commitString, d->nextCharCursor.charFormat());
        d->imAdded = commitString.length();
    }

    if (isGettingInput) {
        layout->setPreeditArea(d->cursor.position() - block.position(), e->preeditString());
    }

    QList<QTextLayout::FormatRange> overrides;
    d->preeditCursor = e->preeditString().length();

    d->preeditCursorVisible = true;
    for (int i = 0; i < e->attributes().size(); ++i) {
        const QInputMethodEvent::Attribute &a = e->attributes().at(i);
        if (a.type == QInputMethodEvent::Cursor) {
            d->preeditCursor = a.start;
            d->preeditCursorVisible = a.length;
        } else if (a.type == QInputMethodEvent::TextFormat) {
            QTextCharFormat f = qvariant_cast<QTextFormat>(a.value).toCharFormat();
            if (f.isValid()) {
                QTextLayout::FormatRange o;
                o.start = a.start + d->cursor.position() - block.position();
                o.length = a.length;
                o.format = f;
                overrides.append(o);
            }
        }
    }
    layout->setAdditionalFormats(overrides);

    d->acceptSignalContentsChange = false;
    d->acceptSignalContentsChanged = false;
    d->cursor.endEditBlock();
    d->acceptSignalContentsChange = true;
    d->acceptSignalContentsChanged = true;

    d->cursorChanged(HbValidator::CursorChangeFromContentUpdate);

    // Input method editing is done
    if (!e->commitString().isEmpty() || e->replacementLength()) {
        d->imEditInProgress = false;
        d->imPosition = e->replacementStart();
        d->imAdded = e->commitString().length();
        d->imRemoved =  e->replacementLength();
        d->_q_contentsChanged();
    }

    if(hasFocus()) {
        d->cursorOn = true;
    }
    d->ensureCursorVisible();
}

/*!
    \reimp
*/
void HbAbstractEdit::keyPressEvent(QKeyEvent *event)
{
    Q_D(HbAbstractEdit);

    if (d->interactionFlags & Qt::NoTextInteraction)
        return;

#ifndef QT_NO_SHORTCUT
#ifndef QT_NO_CLIPBOARD
    if (event == QKeySequence::Copy) {
        event->accept();
        copy();
        return;
    }
    else if (event == QKeySequence::Cut) {
        cut();
        d->acceptKeyPressEvent(event);
        return;
    }
    else if (event == QKeySequence::Paste) {
        paste();
        d->acceptKeyPressEvent(event);
        return;
    }
#endif
#endif // QT_NO_SHORTCUT

    if (d->interactionFlags & Qt::TextSelectableByKeyboard
        && d->cursorMoveKeyEvent(event))
        d->acceptKeyPressEvent(event);

    if (!(d->interactionFlags & Qt::TextEditable)) {
        d->repaintOldAndNewSelection(d->selectionCursor);
        d->cursorChanged(HbValidator::CursorChangeFromContentUpdate);
        //ensureCursorVisible();
        event->ignore();
        return;
    }

    if (event->key() == Qt::Key_Direction_L || event->key() == Qt::Key_Direction_R) {
        QTextBlockFormat fmt;
        fmt.setLayoutDirection((event->key() == Qt::Key_Direction_L) ? Qt::LeftToRight : Qt::RightToLeft);
        d->cursor.mergeBlockFormat(fmt);
        d->acceptKeyPressEvent(event);
    }

    // schedule a repaint of the region of the cursor, as when we move it we
    // want to make sure the old cursor disappears (not noticeable when moving
    // only a few pixels but noticeable when jumping between cells in tables for
    // example)
    d->repaintOldAndNewSelection(QTextCursor());

    if (event->key() == Qt::Key_Backspace && !(event->modifiers() & ~Qt::ShiftModifier)) {
        QTextBlockFormat blockFmt = d->cursor.blockFormat();
        QTextList *list = d->cursor.currentList();
        if (list && d->cursor.atBlockStart()) {
            list->remove(d->cursor.block());
        } else if (d->cursor.atBlockStart() && blockFmt.indent() > 0) {
            blockFmt.setIndent(blockFmt.indent() - 1);
            d->cursor.setBlockFormat(blockFmt);
        } else if (d->selectionCursor.hasSelection()) {
            d->selectionCursor.removeSelectedText();
        } else {
            QTextCursor oldCursor(d->cursor);
            oldCursor.deletePreviousChar();
        }
        d->ensureCursorVisible();
    }
#ifndef QT_NO_SHORTCUT
      else if (event == QKeySequence::InsertParagraphSeparator) {
        d->cursor.insertBlock();
        d->acceptKeyPressEvent(event);
    } else if (event == QKeySequence::InsertLineSeparator) {
        d->cursor.insertText(QString(QChar::LineSeparator));
        d->acceptKeyPressEvent(event);
    }
#endif
    if (false) {
    }
#ifndef QT_NO_SHORTCUT
      else if (event == QKeySequence::Delete) {
        d->cursor.deleteChar();
        d->acceptKeyPressEvent(event);
    } else if (event == QKeySequence::DeleteEndOfWord) {
        d->cursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
        d->cursor.removeSelectedText();
        d->acceptKeyPressEvent(event);
    } else if (event == QKeySequence::DeleteStartOfWord) {
        d->cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
        d->cursor.removeSelectedText();
        d->acceptKeyPressEvent(event);
    } else if (event == QKeySequence::DeleteEndOfLine) {
        QTextBlock block = d->cursor.block();
        if (d->cursor.position() == block.position() + block.length() - 2) {
            d->cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        } else {
            d->cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        }
        d->cursor.removeSelectedText();
        d->acceptKeyPressEvent(event);
    }
#endif // QT_NO_SHORTCUT
    else {

    QString text = event->text();
    if (!text.isEmpty()
        && (text.at(0).isPrint()
        || text.at(0) == QLatin1Char('\t')
        || (d->scrollable
        && (text.at(0) == QLatin1Char('\n')
        || text.at(0) == QLatin1Char('\r'))))) {
            d->cursor.insertText(text, d->nextCharCursor.charFormat());
            d->acceptKeyPressEvent(event);
        }
    }

    d->cursorChanged(HbValidator::CursorChangeFromContentUpdate);
}

/*!
    \reimp
*/
void HbAbstractEdit::keyReleaseEvent (QKeyEvent *event)
{   
    event->accept();
}

/*!
    \reimp
*/
void HbAbstractEdit::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbAbstractEdit);

    if (d->selectionControl) {
        d->selectionControl->panStarted();
    }

    if (d->interactionFlags & Qt::NoTextInteraction)
        return;

    if (!(event->button() & Qt::LeftButton))
        return;

    if (!((d->interactionFlags & Qt::TextSelectableByMouse) || (d->interactionFlags & Qt::TextEditable)))
        return;

    d->mousePressPos = event->pos();
    d->wasGesture = false;

    HbWidgetFeedback::triggered(this, Hb::InstantPressed);

//    d->minimizeInputPanel();

    event->accept();
}

/*!
    \reimp
*/
void HbAbstractEdit::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
}

/*!
    \reimp
*/
void HbAbstractEdit::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbAbstractEdit);

    if (d->selectionControl) {
        d->selectionControl->panFinished();
    }
    if (d->wasGesture) return;

    if (d->interactionFlags & Qt::NoTextInteraction)
        return;

    if (d->mousePressPos != Hb_Invalid_Position) {
        d->mousePressPos = Hb_Invalid_Position;

        bool removeSelection = (d->hitTest(event->pos(), Qt::ExactHit) == -1);

        if (removeSelection && d->cursor.hasSelection()) {
            const QTextCursor oldCursor = d->cursor;
            d->cursor.clearSelection();
            d->repaintOldAndNewSelection(oldCursor);
            emit selectionChanged(oldCursor, d->cursor);
        }

        int newCursorPos = d->hitTest(event->pos(), Qt::FuzzyHit);

        if (d->cursor.hasSelection() &&
            newCursorPos >= d->cursor.selectionStart() &&
            newCursorPos <= d->cursor.selectionEnd()){
            // we have a selection under mouse click
            if (d->contextMenuShownOn.testFlag(Hb::ShowTextContextMenuOnSelectionClicked)) {
                showContextMenu(mapToScene(event->pos()));
            }
        } else {

            // Currently focused widget to listen to InputContext before updating the cursor position
            d->sendMouseEventToInputContext(event);
            setCursorPosition(newCursorPos);

            HbWidgetFeedback::triggered(this, Hb::InstantReleased);

            if (d->interactionFlags & Qt::TextEditable) {
                d->updateCurrentCharFormat();
            }
            d->cursorChanged(HbValidator::CursorChangeFromMouse);
        }
    }

    d->openInputPanel();

    event->accept();
}


/*!
    \reimp
*/
void HbAbstractEdit::focusInEvent(QFocusEvent *event)
{
    HbWidget::focusInEvent(event);

    Q_D(HbAbstractEdit);

    if (d->interactionFlags & Qt::NoTextInteraction)
        return;

    update(d->selectionRect());

    if (d->interactionFlags & Qt::TextEditable) {
        d->setBlinkingCursorEnabled(true);
    } else {
        d->cursorOn = (d->interactionFlags & Qt::TextSelectableByKeyboard);
    }

    d->openInputPanel();

    event->accept();
}

/*!
    \reimp
*/
void HbAbstractEdit::focusOutEvent(QFocusEvent *event)
{
    HbWidget::focusOutEvent(event);

    Q_D(HbAbstractEdit);

    if (d->interactionFlags & Qt::NoTextInteraction)
        return;

    d->setBlinkingCursorEnabled(false);
    d->repaintOldAndNewSelection(d->selectionCursor);

    event->accept();
}

/*!
    \reimp
*/
void HbAbstractEdit::changeEvent (QEvent *event)
{
    Q_D(HbAbstractEdit);
    HbWidget::changeEvent(event);

    switch (event->type()) {
        case QEvent::FontChange: {
            d->doc->setDefaultFont(font());
            updateGeometry();
            break;
        }
        case QEvent::LayoutDirectionChange: {
            QTextOption textOption = d->doc->defaultTextOption();
            textOption.setTextDirection(layoutDirection());
            d->doc->setDefaultTextOption(textOption);
            if (d->selectionControl) {
                d->selectionControl->updatePrimitives();
            }
            break;
        }
        default: {
            if ( event->type() == HbEvent::ThemeChanged ) {
                d->updatePaletteFromTheme();
                updatePrimitives();
                if (d->selectionControl) {
                    d->selectionControl->updatePrimitives();
                }
            }
            break;
        }
    }
}

/*!
    \reimp
 */
void HbAbstractEdit::hideEvent(QHideEvent *event)
{
    HbWidget::hideEvent(event);

    deselect();

#if QT_VERSION >= 0x040600
    // Send close input panel event.
    QInputContext *ic = qApp->inputContext();
    if (ic && !panel()) {
        QEvent *closeEvent = new QEvent(QEvent::CloseSoftwareInputPanel);
        ic->filterEvent(closeEvent);
        delete closeEvent;
    }
#endif
}



/*!
    Returns the cursor position.

    Returned cursor position is a QTextDocucument index for the cursor
    visible on the screen.

    \sa setCursorPosition
 */
int HbAbstractEdit::cursorPosition () const
{
    Q_D(const HbAbstractEdit);
    return d->cursor.position();
}

/*!
    Sets the cursor postions to \a pos. If \a pos is greater than the length of the content
    the cursor is set to the end of the text.

    Additionally it is possible to create selection using \a mode QTextCursor::KeepAnchor.

    \sa cursorPosition
 */
void HbAbstractEdit::setCursorPosition (int pos, QTextCursor::MoveMode mode)
{
    Q_D(HbAbstractEdit);
    if (d->cursor.position() != pos) {
        d->setCursorPosition(pos, mode);
    }
}

/*!
    Returns the cursor visibility mode.

    \sa setCursorVisibility
 */
Hb::TextCursorVisibility HbAbstractEdit::cursorVisibility () const
{
    Q_D(const HbAbstractEdit);
    return d->isCursorVisible() ? Hb::TextCursorVisible : Hb::TextCursorHidden;
}

/*!
    Sets the cursor visibility mode to given \a value.

    In a hidden mode the cursor is always at the end of the inputted content.
    User can't change the position of the cursor, or select the content.

    \sa cursorVisibility
 */
void HbAbstractEdit::setCursorVisibility (Hb::TextCursorVisibility value)
{
    Q_D(HbAbstractEdit);
    bool b = (value == Hb::TextCursorVisible);
    if (d->apiCursorVisible != b) {
        d->apiCursorVisible = b;
        d->repaintOldAndNewSelection(QTextCursor());
    }
}

/*!
    Returns true if edit is in read-only mode; otherwise return false.

    \sa setReadOnly
 */
bool HbAbstractEdit::isReadOnly () const
{
    Q_D(const HbAbstractEdit);

    return !(d->interactionFlags & Qt::TextEditable);
}

/*!
    Sets edit's read-only mode to given \a value. In a read-only text edit the user
    can only navigate through the text and select text; modifying the text
    is not possible.

    In read-only mode text can be set using setPlainText() or setHtml().

    \sa isReadOnly
 */
void HbAbstractEdit::setReadOnly (bool value)
{
    Q_D(HbAbstractEdit);
    Qt::TextInteractionFlags f = d->interactionFlags;

    if (f.testFlag(Qt::TextEditable) != value) {
        return;
    }

    f ^= Qt::TextEditable;

    d->setTextInteractionFlags(f);

    setFlag(QGraphicsItem::ItemAcceptsInputMethod, !value);
}

/*!
    Returns pointer to a \a primitive of HbAbstractEdit.

    Available primitive is HbStyle::P_Edit_text.

    \reimp
    \sa HbStyle::Primitive, HbWidget::primitive()
*/
QGraphicsItem *HbAbstractEdit::primitive (HbStyle::Primitive primitive) const
{
    Q_D(const HbAbstractEdit);

    if (primitive == HbStyle::P_Edit_text) {
        return d->scrollArea;
    } else {
        return HbWidget::primitive(primitive);
    }
}

/*!
    \reimp
*/
void HbAbstractEdit::updatePrimitives()
{
    Q_D(HbAbstractEdit);
    HbWidget::updatePrimitives();

    if (d->scrollArea) {
        d->doc->setTextWidth(d->scrollArea->size().width());
    }
    QRectF canvasGeom(QRectF(QPointF(0,0),d->doc->size()));
    if(d->scrollArea && canvasGeom.height()<d->scrollArea->size().height()) {
        canvasGeom.setHeight(d->scrollArea->size().height());
    }
    d->canvas->setGeometry(canvasGeom);
    if (d->scrollArea) {
        d->scrollArea->updateScrollMetrics();
    }
    d->ensureCursorVisible();
}

/*!
    Replaces editor content with \a text.

    HbAbstractEdit does not support richtext from cut, copy & paste
    operations after setPlainText() has been called.

    If smiley recognition is enabled the textual smiley patterns are replaced
    with the corresponding image from the active smiley theme.

    \sa setSmileysEnabled()
*/
void HbAbstractEdit::setPlainText(const QString &text)
{
    Q_D(HbAbstractEdit);
    d->setContent(Qt::PlainText, text);
}

/*!
    Replaces editor content with HTML-formatted string \a text.

    If smiley recognition is enabled the textual smiley patterns are replaced
    with the corresponding image from the active smiley theme.

    \sa setSmileysEnabled()

    \note It is the responsibility of the caller to make sure that the text
    is correctly decoded when a QString containing HTML is created and passed to setHtml().
*/
void HbAbstractEdit::setHtml(const QString &text)
{
    Q_D(HbAbstractEdit);
    d->setContent(Qt::RichText, text);
}

/*!
     \internal
     This is the cursor blinking timer.
     \reimp
 */
void HbAbstractEdit::timerEvent(QTimerEvent *e)
{
    Q_D(HbAbstractEdit);

    if (e->timerId() == d->cursorBlinkTimer.timerId()) {
        d->cursorOn = !d->cursorOn;

        if (d->cursor.hasSelection())
            d->cursorOn &= (QApplication::style()->styleHint(QStyle::SH_BlinkCursorWhenTextSelected) != 0);

        d->repaintCursor();
    }
}

/*!
    This is a slot for listening a signal with a same name from QTextDocument.
    When creating custom editor this slot can be overridden but this base implementation
    should be called.

    \sa QTextDocument::documentLayoutChanged()
*/
void HbAbstractEdit::documentLayoutChanged()
{
    Q_D(HbAbstractEdit);

    QAbstractTextDocumentLayout *layout = d->doc->documentLayout();
    QObject::connect(layout, SIGNAL(update(QRectF)), this, SLOT(_q_updateRequest(QRectF)));
    QObject::connect(layout, SIGNAL(updateBlock(QTextBlock)), this, SLOT(_q_updateBlock(QTextBlock)));
    QObject::connect(layout, SIGNAL(documentSizeChanged(QSizeF)), this, SLOT(documentSizeChanged(QSizeF)));

    updatePrimitives();
}

/*!
    This is a slot for listening a signal with a same name from QTextDocument.
    When creating custom editor this slot can be overridden but this base implementation
    should be called.

    \sa QTextDocument::documentSizeChanged(QSizeF size)
*/
void HbAbstractEdit::documentSizeChanged(const QSizeF &size)
{
    Q_UNUSED(size)
    updateGeometry();
    updatePrimitives();
}

/*!
    Replaces the QTextDocument that is inside the editor.
*/
void HbAbstractEdit::setDocument(QTextDocument *document)
{
    Q_D(HbAbstractEdit);
    if (d->doc == document)
        return;
    d->removeCurrentDocument();
    d->connectToNewDocument(document);
}

/*!
    Returns the QTextDocument currently in use.
*/
QTextDocument *HbAbstractEdit::document() const
{
    Q_D(const HbAbstractEdit);
    return d->doc;
}

/*!
    Replaces the QTextCursor that is currently in use.
    \sa textCursor
*/
void HbAbstractEdit::setTextCursor(const QTextCursor &cursor)
{
    Q_D(HbAbstractEdit);

    // QTextCursor operator!= compares only position
    if (    cursor.position() != d->cursor.position()
        ||  cursor.anchor() != d->cursor.anchor() ) {
        const QTextCursor oldCursor = d->cursor;
        d->cursor = cursor;

        d->updateCurrentCharFormat();

        d->ensureCursorVisible();
        d->repaintOldAndNewSelection(oldCursor);
        d->cursorChanged(HbValidator::CursorChangeFromContentSet);
    }
}

/*!
    Returns the QTextCursor currently in use.
    \sa setTextCursor
 */
QTextCursor HbAbstractEdit::textCursor() const
{
    Q_D(const HbAbstractEdit);
    return d->cursor;
}

/*!
    Adds content which is currently selected in editor to clipboard and
    deletes the content from editor.

    When clipboad is not available this method does nothing.

    \sa copy paste
 */
void HbAbstractEdit::cut()
{
#ifndef QT_NO_CLIPBOARD
    Q_D(HbAbstractEdit);
    if (!(d->interactionFlags & Qt::TextEditable) || !d->cursor.hasSelection())
        return;
    copy();
    d->cursor.removeSelectedText();
#endif//QT_NO_CLIPBOARD
}

/*!
    Adds content which is currently selected in editor to clipboard.

    When clipboad is not available this method does nothing.

    \sa cut paste
 */
void HbAbstractEdit::copy()
{
#ifndef QT_NO_CLIPBOARD
    Q_D(HbAbstractEdit);
    if (!d->cursor.hasSelection() || !d->canCopy())
        return;
    QMimeData *data = createMimeDataFromSelection();
    QApplication::clipboard()->setMimeData(data);
#endif//QT_NO_CLIPBOARD
}

/*!
    Inserts the content from clipboard to current cursor position in editor.
    If there is a selection, the selection is replaced with the pasted content.

    When clipboad is not available this method does nothing.

    \sa cut copy
 */
void HbAbstractEdit::paste()
{
#ifndef QT_NO_CLIPBOARD
    Q_D(HbAbstractEdit);
    if (d->canPaste()) {
        d->prepDocForPaste();
        const QMimeData *md = QApplication::clipboard()->mimeData();
        if (md) {
            insertFromMimeData(md);
        }
    }

#endif//QT_NO_CLIPBOARD
}

/*!
    Selects the word under the cursor in the editor.
    Word selection rules comes from QTextCursor::WordUnderCursor.
    \sa selectAll deselect
*/
void HbAbstractEdit::selectClickedWord()
{
    Q_D(HbAbstractEdit);

    int cursorPos = d->hitTest(d->mousePressPos, Qt::FuzzyHit);

    if (cursorPos == -1)
        return;

    setCursorPosition(cursorPos);
    const QTextCursor oldSelection = d->cursor;
    d->cursor.select(QTextCursor::WordUnderCursor);
    emit selectionChanged(oldSelection, d->cursor);
    d->repaintOldAndNewSelection(oldSelection);
    d->cursorChanged(HbValidator::CursorChangeFromMouse);
    //TODO: focus is in VKB so needs to re-focus to editor
//    setFocus();
}

/*!
    Selects all the text in the editor.
    \sa selectClickedWord deselect
 */
void HbAbstractEdit::selectAll()
{
    Q_D(HbAbstractEdit);
    const QTextCursor oldSelection = d->cursor;
    d->cursor.select(QTextCursor::Document);
    emit selectionChanged(oldSelection, d->cursor);
    d->repaintOldAndNewSelection(oldSelection);
    d->cursorChanged(HbValidator::CursorChangeFromMouse);
}

/*!
    Deselects the text in the editor if there is text selected.
    \sa selectClickedWord selectAll
 */
void HbAbstractEdit::deselect()
{
    Q_D(HbAbstractEdit);
    const QTextCursor oldSelection = d->cursor;
    d->cursor.clearSelection();
    emit selectionChanged(oldSelection, d->cursor);
}

/*!
    Launches the format user interface.
 */
void HbAbstractEdit::format()
{
    Q_D(HbAbstractEdit);
    if (d->formatDialog) {
        d->formatDialog->launch(this);
    }
}

/*!
    This function returns a new MIME data object to represent the
    contents of the text edit's current selection.

    It is called when the selection needs to be encapsulated into a new
    QMimeData object; for example when data is copyied to the clipboard.

    Ownership of QMimeData is passed to caller.
*/
QMimeData *HbAbstractEdit::createMimeDataFromSelection() const
{
    Q_D(const HbAbstractEdit);
    const QTextDocumentFragment fragment(d->cursor);
    return new HbAbstractEditMimeData(fragment);
}

/*!
    This function returns true if the contents of the MIME data object,
    specified by source, can be decoded and inserted into the document.
 */
bool HbAbstractEdit::canInsertFromMimeData(const QMimeData *source) const
{
    return (source->hasText() && !source->text().isEmpty())
        || source->hasHtml()
        || source->hasFormat(QLatin1String("application/x-qrichtext"))
        || source->hasFormat(QLatin1String("application/x-qt-richtext"));
}

/*!
    This function inserts the contents of the MIME data object,
    specified by \a source, into the text edit at the current cursor
    position.

    It is called for example when text is inserted as the result
    of a clipboard paste operation.
 */
void HbAbstractEdit::insertFromMimeData(const QMimeData *source)
{
    Q_D(HbAbstractEdit);
    if (!(d->interactionFlags & Qt::TextEditable) || !source)
        return;

    bool hasData = false;
    QTextDocumentFragment fragment;
#ifndef QT_NO_TEXTHTMLPARSER
    if (source->hasFormat(QLatin1String("application/x-qrichtext"))) {
        QString richtext = QString::fromUtf8(source->data(QLatin1String("application/x-qrichtext")));
        richtext.prepend(QLatin1String("<meta name=\"qrichtext\" content=\"1\" />"));
        fragment = QTextDocumentFragment::fromHtml(richtext, d->doc);
        hasData = true;
    } else if (source->hasHtml()) {
        fragment = QTextDocumentFragment::fromHtml(source->html(), d->doc);
        hasData = true;
    } else
#endif //QT_NO_TEXTHTMLPARSER
    {
        QString text = source->text();
        if (!text.isNull()) {
            fragment = QTextDocumentFragment::fromPlainText(text);
            hasData = true;
        }
    }

    if (hasData) {
        d->cursor.insertFragment(fragment);
    }
    d->ensureCursorVisible();
}

/*!
    Returns the calculated rect in item coordinates of the editor for the the given \a position inside a document.
 */
QRectF HbAbstractEdit::rectForPosition(int position, QTextLine::Edge edge) const
{
    Q_D(const HbAbstractEdit);

    QRectF rect  = d->rectForPositionInCanvasCoords(position, edge);
    rect = mapRectFromItem(d->canvas,rect);
    return rect;
}

/*!
    Returns the contents as plain text. If smiley recognition is enabled
    all the smiley images will be replaced by their textual representations.

    \sa QTextDocument::toPlainText() setSmileysEnabled()
*/
QString HbAbstractEdit::toPlainText() const
{
    Q_D(const HbAbstractEdit);
    QString retVal;
    if(isSmileysEnabled()) {
        retVal = d->smileyEngineInstance()->toPlainText();
    } else {
        retVal = d->doc->toPlainText();
    }
    return retVal;
}

/*!
    Returns the contents as HTML formatted string. If smiley recognition is enabled
    all the smiley images will be replaced by their textual representations.

    \sa QTextDocument::toHtml() setSmileysEnabled()
*/
QString HbAbstractEdit::toHtml() const
{
    Q_D(const HbAbstractEdit);
    QString retVal;
    if(isSmileysEnabled()) {
        retVal = d->smileyEngineInstance()->toHtml();
    } else {
        retVal = d->doc->toHtml();
    }
    return retVal;
}

/*!
    Validator is used to validate the content and cursor movements.

    \note Validator uses undo stack to back out invalid changes. Therefore undo
    is enabled when validator is set.
 */
void HbAbstractEdit::setValidator(HbValidator* validator)
{
    Q_D(HbAbstractEdit);

    if(d->validator) {
        delete d->validator;
        d->validator = 0;
    }

    if(validator) {
        d->validator = validator;
        d->initValidator();
    }
}

/*!
    \property QLineEdit::acceptableInput
    This property holds whether the input satisfies the validator.

    By default, this property is true.

    \sa setValidator()
*/
bool HbAbstractEdit::hasAcceptableInput() const
{
    Q_D(const HbAbstractEdit);
    return d->hasAcceptableInput();
}

/*!
    Convenience method to manipulate cursor position.
    \sa QTextCursor::movePosition()
 */
void HbAbstractEdit::moveCursor(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode)
{
    Q_D(HbAbstractEdit);
    //const QTextCursor oldCursor = d->cursor;
    d->cursor.movePosition(op, mode);

    d->updateCurrentCharFormat();

    d->ensureCursorVisible();
    QTextCursor previousCursor(d->cursor);
    previousCursor.setPosition(d->previousCursorAnchor);
    previousCursor.setPosition(d->previousCursorPosition, QTextCursor::KeepAnchor);
    d->repaintOldAndNewSelection(previousCursor);
    d->cursorChanged(HbValidator::CursorChangeFromOperation);
}

/*!
    Returns the bounding rect for given text block.

    \sa QAbstractTextDocumentLayout::blockBoundingRect(const QTextBlock& block)
 */
QRectF HbAbstractEdit::blockBoundingRect(const QTextBlock &block) const
{
    Q_D(const HbAbstractEdit);
    return d->doc->documentLayout()->blockBoundingRect(block);
}

/*!
    This is a slot for listening a signal with a same name from QTextDocument.
    When creating custom editor this slot can be overridden but this base implementation
    should be called.

    \sa QTextDocument::blockCountChanged(int newBlockCount)
*/
void HbAbstractEdit::blockCountChanged(int newBlockCount)
{
    Q_UNUSED(newBlockCount);

    updatePrimitives();
}

/*!
    Returns true if the content is scrollable.
 */
bool HbAbstractEdit::isScrollable() const
{
    Q_D(const HbAbstractEdit);

    return d->scrollable;
}

/*!
    Sets the editor as scrollable if \a value is true; othetwise edit is set as non-scrollable.
*/
void HbAbstractEdit::setScrollable(bool value)
{
    Q_D(HbAbstractEdit);
    if (value != d->scrollable) {
        d->scrollable = value;
//            d->scrollArea->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    }
}

/*!
    Content of the editor can be drawn to some outside widget by supplying \a painter and \a option.
    This is used be the HbStyle system.
 */
void HbAbstractEdit::drawContents(QPainter *painter, const QStyleOptionGraphicsItem &option)
{
    Q_D(HbAbstractEdit);
#ifdef HB_DEBUG_EDITOR_DRAW_RECTS
    painter->setPen(Qt::green);
    /*for(QTextBlock tb = d->doc->begin(); tb.isValid(); tb=tb.next()) {
        painter->drawRect(blockBoundingRect(tb));
    }*/
    painter->drawRect(d->doc->documentLayout()->frameBoundingRect(
        d->doc->rootFrame()).adjusted(2,2,-2,-2));
    painter->setOpacity(0.3);
    painter->setPen(Qt::blue);
    painter->drawRect(d->cursorRect());
    painter->setOpacity(1.0);
#endif

    QAbstractTextDocumentLayout::PaintContext ctx = d->getPaintContext();
    if (option.exposedRect.isValid())
        painter->setClipRect(option.exposedRect, Qt::IntersectClip);
    ctx.clip = option.exposedRect;

    d->drawContentBackground(painter, option);
    document()->documentLayout()->draw(painter, ctx);
    // Draw the pins for the selection handle
    d->drawSelectionEdges(painter, ctx);

#ifdef HB_DEBUG_EDITOR_HANDLES
    QRectF a = rectForPosition(d->cursor.anchor());
    QRectF c = rectForPosition(d->cursor.position());

    painter->setPen(Qt::blue);
    painter->drawRect(a);
    painter->setPen(Qt::green);
    painter->drawRect(c);
    painter->setPen(Qt::red);
    painter->drawRect(d->cursorRect());
#endif
}

/*!
    Returns the used scroll area if edit is set as scrollable;
    otherwise returns null pointer.
*/
HbScrollArea *HbAbstractEdit::scrollArea() const
{
    Q_D(const HbAbstractEdit);
    return d->scrollArea;
}

/*!
    Creates a context menu. Default implementation returns 0.
*/
HbMenu* HbAbstractEdit::createContextMenu()
{
    return 0;
}

/*!
    Shows context menu at given \a position. Inheriting class can implement its own context menu instance using createContextMenu function. 

    These actions are added to the context menu when appropriate: cut, copy, paste, select, select all, deselect, format.

    \sa createContextMenu
*/
void HbAbstractEdit::showContextMenu(QPointF position)
{
    Q_D(HbAbstractEdit);
    HbMenu *menu = createContextMenu();

    if (!menu) {
        menu = new HbMenu();
    }

    menu->setAttribute(Hb::InputMethodNeutral);

    if (d->cursor.hasSelection() && d->canCopy()) {
        connect(
            menu->addAction("Cut"), SIGNAL(triggered()),
            this, SLOT(cut()));
        connect(
            menu->addAction("Copy"), SIGNAL(triggered()),
            this, SLOT(copy()));
    }
    else if (!d->doc->isEmpty() && d->canCopy()){
        connect(
            menu->addAction("Select"), SIGNAL(triggered()),
            this, SLOT(selectClickedWord()));
        connect(
            menu->addAction("Select all"), SIGNAL(triggered()),
            this, SLOT(selectAll()));
    }
    if (d->canPaste()) {
        connect(
            menu->addAction("Paste"), SIGNAL(triggered()), 
            this, SLOT(paste()));
    }
    if (d->cursor.hasSelection()) {
        connect(
            menu->addAction("Deselect"), SIGNAL(triggered()), 
            this, SLOT(deselect()));
    }
    if (d->canFormat()) {
        connect(
            menu->addAction("Format"), SIGNAL(triggered()), 
            this, SLOT(format()));
    }

    emit aboutToShowContextMenu(menu, d->mousePressPos);

    d->minimizeInputPanel();

    if(menu->actions().count() > 0){
        menu->setPreferredPos(position);
        menu->show();
    }
}

/*!
    Returns the validator currently in use.

    \sa setValidator HbValidator
*/
HbValidator *HbAbstractEdit::validator() const
{
    Q_D(const HbAbstractEdit);
    return d->validator;
}

/*!
    \reimp
*/
int HbAbstractEdit::type() const
{
    return Type;
}

/*!
    \reimp
*/
void HbAbstractEdit::updateGeometry()
{
    Q_D(HbAbstractEdit);
    d->updateEditingSize();
    HbWidget::updateGeometry();
}
/*!
    Sets text alignment to \a alignment to the current text cursor.

    \sa alignment textCursor setTextCursor
*/
void HbAbstractEdit::setAlignment(Qt::Alignment alignment)
{
    Q_D(HbAbstractEdit);
    d->acceptSignalContentsChanged = false; // no text content changes.
    QTextBlockFormat fmt;
    fmt.setAlignment(alignment);
    QTextCursor cursor = d->cursor;
    cursor.mergeBlockFormat(fmt);
    d->acceptSignalContentsChanged = true;
    setTextCursor(cursor);
    d->mApiProtectionFlags |= HbWidgetBasePrivate::AC_TextAlign;
}

/*!
    Returns text alignment at the current text cursor.

    \sa setAlignment()
*/
Qt::Alignment HbAbstractEdit::alignment() const
{
    Q_D(const HbAbstractEdit);
    return d->cursor.blockFormat().alignment();

}

/*!
    Returns current flags describing on which events context menu is shown

    \sa Hb::TextContextMenuFlag
 */
Hb::TextContextMenuFlags HbAbstractEdit::contextMenuFlags() const
{
    Q_D(const HbAbstractEdit);

    return d->contextMenuShownOn;
}

/*!
    Sets \a flags describing on which events context menu is shown
    \sa Hb::TextContextMenuFlag
 */
void HbAbstractEdit::setContextMenuFlags(Hb::TextContextMenuFlags flags)
{
    Q_D(HbAbstractEdit);
    d->contextMenuShownOn = flags;
}

/*!
    Sets single \a flag on which event context menu is shown
    (other flags remain unchanged)
    \sa Hb::TextContextMenuFlag
 */
void HbAbstractEdit::setContextMenuFlag(Hb::TextContextMenuFlag flag)
{
    Q_D(HbAbstractEdit);
    d->contextMenuShownOn|=flag;
}

/*!
    Clears single \a flag on which event context menu is shown
    (other flags remain unchanged)
    \sa Hb::TextContextMenuFlag
 */
void HbAbstractEdit::clearContextMenuFlag(Hb::TextContextMenuFlag flag)
{
    Q_D(HbAbstractEdit);
    d->contextMenuShownOn&=~flag;
}

/*!
    Returns the reference of the anchor at the given position,
    or an empty string if no anchor exists at that point.
 */
QString HbAbstractEdit::anchorAt(const QPointF &pos) const
{
    Q_D(const HbAbstractEdit);
    return d->doc->documentLayout()->anchorAt(pos);
}

/*!
    Returns the reference of the anchor the given cursor position,
    or an empty string if no anchor exists at that position.
 */
QString HbAbstractEdit::anchorAt(int pos) const
{
    Q_D(const HbAbstractEdit);
    QTextCursor newCursor = QTextCursor(d->doc);
    newCursor.setPosition(pos);
    newCursor.movePosition(QTextCursor::NextCharacter);
    return newCursor.charFormat().anchorHref();
}

/*!
    Sets the smiley theme to be used with the editor.
    \sa HbSmileyTheme smileyTheme()
 */
void HbAbstractEdit::setSmileyTheme(const HbSmileyTheme& theme)
{
    Q_D(HbAbstractEdit);
    d->smileyEngineInstance()->setTheme(theme);

    HbEditorInterface editorInterface(this);
    editorInterface.setSmileyTheme(theme);
}


/*!
    Returns the smiley theme which is used by the editor.
    \sa HbSmileyTheme setSmileyTheme()
 */
HbSmileyTheme HbAbstractEdit::smileyTheme() const
{
    Q_D(const HbAbstractEdit);
    return d->smileyEngineInstance()->theme();
}

/*!
    Returns the default smiley theme.
    \sa HbSmileyTheme resetSmileyTheme()
 */
HbSmileyTheme HbAbstractEdit::defaultSmileyTheme() const
{
    Q_D(const HbAbstractEdit);
    return d->smileyEngineInstance()->defaultTheme();
}

/*!
    Convenience method to reset the smiley theme to the default.
    \sa HbSmileyTheme defaultSmileyTheme()
 */
void HbAbstractEdit::resetSmileyTheme()
{
    HbSmileyTheme theme = defaultSmileyTheme();
    setSmileyTheme(theme);
    HbEditorInterface editorInterface(this);
    editorInterface.setSmileyTheme(theme);

}

/*!
    Activates or deactivates the smiley recognition in the editor.
    \sa HbLineEdit::setText() HbLineEdit::text()
    \sa HbTextEdit::setPlainText() HbTextEdit::toPlainText()
    \sa HbTextEdit::setHtml() HbTextEdit::toHtml()
 */
void HbAbstractEdit::setSmileysEnabled(bool enabled)
{
    Q_D(HbAbstractEdit);
    if(d->smileysEnabled != enabled) {
        d->smileysEnabled = enabled;
        HbEditorInterface editorInterface(this);
        if(enabled) {
            editorInterface.setSmileyTheme(d->smileyEngineInstance()->theme());
        } else {
            editorInterface.setSmileyTheme(HbSmileyTheme());
        }
    }
}

/*!
    Return true if smiley recognition is enabled in the editor otherwise returns false.
 */
bool HbAbstractEdit::isSmileysEnabled() const
{
    Q_D(const HbAbstractEdit);
    return d->smileysEnabled;
}

/*!
    Sets the format dialog for the editor. If it is set zero no format menu item shown 
 */
void HbAbstractEdit::setFormatDialog(HbFormatDialogPointer dialog)
{
    Q_D(HbAbstractEdit);
    d->formatDialog = dialog;
}

/*!
    \reimp
 */
void HbAbstractEdit::polish( HbStyleParameters& params )
{
    Q_D(HbAbstractEdit);
    const QString KTextAlignmentCSSName = "text-align";
    const QString KTextColorCSSName = "color";

    // ------ adding css parameters ------
    params.addParameter(KTextAlignmentCSSName);

    QPalette cssPalette = palette();
    params.addParameter(KTextColorCSSName, cssPalette.color(QPalette::Text));

    HbWidget::polish(params);

    // ------ interpreting css parameters ------
    QVariant param = params.value(KTextAlignmentCSSName);
    if(param.canConvert(QVariant::String)) {
        Qt::Alignment align = HbAbstractEditPrivate::alignmentFromString(param.toString());
        if( align != 0 ) {
            if (!(d->mApiProtectionFlags & HbWidgetBasePrivate::AC_TextAlign)) {
                setAlignment(align);
                d->mApiProtectionFlags &= ~HbWidgetBasePrivate::AC_TextAlign;
            }
        } else {
            qWarning("Unable to read CSS parameter \"text-alignment\" in editor");
        }
    }

    param = params.value(KTextColorCSSName);
    if(param.canConvert(QVariant::Color)) {
        cssPalette.setColor(QPalette::Text, param.value<QColor>());
    }

    setPalette(cssPalette);
}

/*!
    \reimp
 */
QVariant HbAbstractEdit::itemChange(GraphicsItemChange change, const QVariant &value)
{
    Q_D(const HbAbstractEdit);
    if (change == QGraphicsItem::ItemScenePositionHasChanged) {
        if (d->selectionControl) {
            d->selectionControl->updatePrimitives();
        }
    }
    return HbWidget::itemChange(change, value);
}

/*!
  Returns the character at position pos, or a null character if the position is out of range.
*/
QChar HbAbstractEdit::characterAt(int pos) const
{
    return document()->characterAt(pos);
}
