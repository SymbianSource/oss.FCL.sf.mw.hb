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
#ifndef HB_INPUT_METHOD_P_H
#define HB_INPUT_METHOD_P_H

#include <QString>
#include <QPointer>

#include <hbinputmodeproperties.h>
#include <hbinputstate.h>
#include <hbinputlanguage.h>
#include "hbinputcontextproxy_p.h"

class HbInputStateMachine;
class HbInputFilter;
class HbInputMethod;
class HbInputMainWindow;
class HbInputContextProxy;

class HB_CORE_PRIVATE_EXPORT HbInputMethodPrivate
{
    Q_DECLARE_PUBLIC(HbInputMethod)

public:
    explicit HbInputMethodPrivate(HbInputMethod* owner)
        : q_ptr(owner),
        mProxy(0),
        mIsActive(false),
        mFocusObject(0),
        mInputState(HbInputModeNone, HbTextCaseNone, HbKeyboardNone),        
        mFocusLocked(false),
        mStateChangeInProgress(false),
        mInsideVanillaWindow(false)
    {}
    ~HbInputMethodPrivate();

    HbInputFilter *editorFilter() const;
    int editorConstraints() const;
    void inputStateFromEditor(HbInputState& result);
    void inputStateToEditor(const HbInputState& source);
    HbInputLanguage activeLanguage() const;
    bool modeAllowedInEditor(HbInputModeType mode) const;
    bool stateAllowedInEditor(const HbInputState& state);
    HbInputMethod* findStateHandler(HbInputState& startingState);
    HbInputLanguage findStateLanguage() const;
    bool automaticTextCaseNeeded() const;
    bool textCaseApplies() const;
    HbKeyboardType activeKeyboard() const;
    void setFocusCommon();
    void refreshState();
    bool compareWithCurrentFocusObject(HbInputFocusObject* focusObject) const;
    QInputContext* proxy();
    bool isFixedCaseEditor() const;
    bool isLowerCaseOnlyEditor() const;
    bool isUpperCaseOnlyEditor() const;
    void transfer(HbInputMethod* source);
    void contextSwitch(HbInputMethod* toBeActive);
    void editorRootState(HbInputState &result) const;
    void constructLatinState(HbInputState &result) const;
    HbTextCase initialTextCase(HbInputModeType inputMode) const; 
    HbInputModeType initialInputMode(const HbInputLanguage &language) const;
    HbInputModeType defaultInputMode(const HbInputLanguage &inputLanguage) const;  
    void setUpFocusedObjectAsDigitsOnlyEditor();
    void setUpFocusedObjectAsFormattedNumberEditor();
    void setUpFocusedObjectAsPhoneNumberEditor();
    void setUpFocusedObjectAsEmailEditor();
    void setUpFocusedObjectAsUrlEditor();
    void initMainWindow(QWidget *window);
    void checkAndShowMainWindow();
    void showMainWindow();
    void hideMainWindow();
public:
    HbInputMethod *q_ptr;
    QPointer<HbInputContextProxy > mProxy;
    bool mIsActive;
    HbInputFocusObject* mFocusObject;
    HbInputState mInputState;   
    bool mFocusLocked;
    bool mStateChangeInProgress;
    QList<HbInputModeProperties> mInputModes;
    bool mInsideVanillaWindow;
private: // For unit test.
    static HbInputMethodPrivate *d_ptr(HbInputMethod *inputMethod) {
        Q_ASSERT(inputMethod);
        return inputMethod->d_func();
    }
    friend class TestHbInputMethodPrivate;
};

#endif // HB_INPUT_METHOD_P_H

// End of file

