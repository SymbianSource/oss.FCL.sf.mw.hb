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
#ifndef HB_INPUT_METHOD_H
#define HB_INPUT_METHOD_H

#include <QString>
#include <QInputContext>
#include <QApplication>

#include <hbinputmodeproperties.h>
#include <hbinputmethoddescriptor.h>
#include <hbinputstate.h>
#include <hbinputfocusobject.h>
#include <hbinputlanguage.h>

class QAction;
class HbInputMethodPrivate;

class HB_CORE_EXPORT HbInputMethod : public QInputContext
{
    Q_OBJECT

public:
    HbInputMethod();
    virtual ~HbInputMethod();

    static bool initializeFramework(QApplication& app);
    static void forceUnfocus();
    static HbInputMethod* activeInputMethod();
    static QList<HbInputMethodDescriptor> listCustomInputMethods();

    virtual void focusReceived();
    virtual void focusLost(bool focusSwitch = true);

    bool isActiveMethod() const;

    HbInputFocusObject* focusObject() const;
    void setFocusObject(HbInputFocusObject* focusObject);

    void lockFocus();
    void unlockFocus();

    // From QInputContext (do not override).
    void widgetDestroyed(QWidget* widget);
    void setFocusWidget(QWidget* widget);

    HbInputState inputState() const;

    bool activateState(const HbInputState& state);
    void updateState();
    void editorRootState(HbInputState &result) const;
    bool automaticTextCaseNeeded() const;
    bool activateInputMethod(const HbInputMethodDescriptor &inputMethod);

protected:
    virtual void inputStateActivated(const HbInputState& newState);
    virtual void inputLanguageChanged(const HbInputLanguage &newLanguage);
    virtual void secondaryInputLanguageChanged(const HbInputLanguage &newLanguage);  
    bool stateChangeInProgress() const;

    HbInputLanguage activeLanguage() const;

    bool modeAllowedInEditor(HbInputModeType mode) const;
    void inputStateToEditor(const HbInputState& source);

    void constructLatinState(HbInputState &result) const;

    HbInputMethodDescriptor descriptor() const;

public slots:
    void globalInputLanguageChanged(const HbInputLanguage &newLanguage);
    void globalSecondaryInputLanguageChanged(const HbInputLanguage &newLanguage);
    void activeKeyboardChanged(HbKeyboardType newKeyboard);     
    void orientationChanged(Qt::Orientation orientation);
    virtual void orientationAboutToChange();
    void editorDeleted(QObject *obj);

protected:
    HbInputMethodPrivate *const d_ptr;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputMethod)
    Q_DISABLE_COPY(HbInputMethod)

    friend class HbInputModeCachePrivate;
};

#endif // HB_INPUT_METHOD_H

// End of file
