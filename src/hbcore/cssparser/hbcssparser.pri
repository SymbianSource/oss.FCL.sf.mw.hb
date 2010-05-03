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

PUBLIC_HEADERS += $$PWD/hbstyleloader.h

PRIVATE_HEADERS += $$PWD/hbcssparser_p.h
PRIVATE_HEADERS += $$PWD/hblayeredstyleloader_p.h
PRIVATE_HEADERS += $$PWD/hbstyleselector_p.h
PRIVATE_HEADERS += $$PWD/hbwidgetstyleloader_p.h
PRIVATE_HEADERS += $$PWD/hbcssformatter_p.h

SOURCES += $$PWD/hbcssparser_p.cpp
SOURCES += $$PWD/hbcssscanner_p.cpp
SOURCES += $$PWD/hblayeredstyleloader_p.cpp
SOURCES += $$PWD/hbstyleloader.cpp
SOURCES += $$PWD/hbstyleselector_p.cpp
SOURCES += $$PWD/hbwidgetstyleloader_p.cpp
SOURCES += $$PWD/hbcssformatter_p.cpp
