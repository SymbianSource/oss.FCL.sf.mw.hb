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

TEMPLATE = lib
TARGET = $$hbLibraryTarget(HbCore)
QT = core gui svg network xml
DEFINES += BUILD_HB_CORE
DEFINES += HB_BIN_DIR=\"\\\"$${HB_BIN_DIR}\\\"\"
DEFINES += HB_PLUGINS_DIR=\"\\\"$${HB_PLUGINS_DIR}\\\"\"
DEFINES += HB_RESOURCES_DIR=\"\\\"$${HB_RESOURCES_DIR}\\\"\"
DEFINES += HB_BUILD_DIR=\"\\\"$${HB_BUILD_DIR}\\\"\"
DEFINES += HB_INSTALL_DIR=\"\\\"$${HB_INSTALL_DIR}\\\"\"
DEFINES += HB_TRANSLATIONS_DIR=\"\\\"$${HB_TRANSLATIONS_DIR}\\\"\"
*-maemo*:DEFINES += HB_Q_WS_MAEMO

# directories
DESTDIR = $${HB_BUILD_DIR}/lib
win32:DLLDESTDIR = $${HB_BUILD_DIR}/bin

!symbian:CONFIG += settingswindow
settingswindow:DEFINES += HB_SETTINGS_WINDOW

# NVGDECODER
symbian:CONFIG += nvg


# components
include(core/core.pri)
include(cssparser/hbcssparser.pri)
include(decorators/decorators.pri)
include(devicedialogbase/devicedialogbase.pri)
include(effects/effects.pri)
include(ovgeffects/ovgeffects.pri)
include(gui/gui.pri)
include(i18n/i18n.pri)
include(image/image.pri)
include(indicatorplugins/indicatorplugins.pri)
include(inputfw/inputfw.pri)
include(feedback/feedback.pri) 
include(layouts/layouts.pri)
include(primitives/primitives.pri)
include(resources/resources.pri)
include(style/style.pri)
include(theme/theme.pri)
include(utils/utils.pri)
include(vkbhosts/vkbhosts.pri)
include(gestures/gestures.pri)
include(activity/activity.pri)

CONVENIENCE_HEADERS += $${HB_BUILD_DIR}/include/hbcore/hbcore.h
CONVENIENCE_HEADERS += $$files($${HB_BUILD_DIR}/include/hbcore/Hb*)
HEADERS += $$PUBLIC_HEADERS $$RESTRICTED_HEADERS $$PRIVATE_HEADERS $$CONVENIENCE_HEADERS

# installation
!local {
    target.path = $${HB_LIB_DIR}
    win32:dlltarget.path = $${HB_BIN_DIR}

    pubheaders.files = $$PUBLIC_HEADERS
    pubheaders.path = $${HB_INCLUDE_DIR}/hbcore

    restheaders.files = $$RESTRICTED_HEADERS
    restheaders.path = $${HB_INCLUDE_DIR}/hbcore/restricted

    convheaders.files = $$CONVENIENCE_HEADERS
    convheaders.path = $${HB_INCLUDE_DIR}/hbcore

    INSTALLS += target pubheaders restheaders convheaders
    win32:INSTALLS += dlltarget
}

contains(DEFINES, HB_HAVE_QT_MOBILITY) {
    CONFIG += mobility
    MOBILITY += publishsubscribe systeminfo
}

#QMAKE_DISTCLEAN += $$hbNativePath($${HB_BUILD_DIR}/include/hbcore/*)
#QMAKE_DISTCLEAN += $$hbNativePath($${HB_BUILD_DIR}/include/hbcore/private/*)

