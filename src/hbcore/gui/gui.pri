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

PUBLIC_HEADERS += $$PWD/hbabstractbutton.h
PUBLIC_HEADERS += $$PWD/hbdialog.h
PUBLIC_HEADERS += $$PWD/hbdockwidget.h
PUBLIC_HEADERS += $$PWD/hbmainwindow.h
PUBLIC_HEADERS += $$PWD/hbmenu.h
PUBLIC_HEADERS += $$PWD/hbpopup.h
PUBLIC_HEADERS += $$PWD/hbscrollarea.h
PUBLIC_HEADERS += $$PWD/hbscrollbar.h
PUBLIC_HEADERS += $$PWD/hbstackedwidget.h
PUBLIC_HEADERS += $$PWD/hbtoolbar.h
PUBLIC_HEADERS += $$PWD/hbtoolbarextension.h
PUBLIC_HEADERS += $$PWD/hbtoolbutton.h
PUBLIC_HEADERS += $$PWD/hbtooltip.h
PUBLIC_HEADERS += $$PWD/hbview.h
PUBLIC_HEADERS += $$PWD/hbwidget.h
PUBLIC_HEADERS += $$PWD/hbwidgetbase.h

INTERNAL_HEADERS += $$PWD/hbabstractbutton_p.h
INTERNAL_HEADERS += $$PWD/hbactionmanager_p.h
INTERNAL_HEADERS += $$PWD/hbactionmanagerxmlparser_p.h
INTERNAL_HEADERS += $$PWD/hbanchorarrowdrawer_p.h
INTERNAL_HEADERS += $$PWD/hbbackgrounditem_p.h
INTERNAL_HEADERS += $$PWD/hbcssinspector_p.h
INTERNAL_HEADERS += $$PWD/hbdevicefadecontrol_p.h
INTERNAL_HEADERS += $$PWD/hbdialog_p.h
INTERNAL_HEADERS += $$PWD/hbdockwidget_p.h
INTERNAL_HEADERS += $$PWD/hbfocusgroup_p.h
INTERNAL_HEADERS += $$PWD/hblongpressvisualizer_p.h
INTERNAL_HEADERS += $$PWD/hbmenucontainer_p.h
INTERNAL_HEADERS += $$PWD/hbmainwindow_p.h
INTERNAL_HEADERS += $$PWD/hbmenu_p.h
INTERNAL_HEADERS += $$PWD/hbmenuitem_p.h
INTERNAL_HEADERS += $$PWD/hbmenuitem_p_p.h
INTERNAL_HEADERS += $$PWD/hbpopup_p.h
INTERNAL_HEADERS += $$PWD/hbpopupmanager_p.h
INTERNAL_HEADERS += $$PWD/hbpopupmanager_p_p.h
INTERNAL_HEADERS += $$PWD/hbscrollarea_p.h
INTERNAL_HEADERS += $$PWD/hbscrollbar_p.h
INTERNAL_HEADERS += $$PWD/hbtestabilitysignal_p.h
INTERNAL_HEADERS += $$PWD/hbtoolbar_p.h
INTERNAL_HEADERS += $$PWD/hbtoolbarextension_p.h
INTERNAL_HEADERS += $$PWD/hbtoolbutton_p.h
INTERNAL_HEADERS += $$PWD/hbtooltiplabel_p.h
INTERNAL_HEADERS += $$PWD/hbtooltiplabel_p_p.h
INTERNAL_HEADERS += $$PWD/hbview_p.h
INTERNAL_HEADERS += $$PWD/hbviewactionmanager_p.h
INTERNAL_HEADERS += $$PWD/hbwidget_p.h
INTERNAL_HEADERS += $$PWD/hbwidgetbase_p.h
INTERNAL_HEADERS += $$PWD/hbwidgetsequentialshow_p.h
INTERNAL_HEADERS += $$PWD/hbnativewindow_sym_p.h
INTERNAL_HEADERS += $$PWD/hbsplash_p.h
INTERNAL_HEADERS += $$PWD/hbsplashscreen_p.h
INTERNAL_HEADERS += $$PWD/hbfadeitem_p.h
INTERNAL_HEADERS += $$PWD/hbcontentwidget_p.h
INTERNAL_HEADERS += $$PWD/hbscreen_p.h
INTERNAL_HEADERS += $$PWD/hbsplashdefs_p.h

SOURCES += $$PWD/hbabstractbutton.cpp
SOURCES += $$PWD/hbactionmanager.cpp
SOURCES += $$PWD/hbactionmanagerxmlparser.cpp
SOURCES += $$PWD/hbanchorarrowdrawer_p.cpp
SOURCES += $$PWD/hbbackgrounditem.cpp
SOURCES += $$PWD/hbcssinspector_p.cpp
SOURCES += $$PWD/hbdialog.cpp
SOURCES += $$PWD/hbdockwidget.cpp
SOURCES += $$PWD/hbfocusgroup_p.cpp
SOURCES += $$PWD/hblongpressvisualizer.cpp
SOURCES += $$PWD/hbmainwindow.cpp
SOURCES += $$PWD/hbmainwindow_p.cpp
SOURCES += $$PWD/hbmenu.cpp
SOURCES += $$PWD/hbmenucontainer_p.cpp
SOURCES += $$PWD/hbmenuitem_p.cpp
SOURCES += $$PWD/hbpopup.cpp
SOURCES += $$PWD/hbpopupmanager.cpp
SOURCES += $$PWD/hbscrollarea.cpp
SOURCES += $$PWD/hbscrollarea_p.cpp
SOURCES += $$PWD/hbscrollbar.cpp
SOURCES += $$PWD/hbstackedwidget.cpp
SOURCES += $$PWD/hbtestabilitysignal.cpp
SOURCES += $$PWD/hbtoolbar.cpp
SOURCES += $$PWD/hbtoolbar_p.cpp
SOURCES += $$PWD/hbtoolbarextension.cpp
SOURCES += $$PWD/hbtoolbutton.cpp
SOURCES += $$PWD/hbtooltip.cpp
SOURCES += $$PWD/hbtooltiplabel_p.cpp
SOURCES += $$PWD/hbview.cpp
SOURCES += $$PWD/hbviewactionmanager.cpp
SOURCES += $$PWD/hbwidget.cpp
SOURCES += $$PWD/hbwidgetbase.cpp
SOURCES += $$PWD/hbwidgetsequentialshow.cpp
SOURCES += $$PWD/hbsplash.cpp
SOURCES += $$PWD/hbsplashscreen.cpp
SOURCES += $$PWD/hbfadeitem.cpp
SOURCES += $$PWD/hbcontentwidget.cpp
SOURCES += $$PWD/hbscreen.cpp

symbian:SOURCES += $$PWD/hbdevicefadecontrolsym.cpp
win32|linux-*|macx-*:SOURCES += $$PWD/hbdevicefadecontrolwin.cpp
symbian:SOURCES += $$PWD/hbnativewindow_sym.cpp

settingswindow {
    INTERNAL_HEADERS += $$PWD/hbsettingswindow_p.h
    SOURCES += $$PWD/hbsettingswindow_p.cpp
}
