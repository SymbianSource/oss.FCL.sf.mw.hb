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

PUBLIC_HEADERS += $$PWD/hbdocumentloader.h
PUBLIC_HEADERS += $$PWD/hbdocumentloaderplugin.h

PRIVATE_HEADERS += $$PWD/hbdocumentloader_p.h
PRIVATE_HEADERS += $$PWD/hbdocumentloaderactions_p.h
PRIVATE_HEADERS += $$PWD/hbdocumentloaderfactory_p.h
PRIVATE_HEADERS += $$PWD/hbdocumentloadersyntax_p.h

SOURCES += $$PWD/hbdocumentloader.cpp
SOURCES += $$PWD/hbdocumentloader_p.cpp
SOURCES += $$PWD/hbdocumentloaderactions_p.cpp
SOURCES += $$PWD/hbdocumentloaderfactory_p.cpp
SOURCES += $$PWD/hbdocumentloaderplugin.cpp
SOURCES += $$PWD/hbdocumentloadersyntax_p.cpp
