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

PRIVATE_HEADERS += $$PWD/hbdevicedialogmanager_p.h
PRIVATE_HEADERS += $$PWD/hbdevicedialogmanager_p_p.h
PRIVATE_HEADERS += $$PWD/hbdevicedialogscontainer_p.h
PRIVATE_HEADERS += $$PWD/hbdevicedialogpluginmanager_p.h
PRIVATE_HEADERS += $$PWD/hbindicatorclientinfo_p.h
PRIVATE_HEADERS += $$PWD/hbdevicedialogserver_p.h
PRIVATE_HEADERS += $$PWD/hbpluginnamecache_p.h
PRIVATE_HEADERS += $$PWD/hbdevicedialogserverstatus_p.h
contains(DEFINES, HB_HAVE_QT_MOBILITY) {
PRIVATE_HEADERS += $$PWD/hbsysteminfo_p.h
}

SOURCES += $$PWD/hbdevicedialogserver.cpp
SOURCES += $$PWD/hbdevicedialogmanager.cpp
SOURCES += $$PWD/hbdevicedialogmanager_p.cpp
SOURCES += $$PWD/hbdevicedialogscontainer.cpp
SOURCES += $$PWD/hbdevicedialogpluginmanager.cpp
SOURCES += $$PWD/hbpluginnamecache.cpp
contains(DEFINES, HB_HAVE_QT_MOBILITY) {
SOURCES += $$PWD/hbsysteminfo_p.cpp
}

symbian {
PRIVATE_HEADERS += $$PWD/hbdevicedialogserversym_p_p.h
PRIVATE_HEADERS += $$PWD/hbdevicedialogsession_p.h
PRIVATE_HEADERS += $$PWD/hbdevicedialogserverdefs_p.h
contains(DEFINES, HB_HAVE_QT_MOBILITY) {
PRIVATE_HEADERS += $$PWD/hbsysteminfosym_p_p.h
}

SOURCES += $$PWD/hbdevicedialogserversym_p.cpp
SOURCES += $$PWD/hbdevicedialogsession.cpp
SOURCES += $$PWD/hbindicatorsessionhandler.cpp
SOURCES += $$PWD/hbdevicedialogserverstatussym.cpp
contains(DEFINES, HB_HAVE_QT_MOBILITY) {
SOURCES += $$PWD/hbsysteminfosym_p_p.cpp
}
}

!symbian {
PRIVATE_HEADERS += $$PWD/hbdevicedialogserverwin_p_p.h
contains(DEFINES, HB_HAVE_QT_MOBILITY) {
PRIVATE_HEADERS += $$PWD/hbsysteminfowin_p_p.h
}

SOURCES += $$PWD/hbdevicedialogserverwin_p.cpp
SOURCES += $$PWD/hbdevicedialogserverstatuswin.cpp
contains(DEFINES, HB_HAVE_QT_MOBILITY) {
SOURCES += $$PWD/hbsysteminfowin_p_p.cpp
}

}

symbian {
LIBS += -lws32
LIBS += -lcone
}

#DEFINES += ENABLE_TRACE_OUTPUT
