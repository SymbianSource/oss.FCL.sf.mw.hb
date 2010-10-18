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

#include "hblayoutparameters_p.h"
#include "hbsharedcache_p.h"
#include "hbmemoryutils_p.h"
#include "hbdeviceprofile.h"
#include "hbmemorymanager_p.h"
#include "hbhash_p.h"
#include <QtAlgorithms>

static const QString GLOBAL_PARAMETERS_LOCATION =
        QLatin1String(":/themes/style/hbdefault/variables/layout/zoom/0/hbglobalparameters.css");

QStringList HbLayoutParameters::specialVariableNames()
{
    QStringList variables;

    //order must match HbLayoutParameters::SpecialParameter-enum.
    variables << "hb-param-screen-width"
              << "hb-param-screen-height"
              << "hb-param-screen-short-edge"
              << "hb-param-screen-long-edge";

    return variables;
}

#ifndef HB_BIN_CSS
QVector<HbCss::Value> HbLayoutParameters::specialVariableValues(const HbDeviceProfile &profile)
{
    QVector<HbCss::Value> values;
    values.reserve(NumParameters);
    for (int i = 0; i < NumParameters; ++i) {
        values.append(HbCss::Value());
        values.last().type = HbCss::Value::Number;;
    }
    QSizeF pSize = profile.logicalSize();

    values[ScreenWidth].variant = pSize.width();
    values[ScreenHeight].variant = pSize.height();
    values[ShortEdge].variant =  qMin(pSize.height(), pSize.width());
    values[LongEdge].variant = qMax(pSize.height(), pSize.width());

    return values;
}

class BinaryCreatorHelper
{
public:
    BinaryCreatorHelper() : mValueBase(0) {}
    ~BinaryCreatorHelper();
    void constructBinary(const QString &parameterFile);
    
    const HbParameterItem *begin() const {
        return mItems.constData();
    }
    qint32 size() const {
        return mItems.size();
    }
    const char *valueBase() const {
        return mValueBase;
    }
    const char *nameBase() const {
        return mNames.constData();
    }
private:
    QVector<HbParameterItem> mItems;
    QByteArray mNames;
    const char *mValueBase;
};

BinaryCreatorHelper::~BinaryCreatorHelper()
{
    //destroy values
    Q_FOREACH(const HbParameterItem &item, mItems) {
        if (!item.special && item.valueOffset) {
            HbCss::Value *value = HbMemoryUtils::getAddress<HbCss::Value>(
                    HbMemoryManager::HeapMemory, item.valueOffset);
            HbMemoryUtils::release(value);
        }
    }
}

void BinaryCreatorHelper::constructBinary(const QString &parameterFile)
{
    if (!mItems.isEmpty()) return;
    //parse global parameter-file to heap.
    HbCss::Parser parser;
    parser.init(parameterFile, true);
    HbCss::StyleSheet *styleSheet = HbMemoryUtils::create<HbCss::StyleSheet>(HbMemoryManager::HeapMemory);
    parser.parse(styleSheet);
    
    //calculate the total memory needed.
    int parameterCount = 0;
    int parameterNamesTotal = 0;
    const int ruleCount = styleSheet->variableRules.count();
    for (int i = 0; i < ruleCount; ++i) {
        const HbVector<HbCss::Declaration> &decls = styleSheet->variableRules.at(i).declarations;
        parameterCount += decls.count();
        for (int j = 0; j < parameterCount; ++j) {
            parameterNamesTotal += decls.at(j).property.length() + 1; //extra for '\0'
        }
    }
    QStringList specialVariables = HbLayoutParameters::specialVariableNames();
    parameterCount += specialVariables.count();
    Q_FOREACH(const QString &variable, specialVariables) {
        parameterNamesTotal += variable.length() + 1; //extra for '\0'
    }

    mItems.reserve(parameterCount);
    mNames.reserve(parameterNamesTotal);
    
    //create the parameter items and add them to vector,
    //fill the parameterNames bytearray.
    GET_MEMORY_MANAGER(HbMemoryManager::HeapMemory)
    for (int i = 0; i < ruleCount; ++i) {
        const HbVector<HbCss::Declaration> &decls = styleSheet->variableRules.at(i).declarations;
        const int declsCount = decls.count();
        for (int j = 0; j < declsCount; ++j) {
            const HbCss::Declaration &decl = decls.at(j);
            QString parameterName(decl.property);
            quint32 hash = hbHash(QStringRef(&parameterName));
            qptrdiff valueOffset = manager->alloc(sizeof(HbCss::Value));
            new ((char*)manager->base() + valueOffset) HbCss::Value(decl.values.at(0));
            mItems.append(HbParameterItem(hash, valueOffset, mNames.size(), false));
            QByteArray latinName(parameterName.toLatin1());
            mNames.append(latinName.constData(), latinName.length() + 1);
        }
    }
    //add special variables
    for(int i = 0; i < specialVariables.count(); ++i) {
        quint32 hash = hbHash(QStringRef(&specialVariables.at(i)));
        mItems.append(HbParameterItem(hash, i, mNames.size(), true));
        QByteArray latinName(specialVariables.at(0).toLatin1());
        mNames.append(latinName.constData(), latinName.length() + 1);
    }

    qSort(mItems); //sorts by hash value
    HbMemoryUtils::release(styleSheet);
    mValueBase = static_cast<const char*>(manager->base());
}

HbLayoutParameters::HbLayoutParameters()
    : d(0)
{
#ifdef Q_OS_SYMBIAN
    //try get layoutparameters from sharedcache first. 
    d = HbSharedCache::instance()->layoutParameters();
#endif    
    if (!d) {
        static BinaryCreatorHelper binCreator;
        binCreator.constructBinary(GLOBAL_PARAMETERS_LOCATION);
        d = new HbLayoutParametersPrivate(binCreator.begin(), binCreator.size(), 
                                          binCreator.valueBase(), binCreator.nameBase());
    }
}

void HbLayoutParameters::init(const HbDeviceProfile &profile)
{
    if (d->mSpecialVariables.isEmpty() || d->mProfileName != profile.name()) {
        d->mSpecialVariables = specialVariableValues(profile);
        d->mProfileName = profile.name();
    }
}

HbLayoutParameters::const_iterator HbLayoutParameters::find(const QString &parameter) const
{
    return find(hbHash(parameter));
}

HbLayoutParameters::const_iterator HbLayoutParameters::find(quint32 hashValue) const
{
    return qBinaryFind(begin(), end(), HbParameterItem(hashValue, 0, 0, 0));
}

#endif //#ifndef HB_BIN_CSS
