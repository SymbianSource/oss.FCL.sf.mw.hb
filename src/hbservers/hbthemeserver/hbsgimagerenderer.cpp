/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbServers module of the UI Extensions for Mobile.
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

#include "hbsgimagerenderer_p.h"
#include "hbnvg_p.h"

#include <QDebug>
QHash<unsigned long long, RSgImage*> HbSgImageRenderer::sgImageHash;

Q_GLOBAL_STATIC(HbSgImageRenderer, instance)

HbSgImageRenderer* HbSgImageRenderer::global()
{
    return instance();
}

HbSgImageRenderer::HbSgImageRenderer() :
        display(EGL_NO_DISPLAY),
        currentSurface(EGL_NO_SURFACE),
        eglContext(EGL_NO_CONTEXT),
        eglConfig(0),
        init(false),
        engine(0)
{
#ifdef HB_ICON_CACHE_DEBUG
    eglQueryProfilingData = 0;
#endif
}

HbSgImageRenderer::~ HbSgImageRenderer()
{
    terminate();
#ifdef HB_ICON_CACHE_DEBUG
    if (prof_data) {
        free(prof_data);
    }
#endif
}
    
bool HbSgImageRenderer::initialize()
{
    if (!init) {
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY) {
            return false;
        }

        if (eglInitialize(display, 0, 0) == EGL_FALSE) {
            return false;
        }

        eglBindAPI(EGL_OPENVG_API);

        TInt error = sgDriver.Open();
        if (error != KErrNone) {
            return false;
        }
        engine = new HbNvgEngine;
    }

    init = true;
    return true;
}

void HbSgImageRenderer::terminate()
{
    if (init) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (eglContext != EGL_NO_CONTEXT) {
            eglDestroyContext(display, eglContext);
            eglContext = EGL_NO_CONTEXT;
        }

        sgDriver.Close();

        eglTerminate(display);
        eglReleaseThread();
        display = EGL_NO_DISPLAY;
        delete engine;
        engine = 0;
        init = false;
    }
}

bool HbSgImageRenderer::createContext(RSgImage * sgImage)
{
    if (!init && !initialize()) {
        return false;
    }

    const EGLint KConfigAttributes[] = {
        EGL_MATCH_NATIVE_PIXMAP, (EGLint)sgImage,
        EGL_RENDERABLE_TYPE,        EGL_OPENVG_BIT,
        EGL_SURFACE_TYPE,           EGL_PIXMAP_BIT | EGL_VG_ALPHA_FORMAT_PRE_BIT,
        EGL_NONE
    };

    EGLint numConfigs = 0;
    if (eglChooseConfig(display, KConfigAttributes, &eglConfig, 1, &numConfigs) == EGL_FALSE) {
        return false;
    }

    eglContext = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, 0);

    return eglContext != EGL_NO_CONTEXT;
}

bool HbSgImageRenderer::beginRendering(RSgImage * sgImage)
{
    if (eglContext == EGL_NO_CONTEXT && !createContext(sgImage)) {
        return false;
    }

    const EGLint KPixmapAttributes[] = {
        EGL_VG_ALPHA_FORMAT,    EGL_VG_ALPHA_FORMAT_PRE,
        EGL_NONE
    };

    currentSurface = eglCreatePixmapSurface(display, eglConfig, sgImage, KPixmapAttributes);
    if (currentSurface == EGL_NO_SURFACE) {
        return false;
    }

    if (eglMakeCurrent(display, currentSurface, currentSurface, eglContext) == EGL_FALSE) {
        eglDestroySurface(display, currentSurface);
        currentSurface = EGL_NO_SURFACE;
        return false;
    }

    return true;
}

void HbSgImageRenderer::endRendering()
{
    if (currentSurface != EGL_NO_SURFACE) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(display, currentSurface);
        currentSurface = EGL_NO_SURFACE;
    }
}

void HbSgImageRenderer::addSgImageToHash(unsigned long long id, RSgImage* sgImage)
{
    sgImageHash.insert(id, sgImage);
}

void HbSgImageRenderer::removeSgImageFromHash(unsigned long long id)
{
    if (sgImageHash.contains(id)) {
        RSgImage* sgImage = sgImageHash.value(id);
#ifdef HB_ICON_CACHE_DEBUG
        qDebug() << "Closing handle and removing sgImage with id = " << id;
#endif
        sgImage->Close();
        delete sgImage;
        sgImageHash.remove(id);
    }    
}

