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

#include <hbglobal.h>
#include "hbglobal_p.h"
#ifdef HB_TEXT_MEASUREMENT_UTILITY
#include "hbtextmeasurementutility_r.h"
#endif
#include <QString>
#include <QCoreApplication>

/*!
    @stable
    @hbcore
    \file hbglobal.h
    \brief The HbGlobal header file collects global definitions and functions for the %Hb framework.
*/

/*!
    \def HB_VERSION

    This macro expands a numeric value of the form 0xMMNNPP (MM = major, NN =
    minor, PP = patch) that specifies Hb's version number. For example, if you
    compile your application against Hb 1.2.3, the \c HB_VERSION macro will
    expand to 0x010203.

    You can use \c HB_VERSION to use the latest Hb features where available.

    For example:

    \code
    #if HB_VERSION >= 0x000500
        // call APIs that were added in Hb 0.5.0
    #endif
    \endcode

    \sa hbVersion(), HB_VERSION_STR
*/

/*!
    \def HB_VERSION_STR

    This macro expands to a string that specifies Hb's version number and build
    tag (for example, "1.2.3-dev"). This is the version against which the
    application is compiled.

    For example:

    \code
    qDebug() << "Application built with Hb" << HB_VERSION_STR;
    \endcode

    \sa hbVersionString(), HB_VERSION
*/

/*!
    Returns Hb's version number at runtime (for example, 0x010203). This may be
    a different version from the version the application was compiled against.

    \sa HB_VERSION, hbVersionString()
*/
uint hbVersion()
{
    return HB_VERSION;
}

/*!
    Returns Hb's version number and build tag at runtime as a string (for
    example, "1.2.3-dev"). This may be a different version from the version the
    application was compiled against.

    For example:

    \code
    qDebug() << "Application running with Hb" << hbVersionString();
    \endcode

    \sa HB_VERSION_STR, hbVersion()
*/
const char *hbVersionString()
{
    return HB_VERSION_STR;
}

