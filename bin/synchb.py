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
EXCLUDE = ["hbapps", "hbplugins", "hbservers", "hbtools", "3rdparty", 
           "internal", "tsrc", "debug", "release", "bwins", "eabi"]
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

def include_directive(header):
    return "#include \"%s\"\n" % header.replace("\\", "/")

def write_header(header, include, path):
    filename = os.path.basename(header)
    filepath = os.path.join(path, filename)
    skip = False
    if os.path.exists(filepath):
        directive = include_directive(include)
        oldsize = os.path.getsize(filepath)
        newsize = len(directive)
        if oldsize == newsize and directive == read_file(filepath):
            skip = True
    if not skip:
        if VERBOSE:
            print("INFO:\t ==> %s" % os.path.basename(filepath))
        write_file(filepath, include_directive(include))

# ============================================================================
# Component
# ============================================================================
class Component:
    def __init__(self, name):
        self.name = name
        self.headers = []
        self.privates = []
        self.restricted = []

    def read(self, path):
        entries = os.listdir(path)
        for entry in entries:
            entrypath = os.path.join(path, entry)
            if os.path.isdir(entrypath):
                self.read(entrypath)
            elif os.path.isfile(entrypath):
                if re.match(entry, ".*?_[pr]_[pr]\.h"):
                    continue
                elif entry.endswith("_p.h"):
                    self.privates.append(entrypath)
                elif entry.endswith("_r.h"):
                    self.restricted.append(entrypath)
                elif entry.endswith(".h"):
                    self.headers.append(entrypath)

    def write(self, path):
        written = []
        if len(self.headers) > 0:
            self._makedirs(path)
            written += self._write(path, self.headers, True)

        if len(self.restricted) > 0:
            restpath = os.path.join(path, "restricted")
            self._makedirs(restpath)
            written += self._write(restpath, self.restricted, True)

        if len(self.privates) > 0:
            privpath = os.path.join(path, "private")
            self._makedirs(privpath)
            written += self._write(privpath, self.privates, False)
        return written

    def _write(self, path, headers, convenience):
        written = []
        for header in headers:
            write_header(header, os.path.relpath(header, path), path)
            written.append(os.path.join(path, os.path.basename(header)))
            if convenience:
                classes = []
                content = read_file(header)
                for match in re.finditer("(?:class|namespace)\s+(?:HB_[^_]+(?:_RESTRICTED)?_EXPORT\s+)?(Hb\w*)(\s*;)?", content):
                    if not match.group(2):
                        classes.append(match.group(1))
                for match in re.finditer("#pragma hb_header\((\w+)\)", content):
                    classes.append(match.group(1))
                for cls in classes:
                    write_header(cls, os.path.basename(header), path)
                    written.append(os.path.join(path, cls))
        return written

    def _makedirs(self, path):
        if not os.path.exists(path):
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
        global COLLECTIONS, VERBOSE
        path = os.path.join(os.path.abspath(path), self.name)
        if VERBOSE:
            print("INFO: Writing headers to '%s'..." % path)
        # there's no set in python 2.3 so use a list
        leftovers = []
        for root, dirs, files in os.walk(path):
            for file in files:
                leftovers.append(os.path.abspath(os.path.join(root, file)))

        # include/hbcore
        includes = []
        written = []
        for component in self.components:
            written += component.write(path)
            for header in component.headers:
                includes.append("#include \"%s\"\n" % os.path.basename(header))

        if self.name in COLLECTIONS:
            collectionheader = os.path.join(path, self.name + ".h")
            write_file(collectionheader, "".join(includes))
            written.append(collectionheader)
            if collectionheader in leftovers:
                leftovers.remove(collectionheader)
            convenienceheader = os.path.join(path, COLLECTIONS[self.name])
            write_file(convenienceheader, include_directive(self.name + ".h"))
            written.append(convenienceheader)
            if convenienceheader in leftovers:
                leftovers.remove(convenienceheader)

        for filepath in written:
            filepath = os.path.abspath(filepath)
            if filepath in leftovers:
                leftovers.remove(filepath)

        if VERBOSE and len(leftovers) > 0:
            print("INFO: Removing obsolete headers from '%s'..." % path)
        for leftover in leftovers:
            if VERBOSE:
                print("INFO:\t ==> %s" % leftover) # os.path.basename(leftover))
            os.remove(leftover)

# ============================================================================
# Package
# ============================================================================
class Package:
    def __init__(self, name):
        self.name = name
        self.collections = []

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

    package = Package("hb")
    package.read(options.inputdir)
    package.write(options.outputdir)

if __name__ == "__main__":
    main()