#ifdef HB_ICON_CACHE_DEBUG
unsigned long HbSgImageRenderer::totalGPUMemory()
{
    qDebug() <<"Inside  HbSgImageRenderer::totalGPUMemory()  " ;
    EGLint data_count;
    
    EGLint* prof_data = getProfileData(data_count);
    if (!prof_data) {
    qDebug() <<"Inside  HbSgImageRenderer::totalGPUMemory()  :: prof_data is NULL!" ;
        return 0;
    }
        
    unsigned long totalMemory = 0;
    EGLint i = 0;

    while (i < data_count) {
        switch (prof_data[i++]) {
        case EGL_PROF_TOTAL_MEMORY_NOK: {
            totalMemory = prof_data[i++];
            break;
            }

        case EGL_PROF_PROCESS_ID_NOK: {
            i+=2;
            break;
        }
        
        case EGL_PROF_USED_MEMORY_NOK:
        case EGL_PROF_PROCESS_USED_PRIVATE_MEMORY_NOK:
        case EGL_PROF_PROCESS_USED_SHARED_MEMORY_NOK:
        default: {
            i++;
            break;
            }
        }
    }

    free(prof_data);
    qDebug() <<"total GPU memory =   " << totalMemory;
    qDebug() <<"Exiting  HbSgImageRenderer::totalGPUMemory()  " ;
    return totalMemory;
}
    

unsigned long HbSgImageRenderer::freeGPUMemory()
{
    qDebug() <<"Inside  HbSgImageRenderer::freeGPUMemory()  " ;
    EGLint data_count;
    
    EGLint* prof_data = getProfileData(data_count);
    if (!prof_data) {
        qDebug() <<"Inside  HbSgImageRenderer::freeGPUMemory()  :: prof_data is NULL!" ;
        return 0;
    }
        
    unsigned long totalMemory = 0;
    unsigned long usedMemory  = 0;
    
    EGLint i = 0;
    while (i < data_count) {
        switch (prof_data[i++]) {
        case EGL_PROF_TOTAL_MEMORY_NOK: {
            totalMemory = prof_data[i++];
            break;
        }

        case EGL_PROF_USED_MEMORY_NOK: {
            usedMemory = prof_data[i++];
            break;
        }

        case EGL_PROF_PROCESS_ID_NOK: {
            i += 2;
            break;
        }

        case EGL_PROF_PROCESS_USED_PRIVATE_MEMORY_NOK:
        case EGL_PROF_PROCESS_USED_SHARED_MEMORY_NOK:
        default: {
            i++;
            break;
        }
        }
    }

    free(prof_data);
    qDebug() << "free GPU memory =   " << totalMemory - usedMemory;
    qDebug() << "Exiting  HbSgImageRenderer::freeGPUMemory()  " ;
    return (totalMemory - usedMemory);
}

EGLint* HbSgImageRenderer::getProfileData(EGLint & data_count)
{
    EGLint* prof_data = 0;
    data_count = 0;
    if (!init && !initialize()) {
        return 0;
    }
    if (!eglQueryProfilingData) {
        eglQueryProfilingData = (NOK_resource_profiling)eglGetProcAddress("eglQueryProfilingDataNOK");
        if (!eglQueryProfilingData) {
            return 0;
        }
    }
    EGLBoolean success = EGL_TRUE;    
    // Find out how much profiling data is available
    success = eglQueryProfilingData(display,
                                    EGL_PROF_QUERY_GLOBAL_BIT_NOK |
                                    EGL_PROF_QUERY_MEMORY_USAGE_BIT_NOK,
                                    0,
                                    0,
                                    &data_count);

    if (!success) {
        return 0;
    }
    // Allocate room for the profiling data
    prof_data = (EGLint*)malloc(data_count * sizeof(EGLint));
    if (!prof_data) {
        data_count = 0;
        return 0;
    }
    // Retrieve the profiling data
    success = eglQueryProfilingData(display,
                                    EGL_PROF_QUERY_GLOBAL_BIT_NOK |
                                    EGL_PROF_QUERY_MEMORY_USAGE_BIT_NOK,
                                    prof_data,
                                    data_count,
                                    &data_count);
    if (!success) {
        data_count = 0;
        free(prof_data);
        return 0;
    }

    return prof_data;
}
#endif //HB_ICON_CACHE_DEBUG
