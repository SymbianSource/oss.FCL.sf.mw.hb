#
#############################################################################
##
## Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
## All rights reserved.
## Contact: Nokia Corporation (developer.feedback@nokia.com)
##
## This file is part of the UI Extensions for Mobile.
##
## GNU Lesser General Public License Usage
## This file may be used under the terms of the GNU Lesser General Public
## License version 2.1 as published by the Free Software Foundation and
## appearing in the file LICENSE.LGPL included in the packaging of this file.
## Please review the following information to ensure the GNU Lesser General
## Public License version 2.1 requirements will be met:
## http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
##
## In addition, as a special exception, Nokia gives you certain additional
## rights.  These rights are described in the Nokia Qt LGPL Exception
## version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
##
## If you have questions regarding the use of this file, please contact
## Nokia at developer.feedback@nokia.com.
##
#############################################################################

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

PUBLIC_HEADERS += $$PWD/hbaction.h
PUBLIC_HEADERS += $$PWD/hbapplication.h
PUBLIC_HEADERS += $$PWD/hbevent.h
PUBLIC_HEADERS += $$PWD/hbglobal.h
PUBLIC_HEADERS += $$PWD/hbgraphicsscene.h
PUBLIC_HEADERS += $$PWD/hbinstance.h
PUBLIC_HEADERS += $$PWD/hbnamespace.h
PUBLIC_HEADERS += $$PWD/hbs60events.h
PUBLIC_HEADERS += $$PWD/hbcommoncrkeys.h

PRIVATE_HEADERS += $$PWD/hbmemorymanager_p.h
PRIVATE_HEADERS += $$PWD/hbheapmemorymanager_p.h
PRIVATE_HEADERS += $$PWD/hbsharedmemoryallocators_p.h
PRIVATE_HEADERS += $$PWD/hbsharedmemorymanager_p.h
PRIVATE_HEADERS += $$PWD/hbsharedmemorymanagerut_p.h
PRIVATE_HEADERS += $$PWD/hbmemoryutils_p.h
PRIVATE_HEADERS += $$PWD/hbsmartpointer_p.h
PRIVATE_HEADERS += $$PWD/hbsmartoffset_p.h
PRIVATE_HEADERS += $$PWD/hbvector_p.h
PRIVATE_HEADERS += $$PWD/hbstring_p.h
PRIVATE_HEADERS += $$PWD/hbvariant_p.h
PRIVATE_HEADERS += $$PWD/hbstringvector_p.h
PRIVATE_HEADERS += $$PWD/hbaction_p.h
PRIVATE_HEADERS += $$PWD/hbgraphicsscene_p.h
PRIVATE_HEADERS += $$PWD/hbglobal_p.h
PRIVATE_HEADERS += $$PWD/hbinstance_p.h
PRIVATE_HEADERS += $$PWD/hbnamespace_p.h
PRIVATE_HEADERS += $$PWD/hbstandarddirs_p.h
PRIVATE_HEADERS += $$PWD/hbtestabilityinterface_p.h
PRIVATE_HEADERS += $$PWD/hbthemeperf_p.h
PRIVATE_HEADERS += $$PWD/hbstringdata_p.h
PRIVATE_HEADERS += $$PWD/hbthemeindex_p.h
PRIVATE_HEADERS += $$PWD/hbsharedcache_p.h
PRIVATE_HEADERS += $$PWD/hbmainwindoworientation_p.h
PRIVATE_HEADERS += $$PWD/hborientationstatus_p.h
PRIVATE_HEADERS += $$PWD/hbsensorlistener_p.h
PRIVATE_HEADERS += $$PWD/hblocalechangenotifier_p.h
PRIVATE_HEADERS += $$PWD/hbapplication_p.h
PRIVATE_HEADERS += $$PWD/hbthemesystemeffect_p.h
PRIVATE_HEADERS += $$PWD/hbpluginloader_p.h
PRIVATE_HEADERS += $$PWD/hbpluginloader_p_p.h
symbian {
    PRIVATE_HEADERS += $$PWD/hbcorepskeys_p.h
    PRIVATE_HEADERS += $$PWD/hbsensornotifyhandler_p.h
}

SOURCES += $$PWD/hbaction.cpp
SOURCES += $$PWD/hbapplication.cpp
SOURCES += $$PWD/hbevent.cpp
SOURCES += $$PWD/hbglobal.cpp
SOURCES += $$PWD/hbgraphicsscene.cpp
SOURCES += $$PWD/hbinstance.cpp
SOURCES += $$PWD/hbnamespace.cpp
SOURCES += $$PWD/hbstandarddirs.cpp
SOURCES += $$PWD/hbmemorymanager_p.cpp
SOURCES += $$PWD/hbsharedmemorymanager_p.cpp
SOURCES += $$PWD/hbsharedmemorymanagerut_p.cpp
SOURCES += $$PWD/hbsplaytreeallocator_p.cpp
SOURCES += $$PWD/hbheapmemorymanager_p.cpp
SOURCES += $$PWD/hbmultisegmentallocator_p.cpp
SOURCES += $$PWD/hbstring_p.cpp
SOURCES += $$PWD/hbvariant_p.cpp
SOURCES += $$PWD/hbthemeindex.cpp
SOURCES += $$PWD/hbsharedcache.cpp
SOURCES += $$PWD/hbmainwindoworientation.cpp
SOURCES += $$PWD/hborientationstatus.cpp
SOURCES += $$PWD/hbsensorlistener.cpp
SOURCES += $$PWD/hblocalechangenotifier_p.cpp
SOURCES += $$PWD/hbthemesystemeffect.cpp
SOURCES += $$PWD/hbpluginloader.cpp
symbian {
    SOURCES += $$PWD/hbsensornotifyhandler_p.cpp
}
