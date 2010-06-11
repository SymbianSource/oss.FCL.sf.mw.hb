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

#include "hbscreenmode_p.h"
#include <QSize>
#include <QSizeF>

class HbScreenModePrivate
{
public:
    HbScreenModePrivate();

public:
    QSize mSize;
    QSizeF mTwipsSize;
    int mRotation; // in degrees. How the device would be moved to see/go into this mode
    QString mStyleName;
};

HbScreenModePrivate::HbScreenModePrivate() :
    mSize(-1, -1),
    mTwipsSize(-1.0F, -1.0F),
    mRotation(0)
{
}

/*!
    \class HbScreenMode
    \brief Information for a single screen including screen pixel size, rotation, physical size

    A single screen mode represents a physical operation mode of a display. Separate modes may be
    distinguished by a rotation, an opening or closing of a cover, or touch enabled or not

    This class should be considered as a abstraction of the underlying operating system's way of reporting
    its information.

*/

// ======== MEMBER FUNCTIONS ========

/*!
    Construct a screen mode with invalid information
*/
HbScreenMode::HbScreenMode()
    :   d_ptr(new HbScreenModePrivate())
{
}

/*!
    Copy constructor
*/
HbScreenMode::HbScreenMode(const HbScreenMode &mode)
    :   d_ptr(new HbScreenModePrivate())
{
    *d_ptr = *mode.d_ptr;
}

/*!
    operator equals
*/
HbScreenMode &HbScreenMode::operator=(const HbScreenMode &other)
{
    if (this != &other) {
        *d_ptr = *other.d_ptr;
    }
    return *this;
}

HbScreenMode::~HbScreenMode()
{
    delete d_ptr;
}

void HbScreenMode::setName(const QString &name)
{
    d_ptr->mStyleName = name;
}

QString HbScreenMode::name() const
{
    return d_ptr->mStyleName;
}

QSize HbScreenMode::pixelSize() const
{
    return d_ptr->mSize;
}

void HbScreenMode::setPixelWidth(int width)
{
    d_ptr->mSize.setWidth(width);
}

void HbScreenMode::setPixelHeight(int height)
{
    d_ptr->mSize.setHeight(height);
}
