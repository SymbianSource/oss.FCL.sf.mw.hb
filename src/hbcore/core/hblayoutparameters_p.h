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

#ifndef HBLAYOUTPARAMETERS_P_H
#define HBLAYOUTPARAMETERS_P_H

#include "hbcssparser_p.h"
#include <QString>
#include <QSharedData>
#include <QSharedDataPointer>

class HbDeviceProfile;

struct HbParameterItem
{
    HbParameterItem() : hash(0), valueOffset(0), nameOffset(0), special(false) {}
    HbParameterItem(quint32 hash, qptrdiff valueOffset, qptrdiff nameOffset, bool special) :
            hash(hash), valueOffset(valueOffset), nameOffset(nameOffset), special(special) {}
    quint32 hash;
    qptrdiff valueOffset;
    qptrdiff nameOffset;
    bool special;
};

inline
bool operator < (const HbParameterItem &parameterItem1,
                 const HbParameterItem &parameterItem2)
{
    return parameterItem1.hash < parameterItem2.hash;
}

class HbLayoutParametersPrivate : public QSharedData
{
public:
    typedef const HbParameterItem* const_iterator;
    typedef HbParameterItem* iterator;
public:
    HbLayoutParametersPrivate(const HbParameterItem* begin, qint32 size, 
                              const char* valueBase, const char* nameBase)
        : mBegin(begin), mSize(size), mValueBase(valueBase), mNameBase(nameBase)
      {
      }
    HbLayoutParametersPrivate(const HbLayoutParametersPrivate &other) 
        : QSharedData(other),
          mProfileName(other.mProfileName),
          mSpecialVariables(other.mSpecialVariables),
          mBegin(other.mBegin),
          mSize(other.mSize),
          mValueBase(other.mValueBase),
          mNameBase(other.mNameBase)
          {
          }
public:
    QString mProfileName;
    QVector<HbCss::Value> mSpecialVariables;
    const HbParameterItem *mBegin;
    qint32 mSize;
    const char *mValueBase;
    const char *mNameBase;
};

class HB_CORE_PRIVATE_EXPORT HbLayoutParameters
{
public:
    enum SpecialParameter
    {
        ScreenWidth = 0,
        ScreenHeight,
        ShortEdge,
        LongEdge,
        NumParameters
    };
    
    typedef HbLayoutParametersPrivate::const_iterator const_iterator;
    typedef HbLayoutParametersPrivate::iterator iterator;
public:
    static QStringList specialVariableNames();
    static QVector<HbCss::Value> specialVariableValues(const HbDeviceProfile &profile);

    HbLayoutParameters();
    HbLayoutParameters(const HbLayoutParameters &other) : d(other.d) {}
    ~HbLayoutParameters() {}
    void init(const HbDeviceProfile &profile);
    bool isEmpty() const { return d->mSize <= 0; }
    QString name(const const_iterator &iterator) const
    {
        return QString(QLatin1String(d->mNameBase + iterator->nameOffset));
    }
    const HbCss::Value &value(const const_iterator &iterator) const
    {
        if (iterator->special) {
            return d->mSpecialVariables.at(iterator->valueOffset);
        }
        const void *valPtr = d->mValueBase + iterator->valueOffset;
        return *static_cast<const HbCss::Value*>(valPtr);
    }
    const_iterator begin() const { return d->mBegin; }
    const_iterator end() const { return d->mBegin + d->mSize; }
    const_iterator constBegin() const { return begin(); }
    const_iterator constEnd() const { return end(); }
    const_iterator find(const QString &parameter) const;
    const_iterator find(quint32 hashValue) const;

private:
    QSharedDataPointer<HbLayoutParametersPrivate> d;
};

#endif // HBLAYOUTPARAMETERS_P_H