/*!
    Returns the translated text for a given unique text ID if the operation was
    successful, otherwise the text ID. For example, giving the text ID \c
    txt_open_connection could return the translation \c "Open connection?" from
    the English translations.

    \param id The text ID that identifies the translation in the <tt>.ts</tt>
    file.

    \param n A numeric value that indicates quantity, used to determine which
    singular or plural translation to use from the <tt>.ts</tt> file.

    All translations and their text IDs are defined in language-specific
    <tt>.ts</tt> localization files. You use the <tt>.ts</tt> files to generate
    binary <tt>.qm</tt> files, from which the application gets the translations
    at runtime. Use HbTranslator to load the correct <tt>.qm</tt> file for the
    language active in the device.

    You can use QString::arg() with hbTrId() to insert values based on the given
    arguments. You can use up to nine arguments to insert values, as supported
    by QString.

    \note Only use the \a n parameter with integer arguments that indicate
    quantity, that is, when plural handling is needed. With non-plural arguments,
    use QString::arg().

    The text strings in <tt>.ts</tt> files can have placeholders, where you
    insert values at runtime. hbTrId() supports the following placeholder
    formats:

    <ul>

    <li>\c \%x for inserting strings, where \c x is a number (1-9) that
    indicates the order of placeholders in the source text string. This
    corresponds to your argument inserting order when you use hbTrId(). The
    order of placeholders may vary in different translations, and the numbering
    ensures that values are inserted into the correct placeholders.</li>

    <li>\c \%Lx for inserting numbers, where \c x indicates the argument
    inserting order. \c L converts the number to the active locale's digit type,
    and formats it to the locale's number format.</li>

    <li>\c \%Ln for inserting integers that indicate quantity, where \c n
    supports displaying the appropriate singular or plural translation from the
    <tt>.ts</tt> file. This is because many languages use more than one plural
    form, depending on the quantity that is being referred to. Only one \c \%Ln
    placeholder is allowed per text string.

    You can optionally use \c L to convert the number to the active locale's
    digit type, and format it to the locale's number format. Otherwise just use
    \c \%n.</li>

    </ul>

    When numbering the placeholders, always start from 1, and continue in order.
    You can use a mixture of different placeholder types in one string, as long
    as the numbering runs correctly (for example, \c \%1, \c \%Ln, \c \%L2).

    For example:
    <ul>

    <li>The (source) text string \c "Enter %L1-digit passcode for %2" has two
    placeholders. The first one inserts a number formatted for the active
    locale, and the second one inserts a string.</li>

    <li>The (source) text string \c "Open %Ln file(s)" has a placeholder for
    displaying the quantity formatted for the active locale. \c n enables
    displaying the correct singular or plural translation for the active
    language, for example, \c "Open 1 file" or \c "Open 2 files."</li>

    </ul>

    \note The number of characters in a text string can vary widely from
    language to language. Sometimes when space is limited, you may want to limit
    the number of characters that are inserted. To do this, use the
    <tt>%[y]x</tt> placeholder in the text string, and handle the translation
    and arguments using the HbParameterLengthLimiter class instead of %hbTrId().
    Even if the inserted value does not require limiting at the time of coding,
    there is one advantage in using %HbParameterLengthLimiter right away: if you
    later need to limit the value length for some languages, no code changes are
    required. You only need to modify the <tt>.ts</tt> translation files for
    those languages. However, if you already know that all translations will fit
    without any limitations (for example, if you are using scrolling text with
    HbMarqueeItem), use hbTrId().

    hbTrId() is a wrapper for qtTrId() in QtGlobal. %hbTrId() provides
    additional support for the layout metrics process in localization, which is
    used to ensure that all translations fit in the given space (see
    HbTextMeasurementUtility).

    If you need to localize static folder names, use the
    HbDirectoryNameLocalizer class.

    \section _usecases_hbglobal Using the hbTrId() function

    \subsection _uc_001_hbglobal Getting the translation for a text string

    This example shows how you get the translation for a given text ID.

    \code
    // "Open connection?"
    QString text = hbTrId("txt_open_connection");
    \endcode

    \subsection _uc_002_hbglobal Inserting values using runtime arguments

    This example shows how you get the translation, and replace the placeholder
    with a value using a runtime argument.

    \code
    // "Enter %L1-digit passcode"
    // int number = 4
    label->setPlainText(hbTrId("txt_give_numeric_passcode").arg(number));
    \endcode

    This example also uses the 'L' notation in the placeholder in the text
    string, which formats the number in the active locale's display format.

    \subsection _uc_003_hbglobal Supporting singular and plural translations

    This example shows how you get the translation and replace the placeholder
    with a runtime value (an integer that indicates quantity). The example uses
    the \a n parameter in order to display a singular or plural translation that
    corresponds to the quantity.

    \code
    // int number_of_files
    // "Open %Ln file(s)"
    label->setPlainText(hbTrId("text_filebrowser_open_files", number_of_files));
    \endcode

    The singular and plural translations are defined in the <tt>.ts</tt> file as
    follows:

    \code
    <translation>
      <numerusform>Open %Ln file</numerusform>
      <numerusform>Open %Ln files</numerusform>
    </translation>
    \endcode

    %Qt provides a \c numerus.cpp file (located in \c
    sf\\mw\\qt\\tools\\linguist\\shared\\), which defines the singular and
    plural forms used in each of the supported languages. This information is
    used to automatically pick the correct translation from the <tt>.ts</tt>
    file for the number of objects that the argument refers to. In this example,
    this is the number of files to be displayed in the text string. The
    translations (in \c numerusform tags) must be in correct order in the
    <tt>.ts</tt> file, matching the order specified in \c numerus.cpp.

    \section _troubleshooting_hbglobal Troubleshooting

    If hbTrId() returns the text ID instead of the translation, make sure you
    have used HbTranslator correctly to load the translations.

    If hbTrId() returns a translation that has the placeholder instead of
    inserted value, check if the text string has a [] notation in the
    <tt>.ts</tt> file of that language. If this notation is used, you need to
    use the HbParameterLengthLimiter class instead of %hbTrId().

    \sa QString::arg(), HbParameterLengthLimiter, HbDirectoryNameLocalizer, HbTranslator, QCoreApplication::translate(), HbTextMeasurementUtility
*/
QString hbTrId(const char *id, int n)
{
    QString loc = qtTrId(id, n);
#ifdef HB_TEXT_MEASUREMENT_UTILITY
    if (HbTextMeasurementUtility::instance()->locTestMode()) {
        loc.append(QChar(LOC_TEST_START));
        loc.append(id);
        loc.append(QChar(LOC_TEST_END));
    }
#endif
    return loc;
}

bool HbRecallFlag::flag = true;

