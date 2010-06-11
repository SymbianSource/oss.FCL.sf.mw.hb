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
#include <qbytearray.h>
#include <QFile>
#include <QBuffer>
#include <QDataStream>
#include <QCoreApplication>
#include <QSharedMemory>
#include <QVector>
#include <QDir>

#include "hbinputsettingproxy.h"
#include "hbinputsettingproxy_p.h"
#include "hbinputmodecache_p.h"
#include "hbinputfilter.h"

#ifdef Q_OS_SYMBIAN

#define HBI_BASE_PATH QString("\\resource\\plugins")
#define HBI_BASE_WRITABLE_PATH QString("c:\\data\\hbinputs")

#else

#ifndef Q_OS_UNIX
#define HBI_BASE_WRITABLE_PATH QString("c:\\Hb\\lib")
#endif

#endif

/*!
@alpha
@hbcore
\class HbInputSettingProxy
\brief A singleton class providing access to system wide input related settings.

HbInputSettingProxy provides access to all system wide input settings. It is implemented
as process specific singleton, but it stores the settings to a shared memory chunk
so that all the processes in the system share the same set of settings.

Setting proxy stores its state to disk when the last instance in memory is destroyed
and loads it back again when the first instance is created.

It also knows file system paths to several important input related folders in the
system.
*/

/// @cond

HbSettingProxyInputMethodDescriptor::HbSettingProxyInputMethodDescriptor()
{
    pluginNameAndPathSize = 0;
    keySize = 0;
    displayNameSize = 0;
}

HbSettingProxyInputMethodDescriptor::HbSettingProxyInputMethodDescriptor(const HbInputMethodDescriptor &descriptor)
{
    *this = descriptor;
}

void HbSettingProxyInputMethodDescriptor::operator=(const HbInputMethodDescriptor &descriptor)
{
    pluginNameAndPathSize = 0;
    keySize = 0;
    displayNameSize = 0;

    if (!descriptor.pluginNameAndPath().isEmpty() &&
        (descriptor.pluginNameAndPath().size() * sizeof(QChar) < HbActiveMethodNameMax)) {
        pluginNameAndPathSize = descriptor.pluginNameAndPath().size();
        memcpy((void*)pluginNameAndPath, (void*)descriptor.pluginNameAndPath().unicode(), descriptor.pluginNameAndPath().size() * sizeof(QChar));
    }
    if (!descriptor.key().isEmpty() &&
        (descriptor.key().size() * sizeof(QChar) < HbActiveMethodKeyMax)) {
        memcpy((void*)key, (void*)descriptor.key().unicode(), descriptor.key().size() * sizeof(QChar));
        keySize = descriptor.key().size();
    }
    if (!descriptor.displayName().isEmpty() &&
        (descriptor.displayName().size() * sizeof(QChar) < HbActiveMethodKeyMax)) {
        memcpy((void*)displayName, (void*)descriptor.displayName().unicode(), descriptor.displayName().size() * sizeof(QChar));
        displayNameSize = descriptor.displayName().size();
    }
}

HbInputMethodDescriptor HbSettingProxyInputMethodDescriptor::descriptor() const
{
    HbInputMethodDescriptor result;

    if (pluginNameAndPathSize > 0) {
        result.setPluginNameAndPath(QString(pluginNameAndPath, pluginNameAndPathSize));
    }
    if (keySize > 0) {
        result.setKey(QString(key, keySize));
    }
    if (displayNameSize > 0) {
        result.setDisplayName(QString(displayName, displayNameSize));
    }

    return result;
}

QByteArray HbSettingProxyInputMethodDescriptor::data() const
{
    if (customDataSize > 0) {
        return QByteArray(customData, customDataSize);
    }

    return QByteArray();
}

void HbSettingProxyInputMethodDescriptor::setData(const QByteArray &data)
{
    customDataSize = 0;

    if (data.size() > 0 && data.size() <= (int)HbActiveMethodKeyMax * 2) {
        memcpy(customData, data.data(), data.size());
        customDataSize = data.size();
    }
}

// Special character classifier class for bookkeeping
// of how popular a SC is.
class HbScClassifier
{
public:
    HbScClassifier(QChar aChar = 0, int aCount = 0)
        : mChar(aChar), mCount(aCount)
    {
    }

    void operator=(const HbScClassifier& aOther)
    {
        mChar = aOther.mChar;
        mCount = aOther.mCount;
    }

public:
    QChar mChar;
    int mCount;
};

/// @endcond

HbInputSettingProxyPrivate::HbInputSettingProxyPrivate()
{
    iSharedMemory = new QSharedMemory(KInputSettingProxyKey);

    if (!iSharedMemory->attach()) {
        if (iSharedMemory->error() != QSharedMemory::NotFound) {
            qDebug("HbInputSettingProxy: QSharedMemory::attached returned error %d", iSharedMemory->error());
            return;
        }

        if (!iSharedMemory->create(sizeof(HbSettingProxyInternalData))) {
            qDebug("HbInputSettingProxy : Unable to create shared memory block!");
            return;
        }

        initializeDataArea();
    }

    lock();

    HbSettingProxyInternalData* prData = proxyData();
    if (prData) {
        ++prData->iReferences;
    }

    unlock();

    // This is needed because qApp doesn't not exist anymore when singleton destructs.
    iSaveFile = dataFileNameAndPath();
}

