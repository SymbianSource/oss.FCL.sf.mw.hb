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
#include <QGraphicsProxyWidget>
#include <QWidget>
#include <QList>

#include <hbaction.h>
#include <hbwidget.h>

#include "hbinputeditorinterface.h"
#include "hbinputeditorinterface_p.h"
#include "hbinputstandardfilters.h"
#include "hbinputvkbhost.h"
#include "hbabstractvkbhost.h"

/*!
@alpha
@hbcore
\class HbEditorInterface
\brief An interface for accessing editor specific input attributes.

This class is an interface for accessing and manipulating editor attributes, such as input mode, text case,
constraints, etc. It also contains some useful convenience and utility methods. This interface is meant to be used
by both client application and input method sides.

Following example shows how to create editor interface, attach editor to it and use some attributes.

\snippet{unittest_hbinputeditorinterface/unittest_hbinputeditorinterface.cpp,1}

If the attached editor is deleted, the editor interface will start to return same values
as when a null pointer is passed to the constructor.

When any of the values is changed, signal modified() is emited.
*/


/*!
Constructs the object and attaches given editor.
*/
HbEditorInterface::HbEditorInterface(QObject* aEditor)
{
    mPrivate = HbEditorInterfacePrivateCache::instance()->attachEditor(aEditor, this);
    connect(mPrivate, SIGNAL(destroyed(QObject*)), this, SLOT(backendDestroyed(QObject*)));
}

/*!
Destructs the object.
*/
HbEditorInterface::~HbEditorInterface()
{
    // NOTE: We do not delete private object here on purpose, it is owned by the cache.
}

/*!
Returns text case. Returned value is HbTextCase

\sa setTextCase
*/
HbTextCase HbEditorInterface::textCase() const
{
    HbTextCase ret = HbTextCaseNone;
    if (mPrivate) {
        mPrivate->lock();
        ret = mPrivate->mTextCase;
        mPrivate->unlock();
    }

    return ret;
}

/*!
Sets text case.

\sa textCase
\sa HbTextCase
*/
void HbEditorInterface::setTextCase(HbTextCase aTextCase)
{
    if (mPrivate) {
        mPrivate->lock();
        mPrivate->mTextCase = aTextCase;
        mPrivate->unlock();
        HbEditorInterfacePrivateCache::instance()->notifyValueChanged(mPrivate->mHostEditor);
    }
}

/*!
Returns active editor input mode. Returned value is HbInputMethodType.

\sa setInputMode
*/
int HbEditorInterface::inputMode() const
{
    int ret = HbInputModeNone;
    if (mPrivate) {
        mPrivate->lock();
        ret = mPrivate->mInputMode;
        mPrivate->unlock();
    }

    return ret;
}

/*!
Sets active editor input mode.

\sa inputMode
*/
void HbEditorInterface::setInputMode(int aInputMode)
{
    if (mPrivate) {
        mPrivate->lock();
        mPrivate->mInputMode = aInputMode;
        mPrivate->unlock();
        HbEditorInterfacePrivateCache::instance()->notifyValueChanged(mPrivate->mHostEditor);
    }
}

/*!
Returns editor constraints. Returned value is a bit vector consisting of
HbEditorConstraint bits.

\sa setConstraints
*/
int HbEditorInterface::constraints() const
{
    int ret = 0;
    if (mPrivate) {
        mPrivate->lock();
        ret = mPrivate->mConstraints;
        mPrivate->unlock();
    }

    return ret;
}

/*!
Sets editor constraints.

\sa constraints
*/
void HbEditorInterface::setConstraints(int aConstraints)
{
    if (mPrivate) {
        mPrivate->lock();
        mPrivate->mConstraints = aConstraints;
        mPrivate->unlock();
        HbEditorInterfacePrivateCache::instance()->notifyValueChanged(mPrivate->mHostEditor);
    }
}

/*!
Returns custom filter id. If this value is set, then corresponding filter
in HbEditorCharacterFilter is used as part of edit operations for this editor.

\sa setFilter
*/
HbInputFilter *HbEditorInterface::filter() const
{
    HbInputFilter *ret = 0;
    if (mPrivate) {
        mPrivate->lock();
        ret = mPrivate->mFilter;
        mPrivate->unlock();
    }

    return ret;
}

/*!
Sets custom filter id.

\sa filter
*/
void HbEditorInterface::setFilter(HbInputFilter *aFilter)
{
    if (mPrivate) {
        mPrivate->lock();
        mPrivate->mFilter = aFilter;
        mPrivate->unlock();
        HbEditorInterfacePrivateCache::instance()->notifyValueChanged(mPrivate->mHostEditor);
    }
}

