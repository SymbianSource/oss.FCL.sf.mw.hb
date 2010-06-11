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
TARGET = $$hbLibraryTarget(HbWidgets)
QT = core gui svg network
DEFINES += BUILD_HB_WIDGETS

# directories
DESTDIR = $${HB_BUILD_DIR}/lib
win32:DLLDESTDIR = $${HB_BUILD_DIR}/bin

# components
include(devicedialogs/devicedialogs.pri)
include(editors/editors.pri)
include(itemviews/itemviews.pri)
include(popups/popups.pri)
include(sliders/sliders.pri)
include(widgets/widgets.pri)
include(dataform/dataform.pri)

CONVENIENCE_HEADERS += $${HB_BUILD_DIR}/include/hbwidgets/hbwidgets.h
CONVENIENCE_HEADERS += $$files($${HB_BUILD_DIR}/include/hbwidgets/Hb*)
HEADERS += $$PUBLIC_HEADERS $$RESTRICTED_HEADERS $$PRIVATE_HEADERS $$CONVENIENCE_HEADERS

# dependencies
hbAddLibrary(hbcore/HbCore)

# installation
!local { 
    target.path = $${HB_LIB_DIR}
    win32:dlltarget.path = $${HB_BIN_DIR}

    pubheaders.files = $$PUBLIC_HEADERS
    pubheaders.path = $${HB_INCLUDE_DIR}/hbwidgets

    restheaders.files = $$RESTRICTED_HEADERS
    restheaders.path = $${HB_INCLUDE_DIR}/hbwidgets/restricted

    convheaders.files = $$CONVENIENCE_HEADERS
    convheaders.path = $${HB_INCLUDE_DIR}/hbwidgets

    INSTALLS += target pubheaders restheaders convheaders
    win32:INSTALLS += dlltarget
}

#QMAKE_DISTCLEAN += $$hbNativePath($${HB_BUILD_DIR}/include/hbwidgets/*)
#QMAKE_DISTCLEAN += $$hbNativePath($${HB_BUILD_DIR}/include/hbwidgets/private/*)

symbian {
    defFilePath = defs

    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = CAP_GENERAL_DLL
    TARGET.UID3 = 0x20022FCC

    # DEPLOYMENT_PLUGIN += qjpeg # TODO: Removed because this is already in qt.sis and that caused problems
    DEFINES += SYMBIAN_TARGET_ICON_CACHE_SIZE # TODO: what's this? why not use Q_OS_SYMBIAN?
    INCLUDEPATH += $${EPOCROOT}epoc32/include/osextensions/stdapis/stlport # TODO: depends on S60 version?
    hbExportHeaders(hbwidgets)
}

include($${HB_SOURCE_DIR}/src/hbcommon.pri)
