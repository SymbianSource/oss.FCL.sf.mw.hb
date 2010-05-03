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

#include "hbeglstate_p.h"
#include <QMutexLocker>

Q_GLOBAL_STATIC(HbEglStates, instance)

HbEglStates::HbEglStates( EGLDisplay d,
                          EGLSurface rs,
                          EGLSurface ds,
                          EGLContext  c,
                          EGLConfig cfg )
        : refCount(0),
        init(0)
       
{
    set(d, rs, ds, c, cfg);
}

HbEglStates::HbEglStates()
        : refCount(0),
        init(0)
       
{   reset();
}

void HbEglStates::handleForegroundLost()
{
    reset();
    init = 0;
    HbNvgEnginePool::instance()->resetNvgEngine();
    clearImages();
}

void HbEglStates::clearImages()
{
    if (!imageList.empty()) {
        QMutexLocker locker(&mutex);
        ImageListIter iter = imageList.begin();
        for (; iter != imageList.end(); ++iter) {
            VGImage * imgPtr = *iter;
            if (*imgPtr) {
                vgDestroyImage(*imgPtr);
                *imgPtr = 0;
            }
        }
        imageList.clear();
    }
}

void HbEglStates::reset( EGLDisplay d,
                         EGLSurface rs,
                         EGLSurface ds,
                         EGLContext  c,
                         EGLConfig cfg )
{
    display = d;
    currentReadSurface = rs;
    currentWriteSurface = ds;
    eglContext = c;
    eglConfig = cfg;
}

void HbEglStateRestorer::restore()
{
    if (restored) {
        return;
    }
    if (eglContext != EGL_NO_CONTEXT && currentWriteSurface != EGL_NO_SURFACE) {
        if (eglMakeCurrent(display, currentWriteSurface,
                           currentReadSurface, eglContext) == EGL_FALSE) {

        } else {
            restored = 1;
        }
    }
}

void HbEglStates::removeVGImage( VGImage * image )
{
    QMutexLocker locker(&mutex);
    if (imageList.contains(image)) {
        if (*image) {
            vgDestroyImage(*image);
            *image = 0;
        }
        imageList.remove(image);
    }
}

HbEglStates * HbEglStates::global()
{
    return instance();
}
