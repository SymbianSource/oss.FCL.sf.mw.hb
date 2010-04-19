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

#ifndef HBDEVICEDIALOGTRACE_P_H
#define HBDEVICEDIALOGTRACE_P_H

#include <QDebug>   // QDebug
#include <QtGlobal> // qDebug()

#include <hbdevicedialogmessagehandler_p.h>

/*!
    \internal
    \file hbdevicedialogtrace_p.h
    \brief Trace macros for file-based debug output.

    The macros defined in this header enable a project to produce debug output that may be
    conditionally compiled (i.e. disabled completely) or dynamically enabled/disabled based on
    the writability of the output file (i.e. temporarily disabled without mofiying the binary
    that produces the output).

    Whether debug output is compiled in, and to which file it is redirected is controlled by
    defining the macros ENABLE_TRACE_OUTPUT, TRACE_OUTPUT_FILE and TRACE_OUTPUT_FILE_REQUIRED in
    the .pro file of the project using trace output macros such as TRACE_ENTRY, TRACE and
    TRACE_EXIT.

    To enable trace file output add the following lines to your .pro file:

    \code
    include($${HB_SOURCE_DIR}/src/hbcore/devicedialogbase/devicedialogdebug/devicedialogtrace.pri)
    DEFINES += ENABLE_TRACE_OUTPUT
    DEFINES += TRACE_OUTPUT_FILE_REQUIRED
    DEFINES += $$traceOutputFile(tracefile.log)
    \endcode

    The ENABLE_TRACE_OUTPUT macro defines whether trace output is compiled in at all. If this
    macro is not defined all the other trace macros are defined as empty, and no code for trace
    output is generated at all.

    As it is defined here, ENABLE_TRACE_OUTPUT has no relation to whether the current build
    configuration is a release or a debug build. This means that release binaries will also
    produce trace output.

    To enable trace file output for debug builds only, change the second line to

    \code
    CONFIG(debug, debug|release) : DEFINES += ENABLE_TRACE_OUTPUT
    \endcode

    The name of the trace output file is given as a paremeter to the qmake replacement function
    traceOutputFile(). In the example above the name of the output file is "tracefile.log". The
    absolute path where the file is actually written depends on the target platform. On Microsoft
    Windows it is "C:/Temp", on Linux "/usr/tmp", on Symbian "C:/" and on all other platforms the
    current directory of the executable that uses the trace macros, i.e. "./".

    These paths are defined in the qmake project include file devicedialogtrace.pri, and can
    be changed by modifying said include file. Note, however, any changes made to this header
    will affect all projects that use the trace macros.

    The directory paths defined in the function traceOutputFile() upto the file (but not the file
    itself) needs to exist for any output to be generated. The slash character '/' must always be
    used as the directory separator, regardless of the operating system for which the project is
    being compiled. See the class HbDeviceDialogMessageHandler for more details.

    \note Compilers on different operating systems are rather particular about the escaping of
    the double quote characters in the DEFINES statement for the output file. When changing the
    path of the output file be sure not to accidentally remove any of the backslash characters,
    or you will encounter compilation errors when attempting to compile your project with tracing
    enabled.

    TRACE_OUTPUT_FILE_REQUIRED controls whether the output file has to exist for the tracing to
    be enabled or not. Defining TRACE_OUTPUT_FILE_REQUIRED means that if the file specified
    by traceOutputFile() does not exist, no trace file output is generated even if the
    ENABLE_TRACE_OUTPUT macro is defined. This allows the generation of trace output to be
    controlled at runtime without having to recompile the project. Not defining
    TRACE_OUTPUT_FILE_REQUIRED causes the output file to created if it does not exist.
    For both settings, the trace output file is appended to, not overwritten, if it exists
    when the executable producing trace output is restarted.

    See the documentation for the macro INSTALL_MESSAGE_HANDLER for an example how to install
    the message handler, and the macro TRACE for an example how to produce actual trace output.

    \sa INSTALL_MESSAGE_HANDLER, UNINSTALL_MESSAGE_HANDLER, TRACE_ENTRY, TRACE_EXIT, TRACE,
    HbDeviceDialogMessageHandler.
*/

