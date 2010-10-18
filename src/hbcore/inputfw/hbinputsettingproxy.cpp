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
#include "hbinputsettingproxy.h"
#include "hbinputsettingproxy_p.h"

#include <qbytearray.h>
#include <QFile>
#include <QBuffer>
#include <QDataStream>
#include <QCoreApplication>
#include <QSharedMemory>
#include <QVector>
#include <QDir>

#include "hbinputmodecache_p.h"
#include "hbinputmethod.h"
#include "hbinputfilter.h"

#ifdef Q_OS_SYMBIAN

#define HBI_BASE_PATH QString("\\resource\\plugins")
#define HBI_BASE_WRITABLE_PATH QString("c:\\data\\hbinputs")

#else

#ifndef Q_OS_UNIX
#define HBI_BASE_WRITABLE_PATH QString("c:\\Hb\\lib")
#endif // Q_OS_UNIX

#endif //Q_OS_SYMBIAN

/*!
@stable
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
        memcpy((void *)pluginNameAndPath, (void *)descriptor.pluginNameAndPath().unicode(), descriptor.pluginNameAndPath().size() * sizeof(QChar));
    }
    if (!descriptor.key().isEmpty() &&
        (descriptor.key().size() * sizeof(QChar) < HbActiveMethodKeyMax)) {
        memcpy((void *)key, (void *)descriptor.key().unicode(), descriptor.key().size() * sizeof(QChar));
        keySize = descriptor.key().size();
    }
    if (!descriptor.displayName().isEmpty() &&
        (descriptor.displayName().size() * sizeof(QChar) < HbActiveMethodKeyMax)) {
        memcpy((void *)displayName, (void *)descriptor.displayName().unicode(), descriptor.displayName().size() * sizeof(QChar));
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

HbInputSettingProxyPrivate::HbInputSettingProxyPrivate()
{
    mSharedMemory = new QSharedMemory(HbInputSettingsSharedMemoryKey);

    if (!mSharedMemory->attach()) {
        if (mSharedMemory->error() != QSharedMemory::NotFound) {
            qDebug("HbInputSettingProxy: QSharedMemory::attach returned error %d", mSharedMemory->error());
            return;
        }

        if (!mSharedMemory->create(sizeof(HbSettingProxyInternalData))) {
            qDebug("HbInputSettingProxy : Unable to create shared memory block!");
            return;
        }

        initializeDataArea();
    }
#ifdef Q_OS_UNIX
#ifndef Q_OS_SYMBIAN
    else if (proxyData()->version != HbProxyDataRequiredVersion) {
        // In unix systems, the shared memory may be left dangling with an outdated version
        // In that case, update all the values with defaults to make sure 
        initializeDataArea();
    }
#endif // Q_OS_SYMBIAN
#endif // Q_OS_UNIX
}

HbInputSettingProxyPrivate::~HbInputSettingProxyPrivate()
{
    // NOTE: mSharedMemory is not deleted on purpose. See HbInputSettingProxy::shutdown.
}

void HbInputSettingProxyPrivate::shutdownDataArea()
{
    lock();
    save(proxyData());
    unlock();
}

QString HbInputSettingProxyPrivate::dataFilePath()
{
    return HbInputSettingProxy::writablePath() + QDir::separator() + QString("settings");
}

QString HbInputSettingProxyPrivate::dataFileNameAndPath()
{
    return dataFilePath() + QDir::separator() + QString("proxy.dat");
}

void HbInputSettingProxyPrivate::initializeDataArea()
{
    lock();
    bool wasLoaded = load(proxyData());
    if (!wasLoaded) {
        HbSettingProxyInternalData *prData = proxyData();
        if (prData) {
            writeDefaultValuesToData(prData);
        }
    }
    unlock();
}

void HbInputSettingProxyPrivate::writeDefaultValuesToData(HbSettingProxyInternalData* data)
{
    data->version = HbProxyDataRequiredVersion;
    data->globalPrimaryInputLanguage = HbInputLanguage(QLocale::English, QLocale::UnitedKingdom);
    data->globalSecondaryInputLanguage = QLocale::Language(0);
    data->predictiveInputState = (HbKeyboardSettingFlags)HbKeyboardSetting12key | HbKeyboardSettingQwerty;
    data->digitType = HbDigitTypeLatin;
    data->qwertyTextCasing = true;
    data->qwertyCharacterPreview = true;
    data->regionalCorrectionStatus = true;
    data->flipStatus = false;
    data->keypressTimeout = 1000;
    data->autocompletion = (HbKeyboardSettingFlags)(HbKeyboardSetting12key | HbKeyboardSettingQwerty);
    data->typingCorrectionLevel = HbTypingCorrectionLevelHigh;
    data->primaryCandidateMode = HbPrimaryCandidateModeBestPrediction;
    data->preferredMethodHorizontal = HbInputMethodDescriptor();
    data->preferredMethodHorizontal.setData(QByteArray());
    data->preferredMethodVertical = HbInputMethodDescriptor();
    data->preferredMethodVertical.setData(QByteArray());
    data->hwrSpeed = HbHwrSpeedNormal;
    data->cangjieMode = HbCangjieNormal;
}

bool HbInputSettingProxyPrivate::load(HbSettingProxyInternalData *data)
{
    if (!data) {
        return false;
    }
    QFile file(dataFileNameAndPath());
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray rawData = file.read(sizeof(HbSettingProxyInternalData));
    file.close();

    if (rawData.size() == sizeof(HbSettingProxyInternalData)) {
        HbSettingProxyInternalData *ldData = (HbSettingProxyInternalData *)rawData.constData();
        if (ldData) {
            if (ldData->version == HbProxyDataRequiredVersion) {
                memcpy((void *)data, (void *)ldData, sizeof(HbSettingProxyInternalData));
                return true;
            }
        }
    }
    // The data size was incorrect or the version number was wrong
    return false;
}

void HbInputSettingProxyPrivate::save(HbSettingProxyInternalData *data)
{
    if (data) {
        // Make sure that the path exists
        QDir settingDir;
        settingDir.mkpath(dataFilePath());

        QFile file(dataFileNameAndPath());
        if (!file.open(QIODevice::WriteOnly)) {
            return;
        }

        file.write((const char *)data, sizeof(HbSettingProxyInternalData));
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

HbSettingProxyInternalData *HbInputSettingProxyPrivate::proxyData() const
{
    return static_cast<HbSettingProxyInternalData *>(mSharedMemory->data());
}

/// @endcond

/*!
Returns pointer to the singleton object.
*/
HbInputSettingProxy *HbInputSettingProxy::instance()
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
    delete d->mSharedMemory;
    d->mSharedMemory = 0;
}

