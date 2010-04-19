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

coverage:!symbian {

    HB_COVERAGE_DIR = $${HB_BUILD_DIR}/coverage
    unix:HB_COVERAGE_EXCLUDE += \"/usr/*\" \"*/3rdparty/*\" \"*.moc\" \"moc_*.cpp\" \"qrc_*.cpp\"
    win32:HB_COVERAGE_EXCLUDE += "EXCLUDE+*/3rdparty/*,*/moc/*,*/rcc/*,*/unit/*"

    root {

        unix {
            pre_coverage.recurse = src
            measure_coverage.recurse = src
            post_coverage.recurse = src

            post_coverage.commands = @[ ! -e $${HB_COVERAGE_DIR}/*.cov ]
            post_coverage.commands += || ( rm -f $${HB_COVERAGE_DIR}/Hb.cov
            post_coverage.commands += && cat $${HB_COVERAGE_DIR}/*.cov > $${HB_COVERAGE_DIR}/Hb.cov
            post_coverage.commands += && genhtml --output-directory $${HB_COVERAGE_DIR}/html $${HB_COVERAGE_DIR}/Hb.cov
            # would open a browser automatically if wanted
            # post_coverage.commands += && xdg-open $${HB_BUILD_DIR}/coverage/html/index.html
            post_coverage.commands += && echo \"\nTest code coverage results stored to: $${HB_COVERAGE_DIR}\n\" )
        }

        win32 {
            pre_coverage.commands = $(DEL_FILE) $$hbNativePath($${HB_COVERAGE_DIR}/MON.dat) > NUL 2>&1

            measure_coverage.commands = ctcwrap -C $${HB_COVERAGE_EXCLUDE} -n $${HB_COVERAGE_DIR}/MON.sym -i m $(MAKE) test

            post_coverage.commands = post_coverage.commands = ctcpost $${HB_COVERAGE_DIR}/MON.sym $${HB_COVERAGE_DIR}/MON.dat -p $${HB_COVERAGE_DIR}/profile.txt $$escape_expand(\n\t)
            post_coverage.commands += ctc2html -i $${HB_COVERAGE_DIR}/profile.txt -o $${HB_COVERAGE_DIR} $$escape_expand(\n\t)
            post_coverage.commands += @echo "Test code coverage results stored to: $${HB_COVERAGE_DIR}"
        }

        measure_coverage.depends = pre_coverage
        unix:measure_coverage.depends = test
        post_coverage.depends = measure_coverage
        coverage.depends = post_coverage
        QMAKE_EXTRA_TARGETS *= coverage pre_coverage measure_coverage post_coverage

    }

    contains(TEMPLATE, .*subdirs$) {

        unix {
            pre_coverage.CONFIG += recursive
            measure_coverage.CONFIG += recursive
            post_coverage.CONFIG += recursive
            QMAKE_EXTRA_TARGETS *= pre_coverage measure_coverage post_coverage
        }

    } else:contains(TEMPLATE, .*lib$)|contains(TEMPLATE, .*app$) {

        unix {
            QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
            QMAKE_LIBS += -lgcov

            pre_coverage.commands = @lcov --directory \$(OBJECTS_DIR) --zerocounters

            measure_coverage.commands = @[ ! -e *.gcda ]
            measure_coverage.commands += || ( mkdir -p $${HB_COVERAGE_DIR}
            measure_coverage.commands += && lcov --directory \$(OBJECTS_DIR) --capture --output-file $${HB_COVERAGE_DIR}/\$(QMAKE_TARGET).cov
            measure_coverage.commands += && lcov --remove $${HB_COVERAGE_DIR}/\$(QMAKE_TARGET).cov $${HB_COVERAGE_EXCLUDE} --output-file $${HB_COVERAGE_DIR}/\$(QMAKE_TARGET).cov )

            QMAKE_EXTRA_TARGETS *= pre_coverage measure_coverage post_coverage
        }
    }
}