HbInputSettingProxyPrivate::~HbInputSettingProxyPrivate()
{
    // NOTE: iSharedMemory is not deleted on purpose. See HbInputSettingProxy::shutdown.
}

void HbInputSettingProxyPrivate::shutdownDataArea()
{
    lock();
    HbSettingProxyInternalData* prData = proxyData();
    if (prData) {
        prData->iReferences--;
        if (prData->iReferences <= 0) {
            save();
        }
    }
    unlock();
}

QString HbInputSettingProxyPrivate::dataFilePath()
{
    return HbInputSettingProxy::writablePath()+QDir::separator()+QString("settings");
}

QString HbInputSettingProxyPrivate::dataFileNameAndPath()
{
    return dataFilePath()+QDir::separator()+QString("proxy.dat");
}

void HbInputSettingProxyPrivate::initializeDataArea()
{
    lock();
    bool wasLoaded = load();

    HbSettingProxyInternalData* prData = proxyData();
    if (prData) {
        prData->iReferences = 0;
        prData->iOrientationChangeCompleted = true;
        // Default values, real ones should be set by calling initializeOrientation()
        prData->iScreenOrientation = Qt::Vertical;

        if (!wasLoaded) {
            // There was no permanent storage version, so initialize to defaults.
            prData->iVersion = HbProxyDataRequiredVersion;
            prData->iGlobalPrimaryInputLanguage = HbInputLanguage(QLocale::English, QLocale::UnitedKingdom);
            prData->iGlobalSecondaryInputLanguage = QLocale::Language(0);
            prData->iActiveKeyboard = HbKeyboardVirtual12Key;
            prData->iTouchKeyboard = HbKeyboardVirtual12Key;
            prData->iHwKeyboard = HbKeyboardQwerty;           
            prData->iPredictiveInputState = (HbKeyboardSettingFlags)HbKeyboardSetting12key|HbKeyboardSettingQwerty;
            prData->iDigitType = HbDigitTypeLatin;
            prData->iQwertyTextCasing = true;
            prData->iQwertyCharacterPreview = true;
            prData->iRegionalCorrectionStatus = true;
            prData->iKeypressTimeout = 1000;
            prData->iAutocompletion = (HbKeyboardSettingFlags)(HbKeyboardSetting12key | HbKeyboardSettingQwerty);
            prData->iTypingCorrectionLevel = HbTypingCorrectionLevelHigh;
            prData->iPrimaryCandidateMode = HbPrimaryCandidateModeBestPrediction;           
            prData->iPreferredMethodHorizontal = HbInputMethodDescriptor();
            prData->iPreferredMethodHorizontal.setData(QByteArray());
            prData->iPreferredMethodVertical = HbInputMethodDescriptor();
            prData->iPreferredMethodVertical.setData(QByteArray());
        }
    }
    unlock();
}

bool HbInputSettingProxyPrivate::load()
{
    QFile file(dataFileNameAndPath());
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray rawData = file.read(sizeof(HbSettingProxyInternalData));
    if (rawData.size() == sizeof(HbSettingProxyInternalData)) {
        HbSettingProxyInternalData* ldData = (HbSettingProxyInternalData*)rawData.constData();
        if (ldData) {
            if (ldData->iVersion == HbProxyDataRequiredVersion) {

                HbSettingProxyInternalData* prData = proxyData();
                memcpy((void*)prData, (void*)ldData, sizeof(HbSettingProxyInternalData));
                prData->iActiveKeyboard = ldData->iActiveKeyboard;

                // Temporarily like this, will be moved as part of shared data later...
                int numItems = 0;
                file.read((char *)&numItems, sizeof(int));
                iTopScs.clear();
                for (int jj = 0; jj < numItems; jj++) {
                    HbScClassifier tmpItem;
                    file.read((char*)&tmpItem, sizeof(HbScClassifier));
                    iTopScs.append(tmpItem);
                }

                file.close();
                return true;
            }
        }
    }

    file.close();
    return false;
}

void HbInputSettingProxyPrivate::save()
{
    // Make sure that the path exists
    QDir settingDir;
    settingDir.mkpath(dataFilePath());

    HbSettingProxyInternalData* prData = proxyData();
    if (prData) {
        QFile file(iSaveFile);
        if (!file.open(QIODevice::WriteOnly)) {
            return;
        }

        file.write((const char*)prData, sizeof(HbSettingProxyInternalData));

        // Temporarily like this, will be moved to shared data later...
        int numItems = iTopScs.count();
        file.write((const char*)&numItems, sizeof(int));
        file.write((const char*)iTopScs.constData(), numItems * sizeof(HbScClassifier));
        file.close();
    }
}

QString HbInputSettingProxyPrivate::stringFromProxyDataElement(QChar *string) const
{
    QString result;
    for (int i = 0; string[i] != 0; i++) {
        result.append(string[i]);
    }

    return QString(result);
}

