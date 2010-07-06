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
#

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# public framework headers
PUBLIC_HEADERS += $$PWD/hbinputdef.h
PUBLIC_HEADERS += $$PWD/hbinputmethod.h
PUBLIC_HEADERS += $$PWD/hbinpututils.h
PUBLIC_HEADERS += $$PWD/hbinputkeymap.h
PUBLIC_HEADERS += $$PWD/hbinputkeymapfactory.h
PUBLIC_HEADERS += $$PWD/hbinputpredictionengine.h
PUBLIC_HEADERS += $$PWD/hbinputpredictionfactory.h
PUBLIC_HEADERS += $$PWD/hbinputsettingproxy.h
PUBLIC_HEADERS += $$PWD/hbinputlanguagedatabase.h
PUBLIC_HEADERS += $$PWD/hbinputeditorinterface.h
PUBLIC_HEADERS += $$PWD/hbinputfilter.h
PUBLIC_HEADERS += $$PWD/hbinputfocusobject.h
PUBLIC_HEADERS += $$PWD/hbinputmodeproperties.h
PUBLIC_HEADERS += $$PWD/hbinputstate.h
PUBLIC_HEADERS += $$PWD/hbinputuserdictionary.h
PUBLIC_HEADERS += $$PWD/hbinputextrauserdictionary.h
PUBLIC_HEADERS += $$PWD/hbinputextradictionaryfactory.h
PUBLIC_HEADERS += $$PWD/hbinputextradictionarycollection.h
PUBLIC_HEADERS += $$PWD/hbinputpredictioncallback.h
PUBLIC_HEADERS += $$PWD/hbinputvirtualkeyboard.h
PUBLIC_HEADERS += $$PWD/hbinputvkbhost.h
PUBLIC_HEADERS += $$PWD/hbinputlanguage.h
PUBLIC_HEADERS += $$PWD/hbinputstandardfilters.h
PUBLIC_HEADERS += $$PWD/hbinputmethoddescriptor.h
PUBLIC_HEADERS += $$PWD/hbinputvkbhostbridge.h
PUBLIC_HEADERS += $$PWD/hbinputcontextplugin.h

# private framework headers
PRIVATE_HEADERS += $$PWD/hbinputmodecache_p.h
PRIVATE_HEADERS += $$PWD/hbinputcontextproxy_p.h
PRIVATE_HEADERS += $$PWD/hbinputmethodnull_p.h
PRIVATE_HEADERS += $$PWD/hbinputeditorinterface_p.h
PRIVATE_HEADERS += $$PWD/hbinputextrauserdictionary_p.h
PRIVATE_HEADERS += $$PWD/hbinputmethod_p.h
PRIVATE_HEADERS += $$PWD/hbinputsettingproxy_p.h
PRIVATE_HEADERS += $$PWD/hbinputmainwindow_p.h
PRIVATE_HEADERS += $$PWD/hbinputregioncollector_p.h

# framework sources
SOURCES += $$PWD/hbinputmethod.cpp \
    inputfw/hbinputregioncollector.cpp
SOURCES += $$PWD/hbinputkeymap.cpp
SOURCES += $$PWD/hbinputpredictionengine.cpp
SOURCES += $$PWD/hbinputkeymapfactory.cpp
SOURCES += $$PWD/hbinputsettingproxy.cpp
SOURCES += $$PWD/hbinputmodecache.cpp
SOURCES += $$PWD/hbinputeditorinterface.cpp
SOURCES += $$PWD/hbinputeditorinterface_p.cpp
SOURCES += $$PWD/hbinputcontextproxy.cpp
SOURCES += $$PWD/hbinputfilter.cpp
SOURCES += $$PWD/hbinpututils.cpp
SOURCES += $$PWD/hbinputfocusobject.cpp
SOURCES += $$PWD/hbinputstandardfilters.cpp
SOURCES += $$PWD/hbinputmethodnull.cpp
SOURCES += $$PWD/hbinputpredictionfactory.cpp
SOURCES += $$PWD/hbinputextrauserdictionary.cpp
SOURCES += $$PWD/hbinputextradictionaryfactory.cpp
SOURCES += $$PWD/hbinputextradictionarycollection.cpp
SOURCES += $$PWD/hbinputvirtualkeyboard.cpp
SOURCES += $$PWD/hbinputvkbhost.cpp
SOURCES += $$PWD/hbinputlanguage.cpp
SOURCES += $$PWD/hbinputmethod_p.cpp
SOURCES += $$PWD/hbinputmethoddescriptor.cpp
SOURCES += $$PWD/hbinputvkbhostbridge.cpp
SOURCES += $$PWD/hbinputmainwindow.cpp
SOURCES += $$PWD/hbinputcontextplugin.cpp