/*!
    \def INSTALL_MESSAGE_HANDLER
    \brief Installs a custom Qt message handler redirecting qDebug() output to a file.

    Installing the message handler should be done as soon as possible during application startup
    (preferably as the first thing in the main() function of the application) to ensure that no
    qDebug() messages are missed.

    INSTALL_MESSAGE_HANDLER causes the output file specified by the TRACE_OUTPUT_FILE macro (as
    defined by calling the traceOutputFile() qmake function in the .pro file of the project) to
    be opened and qDebug() output to be redirected to the file.

    The following example shows how to install the message handler on application startup and how
    to uninstall it before exiting:

    \code
    #include <hbdevicedialogtrace_p.h>

    int main(int argc, char *argv[])
    {
        INSTALL_MESSAGE_HANDLER

        HbApplication app(argc, argv);

        // ...

        int returnValue = app.exec();

        UNINSTALL_MESSAGE_HANDLER

        return returnValue;
    }
    \endcode

    \sa UNINSTALL_MESSAGE_HANDLER.
*/

/*!
    \def UNINSTALL_MESSAGE_HANDLER
    \brief Closes the debug output file and uninstalls the custom Qt message handler.

    UNINSTALL_MESSAGE_HANDLER closes the debug output file that was opened when
    INSTALL_MESSAGE_HANDLER was invoked and should be called as the last thing before exiting the
    application (or unloading the DLL, etc.) once no more qDebug() output is to be expected.
    qDebug() calls occurring after invoking UNINSTALL_MESSAGE_HANDLER are not harmful, they
    merely get handled by the standard Qt message handler.

    If the file could not be opened successfully, UNINSTALL_MESSAGE_HANDLER has no effect since
    no message handler was installed in the first place.

    See INSTALL_MESSAGE_HANDLER for an example how to cleanly close the trace output file before
    exiting an application.

    \sa INSTALL_MESSAGE_HANDLER.
*/

/*!
    \def TRACE_UNUSED(name)
    \brief Declares a single variable as unused when tracing is disabled.

    TRACE_UNUSED allows variables (usually method parameters) to be declared as used only if
    tracing is enabled. Without this variables that are included in trace macros, but not otherwise
    used, would cause unusud variable warnings on compilation. If tracing is enabled, TRACE_UNUSED
    has no effect.

    Consider the following class method where the parameter number is not used at all, except for
    tracing its value on entry:

    \code
    #include <hbdevicedialogtrace_p.h>

    void MyClass::myMethod(int number)
    {
        TRACE_UNUSED(number)
        TRACE_ENTRY("number =" << number)

        // ...some more code where the parameter number is not used.

        TRACE_EXIT
    }
    \endcode

    Compiling this method with tracing completely disabled at compile time (i.e. by specifying
    "DEFINES -= ENABLE_TRACE_OUTPUT" in the .pro file) would cause an unused variable warning to
    be issued unless the TRACE_UNUSED macro is used to declare the variable as trace-only.

    \param name The name of the variable to declare as unused if tracing is disabled. To mark
    several variables as trace-only, add a separate TRACE_UNUSED statement for each.
    \sa TRACE_STATIC_ENTRY_ARGS(args), TRACE_ENTRY_ARGS(args).
*/

/*!
    \def TRACE_STATIC_ENTRY
    \brief A method entry trace macro for static class methods or global functions.

    Invoking TRACE_STATIC_ENTRY outputs a timestamp followed by the scope in which the macro
    was invoked and the word "entry".

    TRACE_STATIC_ENTRY is intended to be used as the first line of static class methods or
    global functions. There is a corresponding exit macro, TRACE_EXIT.

    The following example shows proper usage of the TRACE_STATIC_ENTRY macro. Assuming a class
    has been declared with a static method that is implemented like this:

    \code
    #include <hbdevicedialogtrace_p.h>

    void MyClass::myStaticMethod()
    {
        TRACE_STATIC_ENTRY

        int i = 1;
        i++;

        TRACE_EXIT
    }
    \endcode

    calling MyClass::myStaticMethod() generates output lines of the following format:

    \code
    2009-03-25 11:00:50.171 : static void MyClass::myStaticMethod() entry
    2009-03-25 11:00:50.171 : static void MyClass::myStaticMethod() exit
    \endcode

    \sa TRACE_STATIC_ENTRY_ARGS(args), TRACE_EXIT.
*/