void HbInputSettingProxyPrivate::stringToProxyDataElement(QChar *string, const QString &source, int maxSize) const
{
    int i = 0;
    for (; i < source.length() && i < maxSize - 1; i++) {
        string[i] = source[i];
    }
    string[i] = 0;
}

HbSettingProxyInternalData* HbInputSettingProxyPrivate::proxyData() const
{
    return static_cast<HbSettingProxyInternalData*>(iSharedMemory->data());
}

void HbInputSettingProxyPrivate::flipToggle()
{
    setFlipStatus(!flipStatus());
}

bool HbInputSettingProxyPrivate::flipStatus()
{
    HbSettingProxyInternalData* prData = proxyData();
    return prData->iFlipStatus;
}

void HbInputSettingProxyPrivate::setFlipStatus(bool flipStatus)
{
    HbSettingProxyInternalData* prData = proxyData();
    prData->iFlipStatus = flipStatus;

    handleDeviceSpecificOriantationAndFlipChange();
}

void HbInputSettingProxyPrivate::handleDeviceSpecificOriantationAndFlipChange()
{
    HbKeyboardType  keyboard = HbKeyboardNone;

    if (HbInputSettingProxy::instance()->screenOrientation() == Qt::Vertical) {
        keyboard = HbKeyboardVirtual12Key;
    } else {
        if(flipStatus()) {
            keyboard = HbKeyboardQwerty;
        } else {
            keyboard = HbKeyboardVirtualQwerty;
        }
    }

    HbInputSettingProxy::instance()->setActiveKeyboard(keyboard);
}

//
// HbInputSettingProxy
//

/*!
Returns pointer to the singleton object.
*/
HbInputSettingProxy* HbInputSettingProxy::instance()
{
    static HbInputSettingProxy theProxy;
    return &theProxy;
}

/*!
Constructs the object.
*/
HbInputSettingProxy::HbInputSettingProxy() : d_ptr(new HbInputSettingProxyPrivate())
{
}

/*!
Destructs the object
*/
HbInputSettingProxy::~HbInputSettingProxy()
{
    delete d_ptr;
}

/*!
Shuts down the object safely. This is needed mainly for singleton object. There has been a lot
of problems related to random singleton destruction order and additional shutdown step is
needed to guarantee that it will be done safely. The slot is connected to
QCoreApplication::aboutToQuit when the framework is initialized.
*/
void HbInputSettingProxy::shutdown()
{
    Q_D(HbInputSettingProxy);

    d->shutdownDataArea();
    delete d->iSharedMemory;
    d->iSharedMemory = 0;
}

/*!
Toggles prediction mode
*/
void HbInputSettingProxy::togglePrediction()
{
    if (activeKeyboard() & HbQwertyKeyboardMask) {
        setPredictiveInputStatus(HbKeyboardSettingQwerty, !predictiveInputStatus(HbKeyboardSettingQwerty));
    } else {
        setPredictiveInputStatus(HbKeyboardSetting12key, !predictiveInputStatus(HbKeyboardSetting12key));
    }
}

/*!
Setting proxy emits a signal when any of the monitored settings changes. This
method connects those signals to given object.

\sa disconnectObservingObject
\sa globalInputLanguageChanged
\sa predictiveInputStateChanged
\sa orientationAboutToChange
\sa orientationChanged
\sa characterPreviewStateForQwertyChanged
\sa keypressTimeoutChanged
\sa autocompletionStateChanged
\sa typingCorrectionLevelChanged
\sa primaryCandidateModeChanged
*/
void HbInputSettingProxy::connectObservingObject(QObject* aObserver)
{
    if (aObserver) {
        connect(this, SIGNAL(globalInputLanguageChanged(const HbInputLanguage &)), aObserver, SLOT(globalInputLanguageChanged(const HbInputLanguage &)));
        connect(this, SIGNAL(globalSecondaryInputLanguageChanged(const HbInputLanguage &)), aObserver, SLOT(globalSecondaryInputLanguageChanged(const HbInputLanguage &)));
        connect(this, SIGNAL(activeKeyboardChanged(HbKeyboardType)), aObserver, SLOT(activeKeyboardChanged(HbKeyboardType)));
        connect(this, SIGNAL(orientationAboutToChange()), aObserver, SLOT(orientationAboutToChange()));
        connect(this, SIGNAL(orientationChanged(Qt::Orientation)), aObserver, SLOT(orientationChanged(Qt::Orientation)));
    }
}

/*!
Disconnects given object from the setting proxy.

\sa connectObservingObject
*/
void HbInputSettingProxy::disconnectObservingObject(QObject* aObserver)
{
    if (aObserver) {
        disconnect(this, SIGNAL(globalInputLanguageChanged(const HbInputLanguage &)), aObserver, SLOT(globalInputLanguageChanged(const HbInputLanguage &)));
        disconnect(this, SIGNAL(globalSecondaryInputLanguageChanged(const HbInputLanguage &)), aObserver, SLOT(globalSecondaryInputLanguageChanged(const HbInputLanguage &)));
        disconnect(this, SIGNAL(activeKeyboardChanged(HbKeyboardType)), aObserver, SLOT(activeKeyboardChanged(HbKeyboardType)));
        disconnect(this, SIGNAL(orientationAboutToChange()), aObserver, SLOT(orientationAboutToChange()));
        disconnect(this, SIGNAL(orientationChanged(Qt::Orientation)), aObserver, SLOT(orientationChanged(Qt::Orientation)));
    }
}

