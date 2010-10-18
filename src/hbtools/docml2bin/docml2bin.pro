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
TARGET = docml2bin

#DEPENDPATH += .
DEPENDPATH += $${HB_SOURCE_DIR}/src/hbutils/document
DEPENDPATH += $${HB_SOURCE_DIR}/src/hbcore/core
DEPENDPATH += $${HB_SOURCE_DIR}/src/hbcore/cssparser
DEPENDPATH += $${HB_SOURCE_DIR}/src/hbcore/image
DEPENDPATH += $${HB_SOURCE_DIR}/src/hbcore/utils
DEPENDPATH += $${HB_SOURCE_DIR}/src/hbcore/layouts
#INCLUDEPATH += .
INCLUDEPATH += $${HB_SOURCE_DIR}/src/hbutils/document
INCLUDEPATH += $${HB_SOURCE_DIR}/src/hbcore/core
INCLUDEPATH += $${HB_SOURCE_DIR}/src/hbcore/cssparser
INCLUDEPATH += $${HB_SOURCE_DIR}/src/hbcore/image
INCLUDEPATH += $${HB_SOURCE_DIR}/src/hbcore/utils
INCLUDEPATH += $${HB_SOURCE_DIR}/src/hbcore/layouts
DEFINES += HB_BOOTSTRAPPED
CONFIG += console
CONFIG -= app_bundle

# directories
DESTDIR = $${HB_BUILD_DIR}/bin

# dependencies
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/core/hbhash_p.h
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/core/hbhash_p.cpp
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/cssparser/hbexpressionparser_p.h
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/cssparser/hbexpressionparser_p.cpp
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/layouts/hbanchor.h
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/layouts/hbanchor.cpp
HEADERS += $${HB_SOURCE_DIR}/src/hbcore/utils/hbfontspec.h
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/utils/hbfontspec.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/utils/hbxmlloaderabstractsyntax_p.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/utils/hbxmlloaderabstractactions_p.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/utils/hbxmlloaderbinarysyntax_p.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/utils/hbxmlloaderbinaryactions_p.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbcore/utils/hbxmlloaderbasesyntax_p.cpp
HEADERS += $${HB_SOURCE_DIR}/src/hbutils/document/hbdocumentloader.h
SOURCES += $${HB_SOURCE_DIR}/src/hbutils/document/hbdocumentloader.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbutils/document/hbdocumentloader_p.cpp
SOURCES += $${HB_SOURCE_DIR}/src/hbutils/document/hbdocumentloadersyntax_p.cpp

# Input
SOURCES += main.cpp

# installation
!local {
    target.path = $${HB_BIN_DIR}
    INSTALLS += target
}

include($${HB_SOURCE_DIR}/src/hbcommon.pri)
