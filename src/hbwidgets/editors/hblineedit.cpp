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

#include "hblineedit.h"
#include "hblineedit_p.h"

#include "hbstyleoption_p.h"
#include "hbscrollarea.h"
#ifdef HB_TEXT_MEASUREMENT_UTILITY
#include "hbtextmeasurementutility_p.h"
#include "hbfeaturemanager_p.h"
#endif //HB_TEXT_MEASUREMENT_UTILITY

#include <QFontMetrics>
#include <QPainter>
#include <QTextBlock>
#include <QTextDocument>
#include <QGraphicsSceneResizeEvent>

/*!
 \class HbLineEdit
 \brief HbLineEdit is a one line text editor widget
 @alpha
 @hbwidgets

 A line edit allows the user to enter and edit a single paragraph of plain text.
 The length of the text can be constrained to maxLength().
 You can change the text with setText(). The text is retrieved with text().

 The content in \ref HbLineEdit can be split visually into several lines. 
 In other words the content of the editor can be seen in several lines but it 
 does not contain any linefeeds (e.g. a number of email recipients separated with semicolon). 
 In the API there is a group of methods to support this. 
 
 If maxRows() is greater than minRows() the
 editor is expandable and it changes its size based on the content between min and max values. 
 Editor expandability can be checked with isExpandable().

 When the text changes the textChanged() signal is emitted.
 When the cursor is moved the cursorPositionChanged() signal is emitted.
 When editing is finished, either because the line edit lost focus or Return/Enter is pressed
 the editingFinished() signal is emitted.

  Echo mode controls how input to editors is shown in the screen. It is by default \b Normal. 
  Other possible modes are \b NoEcho, \b Password, and \b PasswordEchoOnEdit. 

 The frame can be styled using "P_LineEdit_frame" primitive in HbStyle. Text item is not
  replaceable but it can be styled.
 */

/*!
 \fn void HbLineEdit::editingFinished()

 This signal is emitted when the line edit loses focus or Return/Enter is pressed.
 */

/*!
    \fn void HbLineEdit::textChanged(const QString &text)

    This signal is emitted whenever the text changes. The text argument is the new text.
    This signal is also emitted when the text is changed programmatically, for example,
    by calling setText().

    Note: this signal is not emitted when the editor is in any of password modes, \sa echoMode()
*/

/*!
    \fn void void HbLineEdit::selectionChanged()
    This signal is emitted when selection changes.
*/

/*!
 Constructs an empty HbLineEdit with parent \a parent.
 */
HbLineEdit::HbLineEdit (QGraphicsItem *parent) :
    HbAbstractEdit(*new HbLineEditPrivate, parent)
{
    Q_D(HbLineEdit);
    d->q_ptr = this;

    d->init();
}

/*!
 Constructs a HbLineEdit with parent \a parent.
 The text edit will display the text \a text.
 The text is interpreted as plain text.
 */
HbLineEdit::HbLineEdit (const QString &text, QGraphicsItem *parent) :
    HbAbstractEdit(*new HbLineEditPrivate, parent)
{
    Q_D(HbLineEdit);
    d->q_ptr = this;

    d->init();

    if ( !text.isEmpty() ) {
        setText(text);
    }
}

/*!
 Constructs a HbLineEdit with parent \a parent.
 The text edit will use given private implementation \a dd.
 */
HbLineEdit::HbLineEdit (HbLineEditPrivate &dd, QGraphicsItem *parent) :
    HbAbstractEdit(dd, parent)
{
    Q_D(HbLineEdit);
    d->q_ptr = this;

    d->init();
}

/*!
 Destructor.
 */
HbLineEdit::~HbLineEdit ()
{
}

/*!
 Returns current maximum character length in edit.

 \sa setMaxLength
 */
int HbLineEdit::maxLength () const
{
    Q_D(const HbLineEdit);
    return d->maxLength;
}

/*!
 Sets edit maximum character length to \a length.

 If the text is too long, it is truncated at the limit.
 If truncation occurs any selected text will be unselected.
 By default, this property contains a value of -1 (unlimited).
 */
void HbLineEdit::setMaxLength (int length)
{
    Q_D(HbLineEdit);
    if (length < -1) return; // not allowed.

    if (d->maxLength != length) {
        d->maxLength = length;
        d->validateMaxLength();
    }
}

/*!
 Sets the minimum number of editor rows.

 \sa minRows
 */
void HbLineEdit::setMinRows (int rows)
{
    Q_D(HbLineEdit);
    d->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin, true);

    if (rows<=0) {
        qWarning("HbLineEdit::setMinRows wrong argument, value \"%d\" has been ignored.",
                 rows);
        return;
    }

    if (rows != d->minimumRows) {
        d->minimumRows = rows;

        if (d->minimumRows > d->maximumRows) {
            d->maximumRows = d->minimumRows;
        }

        d->updateWrappingMode();

        if (d->adjustFontSizeToFitHeight) {
            d->readjustStretchFont();
        } else {
            updateGeometry();
        }
    }
}

