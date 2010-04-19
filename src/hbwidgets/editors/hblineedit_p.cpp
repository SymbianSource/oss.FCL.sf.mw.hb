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

#include "hblineedit_p.h"
#include "hblineedit.h"
#include "hbmeshlayout_p.h"
#include "hbcolorscheme.h"

#include <QTextDocument>
#include <QTextBlock>

HbLineEditPrivate::HbLineEditPrivate () :
    HbAbstractEditPrivate(),
    maxLength(-1),
    expandable(false),
    minimumRows(1),
    maximumRows(1),
    echoMode(HbLineEdit::Normal),
    clearOnEdit(false),
    emitTextChanged(true)
{
}

HbLineEditPrivate::~HbLineEditPrivate ()
{
}

void HbLineEditPrivate::init()
{
    Q_Q(HbLineEdit);

    QObject::connect(q, SIGNAL(selectionChanged(const QTextCursor &, const QTextCursor &)),
                     q, SIGNAL(selectionChanged()));
    QObject::connect(doc, SIGNAL(contentsChange(int,int,int)), q, SLOT(_q_textChange(int,int,int)));
    QObject::connect(q, SIGNAL(contentsChanged()), q, SLOT(_q_textChanged()));

    q->setScrollable(true);
    scrollArea->setHorizontalScrollBarPolicy(HbScrollArea::ScrollBarAlwaysOff); 
    defaultWrapMode = doc->defaultTextOption().wrapMode(); // cannot be changed.
    q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    doc->documentLayout()->document()->setDocumentMargin(0);
    q->setBackgroundItem(HbStyle::P_LineEdit_frame_normal);
    updateWrappingMode();
}

void HbLineEditPrivate::updatePaletteFromTheme()
{
    HbAbstractEditPrivate::updatePaletteFromTheme();
    Q_Q(HbLineEdit);

    QColor textColor = HbColorScheme::color("qtc_lineedit_normal");
    QColor selectedColor = HbColorScheme::color("qtc_lineedit_selected");
    QColor selectedBackground = HbColorScheme::color("qtc_lineedit_marker_normal");
    QPalette pal = q->palette();

    if (textColor.isValid()) {
        pal.setColor(QPalette::Text, textColor);
    }

    if (selectedColor.isValid()) {
        pal.setColor(QPalette::HighlightedText, selectedColor);
    }

    if (selectedBackground.isValid()) {
        pal.setColor(QPalette::Highlight, selectedBackground);
    }
    q->setPalette(pal);
}
bool HbLineEditPrivate::forwardKeyEvent (QKeyEvent *event)
{
    Q_Q(HbLineEdit);

    bool result = true;

    // do not allow key events if at maximum length
    if (maxLength != -1 && q->document()->begin().length() - 1 >= maxLength) {
        if (event->key() != Qt::Key_Left
                && event->key() != Qt::Key_Right
                && event->key() != Qt::Key_Up
                && event->key() != Qt::Key_Down
                && event->key() != Qt::Key_Backspace
                && event->key() != Qt::Key_Delete) {
            result = false;
        }
    }

    // do not allow line feed
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        result = false;
    }

    if (result) {
        // Don not allow key events in HbLineEdit::Password or HbLineEdit::NoEcho mode
        if (echoMode == HbLineEdit::Password || echoMode == HbLineEdit::NoEcho) {
            result = false;
        }
    }

    return result;
}

void HbLineEditPrivate::validateMaxLength()
{
    Q_Q(HbLineEdit);
    QString text(q->text());

    if (maxLength >= 0 && (text.length()) > maxLength) {
        text.resize(maxLength);
        QTextCursor cursor(doc);
        cursor.setPosition(maxLength);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        if(isPasswordMode()) {
            passwordText.resize(maxLength);
        }
    }
}

void HbLineEditPrivate::_q_textChanged()
{
    Q_Q(HbLineEdit);

    if(emitTextChanged && !isPasswordMode()) {
        emit q->textChanged(q->text());
    }
}

