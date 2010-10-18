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
TARGET = $$hbLibraryTarget(HbHardwareInput)
CONFIG += plugin

hbAddLibrary(hbcore/HbCore)
hbAddLibrary(hbwidgets/hbwidgets)
hbAddLibrary(hbinput/HbInput)

INCLUDEPATH += ../common

# directories
DESTDIR = $${HB_BUILD_DIR}/plugins/inputmethods

HEADERS += hardwareinputplugin.h \
           hbhardware12key.h \
           hbhardwareqwerty.h \
           hbhardwareinputbasic12keyhandler.h \
           hbhardwareinputbasicqwertyhandler.h \
           hbhardwareinputnumericqwertyhandler.h \
           hbhardwareinputpredictionqwertyhandler.h \
           ../common/hbinputbasichandler_p.h \
           ../common/hbinputmodehandler.h \
           ../common/hbinputmodehandler_p.h \
           hbhardwareinputnumeric12keyhandler.h \
           ../common/hbinputnumerichandler_p.h \
           hbhardwareinputprediction12keyhandler.h \
           ../common/hbinputpredictionhandler_p.h   
SOURCES += hardwareinputplugin.cpp \
           hbhardware12key.cpp \
           hbhardwareqwerty.cpp \
           hbhardwareinputbasic12keyhandler.cpp \
           hbhardwareinputbasicqwertyhandler.cpp \
           hbhardwareinputnumericqwertyhandler.cpp \
           hbhardwareinputpredictionqwertyhandler.cpp \
           ../common/hbinputbasichandler.cpp \
           ../common/hbinputmodehandler.cpp \
           hbhardwareinputnumeric12keyhandler.cpp \
           ../common/hbinputnumerichandler.cpp \
           hbhardwareinputprediction12keyhandler.cpp \
           ../common/hbinputpredictionhandler.cpp

symbian {
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = CAP_GENERAL_DLL
    TARGET.UID3 = 0x2002681D
    MMP_RULES += SMPSAFE

    hbhardwareinput.sources = HbHardwareInput.dll
    hbhardwareinput.path = /resource/plugins/inputmethods
    DEPLOYMENT = hbhardwareinput
}

!local {
    target.path = $$HB_PLUGINS_DIR/inputmethods
    INSTALLS += target
}

include($${HB_SOURCE_DIR}/src/hbcommon.pri)

# End of file
