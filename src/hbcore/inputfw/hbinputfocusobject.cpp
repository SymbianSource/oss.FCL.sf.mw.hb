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
#include "hbinputfocusobject.h"
#include "hbinputfocusobject_p.h"

#include <QInputMethodEvent>
#include <QGraphicsObject>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QTextEdit>

#include "hbinputmethod.h"
#include "hbinputvkbhost.h"
#include "hbinputstandardfilters.h"
#include "hbdeviceprofile.h"
#include "hbnamespace_p.h"
#include "hbmainwindow.h"
#include "hbevent.h"
#include "hbwidget.h"
#include "hbinputmainwindow_p.h"
#include "hbpopup.h"
#include "hbpopup_p.h"

/*!
@stable
@hbcore
\class HbInputFocusObject
\brief A helper class for accessing editor widget in abstract way.

This class is input method side API for accessing editor widgets. It hides
the details of performing editor related operations, such as sending input
method events, querying editor geometry, querying editor attributes and so on.
It implements a collection of convenience methods for most commonly used
editor operations.

This class is purely a convenience or helper type of class in nature. Everything
it does, can be done directly in input method code as well. The benefit from using
this class is that an input method implementation doesn't need to care whether
the focused editor is QWidget or QGraphicsWidget based (or proxied QWidget).

Application developers typically do not need this class, it is for input method
developers only.

The active focus object can be accessed through HbInputMethod::focusObject()
method.

\sa HbEditorInterface
\sa HbInputMethod
*/

/// @cond

/*!
\internal
Returns main window in case the editor is QGraphicsObject based and lives
inside HbGraphicsScene.
*/
HbMainWindow *HbInputFocusObjectPrivate::mainWindow() const
{
    QGraphicsObject *graphicsObject = 0;

    // check for graphics view related widgets.
    if (mWidget) {
        if (mWidget->graphicsProxyWidget()) {
            graphicsObject = mWidget->graphicsProxyWidget();
        } else {
            return HbInputMainWindow::instance();
        }
    } else {
        graphicsObject = mGraphicsObject;
    }

    if (graphicsObject) {
        if (graphicsObject->scene()) {
            QList<QGraphicsView*> views = graphicsObject->scene()->views();
            foreach (QGraphicsView *view, views) {
                HbMainWindow *mainWindow = qobject_cast<HbMainWindow*>(view);
                if (mainWindow) {
                    return mainWindow;
                }
            }
            // not a HbMainWindow.
            return HbInputMainWindow::instance();
        }
    }

    return 0;
}

/*!
\internal
Ensures cursor visibility for known editor types.
*/
void HbInputFocusObjectPrivate::ensureCursorVisible(QObject *widget)
{
    if (widget) {
        QTextEdit *textEdit = qobject_cast<QTextEdit *>(widget);
        if (textEdit) {
            textEdit->ensureCursorVisible();
        }
    }
}

/// @endcond

HbInputFocusObject::HbInputFocusObject(QObject *focusedObject)
    : d_ptr(new HbInputFocusObjectPrivate(focusedObject))
{
    Q_D(HbInputFocusObject);
    d->q_ptr = this;

    if (focusedObject) {
        if (focusedObject->isWidgetType()) {
            d->mWidget = qobject_cast<QWidget*>(focusedObject);
        } else {
            QGraphicsProxyWidget *proxy = qobject_cast<QGraphicsProxyWidget*>(focusedObject);
            if (proxy) {
                d->mWidget = proxy->widget();
            } else {
                d->mGraphicsObject = qobject_cast<QGraphicsObject*>(focusedObject);
            }
        }

        HbEvent *event = new HbEvent(HbEvent::InputMethodFocusIn);
        QCoreApplication::sendEvent(focusedObject, event);
        delete event;

        HbMainWindow *mainWindow = d->mainWindow();
        if (mainWindow) {
            connect(mainWindow, SIGNAL(aboutToChangeOrientation()), this, SIGNAL(aboutToChangeOrientation()));
            connect(mainWindow, SIGNAL(orientationChanged(Qt::Orientation)), this, SIGNAL(orientationChanged()));
        }
    }
}

HbInputFocusObject::~HbInputFocusObject()
{
    QObject *obj = object();
    if (obj) {
        HbEvent *event = new HbEvent(HbEvent::InputMethodFocusOut);
        QCoreApplication::postEvent(obj, event);
    }

    delete d_ptr;
}