/*!
    \def TRACE_STATIC_ENTRY_ARGS(args)
    \brief A method entry trace macro with arguments for static class methods or global functions.

    TRACE_STATIC_ENTRY_ARGS(args) is similar to TRACE_STATIC_ENTRY but it allows arguments to be
    output on the same line without needing to resort to a separate TRACE call. This is especially
    handy for outputting the parameters of the method call.

    The following example shows proper usage of the TRACE_STATIC_ENTRY_ARGS(args) macro. Assuming
    a class has been declared with a static method that is implemented like this:

    \code
    #include <QString>
    #include <hbdevicedialogtrace_p.h>

    void MyClass::myStaticMethod(const QString &text, int number)
    {
        TRACE_STATIC_ENTRY_ARGS("text =" << text << "number =" << number);

        int i = 1;
        i++;

        TRACE_EXIT
    }
    \endcode

    calling MyClass::myStaticMethod(QString("foo"), 74) generates output lines of the following
    format:

    \code
    2009-03-25 11:00:50.171 : static void MyClass::myStaticMethod(const QString&, int) entry, text = "foo" number = 74
    2009-03-25 11:00:50.171 : static void MyClass::myStaticMethod(const QString&, int) exit
    \endcode

    \param args Any number of arguments that can be streamed into a QTextStream, joined together
    by the streaming operator <<.
    \sa TRACE_STATIC_ENTRY.
*/

/*!
    \def TRACE_ENTRY
    \brief A method entry trace macro for class methods.

    Invoking TRACE_ENTRY outputs a timestamp followed by the scope in which the macro
    was invoked, the word "entry" and the this pointer value of the instance invoking the
    macro.

    The this pointer value included in the debug output can help make the output more readable, as
    it allows different instances of the same class to be distinguished from each other.

    TRACE_ENTRY is intended to be used as the first line of class methods. There is a corresponding
    exit macro, TRACE_EXIT.

    The following example shows proper usage of the TRACE_ENTRY macro. Assuming a class has been
    declared with a non-static method that is implemented like this:

    \code
    #include <hbdevicedialogtrace_p.h>

    void MyClass::myMethod()
    {
        TRACE_ENTRY

        int i = 1;
        i++;

        TRACE_EXIT
    }
    \endcode

    calling myMethod() on an instance of MyClass generates output lines of the following format:

    \code
    2009-03-25 11:00:50.171 : void MyClass::myMethod() this 0x6cdab90 entry
    2009-03-25 11:00:50.171 : void MyClass::myMethod() exit
    \endcode

    \sa TRACE_ENTRY_ARGS(args), TRACE_EXIT.
*/

/*!
    \def TRACE_ENTRY_ARGS(args)
    \brief A method entry trace macro with arguments for class methods.

    TRACE_ENTRY_ARGS(args) is similar to TRACE_ENTRY but it allows arguments to be output on the
    same line without needing to resort to a separate TRACE call. This is especially handy for
    outputting the parameters of the method call.

    The following example shows proper usage of the TRACE_ENTRY_ARGS(args) macro. Assuming a
    class has been declared with a non-static method that is implemented like this:

    \code
    #include <QString>
    #include <hbdevicedialogtrace_p.h>

    void MyClass::myMethod(const QString &text, int number)
    {
        TRACE_ENTRY_ARGS("text =" << text << "number =" << number);

        int i = 1;
        i++;

        TRACE_EXIT
    }
    \endcode

    calling myMethod(QString("foo"), 74) on an instance of MyClass generates output lines of the
    following format:

    \code
    2009-03-25 11:00:50.171 : void MyClass::myMethod(const QString&, int) this 0x6cdab90 entry, text = "foo" number = 74
    2009-03-25 11:00:50.171 : void MyClass::myMethod(const QString&, int) exit
    \endcode

    \param args Any number of arguments that can be streamed into a QTextStream, joined together
    by the streaming operator <<.
    \sa TRACE_ENTRY, TRACE_EXIT.
*/

