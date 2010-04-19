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

import os
import re
import sys
import shutil
import optparse

# ============================================================================
# Globals
# ============================================================================
VERBOSE = False
EXCLUDE = ["hbplugins", "hbservers", "3rdparty", "internal", "tsrc", "debug", "release"]
COLLECTIONS = {"hbcore": "HbCore",
               "hbfeedback": "HbFeedback",
               "hbinput": "HbInput",
               "hbutils": "HbUtils",
               "hbwidgets": "HbWidgets"}

# ============================================================================
# OptionParser
# ============================================================================
class OptionParser(optparse.OptionParser):
    def __init__(self):
        optparse.OptionParser.__init__(self)
        self.add_option("-v", "--verbose", action="store_true", dest="verbose",
                        help="print verbose information about each step of the sync process")

        group = optparse.OptionGroup(self, "Input/output options")
        self.add_option("-i", "--input", dest="inputdir", metavar="dir",
                        help="specify the input <dir>")
        self.add_option("-o", "--output", dest="outputdir", metavar="dir",
                        help="specify the output <dir>")
        self.add_option_group(group)

        self.set_defaults(verbose=VERBOSE)

# ============================================================================
# Utils
# ============================================================================
if not hasattr(os.path, "relpath"):
    def relpath(path, start=os.curdir):
        return path
    os.path.relpath = relpath

def read_file(filepath):
    content = ""
    try:
        file = open(filepath, "r")
        content = file.read()
        file.close()
    except IOError, e:
        print(e)
    return content

def write_file(filepath, content):
    try:
        file = open(filepath, "w")
        file.write(content)
        file.close()
    except IOError, e:
        print(e)

def write_header(filepath, include):
    write_file(filepath, "#include \"%s\"\n" % include)

# ============================================================================
# Component
# ============================================================================
class Component:
    def __init__(self, name):
        self.name = name
        self.headers = list()
        self.privates = list()

    def read(self, path):
        entries = os.listdir(path)
        for entry in entries:
            entrypath = os.path.join(path, entry)
            if os.path.isdir(entrypath):
                self.read(entrypath)
            elif os.path.isfile(entrypath):
                if entry.endswith("_p_p.h"):
                    continue
                elif entry.endswith("_p.h"):
                    self.privates.append(entrypath)
                elif entry.endswith(".h"):
                    self.headers.append(entrypath)

    def write(self, path):
        if len(self.headers) > 0:
            self._makedirs(path)
            self._write(path, self.headers, True)

        if len(self.privates) > 0:
            privpath = os.path.join(path, "private")
            self._makedirs(privpath)
            self._write(privpath, self.privates, False)

    def _write(self, path, headers, convenience):
        global VERBOSE
        if VERBOSE:
            print("INFO: Writing headers to '%s'" % path)
        for header in headers:
            filename = os.path.basename(header)
            filepath = os.path.join(path, filename)
            relpath = os.path.relpath(header, path)
            if VERBOSE:
                print("INFO:\t ==> %s" % os.path.basename(filepath))
            write_header(filepath, relpath.replace("\\", "/"))
            if convenience:
                classes = list()
                content = read_file(header)
                for match in re.finditer("(?:class|namespace)\s+(?:HB_[^_]+_EXPORT\s+)?(Hb\w*)(\s*;)?", content):
                    if not match.group(2):
                        classes.append(match.group(1))
                for match in re.finditer("#pragma hb_header\((\w+)\)", content):
                    classes.append(match.group(1))
                for cls in classes:
                    filepath = os.path.join(path, cls)
                    write_header(filepath, filename)

    def _makedirs(self, path):
        global VERBOSE
        if not os.path.exists(path):
            if VERBOSE:
                print("INFO: Creating include dir '%s'" % path)
            os.makedirs(path)

# ============================================================================
# Collection
# ============================================================================
class Collection:
    def __init__(self, name):
        self.name = name
        self.components = []

    def read(self, path):
        global EXCLUDE
        for entry in os.listdir(path):
            entrypath = os.path.join(path, entry)
            if not entry in EXCLUDE and os.path.isdir(entrypath):
                component = Component(entry)
                component.read(entrypath)
                self.components.append(component)

    def write(self, path):
        global COLLECTIONS
        # include/hbcore
        includes = list()
        path = os.path.join(path, self.name)
        for component in self.components:
            component.write(path)
            for header in component.headers:
                includes.append("#include \"%s\"\n" % os.path.basename(header))
        if self.name in COLLECTIONS:
            write_file(os.path.join(path, self.name + ".h"), "".join(includes))
            write_header(os.path.join(path, COLLECTIONS[self.name]), self.name + ".h")

# ============================================================================
# Package
# ============================================================================
class Package:
    def __init__(self, name):
        self.path = name
        self.collections = list()

    def read(self, path):
        global EXCLUDE
        for entry in os.listdir(path):
            # hbcore, hbwidgets, hbutils...
            entrypath = os.path.join(path, entry)
            if not entry in EXCLUDE and os.path.isdir(entrypath):
                collection = Collection(entry)
                collection.read(entrypath)
                self.collections.append(collection)

    def write(self, path):
        for collection in self.collections:
            collection.write(path)

# ============================================================================
# main()
# ============================================================================
def main():
    global VERBOSE

    parser = OptionParser()
    (options, args) = parser.parse_args()

    VERBOSE = options.verbose

    if not options.inputdir:
        options.inputdir = os.path.abspath(sys.path[0])
    if os.path.basename(os.path.normpath(options.inputdir)) == "bin":
        options.inputdir = os.path.normpath(os.path.join(options.inputdir, os.pardir))
    if not os.path.basename(os.path.normpath(options.inputdir)) == "src":
        options.inputdir = os.path.join(options.inputdir, "src")

    if not options.outputdir:
        options.outputdir = os.getcwd()
    if not os.path.basename(os.path.normpath(options.outputdir)) == "include":
        options.outputdir = os.path.join(options.outputdir, "include")

    if os.path.exists(options.outputdir):
        if VERBOSE:
            print("INFO: Removing include dir '%s'" % options.outputdir)
        shutil.rmtree(options.outputdir, ignore_errors=True)

    package = Package("hb")
    package.read(options.inputdir)
    package.write(options.outputdir)

if __name__ == "__main__":
    main()
