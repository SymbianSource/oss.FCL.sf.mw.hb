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
#include "hbinputeditorinterface.h"
#include "hbinputeditorinterface_p.h"

#include <QGraphicsProxyWidget>
#include <QWidget>
#include <QList>

#include <hbaction.h>
#include <hbwidget.h>
#include <hbdialog.h>
#include <hbview.h>

#include "hbinputstandardfilters.h"
#include "hbinputvkbhost.h"
#include "hbabstractvkbhost.h"
#include "hbinpututils.h"

/*!
@stable
@hbcore
\class HbEditorInterface
\brief The HbEditorInterface class provides an interface for accessing
editor-specific input attributes.

HbEditorInterface is an interface for different editor widget types, which
do not always derive from the same base classes. The interface defines
a set of properties that applications can modify to constrain the input
characters that the user can enter into an editor widget. %HbEditorInterface
gives applications a common way of setting properties for both %Hb and %Qt editors
(such as HbLineEdit, HbTextEdit, QLineEdit). The interface is also used in
input method code.

HbEditorInterface does not store the editor widget properties locally or
duplicate them. Its memory footprint is very small. You can create as many
interfaces to the same editor as you require for your application.
The HbInput framework maintains only one copy of the editor attributes for
each editor. All interface instances for an editor use those attributes.

With HbEditorInterface, you can control, for example, the following editor attributes:

<ul>
<li>text case: lower case, upper case, automatic</li>
<li>input mode type: numeric, handwriting, ...</li>
<li>input constraints: Latin only, auto-completing, ...</li>
<li>input filter: digits only, input lower case, phone number, URL, ...</li>
<li>editor class type: e-mail, URL, username, password, phone number, ...</li>
<li>extra dictionary to be used in predictive text input</li>
<li>smiley theme</li>
<li>custom button for an editor-specific or application-specific action</li>
</ul>

The interface also contains useful convenience functions to define the type of data
that the user can enter into an editor. The convenience functions set all the necessary
attributes for some commonly used editor cases:

<ul>
<li>setUpAsCompletingEmailField()</li>
<li>setUpAsCompletingUrlField()</li>
<li>setUpAsLatinAlphabetOnlyEditor()</li>
</ul>

\section _usecases_hbeditorinterface Using HbEditorInterface

\subsection _uc_001_hbeditorinterface Creating and using HbEditorInterface

The following example shows how to create an editor interface, attach an editor
to it, and use some attributes.

\code
QLineEdit *lineEdit = new QLineEdit;           // Create an editor
HbEditorInterface editorInterface(lineEdit);   // Create an editor interface and attach lineEdit to it
editorInterface.setTextCase(HbTextCaseUpper);  // Set text case to upper case
\endcode

If the attached editor is deleted, the editor interface will start to return
the same values as when a null pointer is passed to the constructor.

When any of the values is changed, a signal modified() is emitted.

\subsection _uc_002_hbeditorinterface Using the convenience functions

The following example shows how to use a convenience function for configuring
your editor to be a URL field. The convenience function sets the required
attributes necessary for URL input.

\code
HbLineEdit *myLineEdit = new HbLineEdit;         // Create an editor
HbEditorInterface editorInterface2(myLineEdit);  // Create an editor interface to it
editorInterface2.setUpAsCompletingUrlField();    // Use the convenience function to set the editor as an URL field 
\endcode

\subsection _uc_003_hbeditorinterface Setting input constraints

Input constraints in the HbInput framework are defined by the enumeration HbEditorConstraint,
which is defined in the header file hbinputdef.h. You can set several input constraints
at the same time in a bit vector HbEditorConstraints, as in the following example:

\code
HbLineEdit *mySearchField = new HbLineEdit;         // Create an editor
HbEditorInterface editorInterface3(mySearchField);  // Create an editor interface to it

// Use the input constraint function to set the editor as an auto-completing field
// and to prevent the change of the input mode
editorInterface3.setInputConstraints(HbEditorConstraintAutoCompletingField | HbEditorConstraintFixedInputMode);
\endcode

One useful constraint is HbEditorConstraintIgnoreFocus. By default, a virtual 
keyboard is shown when an editor gets the focus. If you want to use the editor
as a read-only viewer, or prevent the virtual keyboard from opening for some
other reason, you can set the HbEditorConstraintIgnoreFocus constraint. It prevents
the editor from getting a focus event, and the virtual keyboard is not shown.

Here is an example of setting HbEditorConstraintIgnoreFocus:
\code
HbSomeViewerPointer *myViewer;                // Pointer to some kind of editor in read-only mode, so it is a viewer 
HbEditorInterface editorInterface4(myViewer);  // Create an editor interface to it (viewer does not have to be
                                               // an editor for this to work as long as it is derived from QObject)

// Instruct the input framework to ignore viewer focus events
editorInterface4.setInputConstraints(HbEditorConstraintIgnoreFocus);  
\endcode

\note Note that there are also some common editor attributes that cannot be
set with HbEditorInterface, such as disabling the predictive mode. To set this
and other input method hints in application code, use your editor's %Qt base class
function, either QGraphicsItem::setInputMethodHints(Qt::InputMethodHints hints) for
%Hb editors, or QWidget::setInputMethodHints(Qt::InputMethodHints hints) for
%Qt editors. In input method code, use
HbInputFocusObject::setInputMethodHints(Qt::InputMethodHints hints).

\sa HbLineEdit, HbTextEdit, HbAbstractEdit
*/


