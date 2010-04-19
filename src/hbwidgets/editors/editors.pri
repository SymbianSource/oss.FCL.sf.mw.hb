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
PUBLIC_HEADERS += $$PWD/hbabstractedit.h
PUBLIC_HEADERS += $$PWD/hbdatetimeedit.h
PUBLIC_HEADERS += $$PWD/hblineedit.h
PUBLIC_HEADERS += $$PWD/hbtextedit.h
PUBLIC_HEADERS += $$PWD/hbvalidator.h
PUBLIC_HEADERS += $$PWD/hbvalidatorfield.h
PUBLIC_HEADERS += $$PWD/hbformatdialog.h

PUBLIC_HEADERS += $$PWD/hbsmileyengine.h
INTERNAL_HEADERS += $$PWD/hbabstractedit_p.h
INTERNAL_HEADERS += $$PWD/hbdatetimeedit_p.h
INTERNAL_HEADERS += $$PWD/hbdatetimevalidator_p.h
INTERNAL_HEADERS += $$PWD/hbdatetimevalidator_p_p.h
INTERNAL_HEADERS += $$PWD/hblineedit_p.h
INTERNAL_HEADERS += $$PWD/hbtextedit_p.h
INTERNAL_HEADERS += $$PWD/hbvalidator_p.h
INTERNAL_HEADERS += $$PWD/hbselectioncontrol_p.h
INTERNAL_HEADERS += $$PWD/hbformatdialog_p.h
INTERNAL_HEADERS += $$PWD/hbsmileyengine_p.h

SOURCES += $$PWD/hbabstractedit.cpp
SOURCES += $$PWD/hbabstractedit_p.cpp
SOURCES += $$PWD/hbdatetimeedit.cpp
SOURCES += $$PWD/hbdatetimeedit_p.cpp
SOURCES += $$PWD/hbdatetimevalidator_p.cpp
SOURCES += $$PWD/hblineedit.cpp
SOURCES += $$PWD/hblineedit_p.cpp
SOURCES += $$PWD/hbtextedit.cpp
SOURCES += $$PWD/hbtextedit_p.cpp
SOURCES += $$PWD/hbvalidator.cpp
SOURCES += $$PWD/hbvalidator_p.cpp
SOURCES += $$PWD/hbselectioncontrol_p.cpp
SOURCES += $$PWD/hbformatdialog.cpp
SOURCES += $$PWD/hbformatdialog_p.cpp
SOURCES += $$PWD/hbsmileyengine.cpp

