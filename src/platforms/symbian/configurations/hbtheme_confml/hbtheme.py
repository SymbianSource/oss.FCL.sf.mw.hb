#!/usr/bin/env python
# -*- coding: utf-8 -*-
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

import sys, zipfile, os, os.path
import tempfile
import shutil

def get_themes_from_thx(files):
    for theme in files:
        get_themename_from_thx(theme[1], 'themes')
                
def get_themename_from_thx(file, themesfolder):
    ret = None

    outdir = get_output_folder()
    outdir = os.path.join(outdir, 'resource')
    if not os.path.exists(outdir):    
        os.mkdir(outdir)
    outdir = os.path.join(outdir, 'hb')
    if not os.path.exists(outdir):    
        os.mkdir(outdir)
    outdir = os.path.join(outdir, themesfolder)
    if not os.path.exists(outdir):    
        os.mkdir(outdir)
    unzip_file_into_dir(file, outdir)
    
    # Get filesnames in outdir
    files_in_dir = os.listdir(outdir)
    
    # Search for themeindex file
    for fn in files_in_dir:
        if os.path.splitext(fn)[1] == '.themeindex':
            ret = os.path.splitext(fn)[0]
        
    return ret

def extract_folders(fullpath):
    fullpath = fullpath.replace("\\", "/")
    folders = list()    
    if fullpath.find('/') != -1:
        splittedparts = fullpath.partition('/')
        invalidfolders = ['.', '']
        if not splittedparts[0] in invalidfolders:
            folders.append(splittedparts[0]+'/')
        if splittedparts[2].find('/') != -1:
            folders.extend(extract_folders(splittedparts[2]))
    return folders
        
def unzip_file_into_dir(file, dir):
    """
    Unzips file into given folder.
    """
    zfobj = zipfile.ZipFile(file,'r')
    for name in zfobj.namelist():       
        if name.endswith('/'):        
            newdir = os.path.join(dir, name)      
            if not os.path.exists(newdir):    
                os.mkdir(newdir)
        else:
            archivename = name
            if archivename.startswith('/'):
                archivename = archivename.partition('/')[2]                
            file = os.path.join(dir, archivename)
            
            folders = extract_folders(file)
            recursivefolder = ''
            for folder in folders:
                recursivefolder = recursivefolder + folder
                if not os.path.exists(recursivefolder):    
                    os.mkdir(recursivefolder)
            data = zfobj.read(name)
            if not os.path.exists(file) or os.path.getsize(file) != len(data):            
                outfile = open(file, 'wb')
                outfile.write(data)
                outfile.close()

def get_output_folder():
    output = ruleml.context.output
    output = os.path.join(output, 'content')
    if not os.path.exists(output):
        os.mkdir(output)
    return output