/*!
Toggles prediction mode
*/
void HbInputSettingProxy::togglePrediction()
{
    HbInputMethod *im = HbInputMethod::activeInputMethod();
    if (im) {
        HbInputState state = im->inputState();
        if (state.keyboard() & HbQwertyKeyboardMask) {
            setPredictiveInputStatus(HbKeyboardSettingQwerty, !predictiveInputStatus(HbKeyboardSettingQwerty));
        } else {
            setPredictiveInputStatus(HbKeyboardSetting12key, !predictiveInputStatus(HbKeyboardSetting12key));
        }
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
void HbInputSettingProxy::connectObservingObject(QObject *aObserver)
{
    if (aObserver) {
        connect(this, SIGNAL(globalInputLanguageChanged(const HbInputLanguage &)), aObserver, SLOT(globalInputLanguageChanged(const HbInputLanguage &)));
        connect(this, SIGNAL(globalSecondaryInputLanguageChanged(const HbInputLanguage &)), aObserver, SLOT(globalSecondaryInputLanguageChanged(const HbInputLanguage &)));
    }
}

/*!
Disconnects given object from the setting proxy.

\sa connectObservingObject
*/
void HbInputSettingProxy::disconnectObservingObject(QObject *aObserver)
{
    if (aObserver) {
        disconnect(this, SIGNAL(globalInputLanguageChanged(const HbInputLanguage &)), aObserver, SLOT(globalInputLanguageChanged(const HbInputLanguage &)));
        disconnect(this, SIGNAL(globalSecondaryInputLanguageChanged(const HbInputLanguage &)), aObserver, SLOT(globalSecondaryInputLanguageChanged(const HbInputLanguage &)));
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
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        res = prData->globalPrimaryInputLanguage;
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
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        res = prData->globalSecondaryInputLanguage;
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
Stores speed attribute for handwriting recognition.
*/
void HbInputSettingProxy::setHwrWritingSpeed(HbHwrWritingSpeed speed)
{
    Q_D(HbInputSettingProxy);
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->hwrSpeed != speed) {
            prData->hwrSpeed = speed;
            notify = true;
        }
        d->unlock();
        if (notify) {
            emit hwrWritingSpeedChanged(speed);
        }
    }
}

/*!
Returns handwriting recignition speed attribute.
*/
HbHwrWritingSpeed HbInputSettingProxy::hwrWritingSpeed() const
{
    Q_D(const HbInputSettingProxy);
    HbHwrWritingSpeed res = HbHwrSpeedNormal;

    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        res = prData->hwrSpeed;
    }

    return res;
}

/*!
Strores detail mode for Chinese CangJie input mode.
*/
void HbInputSettingProxy::setDetailedCangjieMode(HbCangjieDetailMode cangjieDetail)
{
    Q_D(HbInputSettingProxy);
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->cangjieMode != cangjieDetail) {
            prData->cangjieMode = cangjieDetail;
            notify = true;
        }
        d->unlock();
        if (notify) {
            emit detailedCangjieModeChanged(cangjieDetail);
        }
    }
}

