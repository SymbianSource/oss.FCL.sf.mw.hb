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

#ifndef HBSGIMAGERENDERER_P_H
#define HBSGIMAGERENDERER_P_H

#include <QHash>
#include <EGL/egl.h>
#include <sgresource/sgimage.h>
#include <hbthemecommon_symbian_p.h>

#ifdef HB_ICON_CACHE_DEBUG
typedef EGLBoolean (*NOK_resource_profiling)(EGLDisplay, EGLint, EGLint*, EGLint, EGLint*);
#endif

class HbSgImageRenderer
{
public:

    HbSgImageRenderer();
    ~HbSgImageRenderer();
    bool initialize();
    bool beginRendering(RSgImage * sgImage);
    void endRendering();    
    static HbSgImageRenderer *global();
    static void addSgImageToHash(unsigned long long id, RSgImage* sgImage);
    static void removeSgImageFromHash(unsigned long long id);

#ifdef HB_ICON_CACHE_DEBUG
    unsigned long totalGPUMemory();
    unsigned long freeGPUMemory();
#endif
    
private:
    
    void terminate();
    bool createContext(RSgImage * sgImage);
    
#ifdef HB_ICON_CACHE_DEBUG
    EGLint* HbSgImageRenderer::getProfileData(EGLint & data_count);
    NOK_resource_profiling eglQueryProfilingData;
    EGLint data_count;
    EGLint* prof_data;
#endif
    
    EGLDisplay  display;
    EGLSurface  currentSurface;
    EGLContext  eglContext;
    EGLConfig   eglConfig;
    bool        init;
    RSgDriver sgDriver;
    static QHash<unsigned long long, RSgImage*> sgImageHash;
};

#endif