/*!
Returns active input language. This is system wide value, an editor and input state machine may override this by defining
local input language. Use HbInputMethod::ActiveLanguage for input state related situation and
this method for system wide setting.

\sa setGlobalInputLanguage
*/
HbInputLanguage HbInputSettingProxy::globalInputLanguage() const
{
    Q_D(const HbInputSettingProxy);
    HbInputLanguage res;

    d->lock();
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        res = prData->iGlobalPrimaryInputLanguage;
    }
    d->unlock();

    return HbInputLanguage(res);
}

/*!
Returns active secondary input language. Secondary input language is often used by the prediction engines for predicting
candidates in both the languages.

\sa setGlobalSecondaryInputLanguage
*/
HbInputLanguage HbInputSettingProxy::globalSecondaryInputLanguage() const
{
    Q_D(const HbInputSettingProxy);
    HbInputLanguage res;

    d->lock();
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        res = prData->iGlobalSecondaryInputLanguage;
    }
    d->unlock();

    return HbInputLanguage(res);
}

/*!
Returns available hardware keyboard in the device.
*/
void HbInputSettingProxy::availableHwKeyboard(QList<HbKeyboardType>& aListOfAvailableKeyboards) const
{
    aListOfAvailableKeyboards.append(HbKeyboard12Key);
    aListOfAvailableKeyboards.append(HbKeyboardQwerty);
}

/*!
Returns active hardware keyboard type.

\sa setActiveHwKeyboard
\sa activeTouchKeyboard
*/
HbKeyboardType HbInputSettingProxy::activeHwKeyboard() const
{
    Q_D(const HbInputSettingProxy);
    HbKeyboardType res = HbKeyboardNone;

    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        res = prData->iHwKeyboard;
    }

    return res;
}

/*!
Returns active touch keyboard type.

\sa setActiveTouchKeyboard
\sa activeHwKeyboard
*/
HbKeyboardType HbInputSettingProxy::activeTouchKeyboard() const
{
    Q_D(const HbInputSettingProxy);
    HbKeyboardType res = HbKeyboardNone;

    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        res = prData->iTouchKeyboard;
    }

    return res;
}

/*!
Returns active keyboard type.

\sa setActiveKeyboard
*/
HbKeyboardType HbInputSettingProxy::activeKeyboard() const
{
    Q_D(const HbInputSettingProxy);
    HbKeyboardType res = HbKeyboardNone;

    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        res = prData->iActiveKeyboard;
    }

    return res;
}

/*!
Returns the preferred input method for given screen orientation. Initially this value is empty
and the framework will resolve the default handler.

\sa setPreferredInputMethod
*/
HbInputMethodDescriptor HbInputSettingProxy::preferredInputMethod(Qt::Orientation orientation) const
{
    Q_D(const HbInputSettingProxy);

    HbInputMethodDescriptor result;

    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        d->lock();
        if (orientation == Qt::Horizontal) {
            result = prData->iPreferredMethodHorizontal.descriptor();
        } else {
            result = prData->iPreferredMethodVertical.descriptor();
        }
        d->unlock();
    }

    return result;
}

/*!
Returns the preferred input method for current screen orientation. Initially this value is empty
and the framework will resolve the default handler.

\sa setPreferredInputMethod
*/
HbInputMethodDescriptor HbInputSettingProxy::preferredInputMethod() const
{
    Q_D(const HbInputSettingProxy);

    HbInputMethodDescriptor result;

    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        d->lock();
        if (prData->iScreenOrientation == Qt::Horizontal) {
            result = prData->iPreferredMethodHorizontal.descriptor();
        } else {
            result = prData->iPreferredMethodVertical.descriptor();
        }
        d->unlock();
    }

    return result;
}

/*!
Returns custom data associated to preferred input method.

\sa setPreferredInputMethod
*/
QByteArray HbInputSettingProxy::preferredInputMethodCustomData(Qt::Orientation orientation) const
{
    Q_D(const HbInputSettingProxy);

    QByteArray result;

    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        d->lock();
        if (orientation == Qt::Horizontal) {
            result = prData->iPreferredMethodHorizontal.data();
        } else {
            result = prData->iPreferredMethodVertical.data();
        }
        d->unlock();
    }

    return result;
}