void HbLineEditPrivate::_q_textChange(int position, int charsRemoved,int charsAdded)
{
    Q_UNUSED(position)
    Q_UNUSED(charsRemoved)

    if(charsAdded == 0) return;

    emitTextChanged = false;
    validateMaxLength();
    emitTextChanged = true;
}


void HbLineEditPrivate::updateEditingSize()
{
    Q_Q(HbLineEdit);
    if (scrollArea) {
        const int fontHeight = QFontMetrics(q->font()).height();
        scrollArea->setMinimumHeight(fontHeight * minimumRows);
        if (maximumRows == 1) {
            scrollArea->setPreferredHeight(fontHeight);
            scrollArea->setScrollDirections(Qt::Horizontal);
        } else {
            scrollArea->setPreferredHeight(doc->documentLayout()->documentSize().height());
            scrollArea->setScrollDirections(Qt::Vertical);
        }
        scrollArea->setMaximumHeight(fontHeight * maximumRows);
    }
}

bool HbLineEditPrivate::canCopy() const
{
    bool retVal = HbAbstractEditPrivate::canCopy();
    if(echoMode != HbLineEdit::Normal) {
        retVal = false;
    }
    return retVal;
}

bool HbLineEditPrivate::isCursorVisible() const
{
    return HbAbstractEditPrivate::isCursorVisible() &&
            (echoMode == HbLineEdit::Normal || echoMode == HbLineEdit::PasswordEchoOnEdit);
}

void HbLineEditPrivate::prepDocForPaste()
{
    if(echoMode != HbLineEdit::Normal) {
        doc->clear();
        passwordText.clear();
    }
}

void HbLineEditPrivate::setCapitalization ( QFont::Capitalization caps )
{    
    QTextCursor newCursor = QTextCursor(doc);
    newCursor.select(QTextCursor::Document);
    QTextCharFormat textFormat = newCursor.charFormat();
    textFormat.setFontCapitalization(caps);
    newCursor.setCharFormat(textFormat);

    //Merge capitalization to cursor's charformat
    cursor.mergeCharFormat(textFormat);
}

QFont::Capitalization HbLineEditPrivate::capitalization() const
{
    return cursor.charFormat().fontCapitalization();
}

bool HbLineEditPrivate::isPasswordMode() const
{
    return ((echoMode == HbLineEdit::Password) ||
           (echoMode == HbLineEdit::PasswordEchoOnEdit) ||
           (echoMode == HbLineEdit::NoEcho));
}

void HbLineEditPrivate::setEchoMode(HbLineEdit::EchoMode echoMode)
{
    Q_Q(HbLineEdit);

    if(this->echoMode == HbLineEdit::Normal) {
        passwordText = q->toPlainText();
    }

    this->echoMode = echoMode;
    q->setPlainText(echoString(passwordText));
    if(isPasswordMode()){
        q->setCursorPosition(passwordText.length());
    }
}

QString HbLineEditPrivate::passwordString(const QString &text) const
{
    Q_Q(const HbLineEdit);
    int passChar = q->QGraphicsWidget::style()->styleHint(QStyle::SH_LineEdit_PasswordCharacter);
    return QString(text.length(), passChar);
}

QString HbLineEditPrivate::echoString(const QString &text)
{
    Q_Q(HbLineEdit);

    QString retText(text);

    if(echoMode == HbLineEdit::Password || (echoMode == HbLineEdit::PasswordEchoOnEdit && !q->hasFocus())) {
        retText = passwordString(text);
    } else if (echoMode == HbLineEdit::NoEcho) {
        retText.clear();
    } // else Normal

    return retText;
}

void HbLineEditPrivate::updateWrappingMode()
{
    QTextOption option = doc->defaultTextOption();
    if (maximumRows == 1) {
        option.setWrapMode(QTextOption::NoWrap);
    } else {
        option.setWrapMode(defaultWrapMode);
    }
    doc->setDefaultTextOption(option);
}

#include "moc_hblineedit.cpp"
