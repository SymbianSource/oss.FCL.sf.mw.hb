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
#


TEMPLATE = subdirs
CONFIG += root

SUBDIRS += src
!contains(HB_NOMAKE_PARTS, tutorials):exists(tutorials) {
    tutorials.depends = src
    SUBDIRS += tutorials
}

!symbian {
    feature.files += $$HB_SOURCE_DIR/hb.prf
    feature.files += $$HB_BUILD_DIR/hb_install.prf
    feature.files += $$HB_MKSPECS_DIR/hb_functions.prf
    feature.files += $$HB_MKSPECS_DIR/docml2bin.prf
    feature.path = $$HB_FEATURES_DIR
    INSTALLS += feature
}
else {
    tmp = $$split(HB_FEATURES_DIR, :)
    HB_SYMBIAN_PRF_EXPORT_DIR = $$last(tmp)
    BLD_INF_RULES.prj_exports += "hb.prf $$HB_SYMBIAN_PRF_EXPORT_DIR/hb.prf"
    BLD_INF_RULES.prj_exports += "hb_install.prf $$HB_SYMBIAN_PRF_EXPORT_DIR/hb_install.prf"
    BLD_INF_RULES.prj_exports += "mkspecs/hb_functions.prf $$HB_SYMBIAN_PRF_EXPORT_DIR/hb_functions.prf"
    BLD_INF_RULES.prj_exports += "mkspecs/docml2bin.prf $$HB_SYMBIAN_PRF_EXPORT_DIR/docml2bin.prf"
}

QMAKE_DISTCLEAN += $$hbNativePath($$HB_BUILD_DIR/.qmake.cache)
QMAKE_DISTCLEAN += $$hbNativePath($$HB_BUILD_DIR/hb_install.prf)

hbvar.path = .
hbvar.commands += $(QMAKE) -set HB \"hbcore hbwidgets hbutils\"
QMAKE_EXTRA_TARGETS += hbvar
INSTALLS += hbvar

symbian {
    exists(rom):include(rom/rom.pri)
    install.depends += hbvar
#    install.depends += cssbinary
    install.commands += $$QMAKE_COPY $$hbNativePath($$HB_SOURCE_DIR/hb.prf) $$hbNativePath($$[QMAKE_MKSPECS]/features)
    install.commands += && $$QMAKE_COPY $$hbNativePath($$HB_BUILD_DIR/hb_install.prf) $$hbNativePath($$[QMAKE_MKSPECS]/features)
    install.commands += && $$QMAKE_COPY $$hbNativePath($$HB_SOURCE_DIR/mkspecs/hb_functions.prf) $$hbNativePath($$[QMAKE_MKSPECS]/features)
    install.commands += && $$QMAKE_COPY $$hbNativePath($$HB_BUILD_DIR/mkspecs/docml2bin.prf) $$hbNativePath($$[QMAKE_MKSPECS]/features)
    QMAKE_EXTRA_TARGETS += install
}

# css binary generation

cssbinary.path = . #needed for install target
cssbinary.sourcedir = $$PWD/src/hbcore/resources/themes/style/hbdefault
symbian {
    cssbinary.targetfile = $${EPOCROOT}epoc32/data/z/resource/hb/themes/css.bin
} else {
    cssbinary.targetfile = $$HB_RESOURCES_DIR/themes/css.bin 
}
cssbinary.commands = $$hbToolCommand(hbbincssmaker) -i $$cssbinary.sourcedir -o $$cssbinary.targetfile

# copy generated css binary to symbian emulator directory
symbian {
    cssbinary.commands += && $$QMAKE_COPY $$hbNativePath($$cssbinary.targetfile) $$hbNativePath($${EPOCROOT}epoc32/release/winscw/udeb/z/resource/hb/themes/css.bin)
}

QMAKE_EXTRA_TARGETS += cssbinary
#INSTALLS += cssbinary

!contains(HB_NOMAKE_PARTS, tests):exists(tsrc) {
    test.depends = sub-src
    test.commands += cd tsrc && $(MAKE) test
    autotest.depends = sub-src
    autotest.commands += cd tsrc && $(MAKE) autotest
    loctest.depends = sub-src
    loctest.commands += cd tsrc/loc && $(MAKE) loctest
    QMAKE_EXTRA_TARGETS += test autotest loctest
}

exists(doc):include(doc/doc.pri)
include(src/hbcommon.pri)
