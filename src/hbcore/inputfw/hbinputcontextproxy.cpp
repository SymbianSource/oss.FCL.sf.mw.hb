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
#include "hbinputcontextproxy_p.h"
#include <hbwidget.h>
#include <hbgraphicsscene.h>
#include <hbinputfocusobject.h>
#include <QGraphicsProxyWidget>
#include <QGraphicsView>
#include <hbinputmethod.h>

/*!
@alpha
@hbcore
\class HbInputContextProxy
\brief A proxy class forwarding class from QInputContext to HbInputMethod

This class is needed because Qt's input context system assumes the ownership
of the installed context and deletes the old one when a new context is installed.
HbInput framework wants to cache active input context entities to its local
memory structures and keep the ownership of those objects. That's why a proxy object
is installed between QInputContext and HbInputMehod classes. When Qt's
input context system deletes old context, it will delete the proxy instead of
real implementation.

We also handle certain common events (such as Qt's input panel events) on this level.
This class is not needed outside of framework code.

\sa QInputContext
\sa HbInputMethod
*/

HbInputContextProxy::HbInputContextProxy(HbInputMethod* target) : mTarget(target)
{
}

HbInputContextProxy::~HbInputContextProxy()
{
}

/*!
\internal
\reimp
*/
QList<QAction *> HbInputContextProxy::actions()
{
    if (mTarget) {
        return mTarget->actions();
    }

    return QList<QAction*>();
}

/*!
\internal
Returns true if given editor widget already has input framework focus.
*/
bool HbInputContextProxy::hasAlreadyInputFrameworkFocus(HbInputMethod *activeMethod, QObject *editorWidget) const
{
    if (activeMethod) {
        HbInputFocusObject *focusObject = activeMethod->focusObject();
        if (focusObject) {
            return focusObject->object() == editorWidget;
        }
    }

    return false;
}

/*!
\internal
Sets input framework focus to given widget if it is valid.
*/
void HbInputContextProxy::setInputFrameworkFocus(QObject *widget)
{
    if(mTarget) {
        if(!widget) {
            mTarget->setFocusObject(0);
        } else if (HbInputFocusObject::isEditor(widget) && !HbInputFocusObject::isReadOnlyWidget(widget)) {           
            mTarget->setFocusObject(new HbInputFocusObject(widget));
        }
    }
}

/*!
\internal
\reimp
*/
bool HbInputContextProxy::filterEvent(const QEvent* event)
{
    if (mTarget) {
#if QT_VERSION >= 0x040600
        if (event->type() == QEvent::CloseSoftwareInputPanel) {
            setInputFrameworkFocus(0);
            return true;
        } else if (event->type() == QEvent::RequestSoftwareInputPanel) {
            if(QWidget * focusedWidget =  qApp->focusWidget()) {
                // see if the focused widget is graphics view, if so get the focused graphics item in the view
                // and acivate inputmethod for the focused graphics item
                if(QGraphicsView * graphicsView = qobject_cast<QGraphicsView*>(focusedWidget)) {
                    if(QGraphicsScene * scene = graphicsView->scene()) {
                        if(QGraphicsItem * focusingWidget = scene->focusItem()) {
                            if (focusingWidget->isWidget()) {
                                setInputFrameworkFocus(static_cast<QGraphicsWidget*>(focusingWidget));
                            }
                        }
                    }
                } else {
                    // focused wiget is not graphics view, let see if it is native qt editor
                    // and activate inputmethod for the focused widget
                    setInputFrameworkFocus(focusedWidget);
                }
            }
            return true;
        }
#endif

#ifdef Q_OS_SYMBIAN
        const quint32 HbInputContextProxyExternalKeyboardModifier = 0x00200000;

        if (event->type() == QEvent::QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
            const QKeyEvent *keyEvent = static_cast<const QKeyEvent*>(event);
            if (keyEvent->nativeModifiers() & HbInputContextProxyExternalKeyboardModifier) {
                // Operating system indicates that the event originated from an external keyboard.
                // We let it pass here untouched.
                if (mTarget) {
                    mTarget->reset();
                }
                return false;
            }
        } 
#endif

        return mTarget->filterEvent(event);
    }

    return false;
}

/*!
\internal
\reimp
*/
QFont HbInputContextProxy::font() const
{
    if (mTarget) {
        return mTarget->font();
    }

    return QFont();
}

/*!
\internal
\reimp
*/
QString HbInputContextProxy::identifierName()
{
    if (mTarget) {
        return mTarget->identifierName();
    }

    return QString();
}

/*!
\internal
\reimp
*/
bool HbInputContextProxy::isComposing() const
{
    if (mTarget) {
        return mTarget->isComposing();
    }

    return false;
}

/*!
\internal
\reimp
*/
QString HbInputContextProxy::language()
{   
    if (mTarget) {
        return mTarget->language();
    }

    return QString();
}

/*!
\internal
\reimp
*/
void HbInputContextProxy::mouseHandler(int x, QMouseEvent* event)
{
    if (mTarget) {
        mTarget->mouseHandler(x, event);
    }
}

/*!
\internal
\reimp
*/
void HbInputContextProxy::reset()
{
    if (mTarget) {
        mTarget->reset();
    }
}

/*!
\internal
\reimp
*/
void HbInputContextProxy::sendEvent(const QInputMethodEvent& event)
{
    if (mTarget) {
        mTarget->sendEvent(event);
    }
}

/*!
\internal
\reimp
*/
void HbInputContextProxy::update()
{
    if (mTarget) {
        mTarget->update();
    }
}

/*!
\internal
\reimp
*/
void HbInputContextProxy::widgetDestroyed(QWidget* widget)
{
    if (mTarget) {
        mTarget->widgetDestroyed(widget);
    }
}

/*!
\internal
\reimp
*/
void HbInputContextProxy::setFocusWidget(QWidget* widget)
{
    if (mTarget) {
        mTarget->setFocusWidget(widget);
    }
}

// End of file