/*!
Returns local digit type setting. If this value is set, it will override device wide
digit type setting. Returned value is HbInputDigitType.

\sa setLocalDigitType
*/
int HbEditorInterface::localDigitType() const
{
    int ret = HbDigitTypeNone;

    if (mPrivate) {
        mPrivate->lock();
        ret = mPrivate->mLocalDigitType;
        mPrivate->unlock();
    }

    return ret;
}

/*!
Sets local digit type.

\sa localDigitType
*/
void HbEditorInterface::setLocalDigitType(int aDigitType)
{
    if (mPrivate) {
        mPrivate->lock();
        mPrivate->mLocalDigitType = aDigitType;
        mPrivate->unlock();
        HbEditorInterfacePrivateCache::instance()->notifyValueChanged(mPrivate->mHostEditor);
    }
}

/*!
Adds action to this editor's list of actions on the first position.

The action is used in virtual keyboards to define the application specific button. The
keyboard will display a button using the text, icon and tooltip specified in the action.
When the button is clicked, the action is triggered.

If the action is already in the list, it will be removed before adding it again.

Hb input methods currently use only the first action in the list.
Different input methods may display 0 or more than one button.

\sa insertAction
\sa removeAction
\sa actions
*/
void HbEditorInterface::addAction(HbAction *action)
{
    insertAction(0, action);
}

/*!
Inserts action to this editor's list of actions before the action before.
If the action is already in the list, it will be removed before inserting it again.

\sa addAction
\sa removeAction
\sa actions
*/
void HbEditorInterface::insertAction(HbAction *before, HbAction *action)
{
    if (!action) {
        return;
    }

    if (mPrivate) {
        mPrivate->lock();

        // Remove the action first if it's already in the list
        int index = mPrivate->mActions.indexOf(action);
        if (index >= 0)
            mPrivate->mActions.removeAt(index);
    
        int pos = mPrivate->mActions.indexOf(before);
        if (pos < 0) {
            pos = mPrivate->mActions.size();
        }
        mPrivate->mActions.insert(pos, action);

        mPrivate->unlock();
        HbEditorInterfacePrivateCache::instance()->notifyValueChanged(mPrivate->mHostEditor);
    }
}

/*!
Removes the specified action.

\sa addAction
\sa insertAction
\sa actions
*/
void HbEditorInterface::removeAction(HbAction *action)
{
    if (!action) {
        return;
    }
    if (mPrivate) {
        mPrivate->lock();
        mPrivate->mActions.removeAll(action);
        mPrivate->unlock();
        HbEditorInterfacePrivateCache::instance()->notifyValueChanged(mPrivate->mHostEditor);
    }
}

/*!
Returns this editor's list of actions.

\sa addAction
\sa insertAction
\sa removeAction
*/
QList<HbAction*> HbEditorInterface::actions() const
{
    QList<HbAction*> ret;
    if (mPrivate) {
        mPrivate->lock();
        ret = mPrivate->mActions;
        mPrivate->unlock();
    }
    return ret;
}

/*!
Returns id value for attached extra user dictionary. Returns zero if no extra
user dictinaries are attached to this editor.

\sa setExtraDictionaryId
\sa HbUserDictionary
\sa HbExtraUserDictionary
*/
int HbEditorInterface::extraDictionaryId() const
{
    int ret = 0;

    if (mPrivate) {
        mPrivate->lock();
        ret = mPrivate->mExtraDictionaryId;
        mPrivate->unlock();
    }

    return ret;
}

/*!
Sets extra user dictionary id value. After setting this value those prediction
implementations that support extra user dictionaries attach given user additional dictionary
in case it is available.

\sa extraDictionaryId
\sa HbUserDictionary
\sa HbExtraUserDictionary
*/
void HbEditorInterface::setExtraDictionaryId(int aId)
{
    if (mPrivate) {
        mPrivate->lock();
        mPrivate->mExtraDictionaryId = aId;
        mPrivate->unlock();
        HbEditorInterfacePrivateCache::instance()->notifyValueChanged(mPrivate->mHostEditor);
    }
}

/*!
Returns editor class.
*/
HbInputEditorClass HbEditorInterface::editorClass() const
{
    HbInputEditorClass ret = HbInputEditorClassUnknown;

    if (mPrivate) {
        mPrivate->lock();
        ret = (HbInputEditorClass)mPrivate->mClass;
        mPrivate->unlock();
    }

    return ret;
}

