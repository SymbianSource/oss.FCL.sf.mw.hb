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

#include "hbstyleoptionslider.h"

/*!
    \class HbStyleOptionSlider
    \brief HbStyleOptionSlider has the style component for slider primitives
 */

HbStyleOptionSlider::HbStyleOptionSlider() :
    maximum(0),
    minimum(0),
    orientation(Qt::Horizontal),
    pageStep(0),
    singleStep(0),
    sliderPosition(0),
    sliderValue(0),
    tickInterval(0),
    tickPosition(0),
    upsideDown(false),
    minorTickInterval(-1),
    activeSubControls(0),
    subControls(0),
    groovePressed(false),
    sliderElementIcon(),
    text(QString()),
    sliderType(CustomType),
    handleRect(QRectF()),
    span( 0 )
{
    type = Type;
    version = Version;
}

HbStyleOptionSlider::HbStyleOptionSlider(const HbStyleOptionSlider &other) :
    HbStyleOption(other),
    maximum(other.maximum),
    minimum(other.minimum),
    orientation(other.orientation),
    pageStep(other.pageStep),
    singleStep(other.singleStep),
    sliderPosition(other.sliderPosition),
    sliderValue(other.sliderValue),
    tickInterval(other.tickInterval),
    tickPosition(other.tickPosition),
    upsideDown(other.upsideDown),
    minorTickInterval(other.minorTickInterval),
    activeSubControls(other.activeSubControls),
    subControls(other.subControls),
    groovePressed(other.groovePressed),
    layoutOption(other.layoutOption),
    sliderElementIcon(other.sliderElementIcon),
    text(other.text),
    sliderType(other.sliderType),
    handleRect(other.handleRect),
    span( other.span )
{
    type = Type;
    version = Version;
}

HbStyleOptionSlider::~HbStyleOptionSlider()
{
}
