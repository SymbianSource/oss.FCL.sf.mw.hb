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

PUBLIC_HEADERS += $$PWD/hbi18ndef.h
PUBLIC_HEADERS += $$PWD/hbextendedlocale.h
PUBLIC_HEADERS += $$PWD/hbstringutil.h
PUBLIC_HEADERS += $$PWD/hbdirectorynamelocalizer.h
PUBLIC_HEADERS += $$PWD/hbnumbergrouping.h
PUBLIC_HEADERS += $$PWD/hbparameterlengthlimiter.h
PUBLIC_HEADERS += $$PWD/hbfindfile.h
PUBLIC_HEADERS += $$PWD/hbtranslator.h
PUBLIC_HEADERS += $$PWD/hblanguageutil.h

PRIVATE_HEADERS += $$PWD/hbdntxmlreader_p.h
PRIVATE_HEADERS += $$PWD/hbngnormalnumber_p.h
PRIVATE_HEADERS += $$PWD/hbnumbergrpxmlreader_p.h
PRIVATE_HEADERS += $$PWD/hbdirectorynamelocalizer_p.h
PRIVATE_HEADERS += $$PWD/hbstringutil_p.h
PRIVATE_HEADERS += $$PWD/hbparameterlengthlimiter_p.h
PRIVATE_HEADERS += $$PWD/hbtranslator_p.h

SOURCES += $$PWD/hbextendedlocale.cpp
SOURCES += $$PWD/hbstringutil.cpp
SOURCES += $$PWD/hbdirectorynamelocalizer.cpp
SOURCES += $$PWD/hbdntxmlreader.cpp
SOURCES += $$PWD/hbnumbergrouping.cpp
SOURCES += $$PWD/hbngnormalnumber.cpp
SOURCES += $$PWD/hbnumbergrpxmlreader.cpp
SOURCES += $$PWD/hbparameterlengthlimiter.cpp
SOURCES += $$PWD/hbfindfile.cpp
SOURCES += $$PWD/hbtranslator.cpp
SOURCES += $$PWD/hblanguageutil.cpp

symbian:LIBS += -leuser -lhal -lcentralrepository -lptiengine -lSysLangUtil
symbian:TARGET.CAPABILITY += WriteDeviceData

