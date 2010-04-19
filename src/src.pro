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

!contains(HB_NOMAKE_PARTS, hbcore) {
    SUBDIRS += hbcore
}

!contains(HB_NOMAKE_PARTS, hbwidgets) {
    hbwidgets.depends = hbcore
    SUBDIRS *= hbcore
    SUBDIRS += hbwidgets
}

!contains(HB_NOMAKE_PARTS, hbutils) {
    hbutils.depends = hbcore hbwidgets
    SUBDIRS *= hbcore hbwidgets
    SUBDIRS += hbutils
}

!contains(HB_NOMAKE_PARTS, hbinput) {
    hbplugins.depends += hbinput
    hbinput.depends = hbcore hbwidgets
    SUBDIRS *= hbcore hbwidgets
    SUBDIRS += hbinput
}

!contains(HB_NOMAKE_PARTS, hbfeedback) {
    hbplugins.depends += hbfeedback
    hbfeedback.depends = hbcore
    SUBDIRS *= hbcore
    SUBDIRS += hbfeedback
}

!contains(HB_NOMAKE_PARTS, hbservers) {
    hbservers.depends = hbcore hbwidgets hbutils
    SUBDIRS *= hbcore hbwidgets
    SUBDIRS += hbservers
}

!contains(HB_NOMAKE_PARTS, hbplugins) {
    hbplugins.depends = hbcore hbwidgets hbinput
    SUBDIRS *= hbcore hbwidgets hbinput
    SUBDIRS += hbplugins
}

# TODO: remove these line when others have suitable .pro files for styling
astyle.recurse = hbcore hbservers hbwidgets hbutils

include(hbcommon.pri)
