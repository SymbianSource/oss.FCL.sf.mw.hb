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

# Treat warnings as errors
!symbian:unix|*-g++:QMAKE_CXXFLAGS += -Werror
win32-msvc*:QMAKE_CXXFLAGS += /WX

# Do not define obj/moc/rcc/uic dirs on Symbian
# because it causes builds troubles
!symbian {

    # Do not define obj/moc/rcc/uic dirs on Unix when building
    # with coverage support because it confuses GCOV
    !unix:!coverage {

        CONFIG(release, debug|release) {
            MOC_DIR = release/moc
            OBJECTS_DIR = release/obj
            RCC_DIR = release/rcc
            UI_DIR = release/ui
        } else {
            MOC_DIR = debug/moc
            OBJECTS_DIR = debug/obj
            RCC_DIR = debug/rcc
            UI_DIR = debug/ui
        }
    }
}

# integrity check for public/private headers
contains(TEMPLATE, .*lib$) {
    for(pubheader, $$list($$lower($$unique(PUBLIC_HEADERS)))) {
        contains(pubheader, .*_[pr].h$):warning($$basename(pubheader) is listed in PUBLIC_HEADERS but has a \"_[pr].h\" suffix.)
    }
    for(restheader, $$list($$lower($$unique(RESTRICTED_HEADERS)))) {
        !contains(restheader, .*_r.h$):warning($$basename(restheader) is listed in RESTRICTED_HEADERS but has no \"_r.h\" suffix.)
    }
    for(privheader, $$list($$lower($$unique(PRIVATE_HEADERS)))) {
        !contains(privheader, .*_p.h$):warning($$basename(privheader) is listed in PRIVATE_HEADERS but has no \"_p.h\" suffix.)
    }
    !isEmpty(INTERNAL_HEADERS) {
        warning(INTERNAL_HEADERS is obsolete. Use PUBLIC|RESTRICTED|PRIVATE_HEADERS for the following headers:)
        for(intheader, $$list($$unique(INTERNAL_HEADERS))) {
            warning($$intheader)
        }
    }
}

# common extra targets
include(flint.pri)
include(astyle.pri)
include(coverage.pri)