/*!
    \def TRACE_EXIT
    \brief A method exit trace macro for class methods or global functions.

    Invoking TRACE_EXIT outputs a timestamp followed by the scope in which the macro
    was invoked and the word "exit".

    TRACE_EXIT is intended to be used as the last line of class methods and global functions,
    just before the return statement, if any. There are two corresponding entry macros,
    TRACE_ENTRY and TRACE_STATIC_ENTRY, depending on whether the method being traced is a
    non-static or a static class method. TRACE_EXIT makes no distinction between these two types
    of methods and is to be used for both.

    See TRACE_ENTRY or TRACE_STATIC_ENTRY for an example of how to use TRACE_EXIT.

    \sa TRACE_EXIT_ARGS(args), TRACE_ENTRY, TRACE_STATIC_ENTRY.
*/

/*!
    \def TRACE_EXIT_ARGS(args)
    \brief A method exit trace macro with arguments for class methods or global functions.

    TRACE_EXIT_ARGS(args) is similar to TRACE_EXIT but it allows arguments to be output on the
    same line without needing to resort to a separate TRACE call. This is especially handy for
    outputting the return value of the method call.

    The following example shows proper usage of the TRACE_EXIT_ARGS(args) macro. Assuming a
    class has been declared with a static method that is implemented like this:

    \code
    #include <QString>
    #include <hbdevicedialogtrace_p.h>

    int MyClass::myStaticMethod(const QString &text)
    {
        TRACE_STATIC_ENTRY_ARGS("text =" << text);

        int length = text.length();

        TRACE_EXIT_ARGS("length" << length);

        return length;
    }
    \endcode

    calling MyClass::myStaticMethod(QString("foo")) generates output lines of the following format:

    \code
    2009-03-25 13:20:36.448 : static int MyClass::myStaticMethod(const QString&) entry, text = "foo"
    2009-03-25 13:20:36.448 : static int MyClass::myStaticMethod(const QString&) exit, length 3
    \endcode

    Although the example above is a static method, TRACE_EXIT_ARGS(args) works identically for
    non-static class methods and global functions.

    \param args Any number of arguments that can be streamed into a QTextStream, joined together
    by the streaming operator <<.
    \sa TRACE_EXIT
*/

/*!
    \def TRACE(args)
    \brief A generic trace macro with arguments for class methods or global functions.

    The following example shows how to produce arbitrary debug output:

    \code
    #include <QString>
    #include <hbdevicedialogtrace_p.h>

    void MyClass::myMethod()
    {
        TRACE_ENTRY

        QString myString("This is a string.");
        int myValue = 109;

        TRACE("this is a debug message, myString =" << myString << "myValue =" << myValue)

        TRACE_EXIT
    }
    \endcode

    calling myMethod() on an instance of MyClass generates output lines of the following format:

    \code
    2009-03-25 13:45:22.083 : void MyClass::myMethod() this 0x6cdab90 entry
    2009-03-25 13:45:22.083 : void MyClass::myMethod() this is a debug message, myString = "This is a string." myValue = 109
    2009-03-25 13:45:22.083 : void MyClass::myMethod() exit
    \endcode

    Any number of arguments may be printed by chaining them together with the streaming operator
    <<. Notice that a single space character is automatically added between each streamed
    argument, hence the hardcoded strings in the example above, such as "myValue =", do not have
    a space at the beginning and end of the string. This automatic space addition is a feature
    of qDebug() streaming and cannot be disabled.

    Compiler specific features are used to output the method signature, e.g. "void MyClass::myMethod()"
    in the example output above. Because of this, the exact format and amount of information
    printed for method signatures in TRACE statements varies depending on the capabilities of
    the compiler used.

    Hence the output may be slightly different depending on the compiler used, but typically
    includes at least the name of the function containing the TRACE statement and the class in
    which the function is defined. This applies not just to TRACE, but to all the other trace
    macros that output method signatures as well. Note that this has no effect on how the macros
    themselves should be used - the usage is identical on all platforms, across all compilers,
    even though the output may not be.

    \param args Any number of arguments that can be streamed into a QTextStream, joined together
    by the streaming operator <<.
*/

