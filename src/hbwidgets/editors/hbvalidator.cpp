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

#include "hbvalidator.h"
#include "hbvalidator_p.h"
#include <QTextCursor>
#include <QDebug>
#include <QRegExp>
#include <QRegExpValidator>
#include <QTextDocument>
#include <QPair>

#ifdef HBVALIDATOR_DEBUG_ENABLE
#define VALIDATORDEBUG(x) d->printFields(x)
#else
#define VALIDATORDEBUG(x)
#endif

/*!
\class HbValidator
\brief Provides validation services for \ref HbAbstractEdit derived editors.
@proto

Validators are a kind of programmable elements to restrict what sort of input user can enter in an editor. 

It provides fields which can contain different types of data, which can be separately validated by QValidator.
Fields are separated by a string (see addField and setDefaultSeparator).

\section usage Creating validating editors

Example for simple integer validator. This can be used to validate IPv4 addresses. 

\code
#include <hbvalidator.h>

HbValidator *validator = new HbValidator();

validator->setDefaultSeparator(".");
validator->addField(new QIntValidator(0, 255, 0), "0");
validator->addField(new QIntValidator(0, 255, 0), "0");
validator->addField(new QIntValidator(0, 255, 0), "0");
validator->addField(new QIntValidator(0, 255, 0), "0");

editor->setValidator(validator);
\endcode

\ref HbValidator can also be subclassed to build even more complex validators.


*/

/*!
    Constructs a HbValidator object with a \a parent
*/
HbValidator::HbValidator(QObject* parent) :
    QObject(parent), d_ptr(new HbValidatorPrivate())

{
    Q_D(HbValidator);
    d->q_ptr = this;
}

HbValidator::HbValidator(HbValidatorPrivate &dd, QObject* parent) :
    QObject(parent), d_ptr(&dd)
{
    Q_D(HbValidator);
    d->q_ptr = this;
}

/*!
    Destructor
*/
HbValidator::~HbValidator()
{
    Q_D(HbValidator);
    d->clear();
    delete d;
}

/*!
    returns default value for QTextDocument as a plain text. This value is build using 
    separators and default values of fields \sa addField \sa setDefaultSeparator
*/
QString HbValidator::defaultValue()
{
    Q_D(HbValidator);
    QString result;

    foreach(HbValidatorField *field, d->fields) {
        result.append(field->mDefaultValue);
        result.append(field->mNextSeparator);
    }

    return result;
}

/*!
    sets string which will be used as separator when adding new field(s). 
    
    This string should be unique comparing to previews field. For example separators 
    "." or "," should be used after field with floating point numbers (since this 
    separators are treated as decimal separators depending on locale).

    \a str contains separator to used i next call of addField

    Example of IP address validation:
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,33}

    \sa addField
    \sa defaultSeparator()
 */
void HbValidator::setDefaultSeparator(const QString &str)
{
    if(!str.isEmpty()) {
        Q_D(HbValidator);
        d->separatorToAdd = str;
    }
}

/*!
    returns string which is used as separator when adding new field(s).

    \sa setDefaultSeparator
    \sa addField
*/
QString HbValidator::defaultSeparator()
{
    Q_D(HbValidator);

    return d->separatorToAdd;
}

/*!
    returns string for separator of specyfied index (index of field before separator).
    Index can be from 0 to number of fields -2 (nuber of fields-1 is current nuber of
    separators in use)

    \a index index separator or index (field index before separator)
    \return separator after field with index
*/
QString HbValidator::separatorAt(int index)
{
    Q_D(HbValidator);
    if(index<0 || index >= d->fields.count() ) {
        return QString(); // out of range return empty string
    }
    return d->fields.at(index)->mNextSeparator;
}

/*!
    Appends QValidator field to HbValidator.

    This function adds normal editable field to validator. Field consist of QValidator and default value. Field's
    default value is used when validator initialized and document content is invalid or empty in initialization phase.
    Field is validated against validator when a change happens in field content.
    In case it is not first field in HbValidator default separatpor is added (see setDefaultSeparator) before new field.

    \a validator is used to validate field content. Ownership of validator is passed to HbValidator
    \a defaultValue is a string that is used in validator initialization phase. defaultValue is used field content if
        fields and separators can't be created from current document content.

    Simplest example:
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,32}

    \sa setDefaultSeparator
 */
void HbValidator::addField(QValidator *validator, const QString &defaultValue)
{
    Q_D(HbValidator);

    validator->setParent(this);
    d->addField(d->separatorToAdd, validator, defaultValue);
}

