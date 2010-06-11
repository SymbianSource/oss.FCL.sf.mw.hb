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
#include <QInputMethodEvent>
#include <QGraphicsWidget>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QPointer>
#include <QGraphicsView>

#include "hbinputmethod.h"
#include "hbinputfocusobject.h"
#include "hbinputeditorinterface.h"
#include "hbinputvkbhost.h"
#include "hbinputstandardfilters.h"
#include "hbinpututils.h"
#include "hbnamespace_p.h"



/*!
@alpha
@hbcore
\class HbInputFocusObject
\brief A helper class for accessing editor widget in abstract way.

This class is input method side API for accessing editor widgets. It was added because
in some cases Qt's QInputMethodEvent/inputMethodQuery system is not enough and direct
access via type casting between QWidget and QGraphiscWidget based editors is needed.
Focus object hides those cases behind a convinience API.

This class is purely a convenience or helper type of class in nature. Everything
it does, can be done directly in input method code as well. It just wraps
most commonly used operations behind one API to avoid duplicate code.

Application developers should never need to use this class, it is for input method developers only.

\sa HbEditorInterface
*/

/// @cond

/*
This function ensures cursor visibility for known editor types.
*/
void ensureCursorVisible(QObject *widget)
{
    if (widget) {
        QTextEdit *textEdit = qobject_cast<QTextEdit*>(widget);
        if (textEdit) {
            textEdit->ensureCursorVisible();
        }
    }
}


class HbInputFocusObjectPrivate
{
public:
    HbInputFocusObjectPrivate(QObject *focusedObject)
        : mFocusedObject(focusedObject),
          mEditorInterface(focusedObject)
    {}

public:
    QPointer<QObject> mFocusedObject;
    HbEditorInterface mEditorInterface;
    QString mPreEditString;
};


/// @endcond

HbInputFocusObject::HbInputFocusObject(QObject *focusedObject)
    : d_ptr(new HbInputFocusObjectPrivate(focusedObject))
{
}


HbInputFocusObject::~HbInputFocusObject()
{
    delete d_ptr;
}

/*!
Creates an input method event where given string is a pre-edit string and sends
it to focused editor. See QInputMethodEvent for more information on pre-edit strings.
*/
void HbInputFocusObject::sendPreEditString(const QString& string)
{
    QList<QInputMethodEvent::Attribute> list;
    QInputMethodEvent event(string, list);
    sendEvent(event);
}

/*!
Creates an input method event where given string is a commit string and sends
it to focused editor. See QInputMethodEvent for more information on commit strings.
*/
void HbInputFocusObject::sendCommitString(const QString& string)
{
    QList<QInputMethodEvent::Attribute> list;
    QInputMethodEvent event(QString(), list);
    event.setCommitString(string);
    sendEvent(event);
}

