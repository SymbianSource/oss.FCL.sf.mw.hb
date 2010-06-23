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

#include "hbmemorymanager_p.h"
#include "hbsharedmemorymanager_p.h"
#ifndef HB_BIN_CSS
#include "hbheapmemorymanager_p.h"
#include "hbsharedmemorymanagerut_p.h"
#endif // HB_BIN_CSS

#include "hbmemoryutils_p.h"

#define SHARED_CONTAINER_UNITTEST_PREFIX "unittest_hbsharedcontainer_"
#define SHARED_MEMORYMANAGER_UNITTEST_PREFIX "unittest_hbsharedmemory"

/**
* helper function to know whether process is a shared container unittest
*/
#ifndef HB_BIN_CSS
static bool isSharedContainerUnitTest()
{
    static bool isSharedContainerUnit = false;
    // to avoid string operations multiple times in each call
    static bool appNameNotTested=true;

    if (appNameNotTested) {
        appNameNotTested = false;
        if ( HbMemoryUtils::getCleanAppName().startsWith(SHARED_CONTAINER_UNITTEST_PREFIX) ||
             HbMemoryUtils::getCleanAppName().startsWith(SHARED_MEMORYMANAGER_UNITTEST_PREFIX) ) {
            isSharedContainerUnit = true;
        } else {
            isSharedContainerUnit = false;
        }
    }
    return isSharedContainerUnit;
}
#endif
/**
* to get instance of hbmemory manager
*/
HbMemoryManager * HbMemoryManager::instance( MemoryType type )
{
    Q_UNUSED(type)

#ifndef HB_BIN_CSS
    switch(type) {
        case SharedMemory :
            if ( isSharedContainerUnitTest() )    {
                return HbSharedMemoryManagerUt::instance();
            } else {
                return HbSharedMemoryManager::instance();
            }
        case HeapMemory :          
            return HbHeapMemoryManager::instance();
        default:
            return 0;
    }
#else
    return HbSharedMemoryManager::instance();
#endif
}

/**
* release the HbSharedMemoryManager-instance.
*/
void HbMemoryManager::releaseInstance( MemoryType type )
{
    Q_UNUSED(type)

#ifndef HB_BIN_CSS
    if ( type == SharedMemory ) {
        if ( isSharedContainerUnitTest() ) {
            HbSharedMemoryManagerUt::releaseInstance();
        } else {
            HbSharedMemoryManager::releaseInstance();
        }

    } else if ( type == HeapMemory ) {
        HbHeapMemoryManager::releaseInstance();
    }
#else
    return HbSharedMemoryManager::releaseInstance();
#endif
}
