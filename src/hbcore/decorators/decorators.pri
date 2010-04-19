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

INTERNAL_HEADERS += $$PWD/hbbatteryindicator_p.h
INTERNAL_HEADERS += $$PWD/hbbatteryindicator_p_p.h
INTERNAL_HEADERS += $$PWD/hbdecorator_p.h
INTERNAL_HEADERS += $$PWD/hbdecorator_p_p.h
INTERNAL_HEADERS += $$PWD/hbindicatorgroup_p.h
INTERNAL_HEADERS += $$PWD/hbindicatorgroup_p_p.h
INTERNAL_HEADERS += $$PWD/hbsignalindicator_p.h
INTERNAL_HEADERS += $$PWD/hbsignalindicator_p_p.h
INTERNAL_HEADERS += $$PWD/hbsleepmodelistener_p.h
INTERNAL_HEADERS += $$PWD/hbsleepmodelistener_p_p.h
INTERNAL_HEADERS += $$PWD/hbsoftkey_p.h
INTERNAL_HEADERS += $$PWD/hbsoftkey_p_p.h
INTERNAL_HEADERS += $$PWD/hbsoftkeygroup_p.h
INTERNAL_HEADERS += $$PWD/hbstatusbar_p.h
INTERNAL_HEADERS += $$PWD/hbstatusbar_p_p.h
INTERNAL_HEADERS += $$PWD/hbtitlebar_p.h
INTERNAL_HEADERS += $$PWD/hbtitlebar_p_p.h
INTERNAL_HEADERS += $$PWD/hbtitlebarhandle_p.h
INTERNAL_HEADERS += $$PWD/hbtitlepane_p.h
INTERNAL_HEADERS += $$PWD/hbtitlepane_p_p.h
INTERNAL_HEADERS += $$PWD/hbsysteminfo_p.h
INTERNAL_HEADERS += $$PWD/hbnavigationbutton_p.h
INTERNAL_HEADERS += $$PWD/hbnavigationbutton_p_p.h
INTERNAL_HEADERS += $$PWD/hbindicatorbutton_p.h
INTERNAL_HEADERS += $$PWD/hbindicatorbutton_p_p.h
INTERNAL_HEADERS += $$PWD/hbindicatorleveliconitem_p.h

SOURCES += $$PWD/hbbatteryindicator.cpp
SOURCES += $$PWD/hbdecorator.cpp
SOURCES += $$PWD/hbindicatorgroup.cpp
SOURCES += $$PWD/hbsignalindicator.cpp
SOURCES += $$PWD/hbsleepmodelistener.cpp
SOURCES += $$PWD/hbsoftkey.cpp
SOURCES += $$PWD/hbsoftkeygroup.cpp
SOURCES += $$PWD/hbstatusbar.cpp
SOURCES += $$PWD/hbtitlebar.cpp
SOURCES += $$PWD/hbtitlebarhandle.cpp
SOURCES += $$PWD/hbtitlepane.cpp
SOURCES += $$PWD/hbsysteminfo.cpp
SOURCES += $$PWD/hbnavigationbutton.cpp
SOURCES += $$PWD/hbindicatorbutton.cpp
SOURCES += $$PWD/hbindicatorleveliconitem.cpp

symbian {
INTERNAL_HEADERS += $$PWD/hbsysteminfo_sym_p_p.h
INTERNAL_HEADERS += $$PWD/hbbatterymonitor_sym_p.h
INTERNAL_HEADERS += $$PWD/hbnetworksignalmonitor_sym_p.h
INTERNAL_HEADERS += $$PWD/hbindicatormonitor_sym_p.h
INTERNAL_HEADERS += $$PWD/hbnetworkmodemonitor_sym_p.h
SOURCES += $$PWD/hbsysteminfo_sym.cpp
SOURCES += $$PWD/hbbatterymonitor_sym.cpp
SOURCES += $$PWD/hbnetworksignalmonitor_sym.cpp
SOURCES += $$PWD/hbindicatormonitor_sym.cpp
SOURCES += $$PWD/hbnetworkmodemonitor_sym.cpp

LIBS += -letel3rdparty \
    -lsysutil \
    -lefsrv \
    -lfeatdiscovery \
    -letelmm \
    -letel
}

win32* {
INTERNAL_HEADERS += $$PWD/hbsysteminfo_win_p_p.h
INTERNAL_HEADERS += $$PWD/hbwmihelper_win_p.h
SOURCES += $$PWD/hbsysteminfo_win.cpp
SOURCES += $$PWD/hbwmihelper_win.cpp

!win32-g++ {
LIBS += Ole32.lib \
        Strmiids.lib \
        Bthprops.lib \
        User32.lib \
        Gdi32.lib \
        Ws2_32.lib \
        Wbemuuid.lib \
        Oleaut32.lib 
}
}

linux-*|macx-* {
INTERNAL_HEADERS += $$PWD/hbsysteminfo_linux_p_p.h
SOURCES += $$PWD/hbsysteminfo_linux.cpp
}
