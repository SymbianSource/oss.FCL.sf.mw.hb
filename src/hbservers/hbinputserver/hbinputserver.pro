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
TARGET = hbinputserver

DESTDIR = $${HB_BUILD_DIR}/bin

hbAddLibrary(hbcore/HbCore)

DEFINES += ENABLE_INPUTSRVDEBUG

SOURCES += $$PWD/main.cpp
SOURCES += $$PWD/hbinputserver.cpp
SOURCES += $$PWD/hbinputserversettings.cpp
SOURCES += $$PWD/hbinputservermethods.cpp
HEADERS += $$PWD/hbinputserver_p.h
HEADERS += $$PWD/hbinputserversettings_p.h
HEADERS += $$PWD/hbinputservermethods_p.h

symbian {
    TARGET.CAPABILITY = CAP_APPLICATION
    TARGET.UID3 = 0x20031E29

    inputserverrssrules = \
     "hidden = KAppIsHidden;"

    RSS_RULES += inputserverrssrules

    LIBS += -lapgrfx
    LIBS += -lws32
    LIBS += -lavkon
    LIBS += -lcone
    LIBS += -leikcore
    LIBS += -lfbscli
    LIBS += -lefsrv
}

!local {
    target.path = $${HB_BIN_DIR}
    INSTALLS += target
}

include($${HB_SOURCE_DIR}/src/hbcommon.pri)