/*!
Creates an input method event where given string is a pre-edit string and sends
it to focused editor. See QInputMethodEvent for more information on pre-edit strings.
*/
void HbInputFocusObject::sendPreEditString(const QString &string)
{
    QList<QInputMethodEvent::Attribute> list;
    QInputMethodEvent event(string, list);
    sendEvent(event);
}

/*!
Creates an input method event where given string is a commit string and sends
it to focused editor. See QInputMethodEvent for more information on commit strings.
*/
void HbInputFocusObject::sendCommitString(const QString &string)
{
    QList<QInputMethodEvent::Attribute> list;
    QInputMethodEvent event(QString(), list);
    event.setCommitString(string);
    sendEvent(event);
}

/*!
Sends given event to focused editor.
*/
void HbInputFocusObject::sendEvent(QEvent &event)
{
    Q_D(HbInputFocusObject);

    if (event.type() == QEvent::InputMethod) {
        QInputMethodEvent *imEvent = static_cast<QInputMethodEvent *>(&event);
        if (imEvent->commitString().size() > 0) {
            d->mPreEditString.clear();
        } else {
            d->mPreEditString = imEvent->preeditString();
        }
    }

    QObject *obj = object();
    if (obj) {
        if (event.type() == QEvent::InputMethod) {
            QInputContext *ic = qApp->inputContext();
            QInputMethodEvent *imEvent = static_cast<QInputMethodEvent *>(&event);
            if (ic) {
                ic->sendEvent(*imEvent);
            }
            // Currently in Qt, QTextEdit doesn't ensure cursor visibility
            // in case we are sending text in the form of QInputMethodEvent. So we need
            // to call QTextEdit:ensureCursorVisible() here till we get a fix from Qt.
            d->ensureCursorVisible(obj);
        } else {
            QInputContext *ic = qApp->inputContext();
            if (ic && ic->focusWidget()) {
                QApplication::sendEvent(ic->focusWidget(), &event);
            }
        }
    }
}

/*!
Posts given event to focused editor in an asynchronous manner.
*/
void HbInputFocusObject::postEvent(QEvent &event)
{
    Q_D(HbInputFocusObject);

    if (event.type() == QEvent::InputMethod) {
        QInputMethodEvent *imEvent = static_cast<QInputMethodEvent *>(&event);
        if (imEvent->commitString().size() > 0) {
            d->mPreEditString.clear();
        } else {
            d->mPreEditString = imEvent->preeditString();
        }
    }

    QObject *obj = object();
    if (obj) {
        QApplication::postEvent(obj, &event);
    }
}

/*!
Passes input method query to focused editor widget.
*/
QVariant HbInputFocusObject::inputMethodQuery(Qt::InputMethodQuery query) const
{
    Q_D(const HbInputFocusObject);

    QGraphicsObject *graphicsObject = d->mGraphicsObject;
    if (graphicsObject) {
        if (graphicsObject->scene()) {
            return graphicsObject->scene()->inputMethodQuery(query);
        }

        return QVariant();
    }

    QWidget *widget = d->mWidget;
    if (widget) {
        // QWidget returns microfocus in local coordinate.
        // we need to map it to global coordinate.
        QVariant v = widget->inputMethodQuery(query);
        if (v.type() == QVariant::Rect) {
            v = v.toRect().translated(widget->mapToGlobal(QPoint(0, 0)));
        }
        return v;
    }

    return QVariant();
}

/*!
Returns editor cursor position by sending Qt::ImCursorPosition event to it.
*/
int HbInputFocusObject::editorCursorPosition()
{
    return inputMethodQuery(Qt::ImCursorPosition).toInt();
}

/*!
Returns editor's font by sending Qt::ImFont input method query to it.
*/
QFont HbInputFocusObject::editorFont()
{
    return inputMethodQuery(Qt::ImFont).value<QFont>();
}

/*!
Returns text selection by sending Qt::ImCurrentTextSelection to editor.
*/
QString HbInputFocusObject::editorTextSelection()
{
    return inputMethodQuery(Qt::ImCurrentSelection).toString();
}

/*!
Returns text surrounding the editor cursor position by sending Qt::ImSurroundingText event to editor.
*/
QString HbInputFocusObject::editorSurroundingText()
{
    return inputMethodQuery(Qt::ImSurroundingText).toString();
}

/*!
Returns editor interface object pointing to focused editor.
*/
HbEditorInterface &HbInputFocusObject::editorInterface() const
{
    return d_ptr->mEditorInterface;
}