/*!
 Returns the minimum number of editor rows.

 \sa setMinRows
 */
int HbLineEdit::minRows () const
{
    Q_D(const HbLineEdit);

    return d->minimumRows;
}

/*!
 .Tells whether the editor is expandable or fixed.
 */
bool HbLineEdit::isExpandable () const
{
    Q_D(const HbLineEdit);

    return d->maximumRows > d->minimumRows;
}

/*!
 Sets the maximum number of editor rows to \a rows.

 \sa maxRows
*/
void HbLineEdit::setMaxRows (int rows)
{
    Q_D(HbLineEdit);
    d->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMax, true);

    if (rows<=0) {
        qWarning("HbLineEdit::setMaxRows wrong argument, value \"%d\" has been ignored.",
                 rows);
        return;
    }

    if (rows != d->maximumRows) {
        d->maximumRows = rows;

        if (d->maximumRows  < d->minimumRows) {
            d->minimumRows = d->maximumRows;
        }

        d->updateWrappingMode();

        if (d->adjustFontSizeToFitHeight) {
            d->readjustStretchFont();
        } else {
            updateGeometry();
        }

#ifdef HB_TEXT_MEASUREMENT_UTILITY
        if ( HbFeatureManager::instance()->featureStatus( HbFeatureManager::TextMeasurement ) ) {
            setProperty( HbTextMeasurementUtilityNameSpace::textMaxLines, d->maximumRows );
        }
#endif
    }
}

/*!
 Returns the maximum number of editor rows.

 \sa setMaxRows
 */
int HbLineEdit::maxRows () const
{
    Q_D(const HbLineEdit);

    return d->maximumRows;
}

/*!
 \reimp
 */
void HbLineEdit::inputMethodEvent(QInputMethodEvent *e)
{
    Q_D(HbLineEdit);
    
    if(d->echoMode == HbLineEdit::PasswordEchoOnEdit && d->clearOnEdit) {
        d->doc->clear();
        d->passwordText.clear();
        d->clearOnEdit = false;
    }

    if (e->commitString().contains("\n")) {
        QString str = e->commitString();
        str.replace("\n", " ");
        e->setCommitString(str, e->replacementStart(), e->replacementLength());
    }
    
    if (!e->commitString().isEmpty() || e->replacementLength()) {
        // change the commit string and update the password string in case we are in password mode
        if(d->echoMode == HbLineEdit::Password) {
            d->passwordText.replace(cursorPosition()+e->replacementStart(), e->replacementLength(), e->commitString());
            e->setCommitString(d->passwordString(e->commitString()), e->replacementStart(), e->replacementLength());
        } else if(d->echoMode == HbLineEdit::NoEcho) {
            d->passwordText.append(e->commitString());
            e->setCommitString("");
        } else if(d->echoMode == HbLineEdit::PasswordEchoOnEdit) {
            // in PasswordEchoOnEdit we still can see what we are typing
            d->passwordText.replace(cursorPosition()+e->replacementStart(), e->replacementLength(), e->commitString());
        }
    }
    HbAbstractEdit::inputMethodEvent(e);
}

/*!
 \reimp
*/

void HbLineEdit::keyPressEvent (QKeyEvent *event)
{
    Q_D(HbLineEdit);

    if(d->echoMode == HbLineEdit::PasswordEchoOnEdit && d->clearOnEdit) {
        d->doc->clear();
        d->passwordText.clear();
        d->clearOnEdit = false;
    }

    if(d->forwardKeyEvent(event)) {
        HbAbstractEdit::keyPressEvent(event);
    } else if (d->echoMode == HbLineEdit::Password || d->echoMode == HbLineEdit::NoEcho){
        // Keep doc and passwordText in sync
        bool update = false;
        if (event->key() == Qt::Key_Backspace && !(event->modifiers() & ~Qt::ShiftModifier)) {
            update = true;
            d->passwordText.remove(d->passwordText.length()-1,1);
        } else if (!event->text().isEmpty() && event->text().at(0).isPrint()) {
            update = true;
            d->passwordText.append(event->text());
        }

        if (update) {
            setPlainText(((d->echoMode == HbLineEdit::Password)?d->passwordString(d->passwordText):""));
        }
        setCursorPosition(d->passwordText.length());
    }
    if (d->echoMode == HbLineEdit::PasswordEchoOnEdit) {
        // Keep doc and passwordText in sync
        d->passwordText = toPlainText();
    }    
}

/*!
 \reimp
 */