/*!
Sets preferred input method for given screen orientation. The parameter \a customdata may contain
any information the preferred input method needs to remember as part of settings data.
Note that only 128 bytes is reserved for custom data. Larger amount of it needs to be
handled by other means.
This method is for input method developers only. There should never be need to call it from application code.

\sa preferredInputMethod
*/
void HbInputSettingProxy::setPreferredInputMethod(Qt::Orientation orientation, const HbInputMethodDescriptor &inputMethod, const QByteArray &customData)
{
    Q_D(HbInputSettingProxy);
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        d->lock();
        if (orientation == Qt::Horizontal) {
            prData->iPreferredMethodHorizontal = inputMethod;
            prData->iPreferredMethodHorizontal.setData(customData);
        } else {
            prData->iPreferredMethodVertical = inputMethod;
            prData->iPreferredMethodVertical.setData(customData);
        }
        d->unlock();
    }
}

/*!
Sets system wide input language. Will emit signal globalInputLanguageChanged if language is changed.

\sa globalInputLanguage
*/
void HbInputSettingProxy::setGlobalInputLanguage(const HbInputLanguage& language)
{
    Q_D(HbInputSettingProxy);
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->iGlobalPrimaryInputLanguage != language) {
            prData->iGlobalPrimaryInputLanguage = language;
            notify = true;
        }
        d->unlock();
        if (notify) {
            emit globalInputLanguageChanged(language);
        }
    }
}

/*!
Sets system wide secondary input language. Will emit signal globalSecondaryInputLanguageChanged if language is changed.

\sa globalSecondaryInputLanguage
*/
void HbInputSettingProxy::setGlobalSecondaryInputLanguage(const HbInputLanguage &language)
{
    Q_D(HbInputSettingProxy);
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->iGlobalSecondaryInputLanguage != language) {
            prData->iGlobalSecondaryInputLanguage = language;
            notify = true;
        }
        d->unlock();
        if (notify) {
            emit globalSecondaryInputLanguageChanged(language);
        }
    }
}

/*!
Sets active hardware keyboard type. Will emit signal activeHwKeyboardChanged if keyboard is changed.

\sa activeHwKeyboard
\sa activeTouchKeyboard
\sa setActiveTouchKeyboard
\sa setActiveHwKeyboard
*/
void HbInputSettingProxy::setActiveHwKeyboard(HbKeyboardType keyboard)
{
    Q_D(HbInputSettingProxy);
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->iHwKeyboard != keyboard) {
            prData->iHwKeyboard = keyboard;
            notify = true;
        }
        d->unlock();
        if (notify) {
            emit activeHwKeyboardChanged(keyboard);
        }
    }
}

/*!
Sets active touch keyboard type. Will emit signal activeTouchKeyboardChanged keyboard is changed.

\sa activeTouchKeyboard
\sa activeHwKeyboard
\sa setActiveTouchKeyboard
\sa setActiveHwKeyboard
*/
void HbInputSettingProxy::setActiveTouchKeyboard(HbKeyboardType keyboard)
{
    Q_D(HbInputSettingProxy);
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->iTouchKeyboard != keyboard) {
            prData->iTouchKeyboard = keyboard;
            notify = true;
        }
        d->unlock();
        if (notify) {
            emit activeTouchKeyboardChanged(keyboard);
        }
    }
}

/*!
Sets active keyboard type. Will emit signal activeKeyboardChanged if keyboard is changed.

\sa activeKeyboard
\sa activeHwKeyboard
\sa setactiveKeyboard
\sa setActiveHwKeyboard
*/
void HbInputSettingProxy::setActiveKeyboard(HbKeyboardType keyboard)
{
    Q_D(HbInputSettingProxy);
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->iActiveKeyboard != keyboard) {
            prData->iActiveKeyboard = keyboard;
            notify = true;
        }
        d->unlock();
        if (notify) {
            emit activeKeyboardChanged(keyboard);
        }
    }
}

/*!
Returns the status of predictive input feature. Returns true if any one of given
keyboard types has the prediction enabled. An editor instance may still forbid
predictive input feature, even if the device wide status allows it.

\sa setPredictiveInputStatus.
*/
bool HbInputSettingProxy::predictiveInputStatus(HbKeyboardSettingFlags keyboardType) const
{
    Q_D(const HbInputSettingProxy);
    bool res = false;

    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        res = prData->iPredictiveInputState & keyboardType;
    }

    return res;
}

/*!
Sets the status of predictive text input feature. Will emit signal predictiveInputStateChanged if status is changed.

\sa predictiveInputStatus
*/
void HbInputSettingProxy::setPredictiveInputStatus(HbKeyboardSettingFlags keyboardType, bool newStatus)
{
    Q_D(HbInputSettingProxy);
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        
        HbKeyboardSettingFlags newValue = prData->iPredictiveInputState;
        if (newStatus) {
            newValue |= keyboardType;
        } else {
            newValue &= ~keyboardType;
        }
        if (prData->iPredictiveInputState != newValue) {
            prData->iPredictiveInputState = newValue;
            notify = true;
        }
        d->unlock();
        if (notify) {
            emit predictiveInputStateChanged(keyboardType, newStatus);
        }
    }
}

