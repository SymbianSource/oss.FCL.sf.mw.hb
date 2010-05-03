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

# hb input widget public headers
PUBLIC_HEADERS += $$PWD/hbinputvkbwidget.h
PUBLIC_HEADERS += $$PWD/hbinputvirtualrocker.h
PUBLIC_HEADERS += $$PWD/hbinputtouchkeypadbutton.h
PUBLIC_HEADERS += $$PWD/hbinput12keytouchkeypad.h
PUBLIC_HEADERS += $$PWD/hbinputsctportrait.h
PUBLIC_HEADERS += $$PWD/hbinputusedsymbolpane.h
PUBLIC_HEADERS += $$PWD/hbinputcandidatelist.h
PUBLIC_HEADERS += $$PWD/hbinputqwertytouchkeyboard.h
PUBLIC_HEADERS += $$PWD/hbinputpreviewlabel.h
PUBLIC_HEADERS += $$PWD/hbinputcharpreviewpane.h
PUBLIC_HEADERS += $$PWD/hbinputmodeindicator.h
PUBLIC_HEADERS += $$PWD/hbinputcustombuttonlist.h
PUBLIC_HEADERS += $$PWD/hbinputexactwordpopup.h
PUBLIC_HEADERS += $$PWD/hbinputsctlandscape.h
PUBLIC_HEADERS += $$PWD/hbinputcommondialogs.h
PUBLIC_HEADERS += $$PWD/hbinputsettingdialog.h
PUBLIC_HEADERS += $$PWD/hbinputhwtoolcluster.h
PUBLIC_HEADERS += $$PWD/hbinputsmileypicker.h
PUBLIC_HEADERS += $$PWD/hbinputscreenshotwidget.h
PUBLIC_HEADERS += $$PWD/hbinputsettinglist.h
PUBLIC_HEADERS += $$PWD/hbinputsettingwidget.h
# hb input widget private headers
PRIVATE_HEADERS += $$PWD/hbinputvkbwidget_p.h
PRIVATE_HEADERS += $$PWD/hbinput12keytouchkeypad_p.h
PRIVATE_HEADERS += $$PWD/hbinputqwertytouchkeyboard_p.h
PRIVATE_HEADERS += $$PWD/hbinputsctlandscape_p.h
PRIVATE_HEADERS += $$PWD/hbinputsctportrait_p.h
PRIVATE_HEADERS += $$PWD/hbinputcheckboxlist_p.h

# hb input widget sources
SOURCES += $$PWD/hbinputvkbwidget.cpp
SOURCES += $$PWD/hbinputvirtualrocker.cpp
SOURCES += $$PWD/hbinputtouchkeypadbutton.cpp
SOURCES += $$PWD/hbinput12keytouchkeypad.cpp
SOURCES += $$PWD/hbinputsctportrait.cpp
SOURCES += $$PWD/hbinputusedsymbolpane.cpp
SOURCES += $$PWD/hbinputcandidatelist.cpp
SOURCES += $$PWD/hbinputqwertytouchkeyboard.cpp
SOURCES += $$PWD/hbinputpreviewlabel.cpp
SOURCES += $$PWD/hbinputcharpreviewpane.cpp
SOURCES += $$PWD/hbinputmodeindicator.cpp
SOURCES += $$PWD/hbinputcustombuttonlist.cpp
SOURCES += $$PWD/hbinputexactwordpopup.cpp
SOURCES += $$PWD/hbinputsctlandscape.cpp
SOURCES += $$PWD/hbinputcommondialogs.cpp
SOURCES += $$PWD/hbinputsettingdialog.cpp
SOURCES += $$PWD/hbinputhwtoolcluster.cpp
SOURCES += $$PWD/hbinputsmileypicker.cpp
SOURCES += $$PWD/hbinputscreenshotwidget.cpp
SOURCES += $$PWD/hbinputsettinglist.cpp
SOURCES += $$PWD/hbinputsettingwidget.cpp
SOURCES += $$PWD/hbinputcheckboxlist.cpp
