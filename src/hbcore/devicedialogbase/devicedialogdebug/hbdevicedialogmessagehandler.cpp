/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbCore module of the UI Extensions for Mobile.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at developer.feedback@nokia.com.
**
****************************************************************************/

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include "hbdevicedialogmessagehandler_p.h"

QFile debugFile;

/*
    \internal
    \class HbDeviceDialogMessageHandler
    \brief HbDeviceDialogMessageHandler enables qDebug() output to be redirected a file.

    HbDeviceDialogMessageHandler is a static utility class for redirecting qDebug() output to
    a specific file.

    qDebug() output is redirected to a text file by calling installMessageHandler() with the path
    of the file as a parameter. This method installs a custom Qt message handler that writes all
    messages of type QtDebugMsg to the file that was opened when installMessageHandler() was
    called. This handler has an effect only for messages output by calling qDebug(). If the
    message type is anything other than QtDebugMsg, the message is ignored.

    Each message is prefixed with a time stamp and forms a line of its own in the output file. The
    time format used is "yyyy-MM-dd hh:mm:ss.zzz". There is no limit to the length of the message,
    but obviously the messages should be kept as concise and to the point as possible to avoid
    excessive file operations.

    For example calling

    \code
    qDebug("This is a debug message.");
    \endcode

    generates an output line of the following format:

    \code
    2009-03-24 08:44:30.479 : This is a debug message.
    \endcode

    \note Even though HbDeviceDialogMessageHandler can be used as is, it is intended to be used
    through macros such as INSTALL_MESSAGE_HANDLER, UNINSTALL_MESSAGE_HANDLER and TRACE, all of
    which are declared in the header hbdevicedialogtrace_p.h. Use of these macros is strongly
    encouraged, as it ensures that the output is uniform, and method names and instance pointer
    values are automatically printed for all debug messages.

    \sa installMessageHandler(), uninstallMessageHandler(), QtMsgHandler, qDebug(),
    hbdevicedialogtrace_p.h.
*/

/*
    \brief Opens the debug output file and installs a custom message handler function.

    This method should be called before any qDebug() calls have been made, preferebly as the first
    thing in the main() function of an application. Failing to do so causes some of the debug
    messages to be sent to the standard output instead.

    Since the supplied path is used to open a QFile instance, '/' must be used as the directory
    separator in \a fileName, regardless of the separator conventions of the current operating
    system. Whether or not the path is case sensitive is, on the other hand, operating system
    specific.

    If \a requireFile is true the output file has to exists, or no output is generated. If the file
    does not exists, the standard message handler is left in place and no file redirection takes
    place.

    By setting \a requireFile to true the debug output can be conveniently enabled by simply
    creating the output file and disabled by deleting it, renaming it or making it read-only.
    This way the generation of debug output can be controlled on the target device without having
    to recompile the source code. The performance impact for applications with debug output
    temporarily disabled (i.e. whose output file is not writable) is minimized, since the file
    check is performed only once when installMessageHandler() is called.

    By default \a requireFile is false, meaning the output file is always created if it does not
    exist. Any directories in the path to the file that do not already exists are created as
    well. For example specifying "C:/Temp/Debug/Log_File.txt" as \a fileName causes either the
    Temp/ or both Temp/ and Debug/ directories to be created if they do not already exist.

    Any errors that may occur (such as process not having write privileges to the directory
    specified in \a outputFile) while attempting to create the directory path and/or output
    file cause the message handler not to be installed. Subsequent qDebug() calls will be
    handled by the default Qt message handler instead.

    \note Since replacing the standard Qt message handler function has an effect on the entire
    process, not just your code, you may see output generated by other components as well. This
    may occur for example if some library you are using also uses qDebug(). These lines are
    relatively easy to distinguish, as they are most probably not using the TRACE_ENTRY, TRACE,
    TRACE_EXIT, etc. macros, the lines will include the timestamp (since this is printed in the
    messageHandler() function, not by the trace macros), but not the class and method name.

    \param fileName The absolute path of the file to use for debug output, e.g.
    "C:/Temp/debug.txt".
    \param requireFile If true then \a fileName has to exist for any output to be generated, if
    false the file is created if does not exist.
    \sa uninstallMessageHandler().
*/
void HbDeviceDialogMessageHandler::installMessageHandler(const QString &fileName, bool requireFile)
{
    if (!debugFile.isOpen()) {
        debugFile.setFileName(fileName);

        if (debugFile.exists()) {
            debugFile.open(QIODevice::Text | QIODevice::Append);
        } else if (!requireFile) {
            const QString filePath(QFileInfo(fileName).absolutePath());
            const QDir dir(filePath);

            if (dir.exists() || dir.mkpath(filePath)) {
                debugFile.open(QIODevice::Text | QIODevice::WriteOnly);
            }
        }

        if (debugFile.isOpen()) {
            qInstallMsgHandler(HbDeviceDialogMessageHandler::messageHandler);
        }
    }
}

/*
    \brief Closes the debug output file and uninstalls the custom message handler function.

    This method should be called to cleanly end debug output generation, typically when the
    application producing the output is exiting.

    If the debug output file was not successfully opened when installMessageHandler() was called
    this method does nothing, since no message handler was installed in the first place.

    \sa installMessageHandler().
*/
void HbDeviceDialogMessageHandler::uninstallMessageHandler()
{
    if (debugFile.isOpen()) {
        qInstallMsgHandler(0);
        debugFile.close();
    }
}

void HbDeviceDialogMessageHandler::messageHandler(QtMsgType type, const char *msg)
{
    QTextStream debugStream(&debugFile);
    QDateTime dateTime(QDateTime::currentDateTime());

    switch (type) {
    case QtDebugMsg:
        debugStream << dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz") << " : " << QString(msg) << endl;
        break;
    case QtWarningMsg:
    case QtCriticalMsg:
    case QtFatalMsg:
    default:
        break;
    }
}
