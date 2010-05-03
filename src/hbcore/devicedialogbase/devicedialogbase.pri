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

include(devicedialogdebug/devicedialogdebug.pri)
include(devicedialogserver/devicedialogserver.pri)

PUBLIC_HEADERS += $$PWD/hbdevicedialog.h
PUBLIC_HEADERS += $$PWD/hbindicator.h
PUBLIC_HEADERS += $$PWD/hbdevicedialogplugin.h
PUBLIC_HEADERS += $$PWD/hbdevicedialoginterface.h
PUBLIC_HEADERS += $$PWD/hbsymbiandevicedialog.h #deprecated
PUBLIC_HEADERS += $$PWD/hbdevicedialogsymbian.h 
PUBLIC_HEADERS += $$PWD/hbsymbianvariant.h
PUBLIC_HEADERS += $$PWD/hbindicatorsymbian.h
PUBLIC_HEADERS += $$PWD/hbtextresolversymbian.h
PRIVATE_HEADERS += $$PWD/hbdevicedialogerrors_p.h

SOURCES += $$PWD/hbdevicedialog.cpp
SOURCES += $$PWD/hbindicator.cpp
SOURCES += $$PWD/hbdevicedialogplugin.cpp

symbian {
PRIVATE_HEADERS += $$PWD/hbdevicedialogsym_p.h
PRIVATE_HEADERS += $$PWD/hbdevicedialogclientsession_p.h
PRIVATE_HEADERS += $$PWD/hbindicatorsym_p.h
PRIVATE_HEADERS += $$PWD/hbsymbianvariantconverter_p.h
PRIVATE_HEADERS += $$PWD/hbdeleteguardsymbian_p.h

SOURCES += $$PWD/hbsymbianvariant.cpp
SOURCES += $$PWD/hbsymbiandevicedialog.cpp #deprecated
SOURCES += $$PWD/hbdevicedialogsymbian.cpp 

SOURCES += $$PWD/hbdevicedialogsym_p.cpp
SOURCES += $$PWD/hbdevicedialogclientsession.cpp
SOURCES += $$PWD/hbindicatorsym_p.cpp
SOURCES += $$PWD/hbindicatorsymbian.cpp
SOURCES += $$PWD/hbsymbianvariantconverter.cpp
SOURCES += $$PWD/hbtextresolversymbian.cpp
SOURCES += $$PWD/hbdeleteguardsymbian.cpp
}

!symbian {
PRIVATE_HEADERS += $$PWD/hbdevicedialogwin32_p.h
PRIVATE_HEADERS += $$PWD/hbindicatorwin32_p.h
SOURCES += $$PWD/hbdevicedialogwin32_p.cpp
SOURCES += $$PWD/hbindicatorwin32_p.cpp
}