/*!
Returns detail mode for Chinese CangJie input mode.
*/
HbCangjieDetailMode HbInputSettingProxy::detailedCangjieMode() const
{
    Q_D(const HbInputSettingProxy);
    HbCangjieDetailMode res = HbCangjieNormal;

    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        res = prData->cangjieMode;
    }

    return res;
}

/*!
Returns active keyboard for given screen oriention.
*/
HbKeyboardType HbInputSettingProxy::activeKeyboard(Qt::Orientation orientation) const
{
    Q_D(const HbInputSettingProxy);

    if (orientation == Qt::Horizontal) {
        HbSettingProxyInternalData *prData = d->proxyData();
        if (prData) {
            d->lock();
            if (prData->flipStatus == true) {
                return HbKeyboardHardwareLandcape;
            }
            d->unlock();
        }
        return HbKeyboardTouchLandscape;
    } else {
        return HbKeyboardTouchPortrait;
    }
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

    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        d->lock();
        if (orientation == Qt::Horizontal) {
            result = prData->preferredMethodHorizontal.descriptor();
        } else {
            result = prData->preferredMethodVertical.descriptor();
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

    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        d->lock();
        if (orientation == Qt::Horizontal) {
            result = prData->preferredMethodHorizontal.data();
        } else {
            result = prData->preferredMethodVertical.data();
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
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        d->lock();
        if (orientation == Qt::Horizontal) {
            prData->preferredMethodHorizontal = inputMethod;
            prData->preferredMethodHorizontal.setData(customData);
        } else {
            prData->preferredMethodVertical = inputMethod;
            prData->preferredMethodVertical.setData(customData);
        }
        d->unlock();
    }
}

/*!
Sets system wide input language. Will emit signal globalInputLanguageChanged if language is changed.

\sa globalInputLanguage
*/
void HbInputSettingProxy::setGlobalInputLanguage(const HbInputLanguage &language)
{
    Q_D(HbInputSettingProxy);
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->globalPrimaryInputLanguage != language) {
            prData->globalPrimaryInputLanguage = language;
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
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->globalSecondaryInputLanguage != language) {
            prData->globalSecondaryInputLanguage = language;
            notify = true;
        }
        d->unlock();
        if (notify) {
            emit globalSecondaryInputLanguageChanged(language);
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

    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        res = prData->predictiveInputState & keyboardType;
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
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();

        HbKeyboardSettingFlags newValue = prData->predictiveInputState;
        if (newStatus) {
            newValue |= keyboardType;
        } else {
            newValue &= ~keyboardType;
        }
        if (prData->predictiveInputState != newValue) {
            prData->predictiveInputState = newValue;
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

    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        HbInputMethod *im = HbInputMethod::activeInputMethod();
        if (im) {
            HbInputState state = im->inputState();
            if (state.keyboard() & HbQwertyKeyboardMask) {
                res = prData->predictiveInputState & HbKeyboardSettingQwerty;
            } else {
                res = prData->predictiveInputState & HbKeyboardSetting12key;
            }
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
    HbInputMethod *im = HbInputMethod::activeInputMethod();
    if (im) {
        HbInputState state = im->inputState();
        if (state.keyboard() & HbQwertyKeyboardMask) {
            setPredictiveInputStatus(HbKeyboardSettingQwerty, newStatus);
        } else {
            setPredictiveInputStatus(HbKeyboardSetting12key, newStatus);
        }
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
Returns paths to input method plugin folders.

All the paths will not necessarily exist in the filesystem.
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
Returns list of paths to all possible keymap locations.

All the paths will not necessarily exist in the filesystem.
*/
QStringList HbInputSettingProxy::keymapPluginPaths()
{
    QStringList result;
#ifdef Q_OS_SYMBIAN
    QFileInfoList list = QDir::drives();
    for (int counter = 0; counter < list.count(); counter ++) {
        result.append(list.at(counter).absoluteFilePath() + QString("/resource/keymaps"));
    }
#else
    result.append(HB_RESOURCES_DIR + (QDir::separator() + QString("keymaps")));
#endif
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

All the paths will not necessarily exist in the filesystem.
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

    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        res = prData->digitType;
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
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        d->lock();
        if (prData->digitType != digitType) {
            prData->digitType = digitType;
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

    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        res = prData->qwertyTextCasing;
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
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->qwertyTextCasing != status) {
            prData->qwertyTextCasing = status;
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
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->qwertyCharacterPreview != previewEnabled) {
            prData->qwertyCharacterPreview = previewEnabled;
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

    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        res = prData->qwertyCharacterPreview;
    }

    return res;
}

/*!
Returns the status of regional input correction feature.

\sa enableRegionalCorrection.
*/
bool HbInputSettingProxy::regionalCorrectionEnabled()
{
    Q_D(const HbInputSettingProxy);
    bool res = false;
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        res = prData->regionalCorrectionStatus;
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
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->regionalCorrectionStatus != newStatus) {
            prData->regionalCorrectionStatus = newStatus;
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
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->keypressTimeout != timeout) {
            prData->keypressTimeout = timeout;
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
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        res = prData->keypressTimeout;
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
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        HbKeyboardSettingFlags newValue = prData->autocompletion;
        if (state) {
            newValue |= keyboardType;
        } else {
            newValue &= ~keyboardType;
        }
        if (prData->autocompletion != newValue) {
            prData->autocompletion = newValue;
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
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        res = prData->autocompletion & keyboardType;
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
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->typingCorrectionLevel != level) {
            prData->typingCorrectionLevel = level;
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
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        res = prData->typingCorrectionLevel;
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
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if (prData->primaryCandidateMode != mode) {
            prData->primaryCandidateMode = mode;
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
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        res = prData->primaryCandidateMode;
    }
    return res;
}

/*!
Set the variable to true if the default keypad is western for chinese input method

\sa useWesternDefaultKeypadForChinese
*/
void HbInputSettingProxy::setWesternDefaultKeypadForChinese(bool set)
{
    Q_D(const HbInputSettingProxy);

    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        bool notify = false;
        d->lock();
        if(prData->useWesternDefaultKeypadForChinese != set) {
            prData->useWesternDefaultKeypadForChinese = set;
            notify = true;
        }
        d->unlock();
        if(notify) {
            emit chineseDefaultKeypadChanged(prData->useWesternDefaultKeypadForChinese);
        }
    }
}

/*!
Get whether the default keypad is western for chinese input method

\sa setWesternDefaultKeypadForChinese
*/
bool HbInputSettingProxy::useWesternDefaultKeypadForChinese() const
{
    Q_D(const HbInputSettingProxy);
    bool res = false;
    HbSettingProxyInternalData *prData = d->proxyData();
    if (prData) {
        res = prData->useWesternDefaultKeypadForChinese;
    }
    return res;
}

/*!
\deprecated HbInputSettingProxy::setActiveKeyboard(HbKeyboardType)
    is deprecated.
*/
void HbInputSettingProxy::setActiveKeyboard(HbKeyboardType keyboard)
{
    Q_UNUSED(keyboard);
}

// End of file