/*!
Constructor. Attaches the given editor to the interface.
*/
HbEditorInterface::HbEditorInterface(QObject *editor)
{
    mPrivate = HbEditorInterfacePrivateCache::instance()->attachEditor(editor, this);
    if (mPrivate) {
        connect(mPrivate, SIGNAL(destroyed(QObject *)), this, SLOT(backendDestroyed(QObject *)));
    }
}

/*!
Destructor.
*/
HbEditorInterface::~HbEditorInterface()
{
    // NOTE: We do not delete private object here on purpose, it is owned by the cache.
}

/*!
Returns the text case.

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
Sets the text case.

\sa textCase
*/
void HbEditorInterface::setTextCase(HbTextCase textCase)
{
    if (mPrivate) {
        mPrivate->lock();
        mPrivate->mTextCase = textCase;
        mPrivate->unlock();
        HbEditorInterfacePrivateCache::instance()->notifyValueChanged(mPrivate->mHostEditor);
    }
}

/*!
Returns the active editor input mode.

\sa setMode
*/
HbInputModeType HbEditorInterface::mode() const
{
    HbInputModeType ret = HbInputModeNone;
    if (mPrivate) {
        mPrivate->lock();
        ret = mPrivate->mInputMode;
        mPrivate->unlock();
    }

    return ret;
}

/*!
Sets the active editor input mode.

\sa mode
*/
void HbEditorInterface::setMode(HbInputModeType inputMode)
{
    if (mPrivate) {
        mPrivate->lock();
        mPrivate->mInputMode = inputMode;
        if (mPrivate->mLastFocusedState.inputMode() != HbInputModeNone) {
            // Update also the last known input state, otherwise it won't really change.
            mPrivate->mLastFocusedState.setInputMode(inputMode);
        }
        mPrivate->unlock();
        HbEditorInterfacePrivateCache::instance()->notifyValueChanged(mPrivate->mHostEditor);
    }
}

/*!
Returns editor constraints. The returned value is a combination of HbEditorConstraint flags.

\sa setInputConstraints
*/
HbEditorConstraints HbEditorInterface::inputConstraints() const
{
    HbEditorConstraints ret = 0;
    if (mPrivate) {
        mPrivate->lock();
        ret = mPrivate->mConstraints;
        mPrivate->unlock();
    }

    return ret;
}

/*!
Sets editor constraints.

\sa inputConstraints
*/
void HbEditorInterface::setInputConstraints(HbEditorConstraints constraints)
{
    if (mPrivate) {
        mPrivate->lock();
        mPrivate->mConstraints = constraints;
        mPrivate->unlock();
        HbEditorInterfacePrivateCache::instance()->notifyValueChanged(mPrivate->mHostEditor);
    }
}

