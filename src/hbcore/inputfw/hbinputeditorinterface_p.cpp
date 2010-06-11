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
#include <QWidget>
#include <QGraphicsWidget>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QTextEdit>
#include "hbinputeditorinterface_p.h"
#include "hbinputeditorinterface.h"

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
    foreach (HbEditorInterface *iFace, mAttachedInterfaces) {
        if (iFace == toBeChecked) {
            return true;
        }
    }

    return false;
}

void HbEditorInterfacePrivate::setInputMethodHints(Qt::InputMethodHints hints)
{   
    if (mHostEditor) {       
         QGraphicsWidget *graphicsWidget = qobject_cast<QGraphicsWidget*>(mHostEditor); 
         if (graphicsWidget) {               
             graphicsWidget->setInputMethodHints(hints);
         } else {
             QWidget *widget = qobject_cast<QWidget*>(mHostEditor); 
             if (widget) {            
                widget->setInputMethodHints(hints);
             }                   
         } 
    }
}

Qt::InputMethodHints HbEditorInterfacePrivate::inputMethodHints() const
{
   if (mHostEditor) {    
       QGraphicsWidget *graphicsWidget = qobject_cast<QGraphicsWidget*>(mHostEditor); 
       if (graphicsWidget) {              
           return graphicsWidget->inputMethodHints();
       } else {
            QWidget *widget = qobject_cast<QWidget*>(mHostEditor); 
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

HbEditorInterfacePrivateCache* HbEditorInterfacePrivateCache::instance()
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

HbEditorInterfacePrivate* HbEditorInterfacePrivateCache::attachEditor(QObject* editor, HbEditorInterface* interface)
{
    if (editor) {
        for (int i = 0; i < mObjectCache.count(); i++) {
            if (mObjectCache[i]->mHostEditor == editor) {
                if (!mObjectCache[i]->hasInterface(interface)) {
                    mObjectCache[i]->mAttachedInterfaces.append(interface);
                    connect(interface, SIGNAL(destroyed(QObject*)), this, SLOT(interfaceDestroyed(QObject*)));
                    connect(mObjectCache[i], SIGNAL(cursorPositionChanged(int, int)), interface, SIGNAL(cursorPositionChanged(int, int)));
                }
                return mObjectCache[i];
            }
        }

        HbEditorInterfacePrivate* newItem = new HbEditorInterfacePrivate();
        newItem->mHostEditor = editor;
        newItem->mAttachedInterfaces.append(interface);
        mObjectCache.append(newItem);

        connect(editor, SIGNAL(destroyed(QObject*)), this, SLOT(destroyed(QObject*)));
        connect(interface, SIGNAL(destroyed(QObject*)), this, SLOT(interfaceDestroyed(QObject*)));
        connect(newItem, SIGNAL(cursorPositionChanged(int, int)), interface, SIGNAL(cursorPositionChanged(int, int)));

        // Check whether the connected object is a QLineEdit or QTextEdit, plain or wrapped in QGraphicsProxyWidget
        // and if yes, connect its cursorPositionChanged() signal to this item's cursorPositionChanged() signal
        if (editor->inherits("HbAbstractEdit")) {
            connect(editor, SIGNAL(cursorPositionChanged(int, int)), newItem, SIGNAL(cursorPositionChanged(int, int)));
        } else {
            QGraphicsProxyWidget* proxywidget = qobject_cast<QGraphicsProxyWidget*>(editor);
            if (proxywidget) {
                QWidget* editorwidget = proxywidget->widget();
                QLineEdit* lineedit = qobject_cast<QLineEdit*>(editorwidget);
                if (lineedit) {
                connect(lineedit, SIGNAL(cursorPositionChanged(int, int)), newItem, SIGNAL(cursorPositionChanged(int, int)));
                } else {
                    QTextEdit* textedit = qobject_cast<QTextEdit*>(editorwidget);
                    if (textedit) {
                    connect(textedit, SIGNAL(cursorPositionChanged()), newItem, SLOT(cursorPositionChanged()));
                    }
                }
            } else {
                QLineEdit* lineedit = qobject_cast<QLineEdit*>(editor);
                if (lineedit) {
                connect(lineedit, SIGNAL(cursorPositionChanged(int, int)), newItem, SIGNAL(cursorPositionChanged(int, int)));
                } else {
                    QTextEdit* textedit = qobject_cast<QTextEdit*>(editor);
                    if (textedit) {
                    connect(textedit, SIGNAL(cursorPositionChanged()), newItem, SLOT(cursorPositionChanged()));
                    }
                }
            }
        }

        return newItem;
    }

    return 0;
}

void HbEditorInterfacePrivateCache::destroyed(QObject* object)
{
    for (int i = 0; i < mObjectCache.count(); i++) {
        if (mObjectCache[i]->mHostEditor == object) {
            delete mObjectCache[i];
            mObjectCache.remove(i);
            break;
        }
    }
}

void HbEditorInterfacePrivateCache::interfaceDestroyed(QObject* object)
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

void HbEditorInterfacePrivateCache::actionDestroyed(QObject* object)
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

void HbEditorInterfacePrivateCache::notifyValueChanged(QObject* editor)
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

bool HbEditorInterfacePrivateCache::isConnected(QObject* object)
{
    for (int i = 0; i < mObjectCache.count(); i++) {
        if (mObjectCache[i]->mHostEditor == object) {
            return true;
        }
    }

    return false;
}

// End of file