/*!
Returns the status of predictive input feature for active keyboard. An editor instance
may still forbid predictive input feature, even if the device wide status allows it.

\sa setPredictiveInputStatusForActiveKeyboard.
*/
bool HbInputSettingProxy::predictiveInputStatusForActiveKeyboard() const
{
    Q_D(const HbInputSettingProxy);
    bool res = false;

    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        if (activeKeyboard() & HbQwertyKeyboardMask) {
            res = prData->iPredictiveInputState & HbKeyboardSettingQwerty;
        } else {
            res = prData->iPredictiveInputState & HbKeyboardSetting12key;
        }
    }

    return res;
}

/*!
Sets the status of predictive text input feature for active keyboard.

\sa predictiveInputStatusForActiveKeyboard
*/
void HbInputSettingProxy::setPredictiveInputStatusForActiveKeyboard(bool newStatus)
{
    if (activeKeyboard() & HbQwertyKeyboardMask) {
        setPredictiveInputStatus(HbKeyboardSettingQwerty, newStatus);
    } else {
        setPredictiveInputStatus(HbKeyboardSetting12key, newStatus);
    }
}

/*!
Returns path to a writable location that should be used as a base storage folder for
dynamic input data.
*/
QString HbInputSettingProxy::writablePath()
{
#ifdef Q_OS_SYMBIAN
    return HBI_BASE_WRITABLE_PATH ;
#else
    if (QString(HB_BUILD_DIR) == QString(HB_INSTALL_DIR)) {
        // This is local build so also use local writable path.
        return QString(HB_INSTALL_DIR) + QDir::separator() + QString(".hbinputs");
    } else {
#ifdef Q_OS_UNIX
    return QDir::homePath() + QDir::separator() + QString(".hbinputs");
#else
    return HBI_BASE_WRITABLE_PATH ;
#endif
    }
#endif
}

/*!
Returns path to input method plugin folder.
*/
QStringList HbInputSettingProxy::inputMethodPluginPaths()
{
    QStringList result;

#ifdef Q_OS_SYMBIAN
    result.append(QString("z:") + HBI_BASE_PATH + QDir::separator() + QString("inputmethods"));
    result.append(QString("c:") + HBI_BASE_PATH + QDir::separator() + QString("inputmethods"));
    result.append(QString("f:") + HBI_BASE_PATH + QDir::separator() + QString("inputmethods"));
    // Hard coded paths at the moment, we will really do this with QDir::drives() later...
#else
    result.append(HB_PLUGINS_DIR + (QDir::separator() + QString("inputmethods")));
#endif

    return QStringList(result);
}

/*!
Returns list of paths to all possible keymap plugin locations.
*/
QStringList HbInputSettingProxy::keymapPluginPaths()
{
    QStringList result;
    QFileInfoList list = QDir::drives();

#ifdef Q_OS_SYMBIAN
    for(int counter = 0; counter < list.count(); counter ++) {
        result.append(list.at(counter).absoluteFilePath() + QString("/resource/keymaps"));
    }
#else
    result.append(HB_RESOURCES_DIR + (QDir::separator() + QString("keymaps")));
    for(int counter = 0; counter < list.count(); counter ++) {
        result.append(list.at(counter).absoluteFilePath() + QString("resource/keymaps"));
    }
#endif
    result.sort();
    //Append the default resource at the end
    result.append(":/keymaps");
    return QStringList(result);
}

/*!
Returns path to language database folder.
*/
QString HbInputSettingProxy::languageDatabasePath()
{
#ifdef Q_OS_SYMBIAN
#ifdef __WINSCW__
    return (QString("c:") + HBI_BASE_PATH + QDir::separator() + QString("langdata"));
#else
    return (QString("z:") + HBI_BASE_PATH + QDir::separator() + QString("langdata"));
#endif
    // We'll need to do this for other drives too...
#else
    return HB_PLUGINS_DIR + (QDir::separator() + QString("langdata"));
#endif
}

/*!
Returns path to dictionary plugin folder.
*/
QString HbInputSettingProxy::dictionaryPath()
{
    return writablePath() + QDir::separator() + QString("dictionary");
}

/*!
Returns list of paths where prediction engine plugins will be searched.
*/
QStringList HbInputSettingProxy::predictionEnginePaths()
{
    QStringList result;

#ifdef Q_OS_SYMBIAN
    result.append(QString("z:") + HBI_BASE_PATH + QDir::separator() + QString("inputengines"));
    result.append(QString("c:") + HBI_BASE_PATH + QDir::separator() + QString("inputengines"));
    // Add memory card handling here later...
#else
    result.append(HB_PLUGINS_DIR + (QDir::separator() + QString("inputengines")));
#endif

    return QStringList(result);
}

/*!
Returns path to extra user dictionary folder.

\sa HbExtraUserDictionary
*/
QString HbInputSettingProxy::extraDictionaryPath()
{
    return writablePath() + QDir::separator() + QString("eud");
}

/*!
Returns system wide digit type setting.

\sa setGlobalDigitType
*/
HbInputDigitType HbInputSettingProxy::globalDigitType() const
{
    Q_D(const HbInputSettingProxy);
    HbInputDigitType res = HbDigitTypeLatin;

    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        res = prData->iDigitType;
    }

    return res;
}