#if defined(ENABLE_TRACE_OUTPUT) && defined(TRACE_OUTPUT_FILE)
    #if defined(TRACE_OUTPUT_FILE_REQUIRED)
        #define INSTALL_MESSAGE_HANDLER HbDeviceDialogMessageHandler::installMessageHandler(QString(TRACE_OUTPUT_FILE), true);
    #else
        #define INSTALL_MESSAGE_HANDLER HbDeviceDialogMessageHandler::installMessageHandler(QString(TRACE_OUTPUT_FILE), false);
    #endif
    #define UNINSTALL_MESSAGE_HANDLER HbDeviceDialogMessageHandler::uninstallMessageHandler();
    #define TRACE_UNUSED(name)
    #if defined(__GNUC__) // GNU Compiler Collection
        #define TRACE_STATIC_ENTRY qDebug() << __PRETTY_FUNCTION__ << "entry";
        #define TRACE_STATIC_ENTRY_ARGS(args) qDebug() << __PRETTY_FUNCTION__ << "entry," << args;
        #define TRACE_ENTRY qDebug() << __PRETTY_FUNCTION__ << "this" << (void *)this << "entry";
        #define TRACE_ENTRY_ARGS(args) qDebug() << __PRETTY_FUNCTION__ << "this" << (void *)this << "entry," << args;
        #define TRACE_EXIT qDebug() << __PRETTY_FUNCTION__ << "exit";
        #define TRACE_EXIT_ARGS(args) qDebug() << __PRETTY_FUNCTION__ << "exit," << args;
        #define TRACE(args) qDebug() << __PRETTY_FUNCTION__ << args;
    #elif defined(_MSC_VER) // Microsoft Visual C++
        #define TRACE_STATIC_ENTRY qDebug() << __FUNCSIG__ << "entry";
        #define TRACE_STATIC_ENTRY_ARGS(args) qDebug() << __FUNCSIG__ << "entry," << args;
        #define TRACE_ENTRY qDebug() << __FUNCSIG__ << "this" << (void *)this << "entry";
        #define TRACE_ENTRY_ARGS(args) qDebug() << __FUNCSIG__ << "this" << (void *)this << "entry," << args;
        #define TRACE_EXIT qDebug() << __FUNCSIG__ << "exit";
        #define TRACE_EXIT_ARGS(args) qDebug() << __FUNCSIG__ << "exit," << args;
        #define TRACE(args) qDebug() << __FUNCSIG__ << args;
    #else // Standard C99
        #define TRACE_STATIC_ENTRY qDebug() << __FILE__ << "line" << __LINE__ << __func__ << "entry";
        #define TRACE_STATIC_ENTRY_ARGS(args) qDebug() << __FILE__ << "line" << __LINE__ << __func__ << "entry," << args;
        #define TRACE_ENTRY qDebug() << __FILE__ << "line" << __LINE__ << __func__ << "this" << (void *)this << "entry";
        #define TRACE_ENTRY_ARGS(args) qDebug() << __FILE__ << "line" << __LINE__ << __func__ << "this" << (void *)this << "entry," << args;
        #define TRACE_EXIT qDebug() << __FILE__ << "line" << __LINE__ << __func__ << "exit";
        #define TRACE_EXIT_ARGS(args) qDebug() << __FILE__ << "line" << __LINE__ << __func__ << "exit," << args;
        #define TRACE(args) qDebug() << __FILE__ << "line" << __LINE__ << __func__ << args;
    #endif
#else
    #define INSTALL_MESSAGE_HANDLER
    #define UNINSTALL_MESSAGE_HANDLER
    #define TRACE_UNUSED(name) Q_UNUSED(name)
    #define TRACE_STATIC_ENTRY
    #define TRACE_STATIC_ENTRY_ARGS(args)
    #define TRACE_ENTRY
    #define TRACE_ENTRY_ARGS(args)
    #define TRACE_EXIT
    #define TRACE_EXIT_ARGS(args)
    #define TRACE(args)
#endif // ENABLE_TRACE_OUTPUT

#endif // HBDEVICEDIALOGTRACE_P_H
