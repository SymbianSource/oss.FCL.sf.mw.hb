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

INTERNAL_HEADERS += $$PWD/hbdeviceprofile_p.h
INTERNAL_HEADERS += $$PWD/hbtypefaceinfo_p.h
INTERNAL_HEADERS += $$PWD/hbdeviceprofiledatabase_p.h
INTERNAL_HEADERS += $$PWD/hbdeviceprofilemanager_p.h
INTERNAL_HEADERS += $$PWD/hbdeviceprofilereader_p.h
INTERNAL_HEADERS += $$PWD/hbextendeddeviceprofile_p.h
INTERNAL_HEADERS += $$PWD/hbiniparser_p.h
INTERNAL_HEADERS += $$PWD/hbtextmeasurementutility_p.h
INTERNAL_HEADERS += $$PWD/hbthetestwidget_p.h
INTERNAL_HEADERS += $$PWD/hbthetestutility_p.h
INTERNAL_HEADERS += $$PWD/hbtextutils_p.h
INTERNAL_HEADERS += $$PWD/hbtypefacexmlparser_p.h
INTERNAL_HEADERS += $$PWD/hbscreenmode_p.h
INTERNAL_HEADERS += $$PWD/hbdevicemodeinfo_p.h
INTERNAL_HEADERS += $$PWD/hbwsiniparser_p.h
INTERNAL_HEADERS += $$PWD/hbwidgetloader_p.h
INTERNAL_HEADERS += $$PWD/hbforegroundwatcher_p.h
INTERNAL_HEADERS += $$PWD/hboogmwatcher_p.h
INTERNAL_HEADERS += $$PWD/hbfeaturemanager_p.h

symbian {
  INTERNAL_HEADERS += $$PWD/hboogmwatcher_sym_p.h
} else {
  INTERNAL_HEADERS += $$PWD/hboogmwatcher_dummy_p.h
}

INTERNAL_HEADERS += $$PWD/hbxmlloaderabstractsyntax_p.h
INTERNAL_HEADERS += $$PWD/hbxmlloaderabstractactions_p.h
INTERNAL_HEADERS += $$PWD/hbwidgetloadersyntax_p.h
INTERNAL_HEADERS += $$PWD/hbwidgetloaderactions_p.h

INTERNAL_HEADERS += $$PWD/hbtimer_p.h
INTERNAL_HEADERS += $$PWD/hbsmileythemeparser_p.h

SOURCES += $$PWD/hbdeviceprofile.cpp
SOURCES += $$PWD/hbdeviceprofiledatabase_p.cpp
SOURCES += $$PWD/hbdeviceprofilemanager_p.cpp
SOURCES += $$PWD/hbdeviceprofilereader_p.cpp
SOURCES += $$PWD/hbextendeddeviceprofile_p.cpp
SOURCES += $$PWD/hbfontspec.cpp
SOURCES += $$PWD/hbiniparser.cpp
SOURCES += $$PWD/hbtextmeasurementutility_p.cpp
SOURCES += $$PWD/hbthetestwidget_p.cpp
SOURCES += $$PWD/hbthetestutility_p.cpp
SOURCES += $$PWD/hbtextutils_p.cpp
SOURCES += $$PWD/hbtypefaceinfo.cpp
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
SOURCES += $$PWD/hbwidgetloadersyntax_p.cpp
SOURCES += $$PWD/hbwidgetloaderactions_p.cpp

SOURCES += $$PWD/hbtimer.cpp
SOURCES += $$PWD/hbsmileytheme.cpp
SOURCES += $$PWD/hbsmileythemeparser_p.cpp
SOURCES += $$PWD/hbfeaturemanager_p.cpp

