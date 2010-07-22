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

#include "hbeffectfxmldata_p.h"
#include <QDebug>

static const float LoopUndefined = -1;

// 0.0 is min allowed value but comparing it to this value for fuzzy comparison
static const float LoopMinDefined = -0.00001f;

/*

  HbEffectFxmlData, HbEffectFxmlFilterData and HbEffectFxmlParamData are sharable classes,
  HbEffectFxmlData and HbEffectFxmlParamData are sharable classes,
  i.e. they can be created with the HbMemoryManager::SharedMemory type,
  in which case all their internal data will be allocated from a shared
  memory chunk instead of using normal new, malloc, etc. This means that
  they will not contain any pointers to memory allocated within the
  creator process so other processes can access the same data by
  receiving the offset via IPC and performing a cast to get a valid
  HbEffectFxmlData pointer.

  Note that passing HbMemoryManager::SharedMemory will lead to
  assertions and failures in regular applications, this flag should only
  be used in the theme server because the shared memory is not writable
  for other applications.

  The shared behavior can be disabled by passing
  HbMemoryManager::HeapMemory to the constructors (which is also the
  default).

  Note that the assignment and pass-by-value semantics (i.e. when does the data
  get transferred from shared to local memory) depend on the underlying shared
  container behavior, see HbVector and HbString. Clients usually do not have to
  worry about this, as long as it is guaranteed that the shared memory content
  will remain valid.

*/

// class HbEffectFxmlFilterData

HbEffectFxmlFilterData::HbEffectFxmlFilterData(HbMemoryManager::MemoryType memType)
    : mMemoryType(memType),
      mType(memType),
      mBlending(memType),
      mFxmlParams(memType)
{
}

HbEffectFxmlFilterData::~HbEffectFxmlFilterData()
{
}

QString HbEffectFxmlFilterData::type() const
{
    return mType;
}

void HbEffectFxmlFilterData::setType(const QString &value)
{
    mType = value;
}

QString HbEffectFxmlFilterData::blending() const
{
    return mBlending;
}

void HbEffectFxmlFilterData::setBlending(const QString &value)
{
    mBlending = value;
}

void HbEffectFxmlFilterData::appendParamData(const HbEffectFxmlParamData &data)
{
    mFxmlParams.append(data);
}

QList<HbEffectFxmlParamData> HbEffectFxmlFilterData::paramData() const
{
    // Make a regular QList. Note that some of the underlying structures may
    // still stay in shared memory.
    QList<HbEffectFxmlParamData> paramDataList;
    foreach(const HbEffectFxmlParamData & data, mFxmlParams) {
        paramDataList.append(data);
    }
    return paramDataList;
}

HbMemoryManager::MemoryType HbEffectFxmlFilterData::memoryType() const
{
    return mMemoryType;
}

// class HbEffectFxmlParamData

HbEffectFxmlParamData::HbEffectFxmlParamData(HbMemoryManager::MemoryType memType)
    : mMemoryType(memType),
      mName(memType),
      mValue(memType),
      mDuration(memType),
      mStartRef(memType),
      mStartVal(memType),
      mEndRef(memType),
      mEndVal(memType),
      mLoopStart(LoopUndefined),
      mLoopEnd(LoopUndefined),
      mAttributes(memType),
      mKeyFrames(memType)
{
}

HbEffectFxmlParamData::~HbEffectFxmlParamData()
{
}

QString HbEffectFxmlParamData::name() const
{
    return mName;
}

void HbEffectFxmlParamData::setName(const QString &value)
{
    mName = value;
}

QString HbEffectFxmlParamData::duration() const
{
    return mDuration;
}

void HbEffectFxmlParamData::setDuration(const QString &value)
{
    mDuration = value;
}

QString HbEffectFxmlParamData::getAttribute(const QString &attrName) const
{
    foreach(const HbEffectFxmlAttrListEntry & a, mAttributes) {
        if (a.mKey == attrName) {
            return a.mValue;
        }
    }
    return QString();
}

void HbEffectFxmlParamData::setAttribute(const QString &attrName, const QString &value)
{
    for (int i = 0, ie = mAttributes.count(); i != ie; ++i) {
        if (mAttributes.at(i).mKey == attrName) {
            mAttributes[i].mValue = value;
            return;
        }
    }
    HbEffectFxmlAttrListEntry a(mMemoryType);
    a.mKey = attrName;
    a.mValue = value;
    mAttributes.append(a);
}

QString HbEffectFxmlParamData::getValue() const
{
    return mValue;
}

