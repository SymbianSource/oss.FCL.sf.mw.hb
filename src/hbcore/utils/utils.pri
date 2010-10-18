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

PUBLIC_HEADERS += $$PWD/hbfontspec.h
PUBLIC_HEADERS += $$PWD/hbdeviceprofile.h
PUBLIC_HEADERS += $$PWD/hbsmileytheme.h

RESTRICTED_HEADERS += $$PWD/hbfeaturemanager_r.h
RESTRICTED_HEADERS += $$PWD/hbtextmeasurementutility_r.h

PRIVATE_HEADERS += $$PWD/hbdeviceprofile_p.h
PRIVATE_HEADERS += $$PWD/hbtypefaceinfo_p.h
PRIVATE_HEADERS += $$PWD/hbtypefaceinfodatabase_p.h
PRIVATE_HEADERS += $$PWD/hbdeviceprofiledatabase_p.h
PRIVATE_HEADERS += $$PWD/hbdeviceprofilemanager_p.h
PRIVATE_HEADERS += $$PWD/hbdeviceprofilereader_p.h
PRIVATE_HEADERS += $$PWD/hbthetestwidget_p.h
PRIVATE_HEADERS += $$PWD/hbthetestutility_p.h
PRIVATE_HEADERS += $$PWD/hbtextutils_p.h
PRIVATE_HEADERS += $$PWD/hbtextmeasurementutility_r_p.h
PRIVATE_HEADERS += $$PWD/hbtypefacexmlparser_p.h
PRIVATE_HEADERS += $$PWD/hbscreenmode_p.h
PRIVATE_HEADERS += $$PWD/hbdevicemodeinfo_p.h
PRIVATE_HEADERS += $$PWD/hbwsiniparser_p.h
PRIVATE_HEADERS += $$PWD/hbwidgetloader_p.h
PRIVATE_HEADERS += $$PWD/hbforegroundwatcher_p.h
PRIVATE_HEADERS += $$PWD/hboogmwatcher_p.h
PRIVATE_HEADERS += $$PWD/hbsleepmodelistener_p.h
PRIVATE_HEADERS += $$PWD/hbsleepmodelistener_p_p.h

symbian {
  PRIVATE_HEADERS += $$PWD/hboogmwatcher_sym_p.h
} else {
  PRIVATE_HEADERS += $$PWD/hboogmwatcher_dummy_p.h
}

PRIVATE_HEADERS += $$PWD/hbxmlloaderabstractsyntax_p.h
PRIVATE_HEADERS += $$PWD/hbxmlloaderabstractactions_p.h
PRIVATE_HEADERS += $$PWD/hbxmlloaderbasesyntax_p.h
PRIVATE_HEADERS += $$PWD/hbxmlloaderbaseactions_p.h
PRIVATE_HEADERS += $$PWD/hbxmlloaderbinarysyntax_p.h
PRIVATE_HEADERS += $$PWD/hbxmlloaderbinaryactions_p.h
PRIVATE_HEADERS += $$PWD/hbwidgetloadersyntax_p.h
PRIVATE_HEADERS += $$PWD/hbwidgetloaderactions_p.h

PRIVATE_HEADERS += $$PWD/hbtimer_p.h
PRIVATE_HEADERS += $$PWD/hbsmileythemeparser_p.h

SOURCES += $$PWD/hbdeviceprofile.cpp
SOURCES += $$PWD/hbdeviceprofiledatabase_p.cpp
SOURCES += $$PWD/hbdeviceprofilemanager_p.cpp
SOURCES += $$PWD/hbdeviceprofilereader_p.cpp
SOURCES += $$PWD/hbfontspec.cpp
SOURCES += $$PWD/hbtextmeasurementutility.cpp
SOURCES += $$PWD/hbthetestwidget_p.cpp
SOURCES += $$PWD/hbthetestutility_p.cpp
SOURCES += $$PWD/hbtextutils_p.cpp
SOURCES += $$PWD/hbtypefaceinfo.cpp
SOURCES += $$PWD/hbtypefaceinfodatabase_p.cpp
SOURCES += $$PWD/hbtypefacexmlparser.cpp
SOURCES += $$PWD/hbscreenmode_p.cpp
SOURCES += $$PWD/hbdevicemodeinfo_p.cpp
SOURCES += $$PWD/hbwsiniparser_p.cpp
SOURCES += $$PWD/hbwidgetloader.cpp
SOURCES += $$PWD/hbforegroundwatcher.cpp
SOURCES += $$PWD/hboogmwatcher.cpp

symbian: SOURCES += $$PWD/hboogmwatcher_sym_p.cpp

SOURCES += $$PWD/hbxmlloaderabstractsyntax_p.cpp
SOURCES += $$PWD/hbxmlloaderabstractactions_p.cpp
SOURCES += $$PWD/hbxmlloaderbasesyntax_p.cpp
SOURCES += $$PWD/hbxmlloaderbaseactions_p.cpp
SOURCES += $$PWD/hbxmlloaderbinarysyntax_p.cpp
SOURCES += $$PWD/hbxmlloaderbinaryactions_p.cpp
SOURCES += $$PWD/hbwidgetloadersyntax_p.cpp
SOURCES += $$PWD/hbwidgetloaderactions_p.cpp

SOURCES += $$PWD/hbtimer.cpp
SOURCES += $$PWD/hbsmileytheme.cpp
SOURCES += $$PWD/hbsmileythemeparser_p.cpp
SOURCES += $$PWD/hbfeaturemanager.cpp
SOURCES += $$PWD/hbsleepmodelistener_p.cpp

