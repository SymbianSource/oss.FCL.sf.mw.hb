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
#ifndef HB_INPUT_SETTING_PROXY_PRIVATE_H
#define HB_INPUT_SETTING_PROXY_PRIVATE_H

#include <QSharedMemory>
#include <QString>
#include <QVector>

#include "hbinputmethoddescriptor.h"
#include "hbinputlanguage.h"

const int HbProxyDataRequiredVersion = 25;
const char HbInputSettingsSharedMemoryKey[] = "HbInputSettingProxy";
const unsigned int HbActiveMethodNameMax = 255;
const unsigned int HbActiveMethodKeyMax = 64;

class HbInputSettingProxy;

class HB_CORE_PRIVATE_EXPORT HbSettingProxyInputMethodDescriptor
{
public:
    HbSettingProxyInputMethodDescriptor();
    HbSettingProxyInputMethodDescriptor(const HbInputMethodDescriptor &descriptor);
    void operator=(const HbInputMethodDescriptor &descriptor);
    HbInputMethodDescriptor descriptor() const;
    QByteArray data() const;
    void setData(const QByteArray &data);

public:
    unsigned int pluginNameAndPathSize;
    QChar pluginNameAndPath[HbActiveMethodNameMax];
    unsigned int keySize;
    QChar key[HbActiveMethodKeyMax];
    unsigned int displayNameSize;
    QChar displayName[HbActiveMethodKeyMax];
    unsigned int customDataSize;
    char customData[HbActiveMethodKeyMax * 2];
};

// REMEMBER to increase HbProxyDataRequiredVersion every time you add fields
// to this class or change related constants!
struct HbSettingProxyInternalData {
    int version;
    HbInputLanguage globalPrimaryInputLanguage;
    HbInputLanguage globalSecondaryInputLanguage;
    HbKeyboardSettingFlags predictiveInputState;
    HbInputDigitType digitType;
    bool qwertyTextCasing;
    bool qwertyCharacterPreview;
    bool flipStatus;
    bool regionalCorrectionStatus;
    int keypressTimeout;
    HbKeyboardSettingFlags autocompletion;
    HbTypingCorrectionLevel typingCorrectionLevel;
    HbPrimaryCandidateMode primaryCandidateMode;
    HbSettingProxyInputMethodDescriptor preferredMethodHorizontal;
    HbSettingProxyInputMethodDescriptor preferredMethodVertical;
    HbHwrWritingSpeed hwrSpeed;
    HbCangjieDetailMode cangjieMode;
    bool useWesternDefaultKeypadForChinese;
};

class HB_CORE_PRIVATE_EXPORT HbInputSettingProxyPrivate
{
    Q_DECLARE_PUBLIC(HbInputSettingProxy)

public:
    HbInputSettingProxyPrivate();
    ~HbInputSettingProxyPrivate();
    static QString dataFileNameAndPath();
    static QString dataFilePath();
    void initializeDataArea();
    static void writeDefaultValuesToData(HbSettingProxyInternalData* data);
    static bool load(HbSettingProxyInternalData *data);
    static void save(HbSettingProxyInternalData *data);
    void shutdownDataArea();
    HbSettingProxyInternalData *proxyData() const;

    void lock() const {
        mSharedMemory->lock();
    }

    void unlock() const {
        mSharedMemory->unlock();
    }

    QString stringFromProxyDataElement(QChar *string) const;
    void stringToProxyDataElement(QChar *string, const QString &source, int maxSize) const;

public:
    HbInputSettingProxy *q_ptr;
    QSharedMemory *mSharedMemory;
};

#endif // HB_INPUT_SETTING_PROXY_P_H

// End of file
