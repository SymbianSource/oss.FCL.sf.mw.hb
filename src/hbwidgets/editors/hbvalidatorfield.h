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

#ifndef HBVALIDATORFIELD_H
#define HBVALIDATORFIELD_H

#include <hbglobal.h>
#include <QValidator>

class HbValidatorFieldPrivate;
class HbValidator;

class HbValidatorField
{
public:

    enum FieldType {
        Field,
        Separator
    };

    explicit HbValidatorField(HbValidator* parent);
    virtual ~HbValidatorField();

    void setQValidator(QValidator *validator);

    QValidator::State validate(QTextDocument* doc, QTextCursor& cursor, int changePosition, int changeCharsRemoved, int changeCharsAdded);
    virtual void validateCursorPosition(const QTextCursor &oldCursor, QTextCursor& newCursor);

    void setDefaultValue(QString value);
    QString defaultValue() const;

    void setFieldType(FieldType type);
    FieldType fieldType() const;

    void setStartPosition(int startPosition);
    int startPosition() const;

    void setEndPosition(int endPosition);
    int endPosition() const;

    int length() const;

protected:
     HbValidatorFieldPrivate * const d_ptr;

private:
    Q_DISABLE_COPY(HbValidatorField)
    Q_DECLARE_PRIVATE_D(d_ptr, HbValidatorField)
};

#endif // HBVALIDATORFIELD_H
