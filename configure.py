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

import re
import os
import sys
import shutil
import fnmatch
import tempfile
import optparse
if sys.version_info[0] == 2 and sys.version_info[1] < 4:
    # for scratchbox compatibility
    import popen2
else:
    import subprocess

# ============================================================================
# Globals
# ============================================================================
HB_MAKE_PARTS = [ "tutorials" ]
HB_NOMAKE_PARTS = [ "tests", "performance", "localization" ]

QMAKE = None
MAKE = None
BUILDENV = None

# ============================================================================
# Utils
# ============================================================================
def add_remove_part(part, add):
    global HB_MAKE_PARTS, HB_NOMAKE_PARTS
    if add:
        while part in HB_NOMAKE_PARTS:
            HB_NOMAKE_PARTS.remove(part)
        if not part in HB_MAKE_PARTS:
            HB_MAKE_PARTS.append(part)
    else:
        while part in HB_MAKE_PARTS:
            HB_MAKE_PARTS.remove(part)
        if not part in HB_NOMAKE_PARTS:
            HB_NOMAKE_PARTS.append(part)

def run_system(args, cwd=None):
    old_epocroot = None
    env = os.environ.copy()
    if "EPOCROOT" in env:
        epocroot = env.get("EPOCROOT")
        if not (epocroot.endswith("\\") or epocroot.endswith("/")):
            os.putenv("EPOCROOT", "%s/" % epocroot)
            old_epocroot = epocroot

    if type(args) is list:
        args = " ".join(args)
    result = os.system(args)

    if old_epocroot != None:
        os.putenv("EPOCROOT", old_epocroot)

    return result