/*!
Sets system wide digit type setting.

\sa globalDigitType
*/
void HbInputSettingProxy::setGlobalDigitType(HbInputDigitType digitType)
{
    Q_D(HbInputSettingProxy);
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        d->lock();
        if (prData->iDigitType != digitType) {
            prData->iDigitType = digitType;
        }
        d->unlock();
    }
}

/*!
Returns true if automatic text casing should be used with qwerty keyboards.

\sa setAutomaticTextCasingForQwerty
*/
bool HbInputSettingProxy::automaticTextCasingForQwerty()
{
    Q_D(HbInputSettingProxy);
    bool res = false;

    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        res = prData->iQwertyTextCasing;
    }

    return res;
}

/*!
Sets automatic text casing for qwerty keyboards. Will emit signal automaticTextCasingStateForQwertyChanged if status is changed.

\sa automaticTextCasingForQwerty
*/
void HbInputSettingProxy::setAutomaticTextCasingForQwerty(bool status)
{
    Q_D(HbInputSettingProxy);
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->iQwertyTextCasing != status) {
            prData->iQwertyTextCasing = status;
            notify = true;
        }
        d->unlock();
        if (notify) {
            emit automaticTextCasingStateForQwertyChanged(status);
        }
    }
    
}

/*!
Enables/Disables character preview in Qwerty keypad. Will emit signal characterPreviewStateForQwertyChanged if status is changed.

\sa characterPreviewForQwerty
*/
void HbInputSettingProxy::setCharacterPreviewForQwerty(bool previewEnabled)
{
    Q_D(HbInputSettingProxy);
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->iQwertyCharacterPreview != previewEnabled) {
            prData->iQwertyCharacterPreview = previewEnabled;
            notify = true;
        }
        d->unlock();
        if (notify) {
            emit characterPreviewStateForQwertyChanged(previewEnabled);
        }
    }
    
}

/*!
Returns true if the character preview is enabled in Qwerty keypad.

\sa setCharacterPreviewForQwerty
*/
bool HbInputSettingProxy::isCharacterPreviewForQwertyEnabled()
{
    Q_D(HbInputSettingProxy);

    bool res = false;

    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        res = prData->iQwertyCharacterPreview;
    }

    return res;
}

/*!
Returns active custom input method. The pluginNameAndPath field is empty if no custom input methid is active.

\sa setActiveCustomInputMethod
*/
HbInputMethodDescriptor HbInputSettingProxy::activeCustomInputMethod() const
{
    return HbInputMethodDescriptor();
}

/*!
\sa activeCustomInputMethod
*/
void HbInputSettingProxy::setActiveCustomInputMethod(const HbInputMethodDescriptor &inputMethod)
{
    Q_UNUSED(inputMethod)
}

/*!
Returns the current screen orientation in settings 
*/
Qt::Orientation HbInputSettingProxy::screenOrientation()
{
    Q_D(HbInputSettingProxy);

    Qt::Orientation orientation = Qt::Vertical;
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        orientation = prData->iScreenOrientation;
    }
    return orientation;
}

/*!
Sets the current screen orientation in settings. This completes orientation change
started with notifyScreenOrientationChange.
*/
void HbInputSettingProxy::setScreenOrientation(Qt::Orientation screenOrientation)
{
    Q_D(HbInputSettingProxy);

    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        bool notify = false;

        d->lock();
        if (screenOrientation != prData->iScreenOrientation) {
            prData->iScreenOrientation = screenOrientation;
            notify = true;
        }
        d->unlock();

        if (notify) {  
            // notify everyone that the orientation has changed.
            d->handleDeviceSpecificOriantationAndFlipChange();
            emit orientationChanged(screenOrientation);
            // set orientation change operation completed.
            d->lock();
            prData->iOrientationChangeCompleted = true;
            d->unlock();
        }
    }
}

/*!
Starts screen orientation change sequence. Emits orientationAboutToChange signal
and set internal orientation change flag to true. Whoever calls this
method, must also complete the orientation change sequence by calling setScreenOrientation.
Generally this mechanims is connected to operating system level screen orientation attribute
begind the scenes and there is no need to call this directly from application or input
method.
*/
void HbInputSettingProxy::notifyScreenOrientationChange()
{
    Q_D(HbInputSettingProxy);

    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        d->lock();
        prData->iOrientationChangeCompleted = false;
        d->unlock();
    }
    emit orientationAboutToChange();
}

/*!
Returns true if the orientation change is completed
*/
bool HbInputSettingProxy::orientationChangeCompleted() const
{
    Q_D(const HbInputSettingProxy);

    bool completed = true;
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        completed = prData->iOrientationChangeCompleted;
    }
    return completed;
}

/*!
Method for initializing orientation state of the input framework. Needed only on
framework level, should not be called by applications.
*/
void HbInputSettingProxy::initializeOrientation(Qt::Orientation screenOrientation)
{
    Q_D(HbInputSettingProxy);

    // call handleDeviceSpecificOriantationAndFlipChange method
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        d->lock();
        prData->iScreenOrientation = screenOrientation;
        if (screenOrientation == Qt::Vertical) {
            prData->iActiveKeyboard = HbKeyboardVirtual12Key;
        } else {
            prData->iActiveKeyboard = HbKeyboardVirtualQwerty;
        }
        d->unlock();
    }
}