/*!
Sends left arrow key press to focused editor.
*/
void HbInputFocusObject::cursorLeft(Qt::KeyboardModifiers modifiers)
{
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Left, modifiers);
    sendEvent(keyEvent);
}

/*!
Sends right arrow key press to focused editor.
*/
void HbInputFocusObject::cursorRight(Qt::KeyboardModifiers modifiers)
{
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Right, modifiers);
    sendEvent(keyEvent);
}

/*!
Removes focus from the editor.
*/
void HbInputFocusObject::releaseFocus()
{
    Q_D(HbInputFocusObject);

    QGraphicsObject *graphicsObject = d->mGraphicsObject;
    if (!graphicsObject) {
        QWidget *widget = d->mWidget;
        if (widget) {
            if (widget->graphicsProxyWidget()) {
                graphicsObject = widget->graphicsProxyWidget();
            } else {
                widget->clearFocus();
                return;
            }
        }
    }

    if (graphicsObject && graphicsObject->scene()) {
        graphicsObject->scene()->setFocusItem(0);
    }
}

/*!
Runs the given character through active input filter and commits it if it was accepted.
Returns true if the character was accepted.
*/
bool HbInputFocusObject::filterAndCommitCharacter(QChar character)
{
    // Two pass filtering because this may be a case constrained editor
    // with a filter.
    Qt::InputMethodHints hints = inputMethodHints();
    if (hints & Qt::ImhLowercaseOnly) {
        if (!HbInputLowerCaseFilter::instance()->filter(character)) {
            return false;
        }
    } else if (hints & Qt::ImhUppercaseOnly) {
        if (!HbInputUpperCaseFilter::instance()->filter(character)) {
            return false;
        }
    }

    HbInputFilter *filter = editorInterface().filter();
    if (filter) {
        if (!filter->filter(character)) {
            return false;
        }
    }

    QString cString;
    cString.append(character);
    sendCommitString(cString);

    return true;
}

/*!
Returns editor widget geometry. In case of QGraphicsObject, the returned value is in scene coordinates.
*/
QRectF HbInputFocusObject::editorGeometry() const
{
    Q_D(const HbInputFocusObject);

    QGraphicsObject *graphicsObject = d->mGraphicsObject;
    if (!graphicsObject) {
        QWidget *widget = d->mWidget;
        if (widget) {
            // Check if widget is inside a proxy.
            QGraphicsProxyWidget *pw = widget->graphicsProxyWidget();
            if (pw) {
                graphicsObject = pw;
            } else {
                return QRectF(widget->mapToGlobal(QPoint(0, 0)), widget->size());
            }
        }
    }

    if (graphicsObject) {
        return graphicsObject->sceneBoundingRect();
    }

    return QRectF();
}

/*!
Returns cursor micro focus by sending Qt::ImMicroFocus to focused editor.
In case of QGraphicsObject, the returned rectangle is in scene coordinates.
*/
QRectF HbInputFocusObject::microFocus() const
{
    Q_D(const HbInputFocusObject);

    QRectF rect = inputMethodQuery(Qt::ImMicroFocus).toRectF();
    QGraphicsObject *editorWidget = d->mGraphicsObject;
    if (editorWidget) {
        rect = editorWidget->mapRectToScene(rect);
    }

    return rect;
}

/*!
Returns active pre-edit string. Note that this method works only if the pre-edit
string was set by using this class.
*/
QString HbInputFocusObject::preEditString() const
{
    Q_D(const HbInputFocusObject);
    return d->mPreEditString;
}

/*!
Returns the Z-value that should be used with virtual keyboard widget. Usually only HbVkbHost
needs this value.
*/
qreal HbInputFocusObject::findVkbZValue() const
{
    Q_D(const HbInputFocusObject);

    QGraphicsObject *editorWidget = d->mGraphicsObject;
    if (!editorWidget) {
        QWidget *widget = d->mWidget;
        if (widget) {
            editorWidget = widget->graphicsProxyWidget();
        }
    }

    if (editorWidget) {
        qreal result = editorWidget->zValue();
        for (QGraphicsObject *parent = editorWidget->parentObject(); parent; parent = parent->parentObject()) {
            result += parent->zValue();
        }
        result += HbPrivate::VKBValueUnit;
        if (result >= 0) {
            return result;
        }
    }

    return 0.0;
}

