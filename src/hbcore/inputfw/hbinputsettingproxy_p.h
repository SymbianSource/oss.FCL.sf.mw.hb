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

#include "hbinputmethoddescriptor.h"
#include "hbinputlanguage.h"

const int HbProxyDataRequiredVersion = 18;
const QString KInputSettingProxyKey("HbInputSettingProxy");
const unsigned int HbActiveMethodNameMax = 255;
const unsigned int HbActiveMethodKeyMax = 64;

class HbScClassifier;
class HbInputSettingProxy;

class HbSettingProxyInputMethodDescriptor
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
struct HbSettingProxyInternalData
{
    int iVersion;
    int iReferences;
    HbInputLanguage iGlobalPrimaryInputLanguage;
    HbInputLanguage iGlobalSecondaryInputLanguage;
    HbKeyboardType iActiveKeyboard;
    HbKeyboardType iHwKeyboard;
    HbKeyboardType iTouchKeyboard;
    HbKeyboardSettingFlags iPredictiveInputState;
    HbInputDigitType iDigitType;
    bool iQwertyTextCasing;
    bool iQwertyCharacterPreview; 
    Qt::Orientation iScreenOrientation;
    bool iOrientationChangeCompleted;
    bool iFlipStatus;
    bool iRegionalCorrectionStatus;
    int iKeypressTimeout;
    HbKeyboardSettingFlags iAutocompletion;
    HbTypingCorrectionLevel iTypingCorrectionLevel;
    HbPrimaryCandidateMode iPrimaryCandidateMode;
    HbSettingProxyInputMethodDescriptor iPreferredMethodHorizontal;
    HbSettingProxyInputMethodDescriptor iPreferredMethodVertical;
};

class HB_CORE_PRIVATE_EXPORT HbInputSettingProxyPrivate
{
    Q_DECLARE_PUBLIC(HbInputSettingProxy)

public:
    HbInputSettingProxyPrivate();
    ~HbInputSettingProxyPrivate();
    QString dataFileNameAndPath();
    QString dataFilePath();
    void initializeDataArea();
    bool load();
    void save();
    void shutdownDataArea();
    HbSettingProxyInternalData* proxyData() const;

    void flipToggle();
    bool flipStatus();
    void setFlipStatus(bool flipStatus);
    
    void handleDeviceSpecificOriantationAndFlipChange();

    void lock() const {
        iSharedMemory->lock();
    }

    void unlock() const {
        iSharedMemory->unlock();
    }

    QString stringFromProxyDataElement(QChar *string) const;
    void stringToProxyDataElement(QChar *string, const QString &source, int maxSize) const;

public:
    HbInputSettingProxy *q_ptr;
    QSharedMemory *iSharedMemory;
    QString iSaveFile;
    // iTopScs saves the most used (currently, may change to latest
    // used later) special characters on top of the special character
    // table
    QVector<HbScClassifier> iTopScs;  // Move to shared memory later....
};

#endif // HB_INPUT_SETTING_PROXY_P_H

// End of file