void HbLineEdit::keyReleaseEvent (QKeyEvent *event)
{
    Q_D(HbLineEdit);

    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        emit editingFinished();
    }

    if(d->forwardKeyEvent(event)) {
        HbAbstractEdit::keyReleaseEvent(event);
    }
}

/*!
 \reimp
 */
int HbLineEdit::type () const
{
    return Type;
}

/*!
    Returns the edit's contents as plain text. If smiley recognition is enabled
    all the smiley images will be replaced by their textual representations.

    \sa setText() HbAbstractEdit::setSmileysEnabled()
*/
QString HbLineEdit::text() const
{
    Q_D(const HbLineEdit);
    if(d->isPasswordMode()) {
        return d->passwordText;
    }
    return toPlainText();
}

/*!
    This property holds the displayed text.
    If echoMode is Normal this returns the same as text(); if EchoMode is Password or PasswordEchoOnEdit it
    returns a string of asterisks text().length() characters long, e.g. "******";
    if EchoMode is NoEcho returns an empty string, "".

    By default, this property contains an empty string.

    \sa text()
*/
QString HbLineEdit::displayText() const
{
    return toPlainText();
}

/*!
  Returns true if there is any text section selected in the line edit.
  Otherwise returns false.

  \sa setSelection
 */
bool HbLineEdit::hasSelectedText() const
{
    return textCursor().hasSelection();
}

/*!
 Returns the text that is selected in the line edit.
 If no text section is selected, returns an empty string.
 */
QString HbLineEdit::selectedText() const
{
    return textCursor().selectedText();
}

/*!
  Returns the index of the first selected character in the line
  edit or -1 if no text is selected.
 */
int HbLineEdit::selectionStart () const
{
    if (hasSelectedText()) {
        return textCursor().selectionStart();
    } else {
        return -1;
    }
}

/*!
 Selects text from position \a start and for \a length characters.
 Negative lengths are allowed. Setting the selection fails if
 start position is negative or out of the range of line edit text.

 \sa hasSelectedText
 */
void HbLineEdit::setSelection(int start, int length)
{
    int textLength = text().count();
    QString test = text();
    Q_UNUSED(test)
    if (start < 0 || start > textLength) {
        return;
    }

    int end(start+length);
    if (end > textLength) {
        end = textLength;
    } else if (end < 0) {
        end = 0;
    }

    QTextCursor cursor = textCursor();
    cursor.setPosition(start, QTextCursor::MoveAnchor);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    setTextCursor(cursor);

}

/*!
    Sets cursor position to \a pos.

    \sa cursorPosition
 */
void HbLineEdit::setCursorPosition (int pos)
{
    int length = document()->begin().length();

    if (pos >= length) {
        pos = length-1;
    } else if (pos < 0) {
        pos = 0;
    }
    HbAbstractEdit::setCursorPosition(pos);
}


/*!
    \property HbLineEdit::echoMode
    \brief the line edit's echo mode

    The echo mode determines how the text entered in the line edit is
    displayed (or echoed) to the user.

    The most common setting is \l Normal, in which the text entered by the
    user is displayed verbatim, but HbLineEdit also supports modes that allow
    the entered text to be suppressed or obscured: these include \l NoEcho,
    \l Password and \l PasswordEchoOnEdit.

    The widget's display and the ability to copy or drag the text is
    affected by this setting.

    By default, this property is set to \l Normal.

    \sa setEchoMode()
*/

HbLineEdit::EchoMode HbLineEdit::echoMode() const
{
    Q_D(const HbLineEdit);
    return (EchoMode) d->echoMode;
}

void HbLineEdit::setEchoMode(EchoMode mode)
{
    Q_D(HbLineEdit);
    if (mode == (EchoMode)d->echoMode)
        return;
    d->setEchoMode(mode);
#if QT_VERSION >= 0x040600
    Qt::InputMethodHints hints = inputMethodHints();
    if(mode != Normal) {
        hints |= Qt::ImhNoPredictiveText|Qt::ImhHiddenText;
    } else {
        hints &= ~Qt::ImhNoPredictiveText & ~Qt::ImhHiddenText;
    }
    setInputMethodHints(hints);
#endif // QT_VERSION
    // Clear the selection if any
    d->cursor.clearSelection();
    d->selectionChanged();
    update();
}


/*!
 Sets the current capitalization type of editor text.

 \sa capitalization QTextCharFormat::fontCapitalization
 */
void HbLineEdit::setCapitalization ( QFont::Capitalization caps )
{
    Q_D(HbLineEdit);

    d->setCapitalization(caps);
}

/*!
 Returns the current capitalization type of editor text.

 \sa setCapitalization QTextCharFormat::setFontCapitalization
 */
QFont::Capitalization HbLineEdit::capitalization () const
{
    Q_D(const HbLineEdit);
    return d->capitalization();
}