/*!
Returns the active input filter. The input framework will always run any input text
through the active filter before committing the text into the editor buffer.

In some cases, the input framework also automatically sets the filter to match
input method hints. The default filter can still be overridden.

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
Sets the active input filter. The ownership is not transferred.

The input framework will always run any input text through the active filter
before committing the text into the editor buffer.

In some cases, the input framework also automatically sets the filter to match
input method hints. The default filter can still be overridden.

\sa filter
*/
void HbEditorInterface::setFilter(HbInputFilter *filter)
{
    if (mPrivate) {
        mPrivate->lock();
        mPrivate->mFilter = filter;
        mPrivate->unlock();
        HbEditorInterfacePrivateCache::instance()->notifyValueChanged(mPrivate->mHostEditor);
    }
}

/*!
Returns the local digit type setting. If this value is set, it will override
the device-wide digit type setting.

\sa setDigitType
*/
HbInputDigitType HbEditorInterface::digitType() const
{
    HbInputDigitType ret = HbDigitTypeNone;

    if (mPrivate) {
        mPrivate->lock();
        ret = mPrivate->mLocalDigitType;
        mPrivate->unlock();
    }

    return ret;
}

/*!
Sets the local digit type. If this value is set, it will override the device-wide
digit type setting.

\sa digitType
*/
void HbEditorInterface::setDigitType(HbInputDigitType digitType)
{
    if (mPrivate) {
        mPrivate->lock();
        mPrivate->mLocalDigitType = digitType;
        mPrivate->unlock();
        HbEditorInterfacePrivateCache::instance()->notifyValueChanged(mPrivate->mHostEditor);
    }
}

/*!
Adds \a action to this editor's list of actions on the first position.

The action is used in virtual keyboards to define a custom button.
The keyboard will display a button using the icon specified in the action.
When the button is clicked, the action is triggered.

If the action is already in the list, it will be removed before being added
again.

%Hb input methods currently use only the first action in the list and display
only the icon, not the text.
Different input methods may display 0 or more than one button.

Note that the custom button action is only a request to show it. Whether
or not the virtual keyboard widget actually shows the action depends on
the situation and the active input method. That is why a function assigned
to the custom button should never be the only way to use a feature, but
only a shortcut.

\sa insertAction
\sa removeAction
\sa actions
*/
void HbEditorInterface::addAction(HbAction *action)
{
    insertAction(0, action);
}

/*!
Inserts \a action to this editor's list of actions before the action \a before.
If the action is already in the list, it will be removed before being inserted again.

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
        if (index >= 0) {
            mPrivate->mActions.removeAt(index);
            disconnect(action, SIGNAL(destroyed(QObject *)),
                       HbEditorInterfacePrivateCache::instance(), SLOT(actionDestroyed(QObject *)));
        }

        int pos = mPrivate->mActions.indexOf(before);
        if (pos < 0) {
            pos = mPrivate->mActions.size();
        }
        mPrivate->mActions.insert(pos, action);

        connect(action, SIGNAL(destroyed(QObject *)),
                HbEditorInterfacePrivateCache::instance(), SLOT(actionDestroyed(QObject *)));

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
        disconnect(action, SIGNAL(destroyed(QObject *)),
                   HbEditorInterfacePrivateCache::instance(), SLOT(actionDestroyed(QObject *)));
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
QList<HbAction *> HbEditorInterface::actions() const
{
    QList<HbAction *> ret;
    if (mPrivate) {
        mPrivate->lock();
        ret = mPrivate->mActions;
        mPrivate->unlock();
    }
    return ret;
}

/*!
Returns the id value for the attached extra user dictionary. Returns zero if no extra
user dictionaries are attached to this editor.

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
Sets the extra user dictionary id value. After setting this value those prediction
engines that support extra dictionaries attach the given dictionary to be
part of the prediction vocabulary.

\sa extraDictionaryId
\sa HbUserDictionary
\sa HbExtraUserDictionary
*/
void HbEditorInterface::setExtraDictionaryId(int id)
{
    if (mPrivate) {
        mPrivate->lock();
        mPrivate->mExtraDictionaryId = id;
        mPrivate->unlock();
        HbEditorInterfacePrivateCache::instance()->notifyValueChanged(mPrivate->mHostEditor);
    }
}

