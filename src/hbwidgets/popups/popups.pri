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

PUBLIC_HEADERS += $$PWD/hbcolordialog.h
PUBLIC_HEADERS += $$PWD/hbnotificationdialog.h
PUBLIC_HEADERS += $$PWD/hbinputdialog.h
PUBLIC_HEADERS += $$PWD/hbprogressdialog.h
PUBLIC_HEADERS += $$PWD/hbsliderpopup.h
PUBLIC_HEADERS += $$PWD/hbvolumesliderpopup.h
PUBLIC_HEADERS += $$PWD/hbzoomsliderpopup.h
PUBLIC_HEADERS += $$PWD/hbmessagebox.h
PUBLIC_HEADERS += $$PWD/hbselectiondialog.h


PRIVATE_HEADERS += $$PWD/hbcolordialog_p.h
PRIVATE_HEADERS += $$PWD/hbcolorgridviewitem_p.h
PRIVATE_HEADERS += $$PWD/hbnotificationdialog_p.h
PRIVATE_HEADERS += $$PWD/hbnotificationdialogcontent_p.h
PRIVATE_HEADERS += $$PWD/hbinputdialog_p.h
PRIVATE_HEADERS += $$PWD/hbinputdialogcontent_p.h
PRIVATE_HEADERS += $$PWD/hbprogressdialog_p.h
PRIVATE_HEADERS += $$PWD/hbsliderpopup_p.h
PRIVATE_HEADERS += $$PWD/hbmessagebox_p.h
PRIVATE_HEADERS += $$PWD/hbselectiondialog_p.h


SOURCES += $$PWD/hbcolordialog.cpp
SOURCES += $$PWD/hbcolorgridviewitem.cpp
SOURCES += $$PWD/hbnotificationdialog.cpp
SOURCES += $$PWD/hbnotificationdialogcontent.cpp
SOURCES += $$PWD/hbinputdialog_p.cpp
SOURCES += $$PWD/hbinputdialog.cpp
SOURCES += $$PWD/hbprogressdialog.cpp
SOURCES += $$PWD/hbsliderpopup.cpp
SOURCES += $$PWD/hbvolumesliderpopup.cpp
SOURCES += $$PWD/hbzoomsliderpopup.cpp
SOURCES += $$PWD/hbmessagebox.cpp
SOURCES += $$PWD/hbselectiondialog_p.cpp
SOURCES += $$PWD/hbselectiondialog.cpp
