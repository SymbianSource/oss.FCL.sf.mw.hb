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

#include <QWidget>
#include <QGraphicsWidget>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QTextEdit>

#include "hbwidget.h"

void HbEditorInterfacePrivate::lock()
{
    mMutex.lock();
}

void HbEditorInterfacePrivate::unlock()
{
    mMutex.unlock();
}

bool HbEditorInterfacePrivate::hasInterface(HbEditorInterface *toBeChecked) const
{
    foreach(HbEditorInterface *iFace, mAttachedInterfaces) {
        if (iFace == toBeChecked) {
            return true;
        }
    }

    return false;
}

void HbEditorInterfacePrivate::setInputMethodHints(Qt::InputMethodHints hints)
{
    if (mHostEditor) {
        QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject *>(mHostEditor);
        if (graphicsObject) {
            graphicsObject->setInputMethodHints(hints);
        } else {
            QWidget *widget = qobject_cast<QWidget *>(mHostEditor);
            if (widget) {
                widget->setInputMethodHints(hints);
            }
        }
    }
}

Qt::InputMethodHints HbEditorInterfacePrivate::inputMethodHints() const
{
    if (mHostEditor) {
        QGraphicsObject *graphicsObject = qobject_cast<QGraphicsObject *>(mHostEditor);
        if (graphicsObject) {
            return graphicsObject->inputMethodHints();
        } else {
            QWidget *widget = qobject_cast<QWidget *>(mHostEditor);
            if (widget) {
                return widget->inputMethodHints();
            }
        }
    }

    return Qt::ImhNone;
}

void HbEditorInterfacePrivate::notifyCursorPositionChange(int oldPos, int newPos)
{
    emit cursorPositionChanged(oldPos, newPos);
}

void HbEditorInterfacePrivate::cursorPositionChanged()
{
    notifyCursorPositionChange();
}

//
// HbEditorInterfaceCache
//

HbEditorInterfacePrivateCache *HbEditorInterfacePrivateCache::instance()
{
    static HbEditorInterfacePrivateCache myCache;
    return &myCache;
}

HbEditorInterfacePrivateCache::HbEditorInterfacePrivateCache()
{
}

HbEditorInterfacePrivateCache::~HbEditorInterfacePrivateCache()
{
    for (int i = 0; i < mObjectCache.count(); i++) {
        delete mObjectCache[i];
    }
}

HbEditorInterfacePrivate *HbEditorInterfacePrivateCache::attachEditor(QObject *incomingEditor, HbEditorInterface *interface)
{
    QObject *editor = incomingEditor;
    QGraphicsProxyWidget *proxyTmp = qobject_cast<QGraphicsProxyWidget*>(editor);
    if (proxyTmp) {
        editor = proxyTmp->widget();
    }

    if (editor) {
        for (int i = 0; i < mObjectCache.count(); i++) {
            if (mObjectCache[i]->mHostEditor == editor) {
                if (!mObjectCache[i]->hasInterface(interface)) {
                    mObjectCache[i]->mAttachedInterfaces.append(interface);
                    connect(interface, SIGNAL(destroyed(QObject *)), this, SLOT(interfaceDestroyed(QObject *)));
                    connect(mObjectCache[i], SIGNAL(cursorPositionChanged(int, int)), interface, SIGNAL(cursorPositionChanged(int, int)));
                }
                return mObjectCache[i];
            }
        }

        HbEditorInterfacePrivate *newItem = new HbEditorInterfacePrivate();
        newItem->mHostEditor = editor;
        newItem->mAttachedInterfaces.append(interface);
        mObjectCache.append(newItem);

        connect(editor, SIGNAL(destroyed(QObject *)), this, SLOT(destroyed(QObject *)));
        connect(interface, SIGNAL(destroyed(QObject *)), this, SLOT(interfaceDestroyed(QObject *)));
        connect(newItem, SIGNAL(cursorPositionChanged(int, int)), interface, SIGNAL(cursorPositionChanged(int, int)));

        // Check whether the connected object is Hb editor, QLineEdit or QTextEdit,
        // and if yes, connect its cursorPositionChanged() signal to this item's cursorPositionChanged() signal
        HbWidget *hbWidget = qobject_cast<HbWidget*>(editor);
        if (hbWidget && (hbWidget->flags() & QGraphicsItem::ItemAcceptsInputMethod)) {
            // We connect blindly for now without knowing whether it actually implements the signal.
            connect(editor, SIGNAL(cursorPositionChanged(int, int)), newItem, SIGNAL(cursorPositionChanged(int, int)));
        } else {
            QLineEdit *lineedit = qobject_cast<QLineEdit *>(editor);
            if (lineedit) {
                connect(lineedit, SIGNAL(cursorPositionChanged(int, int)), newItem, SIGNAL(cursorPositionChanged(int, int)));
            } else {
                QTextEdit *textedit = qobject_cast<QTextEdit *>(editor);
                if (textedit) {
                    connect(textedit, SIGNAL(cursorPositionChanged()), newItem, SLOT(cursorPositionChanged()));
                }
            }
        }

        return newItem;
    }

    return 0;
}

void HbEditorInterfacePrivateCache::destroyed(QObject *incomingObject)
{
    QObject *object = incomingObject;
    QGraphicsProxyWidget *proxyTmp = qobject_cast<QGraphicsProxyWidget*>(object);
    if (proxyTmp) {
        object = proxyTmp->widget();
    }

    for (int i = 0; i < mObjectCache.count(); i++) {
        if (mObjectCache[i]->mHostEditor == object) {
            delete mObjectCache[i];
            mObjectCache.removeAt(i);
            break;
        }
    }
}

void HbEditorInterfacePrivateCache::interfaceDestroyed(QObject *object)
{
    for (int i = 0; i < mObjectCache.count(); i++) {
        for (int j = 0; j < mObjectCache[i]->mAttachedInterfaces.count(); j++) {
            if (mObjectCache[i]->mAttachedInterfaces[j] == object) {
                mObjectCache[i]->mAttachedInterfaces.removeAt(j);
                return;
            }
        }
    }
}

void HbEditorInterfacePrivateCache::actionDestroyed(QObject *object)
{
    foreach(HbEditorInterfacePrivate *editorInterfacePrivate, mObjectCache) {
        HbAction *action = static_cast<HbAction *>(object);
        if (editorInterfacePrivate->mActions.contains(action)) {
            editorInterfacePrivate->mActions.removeAll(action);
            foreach(HbEditorInterface *editorInterface, editorInterfacePrivate->mAttachedInterfaces) {
                editorInterface->backendModified();
            }
        }
    }
}

void HbEditorInterfacePrivateCache::notifyValueChanged(QObject *editor)
{
    for (int i = 0; i < mObjectCache.count(); i++) {
        if (mObjectCache[i]->mHostEditor == editor) {
            for (int j = 0; j < mObjectCache[i]->mAttachedInterfaces.count(); j++) {
                mObjectCache[i]->mAttachedInterfaces[j]->backendModified();
            }
            break;
        }
    }
}

bool HbEditorInterfacePrivateCache::isConnected(QObject *incomingObject)
{
    QObject *object = incomingObject;
    QGraphicsProxyWidget *proxyTmp = qobject_cast<QGraphicsProxyWidget*>(object);
    if (proxyTmp) {
        object = proxyTmp->widget();
    }

    for (int i = 0; i < mObjectCache.count(); i++) {
        if (mObjectCache[i]->mHostEditor == object) {
            return true;
        }
    }

    return false;
}

// End of file

