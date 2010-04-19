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

PUBLIC_HEADERS += $$PWD/hbframedrawer.h
PUBLIC_HEADERS += $$PWD/hbframebackground.h
PUBLIC_HEADERS += $$PWD/hbicon.h
PUBLIC_HEADERS += $$PWD/hbiconanimationmanager.h
PUBLIC_HEADERS += $$PWD/hbiconanimationdefinition.h
PUBLIC_HEADERS += $$PWD/hbiconanimator.h

INTERNAL_HEADERS += $$PWD/hbframedrawer_p.h
INTERNAL_HEADERS += $$PWD/hbframebackground_p.h
INTERNAL_HEADERS += $$PWD/hbicon_p.h
INTERNAL_HEADERS += $$PWD/hbiconanimation_p.h 
INTERNAL_HEADERS += $$PWD/hbiconanimator_p.h
INTERNAL_HEADERS += $$PWD/hbiconanimationdatatypes_p.h
INTERNAL_HEADERS += $$PWD/hbiconanimationmanager_p.h
INTERNAL_HEADERS += $$PWD/hbiconanimationparser_p.h
INTERNAL_HEADERS += $$PWD/hbiconengine_p.h
INTERNAL_HEADERS += $$PWD/hbiconloader_p.h
INTERNAL_HEADERS += $$PWD/hbicontheme_p.h
INTERNAL_HEADERS += $$PWD/hblayoutdirectionnotifier_p.h
INTERNAL_HEADERS += $$PWD/hbiconimpl_p.h
INTERNAL_HEADERS += $$PWD/hbpixmapiconimpl_p.h
INTERNAL_HEADERS += $$PWD/hbbadgeicon_p.h
INTERNAL_HEADERS += $$PWD/hbbadgeiconinfo_p.h
INTERNAL_HEADERS += $$PWD/hbiconsource_p.h
INTERNAL_HEADERS += $$PWD/hbframedrawerpool_p.h
INTERNAL_HEADERS += $$PWD/hbmaskableiconimpl_p.h
INTERNAL_HEADERS += $$PWD/hbiconimplcreator_p.h

SOURCES += $$PWD/hbframedrawer.cpp
SOURCES += $$PWD/hbframebackground.cpp
SOURCES += $$PWD/hbicon.cpp
SOURCES += $$PWD/hbiconanimation.cpp
SOURCES += $$PWD/hbiconanimationmanager.cpp
SOURCES += $$PWD/hbiconanimationparser.cpp
SOURCES += $$PWD/hbiconanimationdefinition.cpp
SOURCES += $$PWD/hbiconanimator.cpp
SOURCES += $$PWD/hbiconengine.cpp
SOURCES += $$PWD/hbiconloader.cpp
SOURCES += $$PWD/hbicontheme.cpp
SOURCES += $$PWD/hblayoutdirectionnotifier.cpp
SOURCES += $$PWD/hbpixmapiconimpl_p.cpp
SOURCES += $$PWD/hbbadgeicon.cpp
SOURCES += $$PWD/hbbadgeiconinfo.cpp
SOURCES += $$PWD/hbiconsource.cpp
SOURCES += $$PWD/hbframedrawerpool.cpp
SOURCES += $$PWD/hbiconimplcreator_p.cpp

symbian {
    nvg {
        SOURCES += $$PWD/hbnvgiconimpl_p.cpp    
        SOURCES += $$PWD/hbeglstate.cpp
        INTERNAL_HEADERS += $$PWD/hbnvgiconimpl_p.h   
    	INTERNAL_HEADERS += $$PWD/hbeglstate_p.h   
    }
    sgimage {    
        SOURCES += $$PWD/hbsgimageiconimpl_p.cpp
        INTERNAL_HEADERS += $$PWD/hbsgimageiconimpl_p.h
    }
}
