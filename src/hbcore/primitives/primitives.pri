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
#

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

PUBLIC_HEADERS += $$PWD/hbframeitem.h
PUBLIC_HEADERS += $$PWD/hbiconitem.h
PUBLIC_HEADERS += $$PWD/hbtextitem.h
PUBLIC_HEADERS += $$PWD/hbtoucharea.h
PUBLIC_HEADERS += $$PWD/hbrichtextitem.h
PUBLIC_HEADERS += $$PWD/hbmarqueeitem.h

PRIVATE_HEADERS += $$PWD/hbiconitem_p.h
PRIVATE_HEADERS += $$PWD/hbtextitem_p.h
PRIVATE_HEADERS += $$PWD/hbrichtextitem_p.h
PRIVATE_HEADERS += $$PWD/hbtoucharea_p.h
PRIVATE_HEADERS += $$PWD/hbslidertrackitem_p.h
PRIVATE_HEADERS += $$PWD/hbmarqueeitem_p.h

SOURCES += $$PWD/hbframeitem.cpp
SOURCES += $$PWD/hbiconitem.cpp
SOURCES += $$PWD/hbtextitem.cpp
SOURCES += $$PWD/hbtoucharea.cpp
SOURCES += $$PWD/hbrichtextitem.cpp
SOURCES += $$PWD/hbslidertrackitem.cpp
SOURCES += $$PWD/hbmarqueeitem.cpp
