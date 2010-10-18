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

HbMemoryManager *HbMemoryManager::sharedMemoryManager = 0;
HbMemoryManager *HbMemoryManager::heapMemoryManager = 0;

/**
* release the HbSharedMemoryManager-instance.
*/
void HbMemoryManager::releaseInstance( MemoryType type )
{
#ifndef HB_BIN_CSS
    if ( type == SharedMemory ) {
        delete sharedMemoryManager;
        sharedMemoryManager = 0;
    } else if ( type == HeapMemory ) {
        delete heapMemoryManager;
        heapMemoryManager = 0;
    }
#else
    Q_UNUSED(type)

    delete sharedMemoryManager;
    sharedMemoryManager = 0;
#endif
}

void HbMemoryManager::createSharedMemoryManager()
{
    if(!sharedMemoryManager) {
        sharedMemoryManager = HbSharedMemoryManager::create();
        if(!sharedMemoryManager->initialize()) {
            delete sharedMemoryManager;
            sharedMemoryManager = 0;
        }
    }
}

void HbMemoryManager::createHeapMemoryManager()
{
#ifndef HB_BIN_CSS
    if(!heapMemoryManager) {
        heapMemoryManager = HbHeapMemoryManager::create();
    }
#endif
}
