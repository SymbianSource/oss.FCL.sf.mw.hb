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


#ifndef HBPARAMETERLENGTHLIMITER_H
#define HBPARAMETERLENGTHLIMITER_H

#include <QString>
#include <hbglobal.h>
#include <hbi18ndef.h>

class HbParameterLengthLimiterPrivate;

class HB_CORE_EXPORT HbParameterLengthLimiter
{
    public:
        HbParameterLengthLimiter( const HbParameterLengthLimiter& a );
        HbParameterLengthLimiter(QString a);
        HbParameterLengthLimiter(const char* a);
        HbParameterLengthLimiter(const char* a, int n);
        HbParameterLengthLimiter();
        ~HbParameterLengthLimiter();

        HbParameterLengthLimiter& arg(qlonglong a, int fieldwidth=0, int base=10,
                const QChar &fillChar = QLatin1Char(' '));
        HbParameterLengthLimiter& arg(qulonglong a, int fieldwidth=0, int base=10,
                const QChar &fillChar = QLatin1Char(' '));
        HbParameterLengthLimiter& arg(long a, int fieldwidth=0, int base=10,
                const QChar &fillChar = QLatin1Char(' '));
        HbParameterLengthLimiter& arg(ulong a, int fieldwidth=0, int base=10,
                const QChar &fillChar = QLatin1Char(' '));
        HbParameterLengthLimiter& arg(int a, int fieldWidth = 0, int base = 10,
                const QChar &fillChar = QLatin1Char(' '));
        HbParameterLengthLimiter& arg(uint a, int fieldWidth = 0, int base = 10,
                const QChar &fillChar = QLatin1Char(' '));
        HbParameterLengthLimiter& arg(short a, int fieldWidth = 0, int base = 10,
                const QChar &fillChar = QLatin1Char(' '));
        HbParameterLengthLimiter& arg(ushort a, int fieldWidth = 0, int base = 10,
                const QChar &fillChar = QLatin1Char(' '));
        HbParameterLengthLimiter& arg(double a, int fieldWidth = 0, char fmt = 'g', int prec = -1,
                const QChar &fillChar = QLatin1Char(' '));
        HbParameterLengthLimiter& arg(char a, int fieldWidth = 0,
                const QChar &fillChar = QLatin1Char(' '));
        HbParameterLengthLimiter& arg(QChar a, int fieldWidth = 0,
                const QChar &fillChar = QLatin1Char(' '));
        HbParameterLengthLimiter& arg(const QString &a, int fieldWidth = 0,
                const QChar &fillChar = QLatin1Char(' '));
        HbParameterLengthLimiter& arg(const QString &a1, const QString &a2);
        HbParameterLengthLimiter& arg(const QString &a1, const QString &a2, const QString &a3);
        HbParameterLengthLimiter& arg(const QString &a1, const QString &a2, const QString &a3,
                             const QString &a4);
        HbParameterLengthLimiter& arg(const QString &a1, const QString &a2, const QString &a3,
                             const QString &a4, const QString &a5);
        HbParameterLengthLimiter& arg(const QString &a1, const QString &a2, const QString &a3,
                             const QString &a4, const QString &a5, const QString &a6);
        HbParameterLengthLimiter& arg(const QString &a1, const QString &a2, const QString &a3,
                             const QString &a4, const QString &a5, const QString &a6,
                             const QString &a7);
        HbParameterLengthLimiter& arg(const QString &a1, const QString &a2, const QString &a3,
                             const QString &a4, const QString &a5, const QString &a6,
                             const QString &a7, const QString &a8);
        HbParameterLengthLimiter& arg(const QString &a1, const QString &a2, const QString &a3,
                             const QString &a4, const QString &a5, const QString &a6,
                             const QString &a7, const QString &a8, const QString &a9);
        HbParameterLengthLimiter& operator=(const QString &a);
        HbParameterLengthLimiter& operator=(const HbParameterLengthLimiter &a);
        operator QString() const;

    private:
        HbParameterLengthLimiterPrivate* p;
};

#endif // HbParameterLengthLimiter_H
