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

TEMPLATE = lib
TARGET = $$hbLibraryTarget(HbTouchInput)
CONFIG += plugin

hbAddLibrary(hbcore/HbCore)
hbAddLibrary(hbwidgets/HbWidgets )
hbAddLibrary(hbinput/HbInput)

INCLUDEPATH += ../common

# directories
DESTDIR = $${HB_BUILD_DIR}/plugins/inputmethods

HEADERS = virtual12key.h
HEADERS += virtualqwerty.h
HEADERS += touchinputplugin.h
HEADERS += hbinputbasic12keyhandler.h
HEADERS += hbinputprediction12keyhandler.h
HEADERS += ..\common\hbinputbasichandler_p.h
HEADERS += ..\common\hbinputmodehandler.h
HEADERS += ..\common\hbinputmodehandler_p.h
HEADERS += ..\common\hbinputpredictionhandler_p.h
HEADERS += hbinputbasicqwertyhandler.h
HEADERS += hbinputpredictionqwertyhandler.h
HEADERS += ..\common\hbinputnumerichandler_p.h
HEADERS += hbinputnumericqwertyhandler.h
HEADERS += hbinputnumeric12keyhandler.h
HEADERS += ..\common\hbinputabstractbase.h

SOURCES = virtual12key.cpp
SOURCES += virtualqwerty.cpp
SOURCES += touchinputplugin.cpp
SOURCES += hbinputbasic12keyhandler.cpp
SOURCES += hbinputprediction12keyhandler.cpp
SOURCES += ..\common\hbinputbasichandler.cpp
SOURCES += ..\common\hbinputmodehandler.cpp
SOURCES += ..\common\hbinputpredictionhandler.cpp
SOURCES += hbinputbasicqwertyhandler.cpp
SOURCES += hbinputpredictionqwertyhandler.cpp
SOURCES += ..\common\hbinputnumerichandler.cpp
SOURCES += hbinputnumericqwertyhandler.cpp
SOURCES += hbinputnumeric12keyhandler.cpp
SOURCES += ..\common\hbinputabstractbase.cpp

symbian {
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = CAP_GENERAL_DLL
    TARGET.UID3 = 0x20022EA8

    pluginstub.sources = HbTouchInput.dll
    pluginstub.path = /resource/plugins/inputmethods
    DEPLOYMENT += pluginstub
}

!local {
    target.path = $$HB_PLUGINS_DIR/inputmethods
    INSTALLS += target
}

include($${HB_SOURCE_DIR}/src/hbcommon.pri)

# End of file