/*!
    Appends QValidator field to HbValidator

    This method is added for convieniece.
    In case it is not first field in validator separator (separatorBeforeField) is added before new field.
    If separatorBeforeField is empty default separator is used.

    \a separatorBeforeField - separator to be added before new field (if epmty default value is used)
    \a validator is used to validate field content. Ownership of validator is passed to HbValidator
    \a defaultValue is a string that is used in validator initialization phase. defaultValue is used field content if
        fields and separators can't be created from current document content.

    Chess move example:
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,34}

    \sa addField(QValidator *, const QString &)
 */
void HbValidator::addField(const QString &separatorBeforeField, QValidator *validator, const QString &defaultValue)
{
    Q_D(HbValidator);

    validator->setParent(this);
    if(separatorBeforeField.isEmpty()) {
        d->addField(d->separatorToAdd, validator, defaultValue);
    } else {
        d->addField(separatorBeforeField, validator, defaultValue);
    }
}

/*!
    Validates the content
    @proto

    Can alter the content of the document, and cursor parameters.

    \a cursor QTextCursor pointing to document which will be validated
    \a changePosition is the position where the change begins (currently ignored)
    \a changeCharsRemoved if characters were removed, this is the count how many (currently ignored)
    \a changeCharsAdded if characters were added, this is the count how many (currently ignored)
 */
QValidator::State HbValidator::validateContent(QTextCursor& cursor, int changePosition, int changeCharsRemoved, int changeCharsAdded)
{
    Q_D(HbValidator);
    Q_UNUSED(changePosition);
    Q_UNUSED(changeCharsRemoved);
    Q_UNUSED(changeCharsAdded);

    if(cursor.isNull()) {
        return QValidator::Invalid;
    }
    return d->validateText(cursor.document());
}

// #define HBVALIDATOR_REPORTS
/*!
    Validates the cursor position.
    @proto

    \a oldCursor is QTextCursor before cursor movement
    \a newCursor is the new QTextCursor that tells the current cursor state
    \a source tells the source of cursor position change
 */
void HbValidator::validateCursorPosition(const QTextCursor &oldCursor, QTextCursor &newCursor, HbValidator::ChangeSource source)
 {
    Q_D(HbValidator);

#ifdef HBVALIDATOR_REPORTS
    qDebug() << "oldCursor("<<oldCursor.anchor()<<','<<oldCursor.position()<< "): "<<oldCursor.selectedText()
             << "\tnewCursor("<<newCursor.anchor()<<','<<newCursor.position()<< "): "<<newCursor.selectedText();
#endif

    int newFieldIndex = d->findFieldIndexUnderCursorPos(newCursor);
    if(newFieldIndex<0) {
        return;
    }

    if(source == CursorChangeFromContentUpdate ) {
        return;
    }

    int oldFieldIndex = d->findFieldIndexUnderCursorPos(oldCursor);
    if(newFieldIndex!=oldFieldIndex) { // cursor is jumping to new field
        d->highlightField(&newCursor,newFieldIndex);
        return;
    }
    Q_ASSERT(oldFieldIndex>=0);

    if(source == CursorChangeFromOperation && d->fieldHasSelection(oldFieldIndex, oldCursor) ) {
        if(oldCursor.anchor()==newCursor.position()) {
            if( newFieldIndex>0 ) {
                d->highlightField( &newCursor, newFieldIndex-1 );
            }
        } else if(oldCursor.position()==newCursor.position()) {
            if( newFieldIndex+1 < d->fields.count() ) {
                d->highlightField( &newCursor, newFieldIndex+1 );
            }
        }
    }
 }

/*!
    This function can be used for updating cursor position after content validation.
    Default implementation is empty. QTextCursor given as a parameter can be used 
    to change cursor position.

    method is considered to be deleted or reused in a bit different context.
 */
void HbValidator::updateTextCursor(QTextCursor &cursor)
{
    Q_UNUSED(cursor);
}

/*!
   \deprecated HbValidator::addSeparator(const QString&)
        is deprecated. You can use addDefaultSeparator or addField with separator parameter instead of this.

 */
void HbValidator::addSeparator(const QString &str)
{
    qWarning() << "call of DEPRECATED method HbValidator::addSeparator";
    setDefaultSeparator(str);
}

/*!
   \deprecated HbValidator::setMasterValidator(QValidator*)
        is deprecated. Master validator is not needed anymore. Use only addField.

 */
void HbValidator::setMasterValidator(QValidator *validator)
{
    qWarning() << "call of DEPRECATED method HbValidator::setMasterValidator";

    // ownership was taken over, but muster validator is not needed
    delete validator; // DEPRECATED
}

/*!
   \deprecated HbValidator::masterValidator() const
        is deprecated. Master validator is not needed anymore. Use only addField.

 */
const QValidator *HbValidator::masterValidator() const
{
    qWarning() << "call of DEPRECATED method HbValidator::masterValidator";

    // ownership was taken over, but muster validator is not needed
    return NULL;
}

