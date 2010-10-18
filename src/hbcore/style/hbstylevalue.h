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

#ifndef HBSTYLEVALUE_H
#define HBSTYLEVALUE_H

#include <QString>
#include <QVariant>
#include <hbicon.h>

template<typename T>
class HbStyleValue
{
public:
    HbStyleValue();
    ~HbStyleValue();

    bool isSet() const;
    T value() const;
    operator T() const;
    void operator=(const T &value);
    bool operator!=(const T &value) const;
    bool operator==(const T &value) const;
    void clear();

private:
    bool mSet;
    T mVal;
};

template<typename T>
HbStyleValue<T>::HbStyleValue() : mSet(false)
{
}

template<typename T>
HbStyleValue<T>::~HbStyleValue()
{
}

template<typename T>
bool HbStyleValue<T>::isSet() const
{
    return mSet;
}

template<typename T>
T HbStyleValue<T>::value() const
{
    Q_ASSERT(mSet);
    return mVal;
}

template<typename T>
HbStyleValue<T>::operator T() const
{
    Q_ASSERT(mSet);
    return mVal;
}

template<typename T>
void HbStyleValue<T>::operator=(const T &value)
{
    mVal = value;
    mSet = true;
}

template<typename T>
void HbStyleValue<T>::clear()
{
    mSet = false;
}

template<typename T>
bool HbStyleValue<T>::operator!=(const T &value) const
{
    if (!mSet) {
        return true;
    }
    return mVal!=value;
}

template<typename T>
bool HbStyleValue<T>::operator==(const T &value) const
{
    return !(*this!=value);
}

template<typename T>
bool operator!=(const T &value, const HbStyleValue<T> &other)
{
    return other!=value;
}

template<typename T>
inline bool operator==(const T &value, const HbStyleValue<T> &other)
{
    return !(other!=value);
}

template<>
inline void HbStyleValue<QString>::clear()
{
    mSet = false;
    mVal.clear();
}

template<>
inline void HbStyleValue<QVariant>::clear()
{
    mSet = false;
    mVal.clear();
}

template<>
inline void HbStyleValue<HbIcon>::clear()
{
    mSet = false;
    mVal.clear();
}

#endif // HBSTYLEVALUE_H
