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

# Note that HB_EFFECTS_OPENVG should be defined to build the OpenVG-specific code.
# However the files (should) compile also without having it defined (no effects
# are drawn in addition to the source image in this case).

contains(DEFINES, HB_EFFECTS_OPENVG) {
    QT *= openvg
    !isEmpty(QMAKE_LIBS_OPENVG) {
        LIBS += $$QMAKE_LIBS_OPENVG $$QMAKE_LIBS_EGL
    } else {
        symbian: LIBS += -llibopenvg -llibopenvgu -llibegl
        else:win32-msvc*: LIBS += -llibopenvg
        else: LIBS += -lopenvg
    }
}

PRIVATE_HEADERS += $$PWD/hbvgbceffect_p.h
PRIVATE_HEADERS += $$PWD/hbvgbceffect_p_p.h
PRIVATE_HEADERS += $$PWD/hbvgblureffect_p.h
PRIVATE_HEADERS += $$PWD/hbvgblureffect_p_p.h
PRIVATE_HEADERS += $$PWD/hbvgchainedeffect_p.h
PRIVATE_HEADERS += $$PWD/hbvgchainedeffect_p_p.h
PRIVATE_HEADERS += $$PWD/hbvgcolorizeeffect_p.h
PRIVATE_HEADERS += $$PWD/hbvgcolorizeeffect_p_p.h
PRIVATE_HEADERS += $$PWD/hbvgeffect_p.h
PRIVATE_HEADERS += $$PWD/hbvgeffect_p_p.h
PRIVATE_HEADERS += $$PWD/hbvggloweffect_p.h
PRIVATE_HEADERS += $$PWD/hbvghsleffect_p.h
PRIVATE_HEADERS += $$PWD/hbvghsleffect_p_p.h
PRIVATE_HEADERS += $$PWD/hbvgoutlineeffect_p.h
PRIVATE_HEADERS += $$PWD/hbvgoutlineeffect_p_p.h
PRIVATE_HEADERS += $$PWD/hbvgshadoweffect_p.h
PRIVATE_HEADERS += $$PWD/hbvgframeeffect_p.h
PRIVATE_HEADERS += $$PWD/hbvgframeeffect_p_p.h
PRIVATE_HEADERS += $$PWD/hbvgreflectioneffect_p.h
PRIVATE_HEADERS += $$PWD/hbvgreflectioneffect_p_p.h
PRIVATE_HEADERS += $$PWD/hbvgmaskeffect_p.h
PRIVATE_HEADERS += $$PWD/hbvgmaskeffect_p_p.h
PRIVATE_HEADERS += $$PWD/hbvgeffecttraces_p.h

SOURCES += $$PWD/hbvgbceffect.cpp
SOURCES += $$PWD/hbvgblureffect.cpp
SOURCES += $$PWD/hbvgchainedeffect.cpp
SOURCES += $$PWD/hbvgcolorizeeffect.cpp
SOURCES += $$PWD/hbvgeffect.cpp
SOURCES += $$PWD/hbvggloweffect.cpp
SOURCES += $$PWD/hbvghsleffect.cpp
SOURCES += $$PWD/hbvgoutlineeffect.cpp
SOURCES += $$PWD/hbvgshadoweffect.cpp
SOURCES += $$PWD/hbvgframeeffect.cpp
SOURCES += $$PWD/hbvgreflectioneffect.cpp
SOURCES += $$PWD/hbvgmaskeffect.cpp
