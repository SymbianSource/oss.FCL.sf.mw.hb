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

# installation file generation
symbian {
    load(data_caging_paths)

    vendorinfo = \
            "; Localised Vendor name" \
            "%{\"Nokia\"}" \
            " " \
            "; Unique Vendor name" \
            ":\"Nokia\"" \
            " "
    orbitHeader = "$${LITERAL_HASH}{"Hb"},(0x2002FF63),0,1,0,TYPE=SA, RU"
    orbitBinaries.pkg_prerules = vendorinfo orbitHeader

    orbitBinaries.sources = hbthemechanger.exe hbsplashgenerator.exe hbdevicedialogappserver.exe hbiconpreloader.exe \
	    HbDeviceNotificationDialogPlugin.dll HbIndicatorMenuPlugin.dll HbDeviceProgressDialogPlugin.dll HbDeviceMessageBoxPlugin.dll \
	    hbthemeserveroogmplugin.dll hbthemeserver.exe
    orbitBinaries.path = !:/sys/bin
    DEPLOYMENT += orbitBinaries

    orbitPlugins.sources = HbAutoComplete.dll HbTouchInput.dll 
    orbitPlugins.path = !:/resource/plugins
    orbitPlugins.sources += $${EPOCROOT}$${HW_ZDIR}/resource/plugins/hbthemeserveroogmplugin.rsc
    DEPLOYMENT += orbitPlugins

    hbi18n.sources += directorylocalizer_en_GB.qm directorylocalizer_de_DE.qm languages.qm language_list.txt
    hbi18n.path = !:/resource/hbi18n/translations
    DEPLOYMENT += hbi18n

    displaydef.sources += displaydefinition.xml
    displaydef.path = !:/resource
    DEPLOYMENT += displaydef

    cenrep.sources += 2002C304.txt 2002C3AE.txt 2002C384.txt 20022E82.txt
    cenrep.path = !:/system/data/10202BE9
    DEPLOYMENT += cenrep
}
