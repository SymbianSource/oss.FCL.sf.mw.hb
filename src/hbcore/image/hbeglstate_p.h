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

#ifndef HBEGLSTATE_P_H
#define HBEGLSTATE_P_H

#include <EGL/egl.h>
#include <VG/openvg.h>
#include <QMutex>
#include <QSet>

#include "hbnvgiconimpl_p.h"
#include "hbnvgenginepool_p.h"

class HbNvgIconImpl;

class HbEglStates: public QObject
{
    Q_OBJECT

public:
    HbEglStates( EGLDisplay d,
                 EGLSurface rs,
                 EGLSurface ds,
                 EGLContext  c,
                 EGLConfig cfg = 0 );

    HbEglStates();

    ~HbEglStates()
    {
        clearImages();
    }

    void set( EGLDisplay d,
              EGLSurface rs,
              EGLSurface ds,
              EGLContext c,
              EGLConfig cfg = 0 )
    {
        reset(d, rs, ds, c, cfg);
        init = 1;
    }

    void ref()
    {
        ++refCount;
    }

    int initialized() const
    {
        return init;
    }
    
    void deref(HbEglStates *& instance)
    {
        if (instance && --(instance->refCount) == 0) {
        }
    }

    void removeVGImage(VGImage * image);

    void addVGImage(VGImage * image)
    {
        if (image) {
            imageList.insert(image);
        }
    }
    void handleForegroundLost();

    EGLDisplay  display;
    EGLSurface  currentReadSurface;
    EGLSurface  currentWriteSurface;
    EGLContext  eglContext;
    EGLConfig   eglConfig;

    static HbEglStates *global();

private:

    void clearImages();

    void reset( EGLDisplay d = 0,
                EGLSurface rs = 0,
                EGLSurface ds = 0,
                EGLContext  c = 0,
                EGLConfig cfg = 0 );

    int         refCount;
    int         init;
    typedef QSet<VGImage*>     ImageList;
    typedef ImageList::iterator ImageListIter;

    ImageList           imageList;
    QMutex              mutex;
};

/* HbEglStateRestorer
 *
 * Saves the EGL state passed in the constructor.
 * This class will help in saving the state even in the case of exception.
 */
class HbEglStateRestorer
{
public:
    HbEglStateRestorer( EGLDisplay d,
                        EGLSurface rs,
                        EGLSurface ds,
                        EGLContext c )
            : display(d),
              currentReadSurface(rs),
              currentWriteSurface(ds),
              eglContext(c)
    {
        restored = 0;
    }

    void restore();

    ~HbEglStateRestorer()
    {
        restore();
    }

private:
    EGLDisplay  display;
    EGLSurface  currentReadSurface;
    EGLSurface  currentWriteSurface;
    EGLContext  eglContext;
    EGLConfig   eglConfig;
    int         restored;
};

#endif