/*!
Returns a priority value for the editor if the editor is inside a popup and
in other cases zero is returned.
*/
quint8 HbInputFocusObject::editorPriority() const
{
    Q_D(const HbInputFocusObject);

    QGraphicsObject *editorWidget = d->mGraphicsObject;
    if (!editorWidget) {
        QWidget *widget = d->mWidget;
        if (widget) {
            editorWidget = widget->graphicsProxyWidget();
        }
    }

    if (editorWidget) {
        // Check if the editor is inside a popup and if so return the popup priority
        for (QGraphicsObject *object = editorWidget; object; object = object->parentObject()) {
            HbPopup *popup = qobject_cast<HbPopup *>(object);
            if (popup) {
                return HbPopupPrivate::d_ptr(popup)->priority();
            }
        }
    }

    return 0;
}

/*!
Returns input method hints. See QWidget and QGraphicsItem documentation for more information.
*/
Qt::InputMethodHints HbInputFocusObject::inputMethodHints() const
{
    Q_D(const HbInputFocusObject);

    QGraphicsObject *graphicsObject = d->mGraphicsObject;
    if (graphicsObject) {
        return graphicsObject->inputMethodHints();
    }

    QWidget *widget = d->mWidget;
    if (widget) {
        return widget->inputMethodHints();
    }

    return Qt::ImhNone;
}

/*!
Sets input method hints. See QWidget and QGraphicsWidget documentation for more information.
*/
void HbInputFocusObject::setInputMethodHints(Qt::InputMethodHints hints)
{
    Q_D(HbInputFocusObject);

    QGraphicsObject *graphicsObject = d->mGraphicsObject;
    if (graphicsObject) {
        graphicsObject->setInputMethodHints(hints);
        return;
    }

    QWidget *widget = d->mWidget;
    if (widget) {
        widget->setInputMethodHints(hints);
    }
}

/*!
A convenience method for filtering strings. Uses filter attached to connected editor
and filters given string with it.
*/
void HbInputFocusObject::filterStringWithEditorFilter(const QString &source, QString &result)
{
    QString intermediate = source;

    // Chained two-pass filtering because this can be case-constrained editor with a filter.
    Qt::InputMethodHints hints = inputMethodHints();
    if (hints & Qt::ImhLowercaseOnly) {
        intermediate.clear();
        HbInputLowerCaseFilter::instance()->filterString(source, intermediate);
    } else if (hints & Qt::ImhUppercaseOnly) {
        intermediate.clear();
        HbInputUpperCaseFilter::instance()->filterString(source, intermediate);
    }

    HbInputFilter *filter = editorInterface().filter();
    if (filter) {
        filter->filterString(intermediate, result);
        return;
    }

    result = intermediate;
}

/*!
Returns true if given character is allowed in active editor.
*/
bool HbInputFocusObject::characterAllowedInEditor(QChar character) const
{
    // Two pass filtering, this can be case constrained editor with a filter.
    Qt::InputMethodHints hints = inputMethodHints();
    if (hints & Qt::ImhLowercaseOnly) {
        if (HbInputLowerCaseFilter::instance()->filter(character) == false) {
            return false;
        }
    } else if (hints & Qt::ImhUppercaseOnly) {
        if (HbInputUpperCaseFilter::instance()->filter(character) == false) {
            return false;
        }
    }

    HbInputFilter *filter = editorInterface().filter();
    if (filter) {
        return filter->filter(character);
    }

    return true;
}

/*!
Returns the scenePos of the associated editor widget, if the concept makes sense
in its context (i.e. the editor is part of a scene, either being a QGraphicsObject or
a QWidget embedded in a QGraphicsProxyWidget). Otherwise returns QPointF(0.0, 0.0).
*/
QPointF HbInputFocusObject::scenePos() const
{
    Q_D(const HbInputFocusObject);

    QGraphicsObject *graphicsObject = d->mGraphicsObject;
    if (graphicsObject) {
        return graphicsObject->scenePos();
    }

    QWidget *widget = d->mWidget;
    if (widget) {
        QGraphicsProxyWidget *proxy = widget->graphicsProxyWidget();
        if (proxy) {
            return proxy->scenePos();
        }
        return widget->mapToGlobal(QPoint(0, 0));
    }

    return QPointF(0.0, 0.0);
}

