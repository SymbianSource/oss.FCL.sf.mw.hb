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

#include "hbvalidator_p.h"
#include <QTextDocument>
#include <QTextCursor>
#include <QDebug>
#include <QtAlgorithms>

#ifdef HBVALIDATOR_DEBUG_ENABLE
#define VALIDATORDEBUG(x) printFields(x)
#else
#define VALIDATORDEBUG(x)
#endif

HbValidatorPrivate::HbValidatorPrivate() :
    separatorToAdd("/")
{
}

void HbValidatorPrivate::init()
{
    // no implemantation needed
}

void HbValidatorPrivate::clear()
{
    deleteAllFields();
}

void HbValidatorPrivate::addField(const QString &separator, QValidator *validator, const QString &defaultValue)
{
    if(!fields.isEmpty()) {
        // set separator between this and previous field
        fields.back()->mNextSeparator = separator;
    }

    // creating new filed
    HbValidatorField *newField = new HbValidatorField();
    fields.append(newField);
    
    newField->mDefaultValue = defaultValue;
    newField->mValidator = validator;
}

QValidator::State HbValidatorPrivate::validateText(const QTextDocument *doc)
{
    QValidator::State state = QValidator::Acceptable;
    int lastSeparatorPos = 0;
    QTextCursor suportCursor;

    foreach(HbValidatorField* field, fields) {
        int separatorPos;

        if(!field->mNextSeparator.isEmpty()) {
            suportCursor = doc->find(field->mNextSeparator, lastSeparatorPos, QTextDocument::FindCaseSensitively);
            if(suportCursor.isNull()) {
                // parsing failed
                return QValidator::Invalid;
            }
            separatorPos = suportCursor.anchor();
        } else {
            // last posible cursor position
            separatorPos = doc->characterCount()-1;
            Q_ASSERT_X(separatorPos == doc->toPlainText().length(), 
                "HbValidatorPrivate::validateText", 
                "problems with QTextDocument::characterCount (to big value)");
        }

        // select field:
        QTextCursor toValidate(const_cast<QTextDocument *>(doc));
        toValidate.setPosition(lastSeparatorPos);
        toValidate.setPosition(separatorPos, QTextCursor::KeepAnchor);

        QString fieldText = toValidate.selectedText();
        int cursorPos = fieldText.length();
        QValidator::State fieldState = field->mValidator->validate(fieldText, cursorPos);

        // maybe here changes done by field validator shold be copied to document using QTextCursor toValidate?
        // in this case const from method argument should be removed

        if(fieldState<state) {
            state = fieldState;
        }

        if(state == QValidator::Invalid) {
            break;
        }

        // position of cursor after separator
        lastSeparatorPos = suportCursor.position();
    }

    return state;
}

int HbValidatorPrivate::findFieldIndexUnderCursorPos(const QTextCursor &cursor)
{
    const int cursorPosition(cursor.position());
    const int lastFieldIndex(fields.count()-1);
    int findFrom = 0;
    int i;

    QTextDocument *doc = cursor.document();
    for(i = 0; i<lastFieldIndex; ++i) {
        QTextCursor separatorFound = doc->find(fields.at(i)->mNextSeparator, findFrom, QTextDocument::FindCaseSensitively);
        if(separatorFound.isNull()) {
            return -1;
        }
        findFrom = separatorFound.position();
        if(findFrom>cursorPosition) {
            break;
        }
    }
    return i;
}

bool HbValidatorPrivate::fieldHasSelection(int fieldIndex, const QTextCursor &cursor)
{
    if(!cursor.hasSelection()) {
        return false;
    }

    const int lastFieldIndex = fields.count() - 1;

    Q_ASSERT( fieldIndex >= 0 );
    Q_ASSERT( fieldIndex <= lastFieldIndex );

    const int cursorBegin = qMin(cursor.position(), cursor.anchor());
    const int cursorEnd   = qMax(cursor.position(), cursor.anchor());

    // cheching left side of cursor
    if(fieldIndex==0) {// boundary condition (front):
        if(cursorBegin != 0) {
            return false;
        }
    } else { // normal check
        QTextCursor supportCursor = cursor.document()->find( 
                    fields.at(fieldIndex-1)->mNextSeparator,
                    cursorBegin, 
                    QTextDocument::FindCaseSensitively | QTextDocument::FindBackward);
        Q_ASSERT(!supportCursor.isNull());
        if( supportCursor.position() != cursorBegin) {
            return false;
        }
    }

    // checking right side of cursor
    if(fieldIndex==lastFieldIndex) { // boundary condition (tail):
        if( cursorEnd != cursor.document()->characterCount()-1 ) {
            return false;
        }
    } else { // normal check
        QTextCursor supportCursor = cursor.document()->find( 
                    fields.at(fieldIndex)->mNextSeparator,
                    cursorEnd, 
                    QTextDocument::FindCaseSensitively);
        Q_ASSERT(!supportCursor.isNull());
        if( supportCursor.anchor() != cursorEnd) {
            return false;
        }
    }

    return true;
}

void HbValidatorPrivate::highlightField(QTextCursor* cursor, int fieldIndex)
{
    const int lastFieldIndex = fields.count() - 1;

    Q_ASSERT( fieldIndex >= 0 );
    Q_ASSERT( fieldIndex <= lastFieldIndex );

    QTextDocument *doc = cursor->document();
    int lastFind = 0;
    for(int i=0; i<fieldIndex; ++i) {
        lastFind = doc->find(
            fields.at(i)->mNextSeparator, 
            lastFind, 
            QTextDocument::FindCaseSensitively ).position();
    }
    cursor->setPosition(lastFind);

    if(fieldIndex == lastFieldIndex) {
        lastFind = doc->characterCount()-1;
    } else {
        lastFind = doc->find(
                fields.at(fieldIndex)->mNextSeparator, 
                lastFind, 
                QTextDocument::FindCaseSensitively ).anchor();
    }
    cursor->setPosition(lastFind, QTextCursor::KeepAnchor);
}

void HbValidatorPrivate::deleteAllFields()
{
    Q_Q(HbValidator);

    foreach(HbValidatorField *field, fields) {
        if(field->mValidator) {
            if( field->mValidator->parent() == q ) {
                delete field->mValidator;
            }
        }
        delete field;
    }
    fields.clear();
}
