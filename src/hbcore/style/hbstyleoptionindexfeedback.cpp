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

#include <hbstyleoptionindexfeedback.h>

/*!
    \class HbStyleOptionIndexFeedback
    \brief HbStyleOptionIndexFeedback hsa the style components for index feedback primatives.
*/

/*!
    \var HbStyleOptionIndexFeedback::text
    Text shown in the indexFeedbackPopup, if not NULL.
*/

/*!
    \var HbStyleOptionIndexFeedback::fontSpec
    The font specification for text used in the index feedback.
*/

/*!
    \var HbStyleOptionIndexFeedback::textRect
    \brief The bounding rect for the text when displaying the index feedback.
*/

/*!
    \var HbStyleOptionIndexFeedback::popupRect
    \brief The bounding rect for the popup background when displaying index feedback.
*/



/*!

    \deprecated HbStyleOptionIndexFeedback::HbStyleOptionIndexFeedback()
    is deprecated. Styleoptions will not be public.

*/
HbStyleOptionIndexFeedback::HbStyleOptionIndexFeedback() : 
    fontSpec(HbFontSpec::Primary)
{
    type = Type;
    version = Version;
}

/*!

    \deprecated HbStyleOptionIndexFeedback::HbStyleOptionIndexFeedback(const HbStyleOptionIndexFeedback&)
    is deprecated. Styleoptions will not be public.

*/
HbStyleOptionIndexFeedback::HbStyleOptionIndexFeedback(const HbStyleOptionIndexFeedback &other) :
    HbStyleOption(other),
    text(other.text),
    fontSpec(other.fontSpec),
    textRect(other.textRect),
    popupRect(other.popupRect)
{
    type = other.type;
    version = other.version;
}

HbStyleOptionIndexFeedback::~HbStyleOptionIndexFeedback()
{
}
