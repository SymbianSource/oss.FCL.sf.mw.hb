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

#ifndef HB_INPUT_FOCUS_OBJECT_P_H
#define HB_INPUT_FOCUS_OBJECT_P_H

#include <QString>
#include <QPointer>

#include "hbinputdef.h"
#include "hbinputeditorinterface.h"

class HbInputFocusObject;
class HbMainWindow;
class QGraphicsObject;

class HB_CORE_PRIVATE_EXPORT HbInputFocusObjectPrivate
{
    Q_DECLARE_PUBLIC(HbInputFocusObject)

public:
    HbInputFocusObjectPrivate(QObject *focusedObject)
        : mEditorInterface(focusedObject)
    {}

    HbMainWindow *mainWindow() const;    
    void ensureCursorVisible(QObject *widget);
   
public:
    HbInputFocusObject *q_ptr;
    QPointer<QWidget> mWidget;
    QPointer<QGraphicsObject> mGraphicsObject;
    HbEditorInterface mEditorInterface;
    QString mPreEditString;

private: // For unit test.
    static HbInputFocusObjectPrivate *d_ptr(HbInputFocusObject *fo) {
        Q_ASSERT(fo);
        return fo->d_func();
    }
    friend class TestHbInputFocusObjectPrivate;
};

#endif // HB_INPUT_FOCUS_OBJECT_P_H

// End of file
