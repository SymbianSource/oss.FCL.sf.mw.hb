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

INTERNAL_HEADERS += $$PWD/hbnvg_p.h
INTERNAL_HEADERS += $$PWD/hbnvgicon_p.h

INTERNAL_HEADERS += $$PWD/hbtlvcommon_p.h
INTERNAL_HEADERS += $$PWD/hbnvgfittoviewbox_p.h
INTERNAL_HEADERS += $$PWD/hbnvgimagebinder_p.h 
INTERNAL_HEADERS += $$PWD/hbtlviconcreator_p.h
INTERNAL_HEADERS += $$PWD/hbtlvrenderer_p.h
INTERNAL_HEADERS += $$PWD/hbtlviconrenderer_p.h
INTERNAL_HEADERS += $$PWD/hbnvgtlvicon_p.h
INTERNAL_HEADERS += $$PWD/hbnvgicondata_p.h
INTERNAL_HEADERS += $$PWD/hbopenvghandlestore_p.h
INTERNAL_HEADERS += $$PWD/hbnvgcsicon_p.h
INTERNAL_HEADERS += $$PWD/hbnvgiconfactory_p.h 
INTERNAL_HEADERS += $$PWD/hbnvg_p_p.h
INTERNAL_HEADERS += $$PWD/hbnvgexception_p.h
INTERNAL_HEADERS += $$PWD/hbnvgenginepool_p.h

SOURCES += $$PWD/hbnvgfittoviewbox.cpp
#SOURCES += $$PWD/hbnvgfittoviewbox_p.inl
SOURCES += $$PWD/hbnvgtlvicon.cpp
SOURCES += $$PWD/hbtlviconcreator.cpp
SOURCES += $$PWD/hbtlvrenderer.cpp
SOURCES += $$PWD/hbtlviconrenderer.cpp
SOURCES += $$PWD/hbnvgicondata.cpp
#SOURCES += $$PWD/hbnvgicondata_p.inl
SOURCES += $$PWD/hbopenvghandlestore.cpp
SOURCES += $$PWD/hbnvgcsicon.cpp
SOURCES += $$PWD/hbnvgiconfactory.cpp
SOURCES += $$PWD/hbnvg.cpp
SOURCES += $$PWD/hbnvgenginepool.cpp
