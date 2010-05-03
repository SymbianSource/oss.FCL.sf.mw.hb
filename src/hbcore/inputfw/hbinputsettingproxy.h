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
    static HbInputSettingProxy* instance();
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
    void connectObservingObject(QObject* observer);
    void disconnectObservingObject(QObject* observer);
    HbInputLanguage globalInputLanguage() const;
    void availableHwKeyboard(QList<HbKeyboardType>& listOfAvailableKeyboards) const;
    HbInputLanguage globalSecondaryInputLanguage() const;
    HbKeyboardType activeKeyboard() const;
    HbKeyboardType activeHwKeyboard() const;
    HbKeyboardType activeTouchKeyboard() const;
    void setGlobalInputLanguage(const HbInputLanguage &langauge);
    void setGlobalSecondaryInputLanguage(const HbInputLanguage &language);
    void setActiveKeyboard(HbKeyboardType keyboard);
    void setActiveHwKeyboard(HbKeyboardType keyboard);
    void setActiveTouchKeyboard(HbKeyboardType keyboard);
    int predictiveInputStatus() const;
    void setPredictiveInputStatus(int newStatus);
    bool predictiveInputStatus(HbKeyboardSettingFlags keyboardType) const;
    void setPredictiveInputStatus(HbKeyboardSettingFlags keyboardType, bool newStatus);
    bool predictiveInputStatusForActiveKeyboard() const;
    void setPredictiveInputStatusForActiveKeyboard(bool newStatus);
    HbInputDigitType globalDigitType() const;
    void setGlobalDigitType(HbInputDigitType digitType);
    bool automaticTextCasingForQwerty();
    void setAutomaticTextCasingForQwerty(bool status);
    void setCharacterPreviewForQwerty(bool previewEnabled);
    bool isCharacterPreviewForQwertyEnabled();
    HbInputMethodDescriptor activeCustomInputMethod() const;
    void setActiveCustomInputMethod(const HbInputMethodDescriptor &inputMethod);
    Qt::Orientation screenOrientation();
    void setScreenOrientation(Qt::Orientation screenOrientation);
    void notifyScreenOrientationChange();
    bool orientationChangeCompleted() const;
    void initializeOrientation(Qt::Orientation screenOrientation);
    bool regionalCorrectionEnabled();
    void enableRegionalCorrection(bool status);
    void setKeypressTimeout(int timeout);
    int keypressTimeout() const;
    void setAutocompletionStatus(HbKeyboardSettingFlags keyboardType, bool newStatus);
    bool isAutocompletionEnabled(HbKeyboardSettingFlags keyboardType) const;
    void setTypingCorrectionLevel(HbTypingCorrectionLevel level);
    HbTypingCorrectionLevel typingCorrectionLevel() const;
    void setPrimaryCandidateMode(HbPrimaryCandidateMode mode);
    HbPrimaryCandidateMode primaryCandidateMode() const;

signals:
    void globalInputLanguageChanged(const HbInputLanguage &newLanguage);
    void globalSecondaryInputLanguageChanged(const HbInputLanguage &newLanguage);
    void activeKeyboardChanged(HbKeyboardType newKeyboard);
    void activeHwKeyboardChanged(HbKeyboardType newKeyboard);
    void activeTouchKeyboardChanged(HbKeyboardType newKeyboard);
    void predictiveInputStateChanged(HbKeyboardSettingFlags keyboardType, bool newState);
    void automaticTextCasingStateForQwertyChanged(bool newState);
    void characterPreviewStateForQwertyChanged(bool newState);
    void orientationAboutToChange();
    void orientationChanged(Qt::Orientation orientation);
    void regionalCorretionStatusChanged(bool newStatus);
    void keypressTimeoutChanged(int newTimeout);
    void autocompletionStateChanged(HbKeyboardSettingFlags keyboardType, bool newState);
    void typingCorrectionLevelChanged(HbTypingCorrectionLevel newLevel);
    void primaryCandidateModeChanged(HbPrimaryCandidateMode newMode);

public slots:
    void togglePrediction();
    void shutdown();

public:
    friend class ContentWidget;

private:
    HbInputSettingProxyPrivate * const d_ptr;

private:
    Q_DISABLE_COPY(HbInputSettingProxy)
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputSettingProxy)
};

#endif // HB_INPUT_SETTING_PROXY_H

// End of file