def run_process(args, cwd=None):
    code = 0
    output = ""

    env = os.environ.copy()
    if "EPOCROOT" in env:
        epocroot = env.get("EPOCROOT")
        if not (epocroot.endswith("\\") or epocroot.endswith("/")):
            env["EPOCROOT"] = "%s/" % epocroot        

    if os.name == "nt":
        args = ["cmd.exe", "/C"] + args
        
    try:
        if cwd != None:
            oldcwd = os.getcwd()
            os.chdir(cwd)
        if sys.version_info[0] == 2 and sys.version_info[1] < 4:
            process = popen2.Popen4(args)
            code = process.wait()
            output = process.fromchild.read()
        else:
            process = subprocess.Popen(args, env=env, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
            (stdout, stderr) = process.communicate()
            code = process.returncode
            output = stdout + stderr
            
        if cwd != None:
            os.chdir(oldcwd)
    except:
        code = -1
    return [code, output]

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
        path = os.path.split(filepath)[0]
        if not os.path.exists(path):
            os.makedirs(path)
        file = open(filepath, "w")
        file.write(content)
        file.close()
    except Exception, e:
        print(e)
        return False
    return True

def grep(path, pattern, include = [], exclude = []):
    result = {}
    expr = re.compile(pattern)
    for root, dirs, files in os.walk(path):
        for filename in files:
            accept = True
            for ipattern in include:
                if not fnmatch.fnmatch(filename, ipattern):
                    accept = False
            for epattern in exclude:
                if fnmatch.fnmatch(filename, epattern):
                    accept = False
            if accept:
                filepath = os.path.normpath(os.path.join(root, filename))
                content = read_file(filepath)
                for match in expr.finditer(content):
                    if match.group(1):
                        if filename not in result:
                            result[filename] = []
                        result[filename].append(match.group(1))
    return result

# ============================================================================
# OptionParser
# ============================================================================
class OptionParser(optparse.OptionParser):
    def __init__(self):
        optparse.OptionParser.__init__(self, formatter=optparse.TitledHelpFormatter())
        self.add_option("-v", "--verbose", action="store_true", dest="verbose",
                        help="Print verbose information during the configure.")
        self.set_defaults(verbose=False)

        if QMAKE.platform() != "symbian":
            group = optparse.OptionGroup(self, "Installation options")
            group.add_option("--prefix", dest="prefix", metavar="dir",
                             help="Install everything relative to <dir>. The default value is '%s'. "
                                  "NOTE: Use '--prefix .' to configure a local setup. A local "
                                  "setup will install nothing else than the qmake "
                                  "feature file." % BUILDENV.default_prefix())
            group.add_option("--bin-dir", dest="bindir", metavar="dir",
                             help="Install executables to <dir>. The default value is 'PREFIX/bin'.")
            group.add_option("--lib-dir", dest="libdir", metavar="dir",
                             help="Install libraries to <dir>. The default value is 'PREFIX/lib'.")
            group.add_option("--doc-dir", dest="docdir", metavar="dir",
                             help="Install documentation to <dir>. The default value is 'PREFIX/doc'.")
            group.add_option("--include-dir", dest="includedir", metavar="dir",
                             help="Install headers to <dir>. The default value is 'PREFIX/include'.")
            group.add_option("--plugin-dir", dest="plugindir", metavar="dir",
                             help="Install plugins to <dir>. The default value is 'PREFIX/plugins'.")
            group.add_option("--features-dir", dest="featuresdir", metavar="dir",
                             help="Install qmake feature files to <dir>. The default value is 'QTDIR/mkspecs/features'.")
            group.add_option("--resources-dir", dest="resourcesdir", metavar="dir",
                             help="Install resources to <dir>. The default value is 'PREFIX/resources'.")
            self.add_option_group(group)
        self.set_defaults(prefix=None)
        self.set_defaults(bindir=None)
        self.set_defaults(libdir=None)
        self.set_defaults(docdir=None)
        self.set_defaults(includedir=None)
        self.set_defaults(pluginsdir=None)
        self.set_defaults(featuresdir=None)
        self.set_defaults(resourcesdir=None)

        group = optparse.OptionGroup(self, "Configure options")
        group.add_option("--platform", dest="platform", metavar="platform",
                         help="Specify the platform (symbian/win32/unix). "
                              "The one detected by qmake is used by default "
                              "if not specified.")
        group.add_option("--make-bin", dest="makebin", metavar="path",
                         help="Specify the make tool (make, nmake, mingw32-make, gmake...). "
                              "The one detected in PATH is used by default if not specified.")
        if QMAKE.platform() == "win32" and MAKE.bin() == "nmake":
            group.add_option("--msvc", action="store_true", dest="msvc",
                             help="Generate a MSVC solution.")
        group.add_option("--release", action="store_const", dest="config", const="release",
                         help="Build in release mode.")
        group.add_option("--debug", action="store_const", dest="config", const="debug",
                         help="Build in debug mode.")
        group.add_option("--debug_and_release", action="store_const", dest="config", const="debug_and_release",
                         help="Build in both debug and release modes.")
        group.add_option("--debug-output", action="store_false", dest="debug_output",
                         help="Do not suppress debug and warning output (suppressed by default in release mode).")
        group.add_option("--no-debug-output", action="store_true", dest="no_debug_output",
                         help="Suppress debug and warning output (not supporessed by default in debug mode).")
        if QMAKE.platform() != "symbian":
            group.add_option("--silent", action="store_true", dest="silent",
                             help="Suppress verbose compiler output.")
            group.add_option("--fast", action="store_true", dest="fast",
                             help="Run qmake in non-recursive mode. Running qmake "
                                  "in recursive mode (default) is more reliable but "
                                  "takes relatively long due to deep project hierarchy. "
                                  "The build tree should be clean ie. no existing "
                                  "makefiles in subdirs, because those won't be "
                                  "regenerated if this option is enabled.")
        group.add_option("--defines", dest="defines", metavar="defines",
                         help="Define compiler macros for selecting features "
                              "and debugging purposes eg. --defines HB_FOO_DEBUG,HB_BAR_ENABLED")
        if QMAKE.platform() == "unix":
            group.add_option("--rpath", action="store_true", dest="rpath",
                             help="Link Hb libraries and executables using the library install "
                                  "path as a runtime library path.")
            group.add_option("--no-rpath", action="store_false", dest="rpath",
                             help="Do not use the library install path as a runtime library path.")
        self.add_option_group(group)
        self.set_defaults(platform=None)
        self.set_defaults(makebin=None)
        self.set_defaults(msvc=None)
        self.set_defaults(config=None)
        self.set_defaults(silent=False)
        self.set_defaults(fast=False)
        self.set_defaults(defines=None)
        self.set_defaults(rpath=None)

        group = optparse.OptionGroup(self, "Host options")
        group.add_option("--host-qmake-bin", dest="hostqmakebin", metavar="path",
                         help="Specify the host qmake tool.")
        group.add_option("--host-make-bin", dest="hostmakebin", metavar="path",
                         help="Specify the host make tool (make, nmake, mingw32-make, gmake...).")
        self.add_option_group(group)
        self.set_defaults(hostqmakebin=None)
        self.set_defaults(hostmakebin=None)

        group = optparse.OptionGroup(self, "qmake options")
        group.add_option("--qmake-bin", dest="qmakebin", metavar="path",
                         help="Specify the path to the qmake. The one "
                              "in PATH is used by default if not specified.")
        group.add_option("--qmake-spec", dest="qmakespec", metavar="spec",
                         help="The operating system and compiler you are building on.")
        group.add_option("--qmake-options", dest="qmakeopt", metavar="options",
                         help="Additional qmake options "
                              "eg. --qmake-options \"CONFIG+=foo DEFINES+=BAR\".")
        self.add_option_group(group)
        self.set_defaults(qmakeopt=None)

        group = optparse.OptionGroup(self, "Feature options")
        group.add_option("--make", action="append", dest="make", metavar="part",
                         help="Do build <part>. Valid parts: tests localization performance")
        group.add_option("--nomake", action="append", dest="nomake", metavar="part",
                         help="Do not build <part>. Valid parts: <collection> tutorials tests fute unit localization performance")
        group.add_option("--no-effects", action="store_false", dest="effects",
                         help="Do not build effects.")
        group.add_option("--no-gestures", action="store_false", dest="gestures",
                         help="Do not build gestures.")
        group.add_option("--no-text-measurement", action="store_false", dest="textMeasurement",
                         help="Do not build text measurement support (needed for localization).")
        self.add_option_group(group)
        self.set_defaults(make=None)
        self.set_defaults(nomake=None)
        self.set_defaults(effects=True)
        self.set_defaults(gestures=True)
        self.set_defaults(textMeasurement=True)

        group = optparse.OptionGroup(self, "Qt options")
        group.add_option("--qt-mobility", action="store_true", dest="qtmobility",
                         help="Assumes that Qt Mobility is available without performing a compilation test.")
        group.add_option("--no-qt-mobility", action="store_false", dest="qtmobility",
                         help="Assumes that Qt Mobility is not available without performing a compilation test.")
        group.add_option("--meegotouch", action="store_true", dest="meegotouch",
                         help="Assumes that MeeGoTouch UI is available without performing a compilation test.")
        group.add_option("--no-meegotouch", action="store_false", dest="meegotouch",
                         help="Assumes that MeeGoTouch UI is not available without performing a compilation test.")
        group.add_option("--qt-openvg", action="store_true", dest="qtopenvg",
                         help="Assumes that OpenVG is available without performing a compilation test.")
        group.add_option("--no-qt-openvg", action="store_false", dest="qtopenvg",
                         help="Assumes that OpenVG is not available without performing a compilation test.")
        self.add_option_group(group)
        self.set_defaults(qtmobility=None)
        self.set_defaults(qtopenvg=None)

        group = optparse.OptionGroup(self, "Developer options")
        group.add_option("--developer", action="store_true", dest="developer",
                         help="Enables the developer mode. The developer mode implies "
                         "a local setup, enables tests and exports extra symbols for "
                         "testing purposes. NOTE: this is equal to: --prefix . --make tests "
                         "--developer-export")
        group.add_option("--developer-export", action="store_true", dest="developerexport",
                         help="Enables developer exports ie. extra symbols for testing purposes.")
        if QMAKE.platform() != "symbian":
            group.add_option("--coverage", action="store_true", dest="coverage",
                             help="Builds with test coverage measurement support. "
                                  "This implies the developer mode.")
        self.add_option_group(group)
        self.set_defaults(developer=False)
        self.set_defaults(developerexport=None)
        self.set_defaults(coverage=False)

# ============================================================================
# Make
# ============================================================================
class Make:
    def __init__(self):
        self._bin = None

    def init(self, cmdline):
        match = re.search("--make-bin[=\s](\S+)", cmdline)
        if match:
            self._bin = match.group(1)
        else:
            self._bin = self._detect_make()

    def command(self, target=None):
        _args = [self._bin]
        if target:
            _args += [target]
        return _args

    def bin(self):
        return self._bin

    def _detect_make(self):
        if QMAKE.platform() == "win32" and self._test_make("nmake", "/?"):
            return "nmake"
        if self._test_make("make", "-v"):
            return "make"
        if self._test_make("gmake", "-v"):
            return "gmake"
        if QMAKE.platform() == "win32" and self._test_make("mingw32-make", "-v"):
            return "mingw32-make"
        return "make"

    def _test_make(self, command, param):
        try:
            return run_process([command, param])[0] == 0
        except:
            return False

# ============================================================================
# QMake
# ============================================================================
class QMake:
    def __init__(self):
        self._bin = "qmake"
        self._platform = None
        self._error = None
        self._spec = None
        self._features = None
        self._qtdir = None
        self._qtversion = None
        self._args = []

    def init(self, cmdline):
        match = re.search("--qmake-bin[=\s](\S+)", cmdline)
        if match:
            self._bin = match.group(1)
        match = re.search("--platform[=\s](\S+)", cmdline)
        if match:
            self._platform = match.group(1)
        match = re.search("--qmake-spec[=\s](\S+)", cmdline)
        if match:
            self._spec = match.group(1)
        return self._run_qmake()

    def command(self, profile=None):
        _args = [self._bin]
        if self._spec:
            _args += ["-spec", self._spec]
        if len(self._args):
            _args += self._args
        if profile:
            _args += [profile]
        return _args

    def add_args(self, args):
        self._args += args

    def platform(self):
        return self._platform

    def bin(self):
        return self._bin

    def error(self):
        return self._error

    def spec(self):
        return self._spec

    def features(self):
        return self._features

    def qtdir(self):
        return self._qtdir

    def qtversion(self):
        return self._qtversion

    def _run_qmake(self):
        # write .pro
        content = """
                  symbian:message(platform:symbian)
                  symbian:message(platform:symbian)
                  else:macx:message(platform:macx)
                  else:unix:message(platform:unix)
                  else:win32:message(platform:win32)
                  message(features:$$[QMAKE_MKSPECS]/features)
                  message(qtversion:$$[QT_VERSION])
                  message(qtdir:$$[QT_INSTALL_LIBS])
                  """
        if not write_file("tmp/qmake.pro", content):
            self._error = "Unable to write 'tmp/qmake.pro'. Make sure to configure in a writable directory."
            return False

        # run qmake
        args = [self._bin, "-nocache", "qmake.pro"]
        if self._spec:
            args += ["-spec", self._spec]
        (code, output) = run_process(args, "tmp")

        # cleanup & check return
        shutil.rmtree("tmp", ignore_errors=True)
        if code != 0:
            self._error = "Unable to execute %s" % self._bin
            if self._bin == "qmake":
                self._error += ". Add qmake to PATH or pass --qmake-bin <path/to/qmake>."
            return False

        # parse output
        try:
            if not self._platform:
                self._platform = re.search("Project MESSAGE: platform:(\S+)", output).group(1)
            self._features = re.search("Project MESSAGE: features:(\S+)", output).group(1)
            self._qtversion = re.search("Project MESSAGE: qtversion:(\S+)", output).group(1)
            self._qtdir = re.search("Project MESSAGE: qtdir:(\S+)", output).group(1)
        except:
            self._error = "Unable to parse qmake output (%s)" % output.strip()
            if output.find("QMAKESPEC") != -1:
                self._error += ". Set QMAKESPEC environment variable or pass --qmake-spec <spec>."
            return False
        return True

# ============================================================================
# BuildEnvironment
# ============================================================================
class BuildEnvironment:
    def __init__(self):
        self._blddir = os.path.abspath(os.getcwd())
        self._srcdir = os.path.abspath(sys.path[0])
        self._prefix = None
        self._bindir = None
        self._libdir = None
        self._docdir = None
        self._includedir = None
        self._pluginsdir = None
        self._featuresdir = None
        self._resourcesdir = None

    def init(self, options):
        # prefix
        if options.prefix:
            # explicit
            self._prefix = options.prefix
        elif options.developer:
            # developer mode implies a "local" build
            self._prefix = self._blddir
        else:
            # fall back to default
            self._prefix = self.default_prefix()
        if QMAKE.platform() != "symbian":
            self._prefix = os.path.abspath(self._prefix)

        self._bindir = self._dir_option(options.bindir, self._prefix + "/bin")
        self._libdir = self._dir_option(options.libdir, self._prefix + "/lib")
        self._docdir = self._dir_option(options.docdir, self._prefix + "/doc")
        self._includedir = self._dir_option(options.includedir, self._prefix + "/include")
        self._pluginsdir = self._dir_option(options.pluginsdir, self._prefix + "/plugins")
        self._featuresdir = self._dir_option(options.featuresdir, QMAKE.features())
        self._resourcesdir = self._dir_option(options.resourcesdir, self._prefix + "/resources")

        # symbian specific adjustments
        if QMAKE.platform() == "symbian":
            # TODO: fix to "$${EPOCROOT}resource/hb/plugins"
            self._pluginsdir = "$${EPOCROOT}resource/qt/plugins/hb"

            if not options.developer:
                if os.path.isdir("/s60"):
                    self._includedir = self._prefix + "/include/hb"
                else:
                    self._includedir = self._prefix + "/include/mw/hb"

    def builddir(self):
        return self._blddir

    def sourcedir(self):
        return self._srcdir

    def prefix(self):
        return self._prefix

    def bindir(self):
        return self._bindir

    def libdir(self):
        return self._libdir

    def docdir(self):
        return self._docdir

    def includedir(self):
        return self._includedir

    def pluginsdir(self):
        return self._pluginsdir

    def featuresdir(self):
        return self._featuresdir

    def resourcesdir(self):
        return self._resourcesdir

    def exportdir(self, category=None):
        if os.path.isdir("/s60"):
            if category:
                return "hb/%1/" + category + "/%2"
            return "hb/%1/%2"
        else:
            if category:
                return "$${EPOCROOT}epoc32/include/mw/hb/%1/" + category + "/%2"
            return "$${EPOCROOT}epoc32/include/mw/hb/%1/%2"

    def default_prefix(self):
        prefixes = { "symbian" : "$${EPOCROOT}epoc32",
                     "unix"    : "/usr/local/hb",
                     "macx"    : "/usr/local/hb",
                     "win32"   : "C:/hb" }
        return prefixes.get(QMAKE.platform(), self._blddir)

    def local(self):
        prefix = self.prefix()
        return os.path.isdir(prefix) and (prefix == self._blddir)

    def _dir_option(self, explicit, default):
        if explicit:
            return explicit
        return default

# ============================================================================
# ConfigTest
# ============================================================================
class ConfigTest:
    def __init__(self, sourcedir=os.getcwd(), builddir=os.getcwd()):
        self._sourcedir = sourcedir
        self._builddir = builddir

    def compile(self, test):
        code = -1
        prevdir = os.getcwd()
        try:
            basename = os.path.basename(test)
            sourcedir = os.path.join(self._sourcedir, test)
            filepath = os.path.join(sourcedir, basename + ".pro")
            builddir = os.path.join(self._builddir, test)

            # create build dir
            if not os.path.exists(builddir):
                os.makedirs(builddir)
            os.chdir(builddir)

            # run qmake & make
            run_process(QMAKE.command(filepath))
            (code, output) = run_process(MAKE.command())

            # make return value is not reliable
            if QMAKE.platform() == "symbian":
                # on symbian, check that no error patterns such as '***' can be found from build output
                patterns = ["\\*\\*\\*", "Errors caused tool to abort"]
                for pattern in patterns:
                    if re.search(pattern, output) != None:
                        code = -1
            else:
                # on other platforms, check that the resulting executable exists
                if os.name == "nt":
                    executable = os.path.join(os.path.join(builddir, "debug"), "hbconftest_" + basename + ".exe")
                    if not os.path.exists(executable) or not os.access(executable, os.X_OK):
                        executable = os.path.join(os.path.join(builddir, "release"), "hbconftest_" + basename + ".exe")
                        if not os.path.exists(executable) or not os.access(executable, os.X_OK):
                            code = -1
                else:
                    executable = os.path.join(builddir, "hbconftest_" + basename)
                    if not os.path.exists(executable) or not os.access(executable, os.X_OK):
                        code = -1

            # clean
            run_process(MAKE.command("clean"))

        except:
            code = -1
        os.chdir(prevdir)
        return code == 0

# ============================================================================
# ConfigFile
# ============================================================================
class ConfigFile:
    def __init__(self):
        self._lines = list()

    def set_value(self, key, value):
        self._lines.append("%s = %s\n" % (key, value))

    def add_value(self, key, value):
        self._lines.append("%s += %s\n" % (key, value))

    def remove_value(self, key, value):
        self._lines.append("%s -= %s\n" % (key, value))

    def format_dir(dir):
        return "$$quote(%s)" % dir.replace("\\", "/")
    format_dir = staticmethod(format_dir)

    def quote_dir(dir):
        return "\\\"\\\\\\\"%s\\\\\\\"\\\"" % ConfigFile.format_dir(dir)
    quote_dir = staticmethod(quote_dir)

    def write(self, filename):
        try:
            file = open(filename, "w+")
            file.writelines(self._lines)
            file.close()
        except IOError, e:
            print(e)
            return False
        return True

# ============================================================================
# main()
# ============================================================================
def main():
    global QMAKE, MAKE, BUILDENV, HB_MAKE_PARTS, HB_NOMAKE_PARTS

    help = False
    cmdline = " ".join(sys.argv[1:])
    match = re.search("--help\s*", cmdline)
    if match or re.search("-h\s*", cmdline):
        help = True

    QMAKE = QMake()
    QMAKE.init(cmdline)

    if not help and not QMAKE.platform():
        print("ERROR: %s" % QMAKE.error())
        return

    MAKE = Make()
    MAKE.init(cmdline)

    BUILDENV = BuildEnvironment()

    # parse command line options
    parser = OptionParser()
    (options, args) = parser.parse_args()

    BUILDENV.init(options)

    # coverage implies developer mode
    if options.coverage:
        options.developer = True

    # developer mode implies tests, including perf & loc
    if options.developer:
        add_remove_part("tests", True)
        add_remove_part("performance", True)
        add_remove_part("localization", True)

    # developer mode implies developer exports
    if options.developer:
        options.developerexport = True

    print("Configuring Hb...")
    print("INFO: Platform: %s" % QMAKE.platform())
    print("INFO: Make: %s" % MAKE.bin())
    print("INFO: Qt: %s in %s" % (QMAKE.qtversion(), QMAKE.qtdir()))

    # compilation tests to detect available features
    config = ConfigFile()
    test = ConfigTest(BUILDENV.sourcedir(), BUILDENV.builddir())
    print("\nDetecting available features...")
    if options.qtmobility == None:
        options.qtmobility = test.compile("config.tests/all/mobility")
    if options.qtmobility:
        config.add_value("DEFINES", "HB_HAVE_QT_MOBILITY")
    print("INFO: Qt Mobility:\t\t\t%s" % options.qtmobility)
    if options.qtopenvg == None:
        options.qtopenvg = test.compile("config.tests/all/openvg")
    if options.qtopenvg:
        config.add_value("DEFINES", "HB_EFFECTS_OPENVG")
        config.add_value("DEFINES", "HB_FILTER_EFFECTS")
    print("INFO: OpenVG:\t\t\t\t%s" % options.qtopenvg)
    if QMAKE.platform() == "symbian":
        sgimagelite_result = test.compile("config.tests/symbian/sgimagelite")
        if sgimagelite_result:
            config.add_value("CONFIG", "sgimage")
        print("INFO: SgImage-Lite:\t\t\t%s" % sgimagelite_result)
        tchunkcreateinfo_result = test.compile("config.tests/symbian/tchunkcreateinfo")
        if tchunkcreateinfo_result:
            config.add_value("DEFINES", "HB_HAVE_PROTECTED_CHUNK")
        touchfeedback_result = test.compile("config.tests/symbian/touchfeedback")
        if touchfeedback_result:
            config.add_value("DEFINES", "HB_TOUCHFEEDBACK_TYPE_IS_LONGPRESS")
        print("INFO: ETouchFeedbackLongPress:\t\t%s" % touchfeedback_result)
    if options.meegotouch == None:
        options.meegotouch = test.compile("config.tests/meego/meegotouch")
    if options.meegotouch:
        config.add_value("CONFIG", "hb_meegotouch")
        config.add_value("DEFINES", "HB_MEEGOTOUCH")
    print("INFO: MeeGo Touch:\t\t\t%s" % options.meegotouch)

    config.set_value("HB_INSTALL_DIR", ConfigFile.format_dir(BUILDENV.prefix()))
    config.set_value("HB_BIN_DIR", ConfigFile.format_dir(BUILDENV.bindir()))
    config.set_value("HB_LIB_DIR", ConfigFile.format_dir(BUILDENV.libdir()))
    config.set_value("HB_DOC_DIR", ConfigFile.format_dir(BUILDENV.docdir()))
    config.set_value("HB_INCLUDE_DIR", ConfigFile.format_dir(BUILDENV.includedir()))
    config.set_value("HB_PLUGINS_DIR", ConfigFile.format_dir(BUILDENV.pluginsdir()))
    config.set_value("HB_FEATURES_DIR", ConfigFile.format_dir(BUILDENV.featuresdir()))
    config.set_value("HB_RESOURCES_DIR", ConfigFile.format_dir(BUILDENV.resourcesdir()))

    # TODO: get rid of this!
    if QMAKE.platform() == "symbian":
        config.set_value("HB_PLUGINS_EXPORT_DIR", ConfigFile.format_dir("$${EPOCROOT}epoc32/winscw/c/resource/qt/plugins/hb"))

    if options.gestures:
        config.add_value("DEFINES", "HB_GESTURE_FW")
    if options.effects:
        config.add_value("DEFINES", "HB_EFFECTS")
    if options.textMeasurement:
        config.add_value("DEFINES", "HB_TEXT_MEASUREMENT_UTILITY")
    if QMAKE.platform() != "symbian" and options.developer:
        config.add_value("DEFINES", "HB_CSS_INSPECTOR")
    if options.defines:
        config.add_value("DEFINES", " ".join(options.defines.split(",")))
    if options.developerexport:
        config.add_value("DEFINES", "HB_DEVELOPER")
    if options.rpath == None or options.rpath == True:
        config.add_value("QMAKE_RPATHDIR", "$${HB_LIB_DIR}")

    if options.verbose:
        print("INFO: Writing hb_install.prf")
    if not config.write("hb_install.prf"):
        print("ERROR: Unable to write hb_install_prf.")
        return

    config.set_value("HB_BUILD_DIR", ConfigFile.format_dir(BUILDENV.builddir()))
    config.set_value("HB_SOURCE_DIR", ConfigFile.format_dir(BUILDENV.sourcedir()))
    if QMAKE.platform() == "symbian":
        config.set_value("HB_EXPORT_DIR", ConfigFile.format_dir(BUILDENV.exportdir()))
        config.set_value("HB_RESTRICTED_EXPORT_DIR", ConfigFile.format_dir(BUILDENV.exportdir("restricted")))

    if options.make:
        for part in options.make:
            add_remove_part(part, True)
    if options.nomake:
        for part in options.nomake:
            add_remove_part(part, False)

    for part in HB_MAKE_PARTS:
        add_remove_part(part, True)

    for nomake in HB_NOMAKE_PARTS:
        config.add_value("HB_NOMAKE_PARTS", nomake)

    if options.qmakeopt:
        for qmakeopt in options.qmakeopt.split():
            config._lines.append(qmakeopt + "\n")

    if BUILDENV.local():
        config.add_value("CONFIG", "local")
    if options.silent:
        config.add_value("CONFIG", "silent")
    if options.effects:
        config.add_value("CONFIG", "effects")
    if options.gestures:
        config.add_value("CONFIG", "gestures")
    if options.developer:
        config.add_value("CONFIG", "developer")
    if options.coverage:
        config.add_value("CONFIG", "coverage")
    if options.config:
        config.add_value("CONFIG", options.config)
    if options.debug_output != None:
        config.add_value("CONFIG", "debug_output")
    if options.no_debug_output != None:
        config.add_value("CONFIG", "no_debug_output")

    # debug & warning outputs:
    #   - release
    #       - disabled by default
    #       - can be enabled by passing --debug_output option
    #   - debug
    #       - enabled by default
    #       - can be disabled by passing --no_debug_output option
    config._lines.append("CONFIG(release, debug|release) {\n")
    config._lines.append("    debug_output|developer {\n")
    config._lines.append("        # debug/warning output enabled\n")
    config._lines.append("    } else {\n")
    config._lines.append("        DEFINES += QT_NO_DEBUG_OUTPUT\n")
    config._lines.append("        DEFINES += QT_NO_WARNING_OUTPUT\n")
    config._lines.append("    }\n")
    config._lines.append("} else {\n")
    config._lines.append("    no_debug_output {\n")
    config._lines.append("        DEFINES += QT_NO_DEBUG_OUTPUT\n")
    config._lines.append("        DEFINES += QT_NO_WARNING_OUTPUT\n")
    config._lines.append("    }\n")
    config._lines.append("}\n")

    # ensure that no QString(0) -like constructs slip in
    config.add_value("DEFINES", "QT_QCHAR_CONSTRUCTOR")

    # TODO: is there any better way to expose functions to the whole source tree?
    config._lines.append("include(%s)\n" % (os.path.splitdrive(BUILDENV.sourcedir())[1] + "/mkspecs/hb_functions.prf"))

    if options.verbose:
        print("INFO: Writing .qmake.cache")
    if not config.write(".qmake.cache"):
        print("ERROR: Unable to write .qmake.cache.")
        return

    if os.name == "posix" or os.name == "mac":
        sharedmem = test.compile("config.tests/unix/sharedmemory")
        if sharedmem:
            (code, output) = run_process(["./hbconftest_sharedmemory"], "config.tests/unix/sharedmemory")
            sharedmem = (code == 0)
            if not sharedmem:
                print("DEBUG:%s" % output)
        print("INFO: Shared Memory:\t\t\t%s" % sharedmem)
        if not sharedmem:
            print("WARNING:The amount of available shared memory is too low!")
            print "\tTry adjusting the shared memory configuration",
            if os.path.exists("/proc/sys/kernel/shmmax"):
                print "(/proc/sys/kernel/shmmax)"
            elif os.path.exists("/etc/sysctl.conf"):
                print "(/etc/sysctl.conf)"

    # generate local build wrapper headers
    print("\nGenerating files...")
    print("INFO: Wrapper headers")
    synchb = "bin/synchb.py"
    if options.verbose:
        print("INFO: Running %s" % synchb)
        synchb = "%s -v" % synchb
    os.system("python %s/%s -i %s -o %s" % (BUILDENV.sourcedir(), synchb, BUILDENV.sourcedir(), BUILDENV.builddir()))

    # generate a qrc for resources
    print("INFO: Qt resource collection")
    args = [os.path.join(BUILDENV.sourcedir(), "bin/resourcifier.py")]
    args += ["-i", "%s" % os.path.join(sys.path[0], "src/hbcore/resources")]
    # TODO: make it BUILDENV.builddir()
    args += ["-o", "%s" % os.path.join(BUILDENV.sourcedir(), "src/hbcore/resources/resources.qrc")]
    args += ["--exclude", "\"*distribution.policy.s60\""]
    args += ["--exclude", "\"*readme.txt\""]
    args += ["--exclude", "\"*.pr?\""]
    args += ["--exclude", "\"*.qrc\""]
    args += ["--exclude", "\"*~\""]
    args += ["--exclude", "variant/*"]
    args += ["--exclude", "\"*hbdefault.cssbin\""]

    if QMAKE.platform() != "symbian":
        args += ["--exclude", "\"*symbian*\""]
    if QMAKE.platform() != "macx":
        args += ["--exclude", "\"*macx*\""]
    if QMAKE.platform() != "unix":
        args += ["--exclude", "\"*unix*\""]
    if QMAKE.platform() != "win32":
        args += ["--exclude", "\"*win32*\""]

    if options.verbose:
        print("INFO: Running %s" % " ".join(args))
    os.system("python %s" % " ".join(args))

    # build host tools
    if QMAKE.platform() == "symbian" or options.hostqmakebin != None or options.hostmakebin != None:
        print("\nBuilding host tools...")
        if options.hostqmakebin != None and options.hostmakebin != None:
            profile = "%s/src/hbtools/hbtools.pro" % BUILDENV.sourcedir()
            if os.path.exists(profile):
                toolsdir = os.path.join(BUILDENV.builddir(), "src/hbtools")
                if not os.path.exists(toolsdir):
                    os.makedirs(toolsdir)
                os.chdir(toolsdir)
                os.system("\"%s\" -config silent %s" % (options.hostqmakebin, profile))
                os.system("\"%s\"" % (options.hostmakebin))
                os.chdir(BUILDENV.builddir())
        else:
            print("WARNING: Cannot build host tools, because no --host-qmake-bin and/or")
            print("         --host-make-bin was provided. Hb will attempt to run host")
            print("         tools from PATH.")

    # run qmake
    profile = os.path.join(BUILDENV.sourcedir(), "hb.pro")
    QMAKE.add_args(["-cache", os.path.join(BUILDENV.builddir(), ".qmake.cache")])
    if options.msvc:
        QMAKE.add_args(["-tp", "vc"])
    if not options.fast:
        QMAKE.add_args(["-r"])
    if options.verbose:
        print("\nRunning %s" % QMAKE.command(profile))
    else:
        print("\nRunning qmake...")
    try:
        ret = run_system(QMAKE.command(profile))
    except KeyboardInterrupt:
        ret = -1
    if ret != 0:
        print("")
        print("ERROR: Aborted!")
        print("")
        return

    if "tests" not in HB_NOMAKE_PARTS:
        # run qmake for tests
        profile = "%s/tsrc/tsrc.pro" % BUILDENV.sourcedir()
        if os.path.exists(profile):
            tsrcdir = os.path.join(BUILDENV.builddir(), "tsrc")
            if not os.path.exists(tsrcdir):
                os.makedirs(tsrcdir)
            os.chdir(tsrcdir)
            if options.verbose:
                print("\nRunning %s" % QMAKE.command(profile))
            else:
                print("\nRunning qmake in tsrc...")
            run_system(QMAKE.command(profile))
            os.chdir(BUILDENV.builddir())

            # create output dirs
            outputdir = os.path.join(BUILDENV.builddir(), "autotest")
            if not os.path.exists(outputdir):
                os.makedirs(outputdir)
            outputdir = os.path.join(BUILDENV.builddir(), "coverage")
            if not os.path.exists(outputdir):
                os.makedirs(outputdir)
        # nag about tests that are commented out
        result = grep(BUILDENV.sourcedir() + "/tsrc", "#\s*SUBDIRS\s*\+=\s*(\S+)", ["*.pr?"])
        maxlen = 0
        for profile in result:
            maxlen = max(maxlen, len(profile))
        if len(result):
            print ""
            print "###############################################################################"
            print "%s THE FOLLOWING TESTS ARE COMMENTED OUT:" % "WARNING:".ljust(maxlen + 1)
            for profile, subdirs in result.iteritems():
                line = (profile + ":").ljust(maxlen + 2)
                init = len(line)
                while len(subdirs):
                    if len(line) > init:
                        line += ", "
                    if len(line) + len(subdirs[-1]) < 80:
                        line += subdirs.pop()
                    elif len(line) == init and init + len(subdirs[-1]) >= 79:
                        line += subdirs.pop()
                    else:
                        print line
                        line = "".ljust(maxlen + 2)
                if len(line) > init:
                    print line
            print "###############################################################################"

    # print summary
    print("")
    if MAKE.bin() == "nmake" and options.msvc:
        conf = "MSVC"
        act = "open 'hb.sln'"
    elif options.coverage:
        conf = "test coverage measurement"
        act = "run '%s coverage'" % MAKE.bin()
    elif options.developer:
        conf = "development"
        act = "run '%s'" % MAKE.bin()
    else:
        conf = "building"
        act = "run '%s'" % MAKE.bin()
    print("Hb is now configured for %s. Just %s." % (conf, act))

    if not options.coverage:
        if QMAKE.platform() == "symbian" or BUILDENV.local():
            print("You must run '%s install' to copy Hb files in place." % MAKE.bin())
        else:
            print("Once everything is built, you must run '%s install'." % MAKE.bin())
        if QMAKE.platform() != "symbian":
            if BUILDENV.local():
                print("Hb will be used from '%s'." % BUILDENV.prefix())
            else:
                print("Hb will be installed to '%s'." % BUILDENV.prefix())
    if QMAKE.platform() == "win32":
        print("NOTE: Make sure that '%s' is in PATH." % BUILDENV.bindir())
        if options.coverage:
            print("Test code coverage measurement will FAIL if wrong Hb DLLs are found in PATH before '%s'." % path)

    print("")
    print("To reconfigure, run '%s clean' and '%s'." % (MAKE.bin(), sys.argv[0]))
    print("")

if __name__ == "__main__":
    main()
