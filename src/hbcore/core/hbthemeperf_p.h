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
#ifndef HBTHEME_PERF_H
#define HBTHEME_PERF_H


//#define HB_PERF_MEM
//#define HB_PERF_TIME


#if !defined(HB_PERF_MEM)
#define HB_START_SHAREDMEMORY_PRINT(str)
#define HB_END_SHAREDMEMORY_PRINT(str)
#else
#include <QDebug>
#define HB_START_SHAREDMEMORY_PRINT(str) unsigned int startMemory =  manager->memoryConsumed(); \
                      qDebug() << str << "SharedMemory Consumed" << "In" << Q_FUNC_INFO << startMemory;             
#define HB_END_SHAREDMEMORY_PRINT(str) unsigned int endMemory = manager->memoryConsumed(); \
                      qDebug() << str << "SharedMemory Consumed" << "In" << Q_FUNC_INFO << endMemory;  \
                      qDebug() << " Difference  : " << endMemory - startMemory;  
#endif // HB_PERF_MEM



#if defined(HB_PERF_MEM) && defined(Q_OS_SYMBIAN)
//  INCLUDES
#include <e32debug.h>
#include <e32base.h>
#include <e32std.h>
#include <hal.h>
#include <e32cmn.h>
#define HB_START_HEAPMEMORY_PRINT(str) TInt startMem; \
                            User::AllocSize(startMem);
#define HB_END_HEAPMEMORY_PRINT(str) TInt endMem; \
                            User::AllocSize(endMem); \
                            qDebug() << str << "HeapMemory Consumed : " << "In" << Q_FUNC_INFO \
                                << (endMem - startMem);
#else
#define HB_START_HEAPMEMORY_PRINT(str)
#define HB_END_HEAPMEMORY_PRINT(str)
#endif  // HB_PERF_MEM && Q_OS_SYMBIAN


#if !defined(HB_PERF_TIME)
#define HB_START_TIME()
#define HB_END_TIME(str,val)
#else
#include <QDateTime>
#include <QDebug>
#define HB_START_TIME() 	QTime time; \
                            	time.start();
#define HB_END_TIME(str,val)   qDebug() << str << val << " time in ms: " << time.elapsed() << "ms";
#endif   //HB_PERF_TIME


#endif // HBTHEME_PERF_H
