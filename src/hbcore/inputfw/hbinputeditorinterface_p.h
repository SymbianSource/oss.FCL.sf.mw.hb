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

#ifndef HB_EDITOR_INTERFACE_PRIVATE_H
#define HB_EDITOR_INTERFACE_PRIVATE_H

#include <QObject>
#include <QVector>
#include <QMutex>
#include <QPointer>

#include <hbaction.h>
#include <hbsmileytheme.h>

#include "hbinputdef.h"
#include "hbinputlanguage.h"
#include "hbinputfilter.h"
#include "hbinputstate.h"

/// @cond

class HbEditorInterface;

class HbEditorInterfacePrivate : public QObject
{
    Q_OBJECT

public:
    HbEditorInterfacePrivate()
        : mInputMode(0),
        mTextCase(HbTextCaseNone),
        mConstraints(0),
        mLocalDigitType(HbDigitTypeNone),
        mExtraDictionaryId(0),
        mClass(0),
        mHostEditor(0)
    {
    }

    void lock();
    void unlock();
    bool hasInterface(HbEditorInterface *toBeChecked) const;
    void notifyCursorPositionChange(int oldPos = 0, int newPos = 0);
    void setInputMethodHints(Qt::InputMethodHints hints);
    Qt::InputMethodHints inputMethodHints() const;

public:
    HbInputModeType mInputMode;
    HbTextCase mTextCase;
    HbEditorConstraints mConstraints;
    QPointer<HbInputFilter> mFilter;
    HbInputDigitType mLocalDigitType;
    int mExtraDictionaryId;
    int mClass;
    HbSmileyTheme mSmileyTheme;
    QObject* mHostEditor;
    QList<HbEditorInterface*> mAttachedInterfaces;
    HbInputState mLastFocusedState;
    QList<HbAction*> mActions;

public slots:
    void cursorPositionChanged();

signals:
    void cursorPositionChanged(int oldPos, int newPos);

private:
    QMutex mMutex;
};


class HbEditorInterfacePrivateCache : public QObject
{
    Q_OBJECT

public:
    static HbEditorInterfacePrivateCache* instance();

private:
    HbEditorInterfacePrivateCache();
    ~HbEditorInterfacePrivateCache();

public:
    HbEditorInterfacePrivate* attachEditor(QObject* editor, HbEditorInterface* interface);
    bool isConnected(QObject* object);
    void notifyValueChanged(QObject *editor);

public slots:
    void destroyed(QObject* object);
    void interfaceDestroyed(QObject* object);

private:
    QVector<HbEditorInterfacePrivate*> mObjectCache;
};

/// @endcond

#endif // HB_EDITOR_INTERFACE_PRIVATE_H

// End of file

