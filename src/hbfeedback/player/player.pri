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

SOURCES  += $$PWD/hbfeedbackplayer.cpp \
            $$PWD/hbfeedbacksettings.cpp \
            $$PWD/hbabstractfeedback.cpp \
            $$PWD/hbinstantfeedback.cpp \
            $$PWD/hbcontinuousfeedback.cpp

PUBLIC_HEADERS +=   $$PWD/hbfeedbacksettings.h \
                    $$PWD/hbabstractfeedback.h \
                    $$PWD/hbinstantfeedback.h \
                    $$PWD/hbcontinuousfeedback.h

PRIVATE_HEADERS += $$PWD/hbfeedbackplayer_p.h \
                   $$PWD/hbfeedbackplayer_p_p.h

symbian {
    SOURCES  += $$PWD/hbfeedbackplayer_symbian.cpp
    PRIVATE_HEADERS  += $$PWD/hbfeedbackplayer_symbian_p.h
} 
else {
    SOURCES += hbfeedbackplayer_stub.cpp
    PRIVATE_HEADERS += hbfeedbackplayer_stub_p.h

    developer|feedback_traces {
        DEFINES += FEEDBACK_TEST_EVENT
        SOURCES +=         $$PWD/hbfeedbacktestevent.cpp
        PRIVATE_HEADERS += $$PWD/hbfeedbacktestevent_p.h
    }
}

