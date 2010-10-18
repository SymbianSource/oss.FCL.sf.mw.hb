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
#ifndef HB_INPUT_SETTING_PROXY_H
#define HB_INPUT_SETTING_PROXY_H

#include <QObject>
#include <QString>

#include <hbinputlanguage.h>
#include <hbinputmethoddescriptor.h>

class HbInputFilter;
class HbInputSettingProxyPrivate;
class ContentWidget;

class HB_CORE_EXPORT HbInputSettingProxy : public QObject
{
    Q_OBJECT

public:
    static HbInputSettingProxy *instance();
    static QStringList inputMethodPluginPaths();
    static QStringList keymapPluginPaths();
    static QString languageDatabasePath();
    static QString dictionaryPath();
    static QStringList predictionEnginePaths();
    static QString extraDictionaryPath();
    static QString writablePath();

private:
    HbInputSettingProxy();
    virtual ~HbInputSettingProxy();

public:
    void connectObservingObject(QObject *observer);
    void disconnectObservingObject(QObject *observer);

    HbInputLanguage globalInputLanguage() const;
    void setGlobalInputLanguage(const HbInputLanguage &language);

    HbInputLanguage globalSecondaryInputLanguage() const;
    void setGlobalSecondaryInputLanguage(const HbInputLanguage &language);

    bool predictiveInputStatus(HbKeyboardSettingFlags keyboardType) const;
    void setPredictiveInputStatus(HbKeyboardSettingFlags keyboardType, bool newStatus);

    bool predictiveInputStatusForActiveKeyboard() const;
    void setPredictiveInputStatusForActiveKeyboard(bool newStatus);

    void availableHwKeyboard(QList<HbKeyboardType> &listOfAvailableKeyboards) const;

    HbKeyboardType activeKeyboard(Qt::Orientation orientation) const;
    void setActiveKeyboard(HbKeyboardType keyboard);

    HbInputDigitType globalDigitType() const;
    void setGlobalDigitType(HbInputDigitType digitType);

    bool automaticTextCasingForQwerty();
    void setAutomaticTextCasingForQwerty(bool status);

    bool isCharacterPreviewForQwertyEnabled();
    void setCharacterPreviewForQwerty(bool previewEnabled);

    bool regionalCorrectionEnabled();
    void enableRegionalCorrection(bool status);

    int keypressTimeout() const;
    void setKeypressTimeout(int timeout);

    bool isAutocompletionEnabled(HbKeyboardSettingFlags keyboardType) const;
    void setAutocompletionStatus(HbKeyboardSettingFlags keyboardType, bool newStatus);

    HbTypingCorrectionLevel typingCorrectionLevel() const;
    void setTypingCorrectionLevel(HbTypingCorrectionLevel level);

    HbPrimaryCandidateMode primaryCandidateMode() const;
    void setPrimaryCandidateMode(HbPrimaryCandidateMode mode);

    HbInputMethodDescriptor preferredInputMethod(Qt::Orientation orientation) const;
    void setPreferredInputMethod(Qt::Orientation orientation, const HbInputMethodDescriptor &inputMethod, const QByteArray &customData = QByteArray());
    QByteArray preferredInputMethodCustomData(Qt::Orientation orientation) const;

    HbHwrWritingSpeed hwrWritingSpeed() const;
    void setHwrWritingSpeed(HbHwrWritingSpeed speed);

    HbCangjieDetailMode detailedCangjieMode() const;
    void setDetailedCangjieMode(HbCangjieDetailMode cangjieDetail);

    bool useWesternDefaultKeypadForChinese() const;
    void setWesternDefaultKeypadForChinese(bool useWestern);

signals:
    void globalInputLanguageChanged(const HbInputLanguage &newLanguage);
    void globalSecondaryInputLanguageChanged(const HbInputLanguage &newLanguage);
    void predictiveInputStateChanged(HbKeyboardSettingFlags keyboardType, bool newState);
    void automaticTextCasingStateForQwertyChanged(bool newState);
    void characterPreviewStateForQwertyChanged(bool newState);
    void regionalCorretionStatusChanged(bool newStatus);
    void keypressTimeoutChanged(int newTimeout);
    void autocompletionStateChanged(HbKeyboardSettingFlags keyboardType, bool newState);
    void typingCorrectionLevelChanged(HbTypingCorrectionLevel newLevel);
    void primaryCandidateModeChanged(HbPrimaryCandidateMode newMode);
    void hwrWritingSpeedChanged(HbHwrWritingSpeed speed);
    void detailedCangjieModeChanged(HbCangjieDetailMode);
    void chineseDefaultKeypadChanged(bool toWestern);

public slots:
    void togglePrediction();
    void shutdown();

public:
    friend class ContentWidget;

private:
    HbInputSettingProxyPrivate *const d_ptr;

private:
    Q_DISABLE_COPY(HbInputSettingProxy)
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputSettingProxy)
};

#endif // HB_INPUT_SETTING_PROXY_H

// End of file
