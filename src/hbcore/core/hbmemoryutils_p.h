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

#ifndef HBMEMORYUTILS_P_H
#define HBMEMORYUTILS_P_H

#include <QCoreApplication>
#include <QFileInfo>
#include <QStringList>

#include "hbsmartoffset_p.h"
#include "hbmemorymanager_p.h"

/*
* Macro to get instance of memory manager of given memory type.
*/
#define GET_MEMORY_MANAGER(type) \
    Q_ASSERT(HbMemoryManager::SharedMemory == type || HbMemoryManager::HeapMemory == type); \
    HbMemoryManager * manager = HbMemoryManager::instance(type);

#define RETURN_IF_READONLY(manager) if (!manager->isWritable()) { return; }
#define RETURN_OBJECT_IF_READONLY(manager, object) if (!manager->isWritable()) { return (object); }

class HB_CORE_PRIVATE_EXPORT HbMemoryUtils
{
public:
    /*
    * To create instance of on object of type T in memory of given type.
    * 
    * The function returns a pointer of type T in case memory allocation is
    * successful, NULL otherwise.
    */
    template <typename T>
    static T * create(HbMemoryManager::MemoryType memType)
    {
        GET_MEMORY_MANAGER(memType);
        T *temp = 0;
        if (manager->isWritable()) {
            HbSmartOffset offset(manager->alloc(sizeof(T)), memType);
            temp = new((char*)manager->base() + offset.get()) T(memType);
            offset.release();
        }
        return temp;
    }
    /*
    * To create instance of on object of type T in memory of given type. The object is copy of the 
    * other passed object of same type.
    * 
    * The function returns a pointer of type T in case memory allocation is
    * successful, NULL otherwise.
    */
    template <typename T>
    static T * create(const T &other, HbMemoryManager::MemoryType memType)
    {
        GET_MEMORY_MANAGER(memType);
        T *temp = 0;
        if (manager->isWritable()) {
            HbSmartOffset offset(manager->alloc(sizeof(T)),memType);
            temp = new((char*)manager->base() + offset.get()) T(other, memType);
            offset.release();
        }
        return temp;
    }
    /*
    * To release the instant properly by taking care of the type of memory
    * (assming that memory allocated for the object is same as its memoryType)
    */
    template <typename T>
    static void release(T *instance)
    {
        GET_MEMORY_MANAGER(instance->memoryType);
        if (manager->isWritable()) {
            instance->~T();
            manager->free((quintptr)instance - (quintptr)manager->base());
        }
    }
    
    /*
    * To allocate memory of given size in given type of memory.
    * 
    * This function can throw
    */
    static qptrdiff allocateMemory(HbMemoryManager::MemoryType type, int size)
    {
        GET_MEMORY_MANAGER(type);
        qptrdiff offset =  manager->alloc(size);
    
        return offset;
    }
    
    /*
    * To free memory at given offset in given type of memory.
    */
    static void freeMemory(HbMemoryManager::MemoryType type, qptrdiff &offset)
    {
        GET_MEMORY_MANAGER(type)
        manager->free(offset);
        offset = -1; // invalid offset
    }
    
    /*
    * To get pointer to an object of type T at given offset in memory of given type.
    */
    template <typename T>
    static T * getAddress(HbMemoryManager::MemoryType type, qptrdiff offset)
    {
        // Do not change the condition to (<0), valid address can be negative.
        if (offset == -1) {
            return 0;
        }
        GET_MEMORY_MANAGER(type)
        return (T*) ((char*)manager->base() + offset);
    }
    
    /*
    * returns application file name without extension.
    */
    static const QString &getCleanAppName()
    {
        // making static to avoid string multiple string operation on each call..
        static QString cleanAppName;
        if(cleanAppName.isEmpty()) {
            cleanAppName = QFileInfo(QCoreApplication::applicationFilePath()).baseName();
        }
        return cleanAppName;
    }

};

#endif //HBMEMORYUTILS_P_H
