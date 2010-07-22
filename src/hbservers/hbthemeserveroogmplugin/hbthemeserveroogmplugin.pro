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

CONFIG-=def_files
CONFIG-=def_files_disabled

INCLUDEPATH += .

# dependencies
hbAddLibrary(hbcore/HbCore)

SOURCES += hbthemeserveroogmplugin.cpp
SOURCES += hbthemeserveroogmpluginDllMain.cpp
SOURCES += hbthemeclientoogm.cpp

symbian {
    
    TEMPLATE = lib
    MMP_RULES += "TARGETTYPE PLUGIN"
    TARGET.CAPABILITY = CAP_ECOM_PLUGIN 
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.UID3 = 0x2002DC75
    TARGET.UID2 = 0x10009d8d
        
    LIBS += -leuser
    LIBS += -lecom
    LIBS += -lgoommonitor
  
  
    myrssrules = \
     "START RESOURCE hbthemeserveroogmplugin.rss" \
	 "TARGETPATH resource/plugins" \
	 "TARGET     hbthemeserveroogmplugin.rsc" \
	 "END"
    
    MMP_RULES += myrssrules


}


