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

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

PUBLIC_HEADERS += $$PWD/hbdataform.h
PUBLIC_HEADERS += $$PWD/hbdataformviewitem.h
PUBLIC_HEADERS += $$PWD/hbdataformmodelitem.h
PUBLIC_HEADERS += $$PWD/hbdataformmodel.h
#PUBLIC_HEADERS += $$PWD/hbdataitemcontainer.h

#INTERNAL_HEADERS += $$PWD/hbdataformitemcontainer_p.h
INTERNAL_HEADERS += $$PWD/hbdatagroup_p.h
INTERNAL_HEADERS += $$PWD/hbdataformviewitem_p.h
INTERNAL_HEADERS += $$PWD/hbdataformmodel_p.h
INTERNAL_HEADERS += $$PWD/hbdataitemcontainer_p.h
INTERNAL_HEADERS += $$PWD/hbdataform_p.h
INTERNAL_HEADERS += $$PWD/hbdataformheadingwidget_p.h
INTERNAL_HEADERS += $$PWD/hbdatagroupheadingwidget_p.h
INTERNAL_HEADERS += $$PWD/hbdataformmodelitem_p.h

SOURCES += $$PWD/hbdataform.cpp
SOURCES += $$PWD/hbdataformviewitem.cpp
SOURCES += $$PWD/hbdataformmodelitem.cpp
SOURCES += $$PWD/hbdataformmodel.cpp
SOURCES += $$PWD/hbdatagroupheadingwidget_p.cpp
SOURCES += $$PWD/hbdatagroup_p.cpp
SOURCES += $$PWD/hbdataformheadingwidget_p.cpp
SOURCES += $$PWD/hbdataform_p.cpp
SOURCES += $$PWD/hbdataformviewitem_p.cpp
SOURCES += $$PWD/hbdataitemcontainer_p.cpp