/*!
Sets editor class.
*/
void HbEditorInterface::setEditorClass(HbInputEditorClass aClass)
{
    if (mPrivate) {
        mPrivate->lock();
        mPrivate->mClass = aClass;
        mPrivate->unlock();
        HbEditorInterfacePrivateCache::instance()->notifyValueChanged(mPrivate->mHostEditor);
    }
}

/*!
Returns editor sepcific smiley theme.

\sa setSmileyTheme
*/
HbSmileyTheme HbEditorInterface::smileyTheme() const
{
    HbSmileyTheme result;

    if (mPrivate) {
        mPrivate->lock();
        result = mPrivate->mSmileyTheme;
        mPrivate->unlock();
    }

    return result;
}

/*!
Sets editor specific smiley theme. The smiley picker will display smileys provided
by given theme.

\sa smileyTheme
*/
void HbEditorInterface::setSmileyTheme(const HbSmileyTheme &theme)
{
    if (mPrivate) {
        mPrivate->lock();
        mPrivate->mSmileyTheme = theme;
        mPrivate->unlock();
        HbEditorInterfacePrivateCache::instance()->notifyValueChanged(mPrivate->mHostEditor);
    }
}

/*!
Finds and returns virtual keyboard host for this editor.
*/
HbVkbHost *HbEditorInterface::vkbHost() const
{
    QObject *theEditor = editor();

    if (theEditor) {
        QGraphicsWidget *graphicsWidgetEditor = 0;
        QWidget *widgetEditor = qobject_cast<QWidget*>(theEditor);
        if (widgetEditor) {
            if (widgetEditor->graphicsProxyWidget()) {
                HbVkbHost *host = HbVkbHost::getVkbHost(widgetEditor);
                if (host) {
                    return host;
                }
                // it is a proxy widget, let graphics widget loop handle the parent chain.
                graphicsWidgetEditor = widgetEditor->graphicsProxyWidget();
            } else {
                for (QWidget *parent = widgetEditor; parent; parent = parent->parentWidget()) {
                    HbVkbHost* host = HbVkbHost::getVkbHost(parent);
                    if (host) {
                        return host;
                    }
                }
                return 0;  // Need to add default handler here...
            }
        }

        if (!graphicsWidgetEditor) {
            graphicsWidgetEditor = qobject_cast<QGraphicsWidget*>(theEditor);
        }

        if (graphicsWidgetEditor) {
            QGraphicsWidget *lastKnownParent = 0;
            for (QGraphicsWidget *parent = graphicsWidgetEditor; parent; parent = parent->parentWidget()) {
                HbVkbHost* host = HbVkbHost::getVkbHost(parent);
                if (host) {
                    return host;
                }
                lastKnownParent = parent;
            }
            if (lastKnownParent) {
                // No host was found from graphics widget parent chain.
                // Use popup host as a fallback.
                return new HbAbstractVkbHost(lastKnownParent);
            }
        }
    }

    return 0;
}

/*!
Returns true if this instance is attached to same editor as given instance.
*/
bool HbEditorInterface::operator==(const HbEditorInterface& aInterface) const
{
    return (mPrivate == aInterface.mPrivate);
}

/*!
Returns true if this instance is not attached to same editor as given instance.
*/
bool HbEditorInterface::operator!=(const HbEditorInterface& aInterface) const
{
    return (mPrivate != aInterface.mPrivate);
}

/*!
Returns pointer to the editor object this interface is attached to.
*/
QObject* HbEditorInterface::editor() const
{
    if (mPrivate) {
        return mPrivate->mHostEditor;
    }

    return 0;
}

/*!
Last focused state remembers the editor state exactly as it was when the focus is taken away. This is framework side
API. There should never be need to use it from application code.

\sa setLastFocusedState
*/
void HbEditorInterface::lastFocusedState(HbInputState &result) const
{
    result = HbInputState();

    if (mPrivate) {
        mPrivate->lock();
        result = mPrivate->mLastFocusedState;
        mPrivate->unlock();
    }
}

/*!
Sets last focused state. This is framework side API, there should neever be need to call it from application code.

\sa lastFocusedState
*/
void HbEditorInterface::setLastFocusedState(const HbInputState &state)
{
    if (mPrivate) {
        mPrivate->lock();
        mPrivate->mLastFocusedState = state;
        mPrivate->unlock();
    }
}

