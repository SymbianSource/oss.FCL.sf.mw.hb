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

TEMPLATE = app
TARGET = hbthemeserver
CONFIG -= app_bundle
DEFINES += HB_LIB_DIR=\"\\\"$${HB_LIB_DIR}\\\"\"
DEFINES += HB_BUILD_DIR=\"\\\"$${HB_BUILD_DIR}\\\"\"

# directories
DESTDIR = $${HB_BUILD_DIR}/bin

# dependencies
hbAddLibrary(hbcore/HbCore)

# Input
SOURCES  += $$PWD/main.cpp 
SOURCES  += $$PWD/hbthemeserver.cpp
SOURCES  += $$PWD/hbthemeserverapplication.cpp
SOURCES  += $$PWD/hbthemeserverutils.cpp
SOURCES  += $$PWD/hbiconcacheitemcreator_p.cpp 
SOURCES  += $$PWD/hbpixmapiconprocessor_p.cpp
SOURCES  += $$PWD/hbpiciconprocessor_p.cpp
SOURCES  += $$PWD/hbicondatacache_p.cpp
SOURCES  += $$PWD/hbcache_p.cpp

HEADERS += $$PWD/hbthemeserver_p.h
HEADERS += $$PWD/hbthemeserverapplication_p.h
HEADERS += $$PWD/hbthemeserverutils_p.h
HEADERS += $$PWD/hbiconcacheitemcreator_p.h
HEADERS += $$PWD/hbiconprocessor_p.h
HEADERS += $$PWD/hbpixmapiconprocessor_p.h
HEADERS += $$PWD/hbpiciconprocessor_p.h
HEADERS += $$PWD/hbicondatacache_p.h
HEADERS += $$PWD/hbcache_p.h
HEADERS += $$PWD/hbdoublelinkedlist_p.h
HEADERS += $$PWD/hbdoublelinkedlistinline_p.h

symbian:CONFIG += nvg

symbian {
    SOURCES  += $$PWD/hbthemeserver_symbian.cpp
    SOURCES  += $$PWD/hbthemewatcher_symbian.cpp
    HEADERS += $$PWD/hbthemeserver_symbian_p_p.h
    HEADERS += $$PWD/hbthemewatcher_symbian_p.h
    LIBS += -lapgrfx -lws32 -lavkon -lcone -leikcore -lNVGDecoder_SW -llibvgi -lfbscli -lefsrv

    nvg {
        DEFINES += HB_NVG_CS_ICON
        SOURCES  += $$PWD/hbnvgiconprocessor_p.cpp
        HEADERS += $$PWD/hbnvgiconprocessor_p.h
        LIBS += -llibopenvg -llibopenvgu -llibegl
    }

    sgimage {
        DEFINES += HB_SGIMAGE_ICON
        HEADERS += $$PWD/hbsgimageiconprocessor_p.h
            HEADERS += $$PWD/hbsgimagerenderer_p.h
            SOURCES  += $$PWD/hbsgimageiconprocessor_p.cpp
            SOURCES  += $$PWD/hbsgimagerenderer.cpp
            INCLUDEPATH += $${EPOCROOT}epoc32/include/platform #For SGImage Support
            LIBS += -lsgresource #For SGImage Support
            LIBS += -llibopenvg #For SGImage Support
            LIBS += -llibopenvgu #For SGImage Support
            LIBS += -llibegl #For SGImage Support
    }

    myrssrules = \
    "hidden = KAppIsHidden;"
    RSS_RULES += myrssrules
    MMP_RULES += "SYSTEMINCLUDE $${EPOCROOT}epoc32/include/middleware"
    # FOR 9.2 FOR INCLUDING aknappui.h
    MMP_RULES += "SYSTEMINCLUDE $${EPOCROOT}epoc32/include/mw"
    TARGET.UID3 = 0x20022E82
    TARGET.EPOCHEAPSIZE = 0x20000 0xA00000

    # AllFiles is needed to be able to access icon and effect files in
    # an application's private folder for example.
    TARGET.CAPABILITY = CAP_SERVER AllFiles
    # TARGET.CAPABILITY += ProtServ TrustedUI

} else {
    SOURCES  += $$PWD/hbthemeserver_generic.cpp
    HEADERS += $$PWD/hbthemeserver_generic_p_p.h
}

QT = core gui svg network

# installation
!local {
    target.path = $${HB_BIN_DIR}
    INSTALLS += target
}

include($${HB_SOURCE_DIR}/src/hbcore/3rdparty/qtsingleapplication/src/qtsingleapplication.pri)
include($${HB_SOURCE_DIR}/src/hbcommon.pri)
