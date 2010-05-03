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
#ifndef HB_INPUT_STANDARD_FILTERS_H
#define HB_INPUT_STANDARD_FILTERS_H

#include <hbinputdef.h>
#include <hbinputfilter.h>

//
// Phone number filter
//
class HB_CORE_EXPORT HbPhoneNumberFilter : public HbInputFilter
{
public:
    static HbPhoneNumberFilter* instance();
    virtual ~HbPhoneNumberFilter();
    bool filter(QChar character);

private:
    HbPhoneNumberFilter();

private:
    Q_DISABLE_COPY(HbPhoneNumberFilter)
};

//
// Formatted numbers filter
//
class HB_CORE_EXPORT HbFormattedNumbersFilter : public HbInputFilter
{
public:
    static HbFormattedNumbersFilter* instance();
    virtual ~HbFormattedNumbersFilter();
    bool filter(QChar character);

private:
   HbFormattedNumbersFilter();

private:
    Q_DISABLE_COPY(HbFormattedNumbersFilter)
};

//
// digits only filter
//
class HB_CORE_EXPORT HbDigitsOnlyFilter : public HbInputFilter
{
public:
    static HbDigitsOnlyFilter* instance();
    virtual ~HbDigitsOnlyFilter();
    bool filter(QChar character);

private:
   HbDigitsOnlyFilter();

private:
    Q_DISABLE_COPY(HbDigitsOnlyFilter)
};

//
// Url filter
//
class HB_CORE_EXPORT HbUrlFilter : public HbInputFilter
{
public: 
    static HbUrlFilter* instance();
    virtual ~HbUrlFilter();
    bool filter(QChar character);

private:
     HbUrlFilter();

private:
    Q_DISABLE_COPY(HbUrlFilter)
};


//
// Email address filter
//
class HB_CORE_EXPORT HbEmailAddressFilter : public HbInputFilter
{
public: 
    static HbEmailAddressFilter* instance();
    virtual ~HbEmailAddressFilter();
    bool filter(QChar character);

private:
    HbEmailAddressFilter();

private:
    Q_DISABLE_COPY(HbEmailAddressFilter)
};

//
// Lower case filter
//
class HB_CORE_EXPORT HbInputLowerCaseFilter : public HbInputFilter
{
public:
    static HbInputLowerCaseFilter* instance();
    virtual ~HbInputLowerCaseFilter() {}
    bool filter(QChar character);

private:
    HbInputLowerCaseFilter() {}

private:
    Q_DISABLE_COPY(HbInputLowerCaseFilter)
};

//
// Upper case filter
//
class HB_CORE_EXPORT HbInputUpperCaseFilter : public HbInputFilter
{
public:
    static HbInputUpperCaseFilter* instance();
    virtual ~HbInputUpperCaseFilter() {}
    bool filter(QChar character);

private:
    HbInputUpperCaseFilter() {}

private:
    Q_DISABLE_COPY(HbInputUpperCaseFilter)
};

#endif // HB_INPUT_STANDARD_FILTERS_H

// End of file
