#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#############################################################################
##
## Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
import sys
import fnmatch
import optparse

# ============================================================================
# Globals
# ============================================================================
INPUT = os.getcwd()
OUTPUT = sys.stdout
INCLUDE = None
EXCLUDE = None
INDENTATION = "    "

# ============================================================================
# OptionParser
# ============================================================================
class OptionParser(optparse.OptionParser):
    def __init__(self):
        optparse.OptionParser.__init__(self)
        self.add_option("-i", "--input", dest="input", metavar="dir",
                        help="specify the input <dir> (default %s)" % INPUT)
        self.add_option("-o", "--output", dest="output", metavar="file",
                        help="specify the output <file> (default stdout)")
        self.add_option("--include", dest="include", action="append", metavar="pattern",
                        help="specify the include <pattern> (default %s)" % INCLUDE)
        self.add_option("--exclude", dest="exclude", action="append", metavar="pattern",
                        help="specify the exclude <pattern> (default %s)" % EXCLUDE)
        self.add_option("--indentation", dest="indentation", metavar="level",
                        help="specify the indentation <level> (default '%s')" % INDENTATION)
        self.set_defaults(input=INPUT)
        self.set_defaults(output=OUTPUT)
        self.set_defaults(include=INCLUDE)
        self.set_defaults(exclude=EXCLUDE)
        self.set_defaults(indentation=INDENTATION)

# ============================================================================
# Utils
# ============================================================================
def index_resources():
    result = []
    for root, dirst, files in os.walk(os.curdir):
        for name in files:
            include = True
            filepath = os.path.normpath(os.path.join(root, name))
            if INCLUDE != None:
                for pattern in INCLUDE:
                    if not fnmatch.fnmatch(filepath, pattern):
                        include = False
            if EXCLUDE != None:
                for pattern in EXCLUDE:
                    if fnmatch.fnmatch(filepath, pattern):
                        include = False
            if include:
                result.append(filepath)
    return result

def write_qrc(out, resources):
    out.write("<!DOCTYPE RCC><RCC version=\"1.0\">\n")
    out.write("<qresource prefix=\"/\">\n")
    for resource in resources:
        out.write("%s<file>%s</file>\n" % (INDENTATION, resource))
    out.write("</qresource>\n")
    out.write("</RCC>\n")

# ============================================================================
# main()
# ============================================================================
def main():
    global INPUT, OUTPUT, INCLUDE, EXCLUDE, INDENTATION

    parser = OptionParser()
    (options, args) = parser.parse_args()

    if options.input:
        INPUT = options.input
    if options.output != sys.stdout:
        outputdir = os.path.dirname(options.output)
        if len(outputdir) and not os.path.exists(outputdir):
            os.makedirs(outputdir)
        OUTPUT = open(options.output, "w")
    if options.include:
        INCLUDE = options.include
    if options.exclude:
        EXCLUDE = options.exclude
    if options.indentation:
        INDENTATION = options.indentation.replace("\\t", "\t")

    os.chdir(INPUT)
    resources = index_resources()
    return write_qrc(OUTPUT, resources)

if __name__ == "__main__":
    sys.exit(main())