/*!
Returns the status of regional input correction feature. 

\sa enableRegionalCorrection.
*/
bool HbInputSettingProxy::regionalCorrectionEnabled()
{
    Q_D(const HbInputSettingProxy);
    bool res = false;
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        res = prData->iRegionalCorrectionStatus;
    }
    return res;
}

/*!
Sets the status of regional input correction feature. Will emit signal regionalCorretionStatusChanged if status is changed.

\sa regionalCorrectionEnabled.
*/
void HbInputSettingProxy::enableRegionalCorrection(bool newStatus)
{
    Q_D(HbInputSettingProxy);
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->iRegionalCorrectionStatus != newStatus) {
            prData->iRegionalCorrectionStatus = newStatus;
            notify = true;
        }
        d->unlock();
        if (notify) {
            emit regionalCorretionStatusChanged(newStatus);
        }
    }
}

/*!
Sets the keypress timeout value. Will emit signal keypressTimeoutChanged if timeout is changed.

\sa keypressTimeout.
*/
void HbInputSettingProxy::setKeypressTimeout(int timeout)
{
    Q_D(HbInputSettingProxy);
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->iKeypressTimeout != timeout) {
            prData->iKeypressTimeout = timeout;
            notify = true;
        }
        d->unlock();
        if (notify) {
            emit keypressTimeoutChanged(timeout);
        }
    }
}

/*!
Returns the keypress timeout value. 

\sa setKeypressTimeout.
*/
int HbInputSettingProxy::keypressTimeout() const
{
    Q_D(const HbInputSettingProxy);
    int res = 0;
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        res = prData->iKeypressTimeout;
    }
    return res;
}

/*!
Sets the autocompletion status. Will emit signal autocompletionStateChanged if status is changed.

\sa isAutocompletionEnabled.
*/
void HbInputSettingProxy::setAutocompletionStatus(HbKeyboardSettingFlags keyboardType, bool state)
{
    Q_D(HbInputSettingProxy);
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        HbKeyboardSettingFlags newValue = prData->iAutocompletion;
        if (state) {
            newValue |= keyboardType;
        } else {
            newValue &= ~keyboardType;
        }
        if (prData->iAutocompletion != newValue) {
            prData->iAutocompletion = newValue;
            notify = true;
        }
        d->unlock();
        if (notify) {
            emit autocompletionStateChanged(keyboardType, state);
        }
    }
}

/*!
Returns the autocompletion status for ITUT. Returns true if any of given
keyboards have autocompletion enabled.

\sa setAutocompletionStatus.
*/
bool HbInputSettingProxy::isAutocompletionEnabled(HbKeyboardSettingFlags keyboardType) const
{
    Q_D(const HbInputSettingProxy);
    bool res = false;
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        res = prData->iAutocompletion & keyboardType;
    }
    return res;
}

/*!
Sets the typing correction level. Will emit signal typingCorrectionLevelChanged if level is changed.

\sa typingCorrectionLevel.
*/
void HbInputSettingProxy::setTypingCorrectionLevel(HbTypingCorrectionLevel level)
{
    Q_D(HbInputSettingProxy);
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->iTypingCorrectionLevel != level) {
            prData->iTypingCorrectionLevel = level;
            notify = true;
        }
        d->unlock();
        if (notify) {
            emit typingCorrectionLevelChanged(level);
        }
        enableRegionalCorrection(level == HbTypingCorrectionLevelHigh);
    }
}

/*!
Returns the typing correction level

\sa setTypingCorrectionLevel.
*/
HbTypingCorrectionLevel HbInputSettingProxy::typingCorrectionLevel() const
{
    Q_D(const HbInputSettingProxy);
    HbTypingCorrectionLevel res = HbTypingCorrectionLevelHigh;
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        res = prData->iTypingCorrectionLevel;
    }
    return res;
}

/*!
Sets the primary candidate mode. Will emit signal primaryCandidateModeChanged if mode is changed.

\sa primaryCandidateMode.
*/
void HbInputSettingProxy::setPrimaryCandidateMode(HbPrimaryCandidateMode mode)
{
    Q_D(HbInputSettingProxy);
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->iPrimaryCandidateMode != mode) {
            prData->iPrimaryCandidateMode = mode;
            notify = true;
        }
        d->unlock();
        if (notify) {
            emit primaryCandidateModeChanged(mode);
        }
    }
}

/*!
Returns the primary candidate mode

\sa setPrimaryCandidateMode.
*/
HbPrimaryCandidateMode HbInputSettingProxy::primaryCandidateMode() const
{
    Q_D(const HbInputSettingProxy);
    HbPrimaryCandidateMode res = HbPrimaryCandidateModeExactTyping;
    HbSettingProxyInternalData* prData = d->proxyData();
    if (prData) {
        res = prData->iPrimaryCandidateMode;
    }
    return res;
}

// End of file
