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

#ifndef HBVALIDATOR_P_H
#define HBVALIDATOR_P_H
#include <QPointer>
#include "hbvalidator.h"

class QTextDocument;

class HbValidatorField
{
public:
    QPointer<QValidator> mValidator;
    QString mDefaultValue;
    QString mNextSeparator;
};

class HbValidatorPrivate
{
    Q_DECLARE_PUBLIC(HbValidator)

public:
    HbValidatorPrivate();

    void init();

    void clear();

    void addField(const QString &separator, QValidator *validator, const QString &defaultValue);

    QValidator::State validateText(const QTextDocument *doc);

    int findFieldIndexUnderCursorPos(const QTextCursor &cursor);

    bool fieldHasSelection(int fieldIndex, const QTextCursor &cursor);

    void highlightField(QTextCursor* cursor, int fieldIndex);

    void deleteAllFields();

    QString separatorToAdd;

    QList<HbValidatorField*> fields;

    HbValidator *q_ptr;
};

#endif // HBVALIDATOR_P_H