symbian {
    defFilePath = defs
    
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = CAP_GENERAL_DLL
    TARGET.UID3=0x20022EDC
    MMP_RULES += SMPSAFE
    #DEPLOYMENT_PLUGIN += qjpeg # TODO: Removed because this is already in qt.sis and that caused problems
    DEFINES += SYMBIAN_TARGET_ICON_CACHE_SIZE # TODO: what's this? why not use Q_OS_SYMBIAN?

    # Executable section is now over 4MB in Symbian. Extend it to 8MB
    QMAKE_LFLAGS.ARMCC += --rw-base 0x800000

    INCLUDEPATH += $${EPOCROOT}epoc32/include/osextensions/stdapis/stlport # TODO: depends on S60 version?
    INCLUDEPATH += $${EPOCROOT}epoc32/include/hwrm # TODO: depends on S60 version?
    nvg:DEFINES += HB_NVG_CS_ICON
    sgimage:DEFINES += HB_SGIMAGE_ICON

    load(symbian_i18n.prf):TRANSLATIONS = directorynamelocalizer.ts

    nvg {
        include(svgext/svgext.pri)
        LIBS += -llibOpenVG
        LIBS += -llibOpenVGU
        LIBS += -lsvgengine
        LIBS += -llibegl
        LIBS += -lNVGDecoder_SW
        LIBS += -llibvgi
    }
    sgimage {
        INCLUDEPATH += $${EPOCROOT}epoc32/include/platform #For SGImage Support
        LIBS += -lsgresource #For SGImage Support
    }

    # For avkon code in HbApplication event handling
    exists($${EPOCROOT}epoc32/include/domain/middleware) {
        INCLUDEPATH += $${EPOCROOT}epoc32/include/domain/middleware
    }
    exists($${EPOCROOT}epoc32/include/middleware) {
        INCLUDEPATH += $${EPOCROOT}epoc32/include/middleware
    }
    exists($${EPOCROOT}epoc32/include/platform/mw) {
        INCLUDEPATH += $${EPOCROOT}epoc32/include/platform/mw
    }
    exists($${EPOCROOT}epoc32/include/mw) {
        INCLUDEPATH += $${EPOCROOT}epoc32/include/mw
    }

    hbExportHeaders(hbcore)

    hbPrjExport("$${PWD}/i18n/translations/language_list.txt", "$${EPOCROOT}epoc32/data/z/resource/hb/translations/language_list.txt")
    hbPrjExport("$${PWD}/i18n/translations/language_list.txt", "$${EPOCROOT}epoc32/release/winscw/udeb/resource/hb/translations/language_list.txt")
    hbPrjExport("$${PWD}/i18n/translations/locale_mappings.txt", "$${EPOCROOT}epoc32/data/z/resource/hb/translations/locale_mappings.txt")
    hbPrjExport("$${PWD}/i18n/translations/locale_mappings.txt", "$${EPOCROOT}epoc32/release/winscw/udeb/resource/hb/translations/locale_mappings.txt")
    hbPrjExport("$${PWD}/i18n/translations/collations.qm", "$${EPOCROOT}epoc32/data/z/resource/hb/translations/collations.qm")
    hbPrjExport("$${PWD}/i18n/translations/collations.qm", "$${EPOCROOT}epoc32/release/winscw/udeb/resource/hb/translations/collations.qm")
    hbPrjExport("$${PWD}/i18n/translations/languages.qm", "$${EPOCROOT}epoc32/data/z/resource/hb/translations/languages.qm")
    hbPrjExport("$${PWD}/i18n/translations/languages.qm", "$${EPOCROOT}epoc32/release/winscw/udeb/resource/hb/translations/languages.qm")
    hbPrjExport("$${PWD}/i18n/translations/regions.qm", "$${EPOCROOT}epoc32/data/z/resource/hb/translations/regions.qm")
    hbPrjExport("$${PWD}/i18n/translations/regions.qm", "$${EPOCROOT}epoc32/release/winscw/udeb/resource/hb/translations/regions.qm")

    LIBS += -lapparc
    LIBS += -lavkon
    LIBS += -lbafl
    LIBS += -lSensrvClient
    LIBS += -lsensrvutil
    LIBS += -lcentralrepository
    LIBS += -lefsrv
    LIBS += -lfbscli
    LIBS += -lgdi
    LIBS += -lws32
    LIBS += -lapgrfx
    LIBS += -lcone
    LIBS += -lsystemtoneservice

    # central repository
    hbPrjExport("$${PWD}/resources/centralrepository/2002C304.txt", "$${EPOCROOT}epoc32/data/z/private/10202BE9/2002C304.txt")
    hbPrjExport("$${PWD}/resources/centralrepository/2002C304.txt", "$${EPOCROOT}epoc32/release/winscw/udeb/z/private/10202BE9/2002C304.txt")
    hbPrjExport("$${PWD}/resources/centralrepository/2002C304.txt", "$${EPOCROOT}epoc32/release/winscw/urel/z/private/10202BE9/2002C304.txt")
    hbPrjExport("$${PWD}/resources/centralrepository/2002C384.txt", "$${EPOCROOT}epoc32/data/z/private/10202BE9/2002C384.txt")
    hbPrjExport("$${PWD}/resources/centralrepository/2002C384.txt", "$${EPOCROOT}epoc32/release/winscw/udeb/z/private/10202BE9/2002C384.txt")
    hbPrjExport("$${PWD}/resources/centralrepository/2002C384.txt", "$${EPOCROOT}epoc32/release/winscw/urel/z/private/10202BE9/2002C384.txt")
    hbPrjExport("$${PWD}/resources/centralrepository/2002C3AE.txt", "$${EPOCROOT}epoc32/data/z/private/10202BE9/2002C3AE.txt")
    hbPrjExport("$${PWD}/resources/centralrepository/2002C3AE.txt", "$${EPOCROOT}epoc32/release/winscw/udeb/z/private/10202BE9/2002C3AE.txt")
    hbPrjExport("$${PWD}/resources/centralrepository/2002C3AE.txt", "$${EPOCROOT}epoc32/release/winscw/urel/z/private/10202BE9/2002C3AE.txt")
    hbPrjExport("$${PWD}/resources/centralrepository/20022E82.txt", "$${EPOCROOT}epoc32/data/z/private/10202BE9/20022E82.txt")
    hbPrjExport("$${PWD}/resources/centralrepository/20022E82.txt", "$${EPOCROOT}epoc32/release/winscw/udeb/z/private/10202BE9/20022E82.txt")
    hbPrjExport("$${PWD}/resources/centralrepository/20022E82.txt", "$${EPOCROOT}epoc32/release/winscw/urel/z/private/10202BE9/20022E82.txt")
}

include($${HB_SOURCE_DIR}/src/hbcommon.pri)
