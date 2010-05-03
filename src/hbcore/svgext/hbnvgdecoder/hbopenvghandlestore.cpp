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

#include "hbopenvghandlestore_p.h"
#include <QScopedPointer>

QT_USE_NAMESPACE

struct HbVgImageDeleter {
    static inline void cleanup(void *pointer) {
        if (pointer) vgDestroyImage(*(VGImage*)pointer);
    }
};

struct HbVgPaintDeleter {
    static inline void cleanup(void *pointer) {
        if (pointer) vgDestroyPaint(*(VGPaint*)pointer);
    }
};

struct HbVgPathDeleter {
    static inline void cleanup(void *pointer) {
        if (pointer) vgDestroyPath(*(VGPath*)pointer);
    }
};

HbOpenVgHandleStore::HbOpenVgHandleStore()
{
}

HbOpenVgHandleStore::~HbOpenVgHandleStore()
{
    qint32 handleListCount    = mHandles.count();
    for (qint32 i = 0; i < handleListCount; i++) {
        if (mHandles[i].mVgHandle) {
            switch (mHandles[i].mHandleType) {
            case HbTlvVgHandlePair::VgPath:
                vgDestroyPath(mHandles[i].mVgHandle);
                break;
            case HbTlvVgHandlePair::VgPaint:
                vgDestroyPaint(mHandles[i].mVgHandle);
                break;
            case HbTlvVgHandlePair::VgImage:
                vgDestroyImage(mHandles[i].mVgHandle);
                break;
            }
        }
    }
}


void HbOpenVgHandleStore::addPath(VGPath handle)
{
    QScopedPointer<VGPath, HbVgPathDeleter> scHandle(&handle);
    mHandles.append(HbTlvVgHandlePair(handle, HbTlvVgHandlePair::VgPath));
    scHandle.take();
}

void HbOpenVgHandleStore::addPaint(VGPaint handle)
{
    QScopedPointer<VGPath, HbVgPaintDeleter> scHandle(&handle);
    mHandles.append(HbTlvVgHandlePair(handle, HbTlvVgHandlePair::VgPaint));
    scHandle.take();
}

void HbOpenVgHandleStore::addImage(VGImage handle)
{
    QScopedPointer<VGPath, HbVgImageDeleter> scHandle(&handle);
    mHandles.append(HbTlvVgHandlePair(handle, HbTlvVgHandlePair::VgImage));
    scHandle.take();
}