/*!
Sends given event to focused editor.
*/
void HbInputFocusObject::sendEvent(QEvent& event)
{
    Q_D(HbInputFocusObject);

    if (event.type() == QEvent::InputMethod) {
        QInputMethodEvent* imEvent = static_cast<QInputMethodEvent*>(&event);
        if (imEvent->commitString().size() > 0) {
            d->mPreEditString = QString();
        } else {
            d->mPreEditString = imEvent->preeditString();
        }
    }

    if (d->mFocusedObject) {
        if (event.type() == QEvent::InputMethod) {
            QInputContext *ic = qApp->inputContext();
            QInputMethodEvent* imEvent = static_cast<QInputMethodEvent*>(&event);
            if (ic) {
                ic->sendEvent(*imEvent);
            }
            // Currently in Qt, QTextEdit doesn't ensure cursor visibility
            // in case we are sending text in the form of QInputMethodEvent. So we need
            // to call QTextEdit:ensureCursorVisible() here till we get a fix from Qt.
            ensureCursorVisible(d->mFocusedObject);
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
void HbInputFocusObject::postEvent(QEvent& event)
{
    Q_D(HbInputFocusObject);

    if (event.type() == QEvent::InputMethod) {
        QInputMethodEvent* imEvent = static_cast<QInputMethodEvent*>(&event);
        if (imEvent->commitString().size() > 0) {
            d->mPreEditString = QString();
        } else {
            d->mPreEditString = imEvent->preeditString();
        }
    }

    if (d->mFocusedObject) {
        QApplication::postEvent(d->mFocusedObject, &event);
    }
}

/*!
Passes input method query to focused editor.
*/
QVariant HbInputFocusObject::inputMethodQuery(Qt::InputMethodQuery query) const
{
    Q_D(const HbInputFocusObject);

    QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(d->mFocusedObject);
    if (graphicsObject && graphicsObject->scene()) {
        return graphicsObject->scene()->inputMethodQuery(query);
    }

    // check if QWidget is embedded as a proxy in scene. If yes try to get details
    // from the scene.
    QWidget *widget = qobject_cast<QWidget*>(d->mFocusedObject);
    QGraphicsProxyWidget *pw = HbInputUtils::graphicsProxyWidget(widget);
    if (pw && pw->scene()) {
        return pw->scene()->inputMethodQuery(query);
    }
    
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
HbEditorInterface& HbInputFocusObject::editorInterface() const
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

    QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(d->mFocusedObject);
    if (!graphicsObject) {
        QWidget *widget = qobject_cast<QWidget*>(d->mFocusedObject);
        if (widget) {
            if (!(graphicsObject = HbInputUtils::graphicsProxyWidget(widget))) {
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

    QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(d->mFocusedObject);
    if (!graphicsObject) {
        QWidget *widget = qobject_cast<QWidget*>(d->mFocusedObject);
        if (widget) {
            // check if widget is inside a proxy.
            QGraphicsProxyWidget *pw = HbInputUtils::graphicsProxyWidget(widget);
            if (pw) {
                // check if we are pointing to the toplevel
                // proxy widget, if not then we must check for
                // the widgets window and see if it is a proxy.
                if (pw->widget() == widget) {
                    graphicsObject = pw;
                } else if (pw->widget() == widget->window()) {
                    // focused object is not a proxy but it is 
                    // inside a proxy, query to proxy about
                    // the focused objects rect.
                    QRectF rect = pw->subWidgetRect(widget);
                    rect.translate(pw->scenePos());
                    return rect;
                }
            } else {
                return QRectF(widget->mapToGlobal(QPoint(0, 0)), widget->size());
            }
        }
    }

    // we need to find the editor which is inside 
    if (graphicsObject) {
        return QRectF(graphicsObject->scenePos(), graphicsObject->boundingRect().size());
    }

    return QRectF();
}

/*!
Returns cursor micro focus by sending Qt::ImMicroFocus to focused editor.
In case of QGraphicsObject, the returned rectangle is in scene coordinates.
*/
QRectF HbInputFocusObject::microFocus() const
{
    return inputMethodQuery(Qt::ImMicroFocus).toRectF();
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

    QGraphicsObject *editorWidget = qobject_cast<QGraphicsObject*>(d->mFocusedObject);
    if (!editorWidget) {
        QWidget *widget = qobject_cast<QWidget*>(d->mFocusedObject);
        if (widget) {
            editorWidget = HbInputUtils::graphicsProxyWidget(widget);
        }
    }

    if (editorWidget) {
        qreal result = editorWidget->zValue();
        for (QGraphicsObject *parent = editorWidget->parentObject(); parent; parent = parent->parentObject()) {
            result += parent->zValue();
        }
        result += HbPrivate::VKBValueUnit;
        if(result >= 0) {
            return result;
        }
    }

    return 0.0;
}

/*!
Returns input method hints. See QWidget and QGraphicsItem documentation for more information.
*/
Qt::InputMethodHints HbInputFocusObject::inputMethodHints() const
{
    Q_D(const HbInputFocusObject);

    QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(d->mFocusedObject);
    if (graphicsObject) {
        return graphicsObject->inputMethodHints();
    }

    QWidget *widget = qobject_cast<QWidget*>(d->mFocusedObject);
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

    QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(d->mFocusedObject);
    if (graphicsObject) {
        graphicsObject->setInputMethodHints(hints);
        return;
    }

    QWidget *widget = qobject_cast<QWidget*>(d->mFocusedObject);
    if (widget) {
        widget->setInputMethodHints(hints);
    }
}

/*!
A convenience method for filtering strings. Uses filter attached to connected editor
and filters given string with it.
*/
void HbInputFocusObject::filterStringWithEditorFilter(const QString& source, QString& result)
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

    QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(d->mFocusedObject);
    if (graphicsObject) {
        return graphicsObject->scenePos();
    }

    QWidget *w = qobject_cast<QWidget*>(d->mFocusedObject);
    // check if widget is inside a proxy.
    QGraphicsProxyWidget *pw = HbInputUtils::graphicsProxyWidget(w);
    if (pw) {
        // check if we are pointing to the toplevel
        // proxy widget, if not then we must check for
        // the widgets window and see if it is a proxy.
        if (pw->widget() == w) {
            return pw->scenePos();
        } else if (pw->widget() == w->window()) {
            QRectF rect = pw->subWidgetRect(w);
            rect.translate(pw->scenePos());
            return rect.topLeft();
        }
    }

    if (w) {
        // not a proxy.. Meaning widget is inside a QWidget window.
        return w->mapToGlobal(QPoint(0,0));
    }

    return QPointF(0.0, 0.0);
}

/*!
Returns true if all the characters in given string are allowed in active editor.
*/
bool HbInputFocusObject::stringAllowedInEditor(const QString& string) const
{
    // Two pass filtering. This can be a case constrained editor with a filter.
    Qt::InputMethodHints hints;
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
     Q_D(HbInputFocusObject);

     if (d->mFocusedObject) {
         d->mFocusedObject->setProperty("SmileyIcon", smiley);
     }
}

/*!
Returns the editor widget as QObject.
*/
QObject *HbInputFocusObject::object() const
{
    Q_D(const HbInputFocusObject);
    return d->mFocusedObject;
}

/*!
Returns true if widget is read-only widget. This works
only for known editor types.
*/
bool HbInputFocusObject::isReadOnlyWidget(QObject *editorObject)
{
    if (editorObject) {
        QWidget *widget = qobject_cast<QWidget*>(editorObject);
        if (widget) {
            if (!widget->testAttribute(Qt::WA_InputMethodEnabled)) {
                return true;
            }

            QLineEdit *lineEdit = qobject_cast<QLineEdit*>(widget);
            if (lineEdit) {
                return lineEdit->isReadOnly();
            }

            QTextEdit *textEdit = qobject_cast<QTextEdit*>(widget);
            if (textEdit) {
                return textEdit->isReadOnly();
            }

            return false;
        } else {
            QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(editorObject);
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
Sets focus to the editor point by this focus objetc. This method is needed because sometimes
input method does something that temporarily removes focus from the original editor,
for example displays a dialog which itself contains an editor in it. This method can
be used to return the focus to the original editor.
*/
void HbInputFocusObject::setFocus()
{
    Q_D(HbInputFocusObject);

    QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject*>(d->mFocusedObject);
    if (graphicsObject && graphicsObject->scene()) {
        graphicsObject->scene()->setFocusItem(graphicsObject);
    } else {
        QWidget *widget = qobject_cast<QWidget*>(d->mFocusedObject);
        if (widget) {
            widget->setFocus();
        }
    }
}

// End of file

