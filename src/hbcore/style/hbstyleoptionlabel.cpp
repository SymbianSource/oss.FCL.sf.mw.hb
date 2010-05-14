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

#include <hbstyleoptionlabel_p.h>

/*!
    \class HbStyleOptionLabel
    \brief HbStyleOptionLabel has the style component for label primitives
*/

/*!
    \var HbStyleOptionLabel::text
    Text shown in the label, if not NULL. 
*/

/*!
    \var HbStyleOptionLabel::alignment
    Alignment of text or icon in the bounding rectangular 
*/

/*!
    \var HbStyleOptionLabel::elideMode
    Elide mode of the text.
*/

/*!
    \var HbStyleOptionLabel::icon
    Icon shown in the label, if not NULL. 
*/

/*!
    \var HbStyleOptionLabel::aspectRatioMode
    Aspect ratio mode of icon. 
*/


/*!

    \deprecated   HbStyleOptionLabel::HbStyleOptionLabel()
    is deprecated. Styleoptions will not be public.

*/
HbStyleOptionLabel::HbStyleOptionLabel() : HbStyleOption(HbSO_Label),
    text(),
    fontSpec(), /*HbFontSpec::Primary*/
    elideMode(Qt::ElideRight),
    textWrapMode(Hb::TextNoWrap),
    icon(HbIcon()),
    aspectRatioMode(Qt::KeepAspectRatio),
    alignment(Qt::AlignLeft | Qt::AlignVCenter)
{
    type = Type;
    version = Version;
}


/*!

    \deprecated HbStyleOptionLabel::HbStyleOptionLabel(const HbStyleOptionLabel&) 
    is deprecated. Styleoptions will not be public.

*/
HbStyleOptionLabel::HbStyleOptionLabel(const HbStyleOptionLabel &other) :
    HbStyleOption(other),
    text(other.text),
    fontSpec(other.fontSpec),
    elideMode(other.elideMode),
    textWrapMode(other.textWrapMode),
    color(other.color),
    icon(other.icon),
    aspectRatioMode(other.aspectRatioMode),
    alignment(other.alignment)
{
    type = other.type;
    version = other.version;
}

HbStyleOptionLabel::~HbStyleOptionLabel()
{
}
