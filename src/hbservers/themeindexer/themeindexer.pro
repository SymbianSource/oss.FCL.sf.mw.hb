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
TARGET = themeindexer
DEPENDPATH += .
DEPENDPATH += $${HB_SOURCE_DIR}/src/hbcore/core
DEPENDPATH += $${HB_SOURCE_DIR}/src/hbcore/image
INCLUDEPATH += .
INCLUDEPATH += $${HB_SOURCE_DIR}/src/hbcore/core
INCLUDEPATH += $${HB_SOURCE_DIR}/src/hbcore/image
DEFINES += HB_BOOTSTRAPPED
QT = core gui svg
CONFIG += console
CONFIG -= app_bundle

# directories
DESTDIR = $${HB_BUILD_DIR}/bin

# dependencies
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/core/hbthemeindex_p.h
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/core/hbthemeindex.cpp

HEADERS += $${HB_SOURCE_DIR}/src/hbcore/image/hbiconsource_p.h
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/image/hbiconsource.cpp

# Input
SOURCES += main.cpp

# installation
!local {
    target.path = $${HB_BIN_DIR}
    INSTALLS += target
}

include($${HB_SOURCE_DIR}/src/hbcommon.pri)
