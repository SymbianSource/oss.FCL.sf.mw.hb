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

PUBLIC_HEADERS += $$PWD/hbabstractslidercontrol.h
PUBLIC_HEADERS += $$PWD/hbprogressbar.h
PUBLIC_HEADERS += $$PWD/hbprogressslider.h
PUBLIC_HEADERS += $$PWD/hbprogressiveslider.h
PUBLIC_HEADERS += $$PWD/hbslider.h
PUBLIC_HEADERS += $$PWD/hbratingslider.h


PRIVATE_HEADERS += $$PWD/hbabstractslidercontrol_p.h
PRIVATE_HEADERS += $$PWD/hbprogresssliderhandle_p.h
PRIVATE_HEADERS += $$PWD/hbprogressbar_p.h
PRIVATE_HEADERS += $$PWD/hbprogressslider_p.h
PRIVATE_HEADERS += $$PWD/hbprogressiveslider_p.h
PRIVATE_HEADERS += $$PWD/hbslider_p.h
PRIVATE_HEADERS += $$PWD/hbslidercontrol_p.h
PRIVATE_HEADERS += $$PWD/hbprogressslidercontrol_p.h
PRIVATE_HEADERS += $$PWD/hbslidercontrol_p_p.h
PRIVATE_HEADERS += $$PWD/hbsliderhandle_p.h
PRIVATE_HEADERS += $$PWD/hbratingslider_p.h
PRIVATE_HEADERS += $$PWD/hbslidertickmarks_p.h
PRIVATE_HEADERS += $$PWD/hbslidertickmarkslabel_p.h
PRIVATE_HEADERS += $$PWD/hbvolumeslider_p.h
PRIVATE_HEADERS += $$PWD/hbzoomslider_p.h


SOURCES += $$PWD/hbabstractslidercontrol.cpp
SOURCES += $$PWD/hbprogressslider.cpp
SOURCES += $$PWD/hbprogressiveslider.cpp
SOURCES += $$PWD/hbprogressbar.cpp
SOURCES += $$PWD/hbprogresssliderhandle_p.cpp
SOURCES += $$PWD/hbslider.cpp
SOURCES += $$PWD/hbslidercontrol.cpp
SOURCES += $$PWD/hbprogressslidercontrol.cpp
SOURCES += $$PWD/hbsliderhandle.cpp
SOURCES += $$PWD/hbslidertickmarks.cpp
SOURCES += $$PWD/hbslidertickmarkslabel.cpp

SOURCES += $$PWD/hbvolumeslider_p.cpp
SOURCES += $$PWD/hbzoomslider_p.cpp
SOURCES += $$PWD/hbratingslider.cpp

