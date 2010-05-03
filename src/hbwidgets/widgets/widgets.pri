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

PUBLIC_HEADERS += $$PWD/hbcheckbox.h
PUBLIC_HEADERS += $$PWD/hbcombobox.h
PUBLIC_HEADERS += $$PWD/hbgroupbox.h
PUBLIC_HEADERS += $$PWD/hblabel.h
PUBLIC_HEADERS += $$PWD/hbpushbutton.h
PUBLIC_HEADERS += $$PWD/hbsearchpanel.h
PUBLIC_HEADERS += $$PWD/hbtransparentwindow.h
PUBLIC_HEADERS += $$PWD/hbdatetimepicker.h

PRIVATE_HEADERS += $$PWD/hbcombobox_p.h
PRIVATE_HEADERS += $$PWD/hbgroupbox_p.h
PRIVATE_HEADERS += $$PWD/hbgroupboxheadingwidget_p.h
PRIVATE_HEADERS += $$PWD/hbgroupboxcontentwidget_p.h
PRIVATE_HEADERS += $$PWD/hbpushbutton_p.h
PRIVATE_HEADERS += $$PWD/hbsearchpanel_p.h
PRIVATE_HEADERS += $$PWD/hbdatetimepicker_p.h
PRIVATE_HEADERS += $$PWD/hbcombodropdown_p.h
SOURCES += $$PWD/hbcheckbox.cpp
SOURCES += $$PWD/hbcombodropdown_p.cpp
SOURCES += $$PWD/hbcombobox.cpp
SOURCES += $$PWD/hbcombobox_p.cpp
SOURCES += $$PWD/hbgroupbox.cpp
SOURCES += $$PWD/hbgroupboxheadingwidget_p.cpp
SOURCES += $$PWD/hbgroupboxcontentwidget_p.cpp
SOURCES += $$PWD/hblabel.cpp
SOURCES += $$PWD/hbpushbutton.cpp
SOURCES += $$PWD/hbsearchpanel.cpp
SOURCES += $$PWD/hbsearchpanel_p.cpp
SOURCES += $$PWD/hbtransparentwindow.cpp
SOURCES += $$PWD/hbdatetimepicker.cpp
SOURCES += $$PWD/hbdatetimepicker_p.cpp