/*!
 Sets the text edit's text. The text is interpreted as plain text.
 If smiley recognition is enabled the textual smiley patterns are replaced
 with the corresponding image from the active smiley theme.

 \sa text HbAbstractEdit::setSmileysEnabled()
 */
void HbLineEdit::setText(const QString &text)
{
    Q_D(HbLineEdit);

    QString txt( text );

#ifdef HB_TEXT_MEASUREMENT_UTILITY
    if ( HbFeatureManager::instance()->featureStatus( HbFeatureManager::TextMeasurement ) ) {
        if (text.endsWith(QChar(LOC_TEST_END))) {
            int index = text.indexOf(QChar(LOC_TEST_START));
            setProperty( HbTextMeasurementUtilityNameSpace::textIdPropertyName,  text.mid(index + 1, text.indexOf(QChar(LOC_TEST_END)) - index - 1) );
            setProperty( HbTextMeasurementUtilityNameSpace::textMaxLines, d->maximumRows );
            txt = text.left(index);
        } else {
            setProperty( HbTextMeasurementUtilityNameSpace::textIdPropertyName,  QVariant::Invalid );
        }
    }
#endif //HB_TEXT_MEASUREMENT_UTILITY

    if(d->isPasswordMode()) {
        d->passwordText = txt;
    }
    setPlainText(d->echoString(txt));
    moveCursor(QTextCursor::EndOfBlock);
}

/*!
    \reimp
 */
bool HbLineEdit::canInsertFromMimeData(const QMimeData *source) const
{
    Q_D(const HbLineEdit);
    if(source->hasText() && !source->text().isEmpty()) {
        QString text(source->text());
        d->filterInputText(text);
        return !text.isEmpty();
    } else {
        return false;
    }
}

/*!
    \reimp
 */
void HbLineEdit::insertFromMimeData(const QMimeData *source)
{
    Q_D(HbLineEdit);
    if (!(d->interactionFlags & Qt::TextEditable) || !source)
        return;

    QString text = source->text().replace(QString("\n"),QString(" "));

    QString txt = filterInputText(text);

    if (!txt.isNull()) {

        if(d->isPasswordMode()) {
            d->passwordText = txt;
        }

        QTextDocumentFragment fragment = QTextDocumentFragment::fromPlainText(d->echoString(txt));
        d->cursor.insertFragment(fragment);
    }

    d->ensureCursorVisible();
}

/*!
    \reimp
 */
void HbLineEdit::focusOutEvent(QFocusEvent * event)
{
    Q_D(HbLineEdit);

    if(echoMode() == HbLineEdit::PasswordEchoOnEdit) {
        setPlainText(d->passwordString(d->passwordText));
    }

    HbAbstractEdit::focusOutEvent(event);

    emit editingFinished();
}

/*!
    \reimp
 */
void HbLineEdit::focusInEvent(QFocusEvent * event)
{
    Q_D(HbLineEdit);

    if(echoMode() == HbLineEdit::PasswordEchoOnEdit) {
        // we need to clear the editor when typing starts
        d->clearOnEdit = true;
    }

    HbAbstractEdit::focusInEvent(event);
}

/*!
    @proto

    Enables or disables vertical font stretch mode.

    In this mode font size is depending on editor size and number of rows.
    Font is adjusted in such way to show as much text as possible and as big
    as possible and still have visible number of rows in range defined by
    \l{HbLineEdit::setMinRows}{minimum} and \l{HbLineEdit::setMaxRows}{maximum}
    number of rows.

    \sa HbLineEdit::setMinRows(int)
    \sa HbLineEdit::setMaxRows(int)
    \sa HbLineEdit::adjustFontSizeToFitHeight()
 */
void HbLineEdit::setAdjustFontSizeToFitHeight(bool active)
{
    Q_D(HbLineEdit);
    if (d->adjustFontSizeToFitHeight != active) {
        d->adjustFontSizeToFitHeight = active;
        if(!active) {
            // clear font stretch
            d->canvas->setFont(QFont());
        }
        updateGeometry();
    }
}

/*!
    @proto

    Returns true if vertical font streach mode is active.
    See HbLineEdit::setAdjustFontSizeToFitHeight for details.

    \sa HbLineEdit::setAdjustFontSizeToFitHeight(bool)
 */
bool HbLineEdit::adjustFontSizeToFitHeight() const
{
    return d_func()->adjustFontSizeToFitHeight;
}

/*!
    \reimp
 */
bool HbLineEdit::eventFilter(QObject *obj, QEvent *event)
{
    Q_D(HbLineEdit);

    if (obj == d->scrollArea && event->type()==QEvent::GraphicsSceneResize) {
        document()->setTextWidth(d->scrollArea->size().width());
        d->onResizeFontChange();
    }
    return HbAbstractEdit::eventFilter(obj, event);
}