/*!
Returns true if all the characters in given string are allowed in active editor.
*/
bool HbInputFocusObject::stringAllowedInEditor(const QString &string) const
{
    // Two pass filtering. This can be a case constrained editor with a filter.
    Qt::InputMethodHints hints = inputMethodHints();
    if (hints & Qt::ImhLowercaseOnly) {
        QString outStr;
        HbInputLowerCaseFilter::instance()->filterString(string, outStr);
        if (string != outStr) {
            return false;
        }
    } else if (hints & Qt::ImhUppercaseOnly) {
        QString outStr;
        HbInputUpperCaseFilter::instance()->filterString(string, outStr);
        if (string != outStr) {
            return false;
        }
    }

    HbInputFilter *filter = editorInterface().filter();
    if (filter) {
        QString outStr;
        filter->filterString(string, outStr);
        return string == outStr;
    }

    return true;
}

/*!
Commits given smiley.
*/
void HbInputFocusObject::commitSmiley(QString smiley)
{
    QObject *obj = object();
    if (obj) {
        obj->setProperty("SmileyIcon", smiley);
    }
}

/*!
Returns the editor widget as QObject.
*/
QObject *HbInputFocusObject::object() const
{
    Q_D(const HbInputFocusObject);

    if (d->mGraphicsObject) {
        return d->mGraphicsObject.data();
    }

    return d->mWidget.data();
}

/*!
Returns true if widget is read-only widget. This works
only for known editor types.
*/
bool HbInputFocusObject::isReadOnlyWidget(QObject *editorObject)
{
    if (editorObject) {
        QWidget *widget = qobject_cast<QWidget *>(editorObject);
        if (widget) {
            if (!widget->testAttribute(Qt::WA_InputMethodEnabled)) {
                return true;
            }

            QLineEdit *lineEdit = qobject_cast<QLineEdit *>(widget);
            if (lineEdit) {
                return lineEdit->isReadOnly();
            }

            QTextEdit *textEdit = qobject_cast<QTextEdit *>(widget);
            if (textEdit) {
                return textEdit->isReadOnly();
            }

            return false;
        } else {
            QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject *>(editorObject);
            if (graphicsObject) {
                if (!(graphicsObject->flags() & QGraphicsItem::ItemAcceptsInputMethod)) {
                    return true;
                }
            }

            return false;
        }
    }

    return true;
}
/*!
Returns true if the input framework recognizes given object as editor.
*/
bool HbInputFocusObject::isEditor(QObject *object)
{
    if (QWidget *w = qobject_cast<QWidget *>(object)) {
        if (w->testAttribute(Qt::WA_InputMethodEnabled)) {
            return true;
        }
    }

    if (QGraphicsObject *gw = qobject_cast<QGraphicsObject *>(object)) {
        if (gw->flags() & QGraphicsItem::ItemAcceptsInputMethod) {
            return true;
        }
    }

    return false;
}

/*!
Sets focus to the editor pointed by this focus object. Sometimes
input method does something that temporarily removes focus from the original editor,
for example displays a dialog which itself contains an editor in it. This method can
be used to return the input focus to the original editor.
*/
void HbInputFocusObject::setFocus()
{
    Q_D(HbInputFocusObject);

    bool sendRequest = false;
    QGraphicsObject *graphicsObject = d->mGraphicsObject;
    if (graphicsObject && graphicsObject->scene()) {
        graphicsObject->scene()->setFocusItem(graphicsObject);
        sendRequest = true;
    } else {
        QWidget *widget = d->mWidget;
        if (widget) {
            widget->setFocus();
            sendRequest = true;
        }
    }

    if (sendRequest) {
        QInputContext* ic = qApp->inputContext();
        if (ic) {
            QEvent *openEvent = new QEvent(QEvent::RequestSoftwareInputPanel);
            ic->filterEvent(openEvent);
            delete openEvent;
        }
    }
}

/*!
Returns the screen orientation in editor widget's context. For widgets
living in HbMainWindow, it is main window's orientation. For everything
else, it is device profile's screen orientation.
*/
Qt::Orientation HbInputFocusObject::orientation() const
{
    Q_D(const HbInputFocusObject);

    HbMainWindow *mainWindow = d->mainWindow();
    if (mainWindow) {
        return mainWindow->orientation();
    }

    return HbDeviceProfile::current().orientation();
}

/*!
Returns true if focused object is visible.
*/
bool HbInputFocusObject::isVisible() const
{
    Q_D(const HbInputFocusObject);

    if (d->mGraphicsObject) {
        return d->mGraphicsObject.data()->isVisible();
    }

    return d->mWidget.data()->isVisible();
}

#include "moc_hbinputfocusobject.cpp"

// End of file