/*!
Returns the editor class type.

\sa setEditorClass
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
Sets the editor class type.

\sa editorClass
*/
void HbEditorInterface::setEditorClass(HbInputEditorClass editorClass)
{
    if (mPrivate) {
        mPrivate->lock();
        mPrivate->mClass = editorClass;
        mPrivate->unlock();
        HbEditorInterfacePrivateCache::instance()->notifyValueChanged(mPrivate->mHostEditor);
    }
}

/*!
Returns the editor-specific smiley theme.

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
Sets the editor-specific smiley theme. The smiley picker will display smileys
provided by \a theme.

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
Finds and returns a virtual keyboard host for the attached editor.
*/
HbVkbHost *HbEditorInterface::vkbHost() const
{
    QObject *theEditor = editor();

    if (theEditor) {
        QGraphicsObject *graphicsObjectEditor = 0;
        QWidget *widgetEditor = qobject_cast<QWidget *>(theEditor);
        if (widgetEditor) {
            if (QGraphicsProxyWidget *pw = HbInputUtils::graphicsProxyWidget(widgetEditor)) {
                HbVkbHost *host = HbVkbHost::getVkbHost(widgetEditor);
                if (host) {
                    return host;
                }
                // it is a proxy widget, let graphics widget loop handle the parent chain.
                graphicsObjectEditor = pw;
            } else {
                for (QWidget *parent = widgetEditor; parent; parent = parent->parentWidget()) {
                    HbVkbHost *host = HbVkbHost::getVkbHost(parent);
                    if (host) {
                        return host;
                    }
                }
                return new HbAbstractVkbHost(widgetEditor->window());
            }
        }

        if (!graphicsObjectEditor) {
            graphicsObjectEditor = qobject_cast<QGraphicsObject *>(theEditor);
        }

        if (graphicsObjectEditor) {
            QGraphicsObject *lastKnownParent = 0;
            QGraphicsObject *lastContainer = 0;
            for (QGraphicsObject *parent = graphicsObjectEditor; parent; parent = parent->parentObject()) {
                HbVkbHost *host = HbVkbHost::getVkbHost(parent);
                if (host) {
                    return host;
                }
                if (qobject_cast<HbView *>(parent) || qobject_cast<HbPopup *>(parent)) {
                    lastContainer = parent;
                }
                lastKnownParent = parent;
            }
            if (lastContainer) {
                // No host was found from graphics widget parent chain.
                return new HbAbstractVkbHost(lastContainer);
            } else if (lastKnownParent) {
                // No host or container was found from graphics widget parent chain.
                return new HbAbstractVkbHost(lastKnownParent);
            }
        }
    }

    return 0;
}

/*!
Returns \c true if this instance is attached to the same editor
as \a editorInterface.
*/
bool HbEditorInterface::operator==(const HbEditorInterface &editorInterface) const
{
    return (mPrivate == editorInterface.mPrivate);
}

/*!
Returns \c true if this instance is not attached to the same editor
as \a editorInterface.
*/
bool HbEditorInterface::operator!=(const HbEditorInterface &editorInterface) const
{
    return (mPrivate != editorInterface.mPrivate);
}

/*!
Returns a pointer to the editor object attached to this interface.
*/
QObject *HbEditorInterface::editor() const
{
    if (mPrivate) {
        return mPrivate->mHostEditor;
    }

    return 0;
}

