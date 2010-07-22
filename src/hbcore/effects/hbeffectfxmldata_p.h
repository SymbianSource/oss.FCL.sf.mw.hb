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
#ifndef HBEFFECTFXMLDATA_P_H
#define HBEFFECTFXMLDATA_P_H

#include <QObject>
#include <QList>
#include <hbglobal.h>
#include <hbstring_p.h>
#include <hbvector_p.h>

QT_FORWARD_DECLARE_CLASS(QGraphicsItem)

struct HbKeyFrame {
    // Cannot change to qreal even though krazy may insist on that.
    // (qreal may be double or float, depending on the platform,
    //  which may cause breaks when combining some template funcs with float literals...)
    float pos;
    float val;
    HbString stringValue; // needed e.g for color value
    HbKeyFrame(HbMemoryManager::MemoryType memType = HbMemoryManager::HeapMemory)
        : stringValue(memType) { }
};

struct HbEffectFxmlAttrListEntry {
    HbEffectFxmlAttrListEntry(HbMemoryManager::MemoryType memType = HbMemoryManager::HeapMemory)
        : mKey(memType), mValue(memType) { }
    HbString mKey;
    HbString mValue;
};

class HB_CORE_PRIVATE_EXPORT HbEffectFxmlParamData
{
public:
    HbEffectFxmlParamData(HbMemoryManager::MemoryType memType = HbMemoryManager::HeapMemory);
    ~HbEffectFxmlParamData();

    // e.g. "scale_x"
    QString name() const;
    void setName(const QString &value);

    QString duration() const;
    void setDuration(const QString &value);

    // Attributes of the param
    QString getAttribute(const QString &attrName) const;
    void setAttribute(const QString &attrName, const QString &value);

    // Value of the param
    QString getValue() const;
    bool getValue(qreal &value) const;

    void setValue(const QString &value);

    QList<HbKeyFrame> keyFrames() const;
    void append(const HbKeyFrame &keyFrame);

    QString startRef() const;
    void setStartRef(const QString &value);
    QString endRef() const;
    void setEndRef(const QString &value);

    bool loopDefined() const;

    float loopStart() const;
    void setLoopStart(float value);
    float loopEnd() const;
    void setLoopEnd(float value);

    HbMemoryManager::MemoryType memoryType() const;

private:
    HbMemoryManager::MemoryType mMemoryType;
    HbString mName;
    HbString mValue;
    HbString mDuration;
    HbString mStartRef;
    HbString mStartVal;
    HbString mEndRef;
    HbString mEndVal;
    float mLoopStart;
    float mLoopEnd;
    HbVector<HbEffectFxmlAttrListEntry> mAttributes; // e.g. style="linear" becomes key="style", value="linear"
    HbVector<HbKeyFrame> mKeyFrames;
};

class HB_CORE_PRIVATE_EXPORT HbEffectFxmlFilterData
{
public:
    HbEffectFxmlFilterData(HbMemoryManager::MemoryType memType = HbMemoryManager::HeapMemory);
    ~HbEffectFxmlFilterData();

    QString type() const;
    void setType(const QString &value);

    QString blending() const;
    void setBlending(const QString &value);

    void appendParamData(const HbEffectFxmlParamData &data);
    QList<HbEffectFxmlParamData> paramData() const;

    HbMemoryManager::MemoryType memoryType() const;

private:
    HbMemoryManager::MemoryType mMemoryType;

    HbString mType;
    HbString mBlending;
    HbVector<HbEffectFxmlParamData> mFxmlParams;
};

class HB_CORE_PRIVATE_EXPORT HbEffectFxmlData
{
public:
    HbEffectFxmlData(HbMemoryManager::MemoryType memType = HbMemoryManager::HeapMemory);
    ~HbEffectFxmlData();

    bool isNull() const;

    void appendParamData(const HbEffectFxmlParamData &data);
    QList<HbEffectFxmlParamData> paramData() const;

    void appendFilterData(const HbEffectFxmlFilterData &data);
    QList<HbEffectFxmlFilterData> filterData() const;

    HbMemoryManager::MemoryType memoryType() const;

private:
    HbMemoryManager::MemoryType mMemoryType;
    HbVector<HbEffectFxmlParamData> mFxmlParams;
    HbVector<HbEffectFxmlFilterData> mFilters;
};

/* Local container for storing effect info, does not store
   parsed data, but does know a pointer to parsed data if such exists*/
class HB_CORE_PRIVATE_EXPORT HbEffectInfo
{
public:
    HbEffectInfo();
    ~HbEffectInfo();

    QString componentType() const;
    QString xmlFileFullPath() const;
    QString effectEvent() const;
    bool inUse() const;
    bool fromTheme() const;
    QGraphicsItem *item() const;
    bool shared() const;

    void setEffectData(HbEffectFxmlData fxmlData);
    HbEffectFxmlData effectData() const;

private:

    QString mComponentType;
    QString mDefFileFullPath;
    bool mFromTheme;
    QString mEffectEvent;

    bool mInUse;
    bool mShared;
    QGraphicsItem *mItem; // for object specific effects
    // local or shared (depends on HbEffectFxmlData memory type)
    HbEffectFxmlData mFxmlData;
    // Effect controller fills the info
    friend class HbEffectController;
};

#endif // HBEFFECTFXMLDATA_P_H
