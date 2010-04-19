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
#include <QString>
#include <QTextDocument>
#include <QTextCursor>

#include "hbvalidatorfield.h"
#include "hbvalidatorfield_p.h"
#include "hbvalidator.h"
#include "hbvalidator_p.h"

HbValidatorFieldPrivate::HbValidatorFieldPrivate():
    parent(0),
    validator(0),
    defaultValue(QString()),
    startPosition(0),
    endPosition(0)
{
}

HbValidatorFieldPrivate::~HbValidatorFieldPrivate()
{
}

/*!
    \class HbValidator
    \brief HbValidator provides validation services for inputs for QAbstractEdit derived editors.
*/

/*!
    Constructs a HbValidator object with a /a parent
*/
HbValidatorField::HbValidatorField(HbValidator* parent) :
    d_ptr(new HbValidatorFieldPrivate())

{
    Q_D(HbValidatorField);
    d->q_ptr = this;
    d->parent = parent;
}

/*!
    Destructor
*/
HbValidatorField::~HbValidatorField()
{
    Q_D(HbValidatorField);
    delete d;
}

void HbValidatorField::setQValidator(QValidator *validator)
{
    Q_D(HbValidatorField);
    d->validator = validator;
}

/*!
    Validates the content, and cursor position.

    Can alter the content of the document, and cursor parameters.
 */
QValidator::State HbValidatorField::validate(QTextDocument* doc, QTextCursor& cursor, int changePosition, int changeCharsRemoved, int changeCharsAdded)
{
    Q_D(HbValidatorField);

    Q_UNUSED(cursor);
    Q_UNUSED(changePosition);
    Q_UNUSED(changeCharsRemoved);
    Q_UNUSED(changeCharsAdded);

    int pos = 0;
    QString content = doc->toPlainText();   //TODO check if formating is needed, if yes use QTextCursor
    QString fieldText = content.mid(startPosition(), length());
    QValidator::State state = d->validator->validate(fieldText, pos);
    return state;
}

void HbValidatorField::validateCursorPosition(const QTextCursor &oldCursor, QTextCursor& newCursor)
{
    Q_UNUSED(oldCursor)
    Q_UNUSED(newCursor)
}

void HbValidatorField::setDefaultValue(QString value)
{
    Q_D(HbValidatorField);
    d->defaultValue = value;
}

QString HbValidatorField::defaultValue() const
{
    Q_D(const HbValidatorField);
    return d->defaultValue;
}

/*! Returns the type of the field. There are two kinds of fields: editable fields and separators.

    \sa fieldType
 */
void HbValidatorField::setFieldType(FieldType type)
{
    Q_D(HbValidatorField);
    d->type = type;
}

/*! Returns the type of the field. There are two kinds of fields: editable fields and separators.

    \sa setFieldType
 */
HbValidatorField::FieldType HbValidatorField::fieldType() const
{
    Q_D(const HbValidatorField);
    return d->type;
}

/*! Sets the position of this field.

    \sa startPosition
 */
void HbValidatorField::setStartPosition(int startPosition)
{
    Q_D(HbValidatorField);
    d->startPosition = startPosition;
}


/*! Returns the start position of the field in the document.

    \sa setStartPosition
 */
int HbValidatorField::startPosition() const
{
    Q_D(const HbValidatorField);
    return d->startPosition;
}

/*! Sets the end position of this field.

    \sa endPosition
 */
void HbValidatorField::setEndPosition(int endPosition)
{
    Q_D(HbValidatorField);
    d->endPosition = endPosition;
}

/*! Returns the end position of the field in the document.

    \sa setEndPosition
 */
int HbValidatorField::endPosition() const
{
    Q_D(const HbValidatorField);
    return d->endPosition;
}

/*! Returns the length of the field.
 */
int HbValidatorField::length() const
{
    Q_D(const HbValidatorField);
    return d->endPosition - d->startPosition;
}

