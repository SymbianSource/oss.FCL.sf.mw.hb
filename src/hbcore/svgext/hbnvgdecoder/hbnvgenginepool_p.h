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
 
#ifndef HB_NVGNEGINE_POOL_H_
#define HB_NVGNEGINE_POOL_H_
 
#include "hbnvg_p.h"

class HB_CORE_PRIVATE_EXPORT HbNVGEngineInstance
{
public:
    HbNVGEngineInstance() : refCount(0), nvgEngine(0) {}
        
    void deref()
    {
        if (--refCount == 0) {
            delete nvgEngine;
            nvgEngine = 0;
        }
    }
    
    void ref()
    {
        if (!nvgEngine) {
            nvgEngine = new HbNvgEngine; 
        }
        ++refCount;
    }
    
    ~HbNVGEngineInstance()
    {
        if (nvgEngine) {
            delete nvgEngine;
        }
    }
    
    HbNvgEngine * engine()
    {
        if (!nvgEngine) {
            nvgEngine = new HbNvgEngine;
        }
        return nvgEngine;
    }
            
    void resetNVGEngine() {
        if (nvgEngine) {
            delete nvgEngine;
            nvgEngine = 0;
        }
    }
    
private:
    int refCount;
    HbNvgEngine * nvgEngine;
};

class HB_CORE_PRIVATE_EXPORT HbPooledNVGEngine
{
public:
    HbPooledNVGEngine(HbNVGEngineInstance & instance)
     : engineInstance(instance)
    {
        engineInstance.ref();
    }
    
    ~HbPooledNVGEngine()
    {
        engineInstance.deref();
    }
    
    HbNvgEngine& operator*() const
    {
        return *(engineInstance.engine());
    }
    
    HbNvgEngine& operator*()
    {
        return *(engineInstance.engine());
    }

    HbNvgEngine * operator->()
    {
        return engineInstance.engine();
    }

    HbNvgEngine * operator->() const
    {
        return engineInstance.engine();
    }

    HbNvgEngine * engine()
    {
        return engineInstance.engine();
    }
    
private:
    HbNVGEngineInstance & engineInstance;
};

class HB_CORE_PRIVATE_EXPORT HbNVGEnginePool
{
public:
    
    ~HbNVGEnginePool() {delete pooledEngine;}
    
    static HbNVGEnginePool * instance()
    {
        if (!pool) {
            pool = new HbNVGEnginePool;
        }
        return pool;
    }
    
 	HbPooledNVGEngine * getNVGEngine();
 	
 	void resetNVGEngine() {
 	    if (pooledEngine) {
            pooledEngine->resetNVGEngine();
 	    }
 	}
 	
private:
    HbNVGEnginePool() : pooledEngine(0) {}
    
 	HbNVGEngineInstance * pooledEngine;
 	static HbNVGEnginePool * pool;
};

#endif
 
  
