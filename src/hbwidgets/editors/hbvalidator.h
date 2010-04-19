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

#ifndef HBVALIDATOR_H
#define HBVALIDATOR_H

//#define HBVALIDATOR_DEBUG_ENABLE

#include <hbglobal.h>
#include <QObject>

#include <QValidator>

QT_FORWARD_DECLARE_CLASS(QTextDocument) 
QT_FORWARD_DECLARE_CLASS(QTextCursor) 
class HbValidatorPrivate;

class HB_WIDGETS_EXPORT HbValidator : public QObject
{
public:
    enum ChangeSource {
        CursorChangeFromOperation,
        CursorChangeFromMouse,
        CursorChangeFromContentUpdate,
        CursorChangeFromContentSet
    };

    explicit HbValidator(QObject *parent=0);
    virtual ~HbValidator();

    QString defaultValue();

    void addField(QValidator *validator, const QString &defaultValue);
    void addField(const QString &separatorBeforeField, QValidator *validator, const QString &defaultValue);

    void setDefaultSeparator(const QString &str);
    QString defaultSeparator();
    QString separatorAt(int index);

    virtual QValidator::State validateContent(QTextCursor &cursor, int changePosition, int changeCharsRemoved, int changeCharsAdded);
    
    virtual void validateCursorPosition(const QTextCursor &oldCursor, QTextCursor &newCursor, HbValidator::ChangeSource source);

    virtual void updateTextCursor(QTextCursor &cursor);

    void addSeparator(const QString &str); // DEPRECATED
    void setMasterValidator(QValidator *validator); // DEPRECATED
    const QValidator *masterValidator() const;  // DEPRECATED

protected:
    HbValidatorPrivate *const d_ptr;
    HbValidator(HbValidatorPrivate &dd, QObject *parent);

private:
    Q_DISABLE_COPY(HbValidator)
    Q_DECLARE_PRIVATE_D(d_ptr, HbValidator)
};

#endif // HBVALIDATOR_H
