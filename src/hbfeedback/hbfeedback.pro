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

TEMPLATE = lib
TARGET = $$hbLibraryTarget(HbFeedback)
DEFINES += BUILD_HB_FEEDBACK
DEFINES += HB_INSTALL_DIR=\"\\\"$${HB_INSTALL_DIR}\\\"\"

INCLUDEPATH += .
DEPENDPATH += .

# directories
DESTDIR = $${HB_BUILD_DIR}/lib
win32:DLLDESTDIR = $${HB_BUILD_DIR}/bin

# components
include(player/player.pri)

CONVENIENCE_HEADERS += $${HB_BUILD_DIR}/include/hbfeedback/hbfeedback.h
CONVENIENCE_HEADERS += $$files($${HB_BUILD_DIR}/include/hbfeedback/Hb*)
HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS $$CONVENIENCE_HEADERS

# dependencies
hbAddLibrary(hbcore/HbCore)

# installation
!local {
    target.path = $${HB_LIB_DIR}
    win32:dlltarget.path = $${HB_BIN_DIR}

    pubheaders.files = $$PUBLIC_HEADERS
    pubheaders.path = $${HB_INCLUDE_DIR}/hbfeedback

    convheaders.files = $$CONVENIENCE_HEADERS
    convheaders.path = $${HB_INCLUDE_DIR}/hbfeedback

    INSTALLS += target pubheaders convheaders
    win32:INSTALLS += dlltarget
}

#QMAKE_DISTCLEAN += $$hbNativePath($${HB_BUILD_DIR}/include/hbfeedback/*)
#QMAKE_DISTCLEAN += $$hbNativePath($${HB_BUILD_DIR}/include/hbfeedback/private/*)

symbian {
    LIBS += -ltouchfeedback -lcone -lws32
    LIBS        += -ltacticon
    INCLUDEPATH += $${EPOCROOT}epoc32/include/platform/mw

    defFilePath = defs
    
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = CAP_GENERAL_DLL
    TARGET.UID3=0x20022E9D
    # for touchfeedback headers
    INCLUDEPATH += $${EPOCROOT}epoc32/include/mw

    hbExportHeaders(hbfeedback)
}

include($${HB_SOURCE_DIR}/src/hbcommon.pri)