bool HbEffectFxmlParamData::getValue(qreal &value) const
{
    bool ok = false;

    QString strValue = getValue();
    if (!strValue.isEmpty()) {
        qreal temp = strValue.toFloat(&ok);
        if (ok) {
            value = temp;
        }
    }
    return ok;
}

void HbEffectFxmlParamData::setValue(const QString &value)
{
    mValue = value;
}

QList<HbKeyFrame> HbEffectFxmlParamData::keyFrames() const
{
    // Make a regular QList. Note that some of the underlying structures may
    // still stay in shared memory.
    QList<HbKeyFrame> keyFrameList;
    foreach(const HbKeyFrame & keyFrame, mKeyFrames) {
        keyFrameList.append(keyFrame);
    }
    return keyFrameList;
}

void HbEffectFxmlParamData::append(const HbKeyFrame &keyFrame)
{
    mKeyFrames.append(keyFrame);
}

QString HbEffectFxmlParamData::startRef() const
{
    return mStartRef;
}

void HbEffectFxmlParamData::setStartRef(const QString &value)
{
    mStartRef = value;
}

QString HbEffectFxmlParamData::endRef() const
{
    return mEndRef;
}

void HbEffectFxmlParamData::setEndRef(const QString &value)
{
    mEndRef = value;
}

bool HbEffectFxmlParamData::loopDefined() const
{
    return mLoopStart > LoopMinDefined && mLoopEnd > LoopMinDefined;
}

float HbEffectFxmlParamData::loopStart() const
{
    return mLoopStart;
}

void HbEffectFxmlParamData::setLoopStart(float value)
{
    mLoopStart = value;
}

float HbEffectFxmlParamData::loopEnd() const
{
    return mLoopEnd;
}

void HbEffectFxmlParamData::setLoopEnd(float value)
{
    mLoopEnd = value;
}

HbMemoryManager::MemoryType HbEffectFxmlParamData::memoryType() const
{
    return mMemoryType;
}


// -----------------------------------------------------------------------------
// Class HbEffectFxmlData
// -----------------------------------------------------------------------------
//
HbEffectFxmlData::HbEffectFxmlData(HbMemoryManager::MemoryType memType)
    : mMemoryType(memType),
      mFxmlParams(memType),
      mFilters(memType)
{
}

HbEffectFxmlData::~HbEffectFxmlData()
{
}

bool HbEffectFxmlData::isNull() const
{
    return !mFxmlParams.count() && !mFilters.count();
}

void HbEffectFxmlData::appendParamData(const HbEffectFxmlParamData &data)
{
    mFxmlParams.append(data);
}

QList<HbEffectFxmlParamData> HbEffectFxmlData::paramData() const
{
    // Make a regular QList. Note that some of the underlying structures may
    // still stay in shared memory.
    QList<HbEffectFxmlParamData> paramDataList;
    foreach(const HbEffectFxmlParamData & data, mFxmlParams) {
        paramDataList.append(data);
    }
    return paramDataList;
}

void HbEffectFxmlData::appendFilterData(const HbEffectFxmlFilterData &data)
{
    mFilters.append(data);
}

QList<HbEffectFxmlFilterData> HbEffectFxmlData::filterData() const
{
    // Make a regular QList. Note that some of the underlying structures may
    // still stay in shared memory.
    QList<HbEffectFxmlFilterData> paramDataList;
    foreach(const HbEffectFxmlFilterData & data, mFilters) {
        paramDataList.append(data);
    }
    return paramDataList;
}

HbMemoryManager::MemoryType HbEffectFxmlData::memoryType() const
{
    return mMemoryType;
}
HbEffectInfo::HbEffectInfo(): mItem(0)
{

}

bool HbEffectInfo::fromTheme() const
{
    return mFromTheme;
}

HbEffectInfo::~HbEffectInfo()
{
}
QString HbEffectInfo::componentType() const
{
    return mComponentType;
}
QString HbEffectInfo::xmlFileFullPath() const
{
    return mDefFileFullPath;
}
QString HbEffectInfo::effectEvent() const
{
    return mEffectEvent;
}
bool HbEffectInfo::inUse() const
{
    return mInUse;
}
QGraphicsItem *HbEffectInfo::item() const
{
    return mItem;
}
bool HbEffectInfo::shared() const
{
    return mShared;
}


void HbEffectInfo::setEffectData(HbEffectFxmlData fxmlData)
{
    mFxmlData = fxmlData;
}
HbEffectFxmlData HbEffectInfo::effectData() const
{
    return mFxmlData;
}
