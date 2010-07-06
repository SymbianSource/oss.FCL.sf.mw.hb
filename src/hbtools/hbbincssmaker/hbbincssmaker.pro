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
TARGET = hbbincssmaker

# dependencies
# hbAddLibrary(hbcore/HbCore)

DEPENDPATH += .

DEPENDPATH += $${HB_SOURCE_DIR}/src/hbcore/core
DEPENDPATH += $${HB_SOURCE_DIR}/src/hbcore/theme
DEPENDPATH += $${HB_SOURCE_DIR}/src/hbcore/cssparser
DEPENDPATH += $${HB_SOURCE_DIR}/src/hbcore/utils
DEPENDPATH += $${HB_SOURCE_DIR}/src/hbcore/gui
DEPENDPATH += $${HB_SOURCE_DIR}/src/hbcore/image
DEPENDPATH += $${HB_SOURCE_DIR}/src/hbcore/style
DEPENDPATH += $${HB_SOURCE_DIR}/src/hbcore/feedback
DEPENDPATH += $${HB_SOURCE_DIR}/src/hbcore/layouts

INCLUDEPATH += .
INCLUDEPATH += $${HB_SOURCE_DIR}/src/hbcore/core
INCLUDEPATH += $${HB_SOURCE_DIR}/src/hbcore/theme
INCLUDEPATH += $${HB_SOURCE_DIR}/src/hbcore/cssparser
INCLUDEPATH += $${HB_SOURCE_DIR}/src/hbcore/utils
INCLUDEPATH += $${HB_SOURCE_DIR}/src/hbcore/gui
INCLUDEPATH += $${HB_SOURCE_DIR}/src/hbcore/image
INCLUDEPATH += $${HB_SOURCE_DIR}/src/hbcore/style
INCLUDEPATH += $${HB_SOURCE_DIR}/src/hbcore/feedback
INCLUDEPATH += $${HB_SOURCE_DIR}/src/hbcore/layouts
QT = core gui svg
QT += network

CONFIG += console
CONFIG -= app_bundle

# directories
DESTDIR = $${HB_BUILD_DIR}/bin

# headers and sources from hbcore
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/core/hbmemorymanager_p.h
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/core/hbsharedmemorymanager_p.h
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/core/hbvector_p.h
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/core/hbvariant_p.h
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/core/hbstring_p.h
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/core/hbstringdata_p.h
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/core/hbstringvector_p.h
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/core/hbsmartpointer_p.h
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/core/hbsmartoffset_p.h
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/core/hbsharedcache_p.h
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/core/hbmemoryutils_p.h
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/core/hbthemeperf_p.h
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/theme/hbthemecommon_p.h
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/cssparser/hbcssparser_p.h

#memory manager source dependencies
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/core/hbmemorymanager_p.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/core/hbsharedmemorymanager_p.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/core/hbsplaytreeallocator_p.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/core/hbmultisegmentallocator_p.cpp

#shared memory container sources
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/core/hbvariant_p.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/core/hbstring_p.cpp

SOURCES += $${HB_SOURCE_DIR}/src/hbcore/core/hbsharedcache.cpp

# css parser source dependencies
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/cssparser/hbcssparser_p.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/cssparser/hblayeredstyleloader_p.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/cssparser/hbwidgetstyleloader_p.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/cssparser/hbstyleselector_p.cpp

# widgetml parsing.
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/utils/hbwidgetloader.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/utils/hbwidgetloaderactions_p.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/utils/hbwidgetloadersyntax_p.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/utils/hbxmlloaderbaseactions_p.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/utils/hbxmlloaderabstractsyntax_p.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/utils/hbxmlloaderbasesyntax_p.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/utils/hbxmlloaderabstractactions_p.cpp

DEFINES += HB_BOOTSTRAPPED
DEFINES += HB_BIN_CSS
DEFINES -= HB_GESTURE_FW
DEFINES -= HB_CSS_INSPECTOR
DEFINES += HB_BUILD_DIR=\"\\\"$${HB_BUILD_DIR}\\\"\"
DEFINES += HB_BIN_DIR=\"\\\"$${HB_BIN_DIR}\\\"\"
DEFINES += HB_PLUGINS_DIR=\"\\\"$${HB_PLUGINS_DIR}\\\"\"

# Input
HEADERS += hbcssconverterutils_p.h
HEADERS += hboffsetmapbuilder_p.h

SOURCES += main.cpp
SOURCES += hbcssconverterutils.cpp
SOURCES += hboffsetmapbuilder.cpp

# installation
!local {
    target.path = $${HB_BIN_DIR}
    INSTALLS += target
}

include($${HB_SOURCE_DIR}/src/hbcommon.pri)
