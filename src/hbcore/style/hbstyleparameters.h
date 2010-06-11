/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbCore module of the UI Extensions for Mobile.
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

#ifndef HBSTYLEPARAMETERS_H
#define HBSTYLEPARAMETERS_H

#include <hbglobal.h>
#include <QVariant> 

class HbStyleParametersPrivate;

class HB_CORE_EXPORT HbStyleParameters
{
public:
    HbStyleParameters();
    virtual ~HbStyleParameters();

    int count();
    int indexOf( const QString& key );
    QString key( int index );
    QString parameter( int index );

    QVariant value( int index );
    QVariant value( const QString &key );

    void setValue( int index, const QVariant &value );
    void setValue( const QString &key, const QVariant &value );

    void addParameter( const QString &key, const QVariant &value = QVariant() );
    void removeAt( int index );

private:
    friend class HbStyle;
    QList<QString> &keys();
    QList<QVariant> &values();

private:
    HbStyleParametersPrivate *const d_ptr;

private:
    Q_DISABLE_COPY( HbStyleParameters )
    Q_DECLARE_PRIVATE_D( d_ptr, HbStyleParameters )
};


#endif // HBSTYLEPARAMETERS_H
