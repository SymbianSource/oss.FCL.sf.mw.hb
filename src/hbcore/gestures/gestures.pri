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

PUBLIC_HEADERS += $$PWD/hbtapandholdgesture.h
PUBLIC_HEADERS += $$PWD/hbpangesture.h
PUBLIC_HEADERS += $$PWD/hbtapgesture.h
PUBLIC_HEADERS += $$PWD/hbswipegesture.h
PUBLIC_HEADERS += $$PWD/hbpinchgesture.h

PRIVATE_HEADERS += $$PWD/hbgestures_p.h
PRIVATE_HEADERS += $$PWD/hbpangesturelogic_p.h
PRIVATE_HEADERS += $$PWD/hbtapgesturelogic_p.h
PRIVATE_HEADERS += $$PWD/hbtapandholdgesturelogic_p.h
PRIVATE_HEADERS += $$PWD/hbpinchgesturelogic_p.h
PRIVATE_HEADERS += $$PWD/hbpointrecorder_p.h
PRIVATE_HEADERS += $$PWD/hbvelocitycalculator_p.h    
PRIVATE_HEADERS += $$PWD/hbswipegesturelogic_p.h
PRIVATE_HEADERS += $$PWD/hbgesturerecognizers_p.h

SOURCES += $$PWD/hbgesturerecognizers_p.cpp
SOURCES += $$PWD/hbpangesturelogic_p.cpp
SOURCES += $$PWD/hbtapgesturelogic_p.cpp
SOURCES += $$PWD/hbtapandholdgesturelogic_p.cpp
SOURCES += $$PWD/hbpinchgesturelogic_p.cpp
SOURCES += $$PWD/hbpointrecorder_p.cpp
SOURCES += $$PWD/hbvelocitycalculator_p.cpp
SOURCES += $$PWD/hbswipegesturelogic_p.cpp
SOURCES += $$PWD/hbtapandholdgesture.cpp
SOURCES += $$PWD/hbtapgesture.cpp
SOURCES += $$PWD/hbpangesture.cpp
SOURCES += $$PWD/hbswipegesture.cpp
SOURCES += $$PWD/hbpinchgesture.cpp
