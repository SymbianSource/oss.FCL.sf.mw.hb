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
TARGET = hbsplashgenerator

DESTDIR = $${HB_BUILD_DIR}/bin

SOURCES += $$PWD/main.cpp 
SOURCES += $$PWD/hbsplashgenerator.cpp
SOURCES += $$PWD/hbsplashindicompositor.cpp
symbian: SOURCES += $$PWD/hbsplashgen_server_symbian.cpp

HEADERS += $$PWD/hbsplashgenerator_p.h
HEADERS += $$PWD/hbsplashdirs_p.h
HEADERS += $$PWD/hbsplashcompositor_p.h
HEADERS += $$PWD/hbsplashindicompositor_p.h
HEADERS += $$PWD/hbsplashblacklist_p.h
HEADERS += $$PWD/hbwidgetenabler_p.h
symbian: HEADERS += $$PWD/hbsplashgen_server_symbian_p.h

symbian {
    TARGET.CAPABILITY = CAP_APPLICATION
    TARGET.EPOCHEAPSIZE = 0x20000 0xA00000
    TARGET.UID3 = 0x2002E68B
    MMP_RULES += SMPSAFE

    splashgenrssrules = \
     "hidden = KAppIsHidden;" \
     "launch = KAppLaunchInBackground;"

    RSS_RULES += splashgenrssrules

    LIBS += -lapgrfx
    LIBS += -lplatformenv
    LIBS += -lefsrv
    LIBS += -lws32
    LIBS += -lcone
    LIBS += -lavkon
    LIBS += -leikcore
    LIBS += -lapparc
    LIBS += -lfbscli
}

hbAddLibrary(hbcore/HbCore)
hbAddLibrary(hbwidgets/HbWidgets)
hbAddLibrary(hbutils/HbUtils)

# installation
!local {
    target.path = $${HB_BIN_DIR}
    INSTALLS += target
}

include($${HB_SOURCE_DIR}/src/hbcommon.pri)