/*!
A convenience method for setting up the editor as number only editor. Sets input mode
to HbInputModeNumeric, activates phone number filter and sets fixed input mode
constraint. In Qt 4.6 sets Qt::ImhDigitsOnly and Qt::ImhDialableCharactersOnly hints.
*/
void HbEditorInterface::setUpAsPhoneNumberEditor()
{
    setInputMode(HbInputModeNumeric);
    mPrivate->setInputMethodHints(Qt::ImhDialableCharactersOnly);
    setConstraints(HbEditorConstraintFixedInputMode);
    setFilter(HbPhoneNumberFilter::instance());
}

/*!
A convinience method for setting up the editor as completing email field. 
*/
void HbEditorInterface::setUpAsCompletingEmailField()
{
    setInputMode(HbInputModeNone);
    setConstraints(HbEditorConstraintLatinAlphabetOnly | HbEditorConstraintAutoCompletingField);
    setFilter(HbEmailAddressFilter::instance());
    setEditorClass(HbInputEditorClassEmail);
    setExtraDictionaryId(HbInputEditorClassEmail);
    setLocalDigitType(HbDigitTypeNone);
    mPrivate->setInputMethodHints(Qt::ImhNoPredictiveText | Qt::ImhPreferLowercase);
}

/*!
A convinience method for setting up the editor as completing url field.
*/
void HbEditorInterface::setUpAsCompletingUrlField()
{
    setInputMode(HbInputModeNone);
    setConstraints(HbEditorConstraintLatinAlphabetOnly | HbEditorConstraintAutoCompletingField);
    setFilter(HbUrlFilter::instance());
    setEditorClass(HbInputEditorClassUrl);
    setExtraDictionaryId(HbInputEditorClassUrl);
    setLocalDigitType(HbDigitTypeNone);
    mPrivate->setInputMethodHints(Qt::ImhNoPredictiveText | Qt::ImhPreferLowercase);
}

/*!
A convinience method for setting up the editor as latin alphabet editor. In this mode, the input framework
will use global input language if it is naturally capable of producing latin aplhabets. Otherwise
it will switch locally to english language (is is assumed that english is alwasy available).
It is also recommended that prediction is disabled in latin only editors. That's because predictive mode in
latin alphabet editor is controversial (which prediction database should be used if global language doesn't
apply and we locally to switch to english? If we used english database, that would lead to situation
where some global languages use their native prediction databases and some don't).
That's why this method disables predictive input by default.
*/
void HbEditorInterface::setUpAsLatinAlphabetOnlyEditor()
{
    setInputMode(HbInputModeNone);
    setConstraints(HbEditorConstraintLatinAlphabetOnly);
    mPrivate->setInputMethodHints(Qt::ImhNoPredictiveText); 
}

/*!
Returns true if connected editor is configured to behave as numeric editor. In Qt 4.6 and beyond, numeric
editors have one of these input method hints set: Qt::ImhDigitsOnly, Qt::ImhDialableCharactersOnly or Qt::ImhFormattedNumbersOnly.
If either Qt::ImhLowercaseOnly or Qt::ImhUppercaseOnly is also set, then the editor is not numeric editor and
this method returns false.
*/
bool HbEditorInterface::isNumericEditor() const
{
    return ((mPrivate->inputMethodHints() & (Qt::ImhDigitsOnly | Qt::ImhDialableCharactersOnly | Qt::ImhFormattedNumbersOnly)) &&
            !(mPrivate->inputMethodHints() & (Qt::ImhLowercaseOnly | Qt::ImhUppercaseOnly)));
}

/*!
Returns true if predictive input mode is allowed in attached editor.
*/
bool HbEditorInterface::isPredictionAllowed() const
{
   return !(mPrivate->inputMethodHints() & Qt::ImhNoPredictiveText);
}

/*!
Returns true if there is existing data record for given object. This method can
be used for testing whether someone has set editor data for given object without
creating a data record for it. This is usually not needed on application side.
*/
bool HbEditorInterface::isConnected(QObject *object)
{
    return HbEditorInterfacePrivateCache::instance()->isConnected(object);
}

/// @cond

void HbEditorInterface::backendModified()
{
    emit modified();
}

void HbEditorInterface::backendDestroyed(QObject *obj)
{
    if (mPrivate == obj) {
        mPrivate = 0;
    }
}

/// @endcond

/*!
For custom editors, connect a signal indicating a change in cursor position to this slot
to enable moving the editor away from under the virtual keyboard if the cursor is moved there.
*/
void HbEditorInterface::notifyCursorPositionChange(int oldPos, int newPos)
{
    if (mPrivate) {
        mPrivate->notifyCursorPositionChange(oldPos, newPos);
    }
}

// End of file

