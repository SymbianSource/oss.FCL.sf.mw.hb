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

PUBLIC_HEADERS += $$PWD/hbstyle.h
PUBLIC_HEADERS += $$PWD/hbstyleparameters.h  

PRIVATE_HEADERS += $$PWD/hbstyleoption_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionabstractviewitem_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptioncheckbox_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptioncolorgridviewitem_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptiongridviewitem_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionindexfeedback_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionindicatorgroup_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionlabel_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionlistviewitem_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionmenuitem_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionnotificationdialog_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionmessagebox_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionpopup_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionprogressbar_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionprogressslider_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionprogressdialog_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionpushbutton_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionscrollbar_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionslider_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionnavigationbutton_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionindicatorbutton_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptiontitlepane_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptiontoolbutton_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptiontooltip_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptiontreeviewitem_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptiondataform_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptiondataformviewitem_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptiongroupbox_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionratingslider_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptioncombobox_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptioninputdialog_p.h
PRIVATE_HEADERS += $$PWD/hbstyleinterface_p.h
PRIVATE_HEADERS += $$PWD/hbstyle_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionsliderelement_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionprogresssliderhandle_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptiondatagroup_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionstatusbar_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionsignalindicator_p.h
PRIVATE_HEADERS += $$PWD/hbstyleoptionbatteryindicator_p.h

SOURCES += $$PWD/hbstyle.cpp
SOURCES += $$PWD/hbstyleoption.cpp
SOURCES += $$PWD/hbstyleoptionabstractviewitem.cpp
SOURCES += $$PWD/hbstyleoptioncheckbox.cpp
SOURCES += $$PWD/hbstyleoptioncolorgridviewitem.cpp
SOURCES += $$PWD/hbstyleoptiongridviewitem.cpp
SOURCES += $$PWD/hbstyleoptionindexfeedback.cpp
SOURCES += $$PWD/hbstyleoptionindicatorgroup.cpp
SOURCES += $$PWD/hbstyleoptionlabel.cpp
SOURCES += $$PWD/hbstyleoptionlistviewitem.cpp
SOURCES += $$PWD/hbstyleoptionmenuitem.cpp
SOURCES += $$PWD/hbstyleoptionnotificationdialog.cpp
SOURCES += $$PWD/hbstyleoptionmessagebox.cpp
SOURCES += $$PWD/hbstyleoptionpopup.cpp
SOURCES += $$PWD/hbstyleoptionprogressbar.cpp
SOURCES += $$PWD/hbstyleoptionprogressslider.cpp
SOURCES += $$PWD/hbstyleoptionprogresssliderhandle.cpp
SOURCES += $$PWD/hbstyleoptionprogressdialog.cpp
SOURCES += $$PWD/hbstyleoptionpushbutton.cpp
SOURCES += $$PWD/hbstyleoptionscrollbar.cpp
SOURCES += $$PWD/hbstyleoptionslider.cpp
SOURCES += $$PWD/hbstyleoptionsliderelement.cpp
SOURCES += $$PWD/hbstyleoptionstatusbar.cpp
SOURCES += $$PWD/hbstyleoptionnavigationbutton.cpp
SOURCES += $$PWD/hbstyleoptionindicatorbutton.cpp
SOURCES += $$PWD/hbstyleoptionsignalindicator.cpp
SOURCES += $$PWD/hbstyleoptionbatteryindicator.cpp
SOURCES += $$PWD/hbstyleoptiontitlepane.cpp
SOURCES += $$PWD/hbstyleoptiontoolbutton.cpp
SOURCES += $$PWD/hbstyleoptiontooltip.cpp
SOURCES += $$PWD/hbstyleoptiontreeviewitem.cpp
SOURCES += $$PWD/hbstyleparameters.cpp
SOURCES += $$PWD/hbstyleinterface.cpp
SOURCES += $$PWD/hbstyleoptiondataform.cpp
SOURCES += $$PWD/hbstyleoptiondatagroup.cpp
SOURCES += $$PWD/hbstyleoptiondatagroupheadingwidget.cpp
SOURCES += $$PWD/hbstyleoptiondataformviewitem.cpp
SOURCES += $$PWD/hbstyleoptionratingslider.cpp
SOURCES += $$PWD/hbstyleoptiongroupbox.cpp
SOURCES += $$PWD/hbstyleoptioncombobox.cpp
SOURCES += $$PWD/hbstyleoptioninputdialog.cpp