/*!
The last focused state remembers the editor state exactly as it was when
the focus was removed. This is for the framework side API, and there
should never be need to call it from application code.

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
Sets the last focused state. This is for the framework side API, and
there should never be need to call it from application code.

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
A convenience function for setting up the editor as an auto-completing
e-mail address field.
*/
void HbEditorInterface::setUpAsCompletingEmailField()
{
    setMode(HbInputModeNone);
    setInputConstraints(HbEditorConstraintLatinAlphabetOnly | HbEditorConstraintAutoCompletingField);
    setFilter(HbEmailAddressFilter::instance());
    setEditorClass(HbInputEditorClassEmail);
    setExtraDictionaryId(HbInputEditorClassEmail);
    setDigitType(HbDigitTypeNone);
    mPrivate->setInputMethodHints(Qt::ImhEmailCharactersOnly | Qt::ImhNoPredictiveText | Qt::ImhPreferLowercase);
}

/*!
A convenience function for setting up the editor as an auto-completing
URL field.
*/
void HbEditorInterface::setUpAsCompletingUrlField()
{
    setMode(HbInputModeNone);
    setInputConstraints(HbEditorConstraintLatinAlphabetOnly | HbEditorConstraintAutoCompletingField);
    setEditorClass(HbInputEditorClassUrl);
    setExtraDictionaryId(HbInputEditorClassUrl);
    setDigitType(HbDigitTypeNone);
    mPrivate->setInputMethodHints(Qt::ImhUrlCharactersOnly | Qt::ImhNoPredictiveText | Qt::ImhPreferLowercase);
}

/*!
A convenience function for setting up the editor as a Latin alphabet editor.
In this mode, the input framework will use the global input language if
it is naturally capable of producing Latin alphabets. Otherwise it will switch
locally to English (is is assumed that English is always available).

It is also recommended that prediction is disabled in Latin-only editors.
That is because the predictive mode in a Latin alphabet editor is controversial:
which prediction database should be used if the global language does not
apply and we switch to English locally? Using the English database would lead
to a situation where some global languages use their native prediction databases
and some do not. To avoid this, the function disables predictive input by default.
*/
void HbEditorInterface::setUpAsLatinAlphabetOnlyEditor()
{
    setMode(HbInputModeNone);
    setInputConstraints(HbEditorConstraintLatinAlphabetOnly);
    mPrivate->setInputMethodHints(Qt::ImhNoPredictiveText);
}

/*!
Returns \c true if the connected editor is configured to behave as
a numeric editor. In Qt 4.6 and beyond, numeric editors have one of these
input method hints set: Qt::ImhDigitsOnly, Qt::ImhDialableCharactersOnly,
or Qt::ImhFormattedNumbersOnly. If either Qt::ImhLowercaseOnly or
Qt::ImhUppercaseOnly is also set, then the editor is not a numeric editor
and this function returns \c false.
*/
bool HbEditorInterface::isNumericEditor() const
{
    return mPrivate && ((mPrivate->inputMethodHints() & (Qt::ImhDigitsOnly | Qt::ImhDialableCharactersOnly | Qt::ImhFormattedNumbersOnly)) &&
            !(mPrivate->inputMethodHints() & (Qt::ImhLowercaseOnly | Qt::ImhUppercaseOnly)));
}

/*!
Returns \c true if the predictive input mode is allowed in the attached editor.
*/
bool HbEditorInterface::isPredictionAllowed() const
{
    return mPrivate && !(mPrivate->inputMethodHints() & Qt::ImhNoPredictiveText);
}

/*!
Returns \c true if there is an existing data record for \a object. This function can
be used for testing whether someone has set editor data for a given object without
creating a data record for it. This is usually not needed on the application side.
*/
bool HbEditorInterface::isConnected(QObject *object)
{
    return HbEditorInterfacePrivateCache::instance()->isConnected(object);
}

/*!
 * \fn void HbEditorInterface::modified()
 * 
 * This signal is emitted when any of the editor settings is changed. 
 */

/*!
 * \fn void HbEditorInterface::cursorPositionChanged(int oldPos, int newPos)
 * 
 * This signal is emitted when the cursor position in the attached editor
 * is changed. 
 */


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

