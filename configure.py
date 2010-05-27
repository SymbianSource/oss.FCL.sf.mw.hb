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

def run_process(args, cwd=None):
    code = 0
    output = ""
    if os.name == "nt":
        env = os.environ.copy()
        epocroot = env.get("EPOCROOT")
        if epocroot:
            if not epocroot.endswith("\\") or epocroot.endswith("/"):
                env["EPOCROOT"] = "%s/" % epocroot
            
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
            if os.name == "nt":
                process = subprocess.Popen(args, env=env, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
                (stdout, stderr) = process.communicate()
                code = process.returncode
                output = stdout + stderr
            else:
                process = subprocess.Popen(args, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
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
    def __init__(self, platform, make, prefix):
        optparse.OptionParser.__init__(self, formatter=optparse.TitledHelpFormatter())
        self.add_option("-v", "--verbose", action="store_true", dest="verbose",
                        help="Print verbose information during the configure.")
        self.set_defaults(verbose=False)

        if platform != "symbian":
            group = optparse.OptionGroup(self, "Installation options")
            group.add_option("--prefix", dest="prefix", metavar="dir",
                             help="Install everything relative to <dir>. The default value is '%s'. "
                                  "NOTE: Use '--prefix .' to configure a local setup. A local "
                                  "setup will install nothing else than the qmake "
                                  "feature file." % prefix)
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
            group.add_option("--resource-dir", dest="resourcedir", metavar="dir",
                             help="Install resources to <dir>. The default value is 'PREFIX/resources'.")
            group.add_option("--feature-dir", dest="featuredir", metavar="dir",
                             help="Install qmake feature files to <dir>. The default value is 'QTDIR/mkspecs/features'.")
            self.add_option_group(group)
        self.set_defaults(prefix=None)
        self.set_defaults(bindir=None)
        self.set_defaults(libdir=None)
        self.set_defaults(docdir=None)
        self.set_defaults(includedir=None)
        self.set_defaults(plugindir=None)
        self.set_defaults(resourcedir=None)
        self.set_defaults(featuredir=None)

        group = optparse.OptionGroup(self, "Configure options")
        group.add_option("--platform", dest="platform", metavar="platform",
                         help="Specify the platform (symbian/win32/unix). "
                              "The one detected by qmake is used by default "
                              "if not specified.")
        group.add_option("--make-bin", dest="makebin", metavar="path",
                         help="Specify the make tool (make, nmake, mingw32-make, gmake...). "
                              "The one detected in PATH is used by default if not specified.")
        if platform == "win32" and make == "nmake":
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
        if platform != "symbian":
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
        self.add_option_group(group)
        self.set_defaults(platform=None)
        self.set_defaults(makebin=None)
        self.set_defaults(msvc=None)
        self.set_defaults(config=None)
        self.set_defaults(silent=False)
        self.set_defaults(fast=False)
        self.set_defaults(defines=None)

        group = optparse.OptionGroup(self, "Host options")
        group.add_option("--host-qmake-bin", dest="hostqmakebin", metavar="path",
                         help="Specify the host qmake tool.")
        group.add_option("--host-make-bin", dest="hostmakebin", metavar="path",
                         help="Specify the host make tool (make, nmake, mingw32-make, gmake...).")
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
        self.set_defaults(qmakebin=None)
        self.set_defaults(qmakespec=None)
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
        group.add_option("--no-inputs", action="store_false", dest="inputs",
                         help="DEPRECATED: Use --nomake hbinput.")
        group.add_option("--no-feedback", action="store_false", dest="feedback",
                         help="DEPRECATED: Use --nomake hbfeedback.")
        group.add_option("--no-tutorials", action="store_false", dest="tutorials",
                         help="DEPRECATED: Use --nomake tutorials.")
        self.add_option_group(group)
        self.set_defaults(make=None)
        self.set_defaults(nomake=None)
        self.set_defaults(effects=True)
        self.set_defaults(gestures=True)
        self.set_defaults(textMeasurement=True)
        self.set_defaults(inputs=None)
        self.set_defaults(feedback=None)
        self.set_defaults(tutorials=None)

        group = optparse.OptionGroup(self, "Qt options")
        group.add_option("--qt-mobility", action="store_true", dest="qtmobility",
                         help="Assumes that Qt Mobility is available without performing a compilation test.")
        group.add_option("--no-qt-mobility", action="store_false", dest="qtmobility",
                         help="Assumes that Qt Mobility is not available without performing a compilation test.")
        group.add_option("--qt-animation", action="store_true", dest="qtanimation",
                         help="DEPRECATED: Qt 4.6 includes the animation framework.")
        group.add_option("--no-qt-animation", action="store_false", dest="qtanimation",
                         help="DEPRECATED: Qt 4.6 includes the animation framework.")
        group.add_option("--qt-gestures", action="store_true", dest="qtgestures",
                         help="DEPRECATED: Qt 4.6 includes the gestures framework.")
        group.add_option("--no-qt-gestures", action="store_false", dest="qtgestures",
                         help="DEPRECATED: Qt 4.6 includes the gestures framework.")
        if platform == "symbian" or platform == None:
            group.add_option("--qt-symbian-eventfilter", action="store_false", dest="s60eventfilter",
                             help="DEPRECATED: Qt 4.6 includes QApplication::symbianEventFilter().")
            group.add_option("--qt-s60-eventfilter", action="store_true", dest="s60eventfilter",
                             help="DEPRECATED: Qt 4.6 includes QApplication::symbianEventFilter().")
        group.add_option("--dui", action="store_true", dest="dui",
                         help="Assumes that Maemo Direct UI is available without performing a compilation test.")
        group.add_option("--no-dui", action="store_false", dest="dui",
                         help="Assumes that Maemo Direct UI is not available without performing a compilation test.")
        self.add_option_group(group)
        self.set_defaults(qtmobility=None)
        self.set_defaults(qtanimation=None)
        self.set_defaults(qtgestures=None)
        self.set_defaults(qts60eventfilter=None)

        group = optparse.OptionGroup(self, "Developer options")
        group.add_option("--developer", action="store_true", dest="developer",
                         help="Enables the developer mode ie. builds tests and exports necessary symbols. "
                              "NOTE: The developer mode implies a local setup by default.")
        if platform != "symbian":
            group.add_option("--coverage", action="store_true", dest="coverage",
                             help="Builds with test coverage measurement support. "
                                  "This implies the developer mode.")
        group.add_option("--tests", action="store_true", dest="tests",
                         help="DEPRECATED: Use --make tests.")
        group.add_option("--performance", action="store_true", dest="performance",
                         help="DEPRECATED: Use --make performance.")
        group.add_option("--localization", action="store_true", dest="localization",
                         help="DEPRECATED: Use --make localization.")
        self.add_option_group(group)
        self.set_defaults(developer=False)
        self.set_defaults(coverage=False)
        self.set_defaults(tests=None)
        self.set_defaults(performance=None)
        self.set_defaults(localization=None)

# ============================================================================
# Platform
# ============================================================================
class Platform:
    def __init__(self, qmake):
        self._platform = None
        self._make = None
        self._error = None
        self._qmake = qmake
        self._spec = None
        self._version = None
        self._features = None
        self._qtdir = None

    def name(self):
        if not self._platform:
            self._detect_qt()
        return self._platform

    def make(self):
        if not self._make:
            self._make = self._detect_make()
        return self._make

    def qmake(self):
        if not self._qmake:
            self._detect_qt()
        return self._qmake

    def error(self):
        return self._error

    def spec(self):
        if not self._spec:
            self._detect_qt()
        return self._spec

    def version(self):
        if not self._version:
            self._detect_qt()
        return self._version

    def features(self):
        if not self._features:
            self._detect_qt()
        return self._features

    def qtdir(self):
        if not self._qtdir:
            self._detect_qt()
        return self._qtdir

    def _detect_qt(self):
        lines = list()
        lines.append("symbian:message(platform:symbian)\n")
        lines.append("else:macx:message(platform:macx)\n")
        lines.append("else:unix:message(platform:unix)\n")
        lines.append("else:win32:message(platform:win32)\n")

        lines.append("message(version:$$[QT_VERSION])\n")
        lines.append("message(libraries:$$[QT_INSTALL_LIBS])\n")
        lines.append("message(features:$$[QMAKE_MKSPECS]/features)\n")

        try:
            if not os.path.exists("tmp"):
                os.makedirs("tmp")
            fd, filepath = tempfile.mkstemp(dir="tmp", text=True, suffix=".pro")
            file = os.fdopen(fd, "w+")
            file.writelines(lines)
            file.close()
        except Exception, e:
            print(e)
            self._error = "Unable to write a temporary file. Make sure to configure in a writable directory."
            return

        # do not use .qmake.cache when detecting the platform
        args = [self._qmake, "-nocache", os.path.split(filepath)[1]]
        if self._spec:
            args += ["-spec", self._spec]
        (code, output) = run_process(args, "tmp")
        shutil.rmtree("tmp", ignore_errors=True)
        if code != 0:
            self._error = "Unable to execute %s" % self._qmake
            if self._qmake == "qmake":
                self._error += ". Add qmake to PATH or pass --qmake-bin <path/to/qmake>."

        try:
            self._platform = re.search("Project MESSAGE: platform:(\S+)", output).group(1)
            self._version = re.search("Project MESSAGE: version:(\S+)", output).group(1)
            self._qtdir = re.search("Project MESSAGE: libraries:(\S+)", output).group(1)
            self._features = re.search("Project MESSAGE: features:(\S+)", output).group(1)
        except:
            self._error = "Unable to parse qmake output (%s)" % output.strip()
            if output.find("QMAKESPEC") != -1:
                self._error += ". Set QMAKESPEC environment variable or pass --qmake-spec <spec>."
        return None

    def _detect_make(self):
        if self.name() == "win32" and Platform._test_make("nmake", "/?"):
            return "nmake"
        if Platform._test_make("make", "-v"):
            return "make"
        if Platform._test_make("gmake", "-v"):
            return "gmake"
        if self.name() == "win32" and Platform._test_make("mingw32-make", "-v"):
            return "mingw32-make"
        return "(n)make"

    def _test_make(command, param):
        try:
            return run_process([command, param])[0] == 0
        except:
            return False

    _test_make = staticmethod(_test_make)

# ============================================================================
# ConfigTest
# ============================================================================
class ConfigTest:
    def __init__(self, platform):
        self._make = platform.make()
        self._qmake = platform.qmake()
        self._platform = platform.name()
        self._spec = platform.spec()

    def setup(self, sourcedir, builddir):
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
            args = [self._qmake, filepath]
            if self._spec:
                args += ["-spec", self._spec]
            run_process(args)
            (code, output) = run_process([self._make])

            # make return value is not reliable
            if self._platform == "symbian":
                # on symbian, check that no error patterns such as '***' can be found from build output
                patterns = ["\\*\\*\\*", "Errors caused tool to abort"]
                for pattern in patterns:
                    if re.search(pattern, output) != None:
                        code = -1
            else:
                # on other platforms, check that the resulting executable exists
                executable = os.path.join(builddir, "hbconftest_" + basename)
                if os.name == "nt":
                    executable.append(".exe")
                if not os.path.exists(executable) or not os.access(executable, os.X_OK):
                    code = -1

            # clean
            run_process([self._make, "clean"])

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
    global HB_MAKE_PARTS, HB_NOMAKE_PARTS

    qmake = "qmake"
    cmdline = " ".join(sys.argv[1:])
    match = re.search("--qmake-bin[=\s](\S+)", cmdline)
    if match:
        qmake = match.group(1)

    # detect platform
    platform = Platform(qmake)
    match = re.search("--platform[=\s](\S+)", cmdline)
    if match:
        platform._platform = match.group(1)

    match = re.search("--qmake-spec[=\s](\S+)", cmdline)
    if match:
        platform._spec = match.group(1)

    help = False
    match = re.search("--help\s*", cmdline)
    if match or re.search("-h\s*", cmdline):
        help = True

    if not help and not platform.name():
        print("ERROR: %s" % platform.error())
        return

    # detect make
    match = re.search("--make-bin[=\s](\S+)", cmdline)
    if match:
        platform._make = match.group(1)
    if not platform.make():
        print("ERROR: %s" % platform.error())
        return

    currentdir = os.path.abspath(os.getcwd())
    sourcedir = os.path.abspath(sys.path[0])

    # default prefixes
    prefixes = { "symbian" : "$${EPOCROOT}epoc32",
                 "unix"    : "/usr/local/hb",
                 "macx"    : "/usr/local/hb",
                 "win32"   : "C:/hb" }

    # parse command line options
    parser = OptionParser(platform.name(), platform.make(), prefixes.get(platform.name(), currentdir))
    (options, args) = parser.parse_args()

    # coverage implies developer mode
    if options.coverage:
        options.developer = True

    print("Configuring Hb...")
    print("INFO: Platform: %s" % platform.name())
    print("INFO: Make: %s" % platform.make())
    print("INFO: Qt: %s in %s" % (platform.version(), platform.qtdir()))

    # warn about deprecated options
    if options.qtanimation != None:
        print("WARNING: --qt-animation and --qt-no-animation are DEPRECATED. Qt 4.6 includes the animation framework.")
    if options.qtgestures != None:
        print("WARNING: --qt-gestures and --qt-no-gestures are DEPRECATED. Qt 4.6 includes the gestures framework.")
    if options.qts60eventfilter != None:
        print("WARNING: --qt-symbian-eventfilter and --qt-s60-eventfilter are DEPRECATED. Qt 4.6 includes QApplication::symbianEventFilter().")
    if options.inputs != None:
        print("WARNING: --no-inputs is DEPRECATED. Use --nomake hbinput.")
        add_remove_part("hbinput", options.inputs)
    if options.feedback != None:
        print("WARNING: --no-feedback is DEPRECATED. Use --nomake hbfeedback.")
        add_remove_part("hbfeedback", options.feedback)
    if options.tutorials != None:
        print("WARNING: --no-tutorials is DEPRECATED. Use --nomake tutorials.")
        add_remove_part("tutorials", options.tutorials)
    if options.tests != None:
        print("WARNING: --tests is DEPRECATED. Use --make tests.")
        add_remove_part("tests", options.tests)
    if options.performance != None:
        print("WARNING: --performance is DEPRECATED. Use --make performance.")
        add_remove_part("performance", options.performance)
    if options.localization != None:
        print("WARNING: --localization is DEPRECATED. Use --make localization.")
        add_remove_part("localization", options.localization)

    # sort out directories
    if not options.prefix:
        # developer mode implies local setup
        if options.developer:
            options.prefix = currentdir
        else:
            options.prefix = prefixes.get(platform.name(), currentdir)
    basedir = options.prefix
    if platform.name() != "symbian":
        basedir = os.path.abspath(basedir)
    local = os.path.isdir(basedir) and (basedir == currentdir)

    # compilation tests to detect available features
    config = ConfigFile()
    test = ConfigTest(platform)
    test.setup(sourcedir, currentdir)
    print("\nDetecting available features...")
    if options.qtmobility == None:
        options.qtmobility = test.compile("config.tests/all/mobility")
    if options.qtmobility:
        config.add_value("DEFINES", "HB_HAVE_QT_MOBILITY")
    print("INFO: Qt Mobility:\t\t\t%s" % options.qtmobility)
    if platform.name() == "symbian":
        sgimagelite_result = test.compile("config.tests/symbian/sgimagelite")
        if sgimagelite_result:
            config.add_value("CONFIG", "sgimage")
        print("INFO: SgImage-Lite:\t\t\t%s" % sgimagelite_result)
    if options.dui == None:
        options.dui = test.compile("config.tests/maemo/dui")
    if options.dui:
        config.add_value("CONFIG", "hb_maemo_dui")
        config.add_value("DEFINES", "HB_MAEMO_DUI")
    print("INFO: Direct UI:\t\t\t%s" % options.dui)

    # directories
    if options.bindir == None:
        # TODO: symbian
        options.bindir = basedir + "/bin"
    if options.libdir == None:
        # TODO: symbian
        options.libdir = basedir + "/lib"
    if options.docdir == None:
        # TODO: symbian
        options.docdir = basedir + "/doc"
    if options.includedir == None:
        if platform.name() == "symbian" and not options.developer:
            if os.path.isdir("/s60"):
                options.includedir = basedir + "/include/hb"
            else:
                options.includedir = basedir + "/include/mw/hb"
        else:
            options.includedir = basedir + "/include"
    if options.plugindir == None:
        if platform.name() == "symbian":
            # TODO: fix to "$${EPOCROOT}resource/hb/plugins"
            options.plugindir = "$${EPOCROOT}resource/qt/plugins/hb"
        else:
            options.plugindir = basedir + "/plugins"
    if options.featuredir == None:
        options.featuredir = platform.features()
    if options.resourcedir == None:
        # TODO: fix this, some components want to write resources...
        #       thus, cannot point to the source tree!
        if not local:
            options.resourcedir = basedir + "/resources"
        else:
            options.resourcedir = sourcedir + "/src/hbcore/resources"

    config.set_value("HB_INSTALL_DIR", ConfigFile.format_dir(basedir))
    config.set_value("HB_BIN_DIR", ConfigFile.format_dir(options.bindir))
    config.set_value("HB_LIB_DIR", ConfigFile.format_dir(options.libdir))
    config.set_value("HB_DOC_DIR", ConfigFile.format_dir(options.docdir))
    config.set_value("HB_INCLUDE_DIR", ConfigFile.format_dir(options.includedir))
    config.set_value("HB_PLUGINS_DIR", ConfigFile.format_dir(options.plugindir))
    config.set_value("HB_RESOURCES_DIR", ConfigFile.format_dir(options.resourcedir))
    config.set_value("HB_FEATURES_DIR", ConfigFile.format_dir(options.featuredir))

    # TODO: get rid of this!
    if platform.name() == "symbian":
        config.set_value("HB_PLUGINS_EXPORT_DIR", ConfigFile.format_dir("$${EPOCROOT}epoc32/winscw/c/resource/qt/plugins/hb"))

    if options.gestures:
        config.add_value("DEFINES", "HB_GESTURE_FW")
    if options.effects:
        config.add_value("DEFINES", "HB_EFFECTS")
    if options.textMeasurement:
        config.add_value("DEFINES", "HB_TEXT_MEASUREMENT_UTILITY")
	if platform.name() != "symbian" and options.developer:
		config.add_value("DEFINES", "HB_CSS_INSPECTOR")
    if options.defines:
        config.add_value("DEFINES", " ".join(options.defines.split(",")))
    if options.developer:
        config.add_value("DEFINES", "HB_DEVELOPER")

    if options.verbose:
        print("INFO: Writing hb_install.prf")
    if not config.write("hb_install.prf"):
        print("ERROR: Unable to write hb_install_prf.")
        return

    config.set_value("HB_BUILD_DIR", ConfigFile.format_dir(currentdir))
    config.set_value("HB_SOURCE_DIR", ConfigFile.format_dir(sourcedir))
    config.set_value("HB_MKSPECS_DIR", ConfigFile.format_dir(basedir + "/mkspecs"))

    if platform.name() == "symbian":
        if os.path.isdir("/s60"):
            config.set_value("HB_EXPORT_DIR", "hb/%1/%2")
            config.set_value("HB_PRIVATE_EXPORT_DIR", "hb/%1/private/%2")
        else:
            config.set_value("HB_EXPORT_DIR", "$${EPOCROOT}epoc32/include/mw/hb/%1/%2")
            config.set_value("HB_PRIVATE_EXPORT_DIR", "$${EPOCROOT}epoc32/include/mw/hb/%1/private/%2")

    if options.developer:
        add_remove_part("tests", True)
        add_remove_part("performance", True)
        add_remove_part("localization", True)

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

    if local:
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
    config._lines.append("include(%s)\n" % (os.path.splitdrive(sourcedir)[1] + "/mkspecs/hb_functions.prf"))

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
    os.system("python %s/%s -i %s -o %s" % (sourcedir, synchb, sourcedir, currentdir))

    # generate a qrc for resources
    print("INFO: Qt resource collection")
    args = [os.path.join(sourcedir, "bin/resourcifier.py")]
    args += ["-i", "%s" % os.path.join(sys.path[0], "src/hbcore/resources")]
    # TODO: make it currentdir
    args += ["-o", "%s" % os.path.join(sourcedir, "src/hbcore/resources/resources.qrc")]
    args += ["--exclude", "\"*distribution.policy.s60\""]
    args += ["--exclude", "\"*readme.txt\""]
    args += ["--exclude", "\"*.pr?\""]
    args += ["--exclude", "\"*.qrc\""]
    args += ["--exclude", "\"*~\""]
    args += ["--exclude", "variant/*"]
    if options.verbose:
        print("INFO: Running %s" % " ".join(args))
    os.system("python %s" % " ".join(args))

    # build host tools
    if platform.name() == "symbian" or options.hostqmakebin != None or options.hostmakebin != None:
        print("\nBuilding host tools...")
        if options.hostqmakebin != None and options.hostmakebin != None:
            profile = "%s/src/hbtools/hbtools.pro" % sourcedir
            if os.path.exists(profile):
                toolsdir = os.path.join(currentdir, "src/hbtools")
                if not os.path.exists(toolsdir):
                    os.makedirs(toolsdir)
                os.chdir(toolsdir)
                os.system("\"%s\" -config silent %s" % (options.hostqmakebin, profile))
                os.system("\"%s\"" % (options.hostmakebin))
                os.chdir(currentdir)
        else:
            print("WARNING: Cannot build host tools, because no --host-qmake-bin and/or")
            print("         --host-make-bin was provided. Hb will attempt to run host")
            print("         tools from PATH.")

    # run qmake
    if options.qmakebin:
        qmake = options.qmakebin
    
    # modify epocroot for symbian to have compatibility between qmake and raptor
    epocroot = os.environ.get("EPOCROOT")
    replace_epocroot = epocroot
    if epocroot:
        if epocroot.endswith("\\") or epocroot.endswith("/"):
            replace_epocroot = epocroot
        else:
            replace_epocroot = "%s/" % epocroot
    
    profile = os.path.join(sourcedir, "hb.pro")
    cachefile = os.path.join(currentdir, ".qmake.cache")
    if options.msvc:
        qmake = "%s -tp vc" % qmake
    if not options.fast:
        qmake = "%s -r" % qmake
    if options.qmakespec:
        qmake = "%s -spec %s" % (qmake, options.qmakespec)
    if options.qmakeopt:
        qmake = "%s \\\"%s\\\"" % (qmake, options.qmakeopt)
    if options.verbose:
        print("\nRunning %s -cache %s %s" % (qmake, cachefile, profile))
    else:
        print("\nRunning qmake...")
    try:
        # replace the epocroot for the qmake runtime
        if replace_epocroot:
            os.putenv("EPOCROOT", replace_epocroot)
        ret = os.system("%s -cache %s %s" % (qmake, cachefile, profile))
        if replace_epocroot:
            os.putenv("EPOCROOT", epocroot)
    except KeyboardInterrupt:
        ret = -1
    if ret != 0:
        print("")
        print("ERROR: Aborted!")
        print("")
        return

    if "tests" not in HB_NOMAKE_PARTS:
        # run qmake for tests
        profile = "%s/tsrc/tsrc.pro" % sourcedir
        if os.path.exists(profile):
            tsrcdir = os.path.join(currentdir, "tsrc")
            if not os.path.exists(tsrcdir):
                os.makedirs(tsrcdir)
            os.chdir(tsrcdir)
            if options.verbose:
                print("\nRunning %s %s" % (qmake, profile))
            else:
                print("\nRunning qmake in tsrc...")
            os.system("%s %s" % (qmake, profile))
            os.chdir(currentdir)

            # create output dirs
            outputdir = os.path.join(currentdir, "autotest")
            if not os.path.exists(outputdir):
                os.makedirs(outputdir)
            outputdir = os.path.join(currentdir, "coverage")
            if not os.path.exists(outputdir):
                os.makedirs(outputdir)
        # nag about tests that are commented out
        result = grep(sourcedir + "/tsrc", "#\s*SUBDIRS\s*\+=\s*(\S+)", ["*.pr?"])
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
    if platform.make() == "nmake" and options.msvc:
        conf = "MSVC"
        act = "open 'hb.sln'"
    elif options.coverage:
        conf = "test coverage measurement"
        act = "run '%s coverage'" % platform.make()
    elif options.developer:
        conf = "development"
        act = "run '%s'" % platform.make()
    else:
        conf = "building"
        act = "run '%s'" % platform.make()
    print("Hb is now configured for %s. Just %s." % (conf, act))

    if not options.coverage:
        if platform.name() == "symbian" or local:
            print("You must run '%s install' to copy the .prf file in place." % platform.make())
        else:
            print("Once everything is built, you must run '%s install'." % platform.make())
        if platform != "symbian":
            if local:
                print("Hb will be used from '%s'." % sourcedir)
            else:
                print("Hb will be installed to '%s'." % basedir)
    if platform.name() == "win32":
        path = os.path.join(basedir, "bin")
        if local:
            path = os.path.join(currentdir, "bin")
        print("NOTE: Make sure that '%s' is in PATH." % path)
        if options.coverage:
            print("Test code coverage measurement will FAIL if wrong Hb DLLs are found in PATH before '%s'." % path)

    print("")
    print("To reconfigure, run '%s clean' and '%s'." % (platform.make(), sys.argv[0]))
    print("")

if __name__ == "__main__":
    main()
