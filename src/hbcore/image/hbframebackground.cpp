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
#include "hbframebackground.h"
#include "hbframebackground_p.h"

#include <QVariant>

/*!
    @stable
    @hbcore
    \class HbFrameBackground
    \brief HbFrameBackground stores the frame data.

    This class is a data container for frame related data. It stores data that defines the
    frame background into single object that can be stored into QVariant.
*/

// Must be initialized dynamically because QIcon cannot be constructed
// when static variables are constructed.
static HbFrameBackgroundPrivate *shared_null = 0;

static const int frameBackgroundMetaType = qRegisterMetaType<HbFrameBackground>();

// This constructor is used for shared_null instance only
HbFrameBackgroundPrivate::HbFrameBackgroundPrivate() :
    frameGraphicsName(),
    type(HbFrameDrawer::Undefined)
{
    ref.ref(); // Need to do extra ref so the shared null does not get destructed
}

HbFrameBackgroundPrivate::HbFrameBackgroundPrivate(const QString &frameGraphicsName, HbFrameDrawer::FrameType type) :
    frameGraphicsName(frameGraphicsName),
    type(type)
{
}

/*! Default constructor.
*/
HbFrameBackground::HbFrameBackground()
{
    // Construct shared_null if not done yet.
    if (!shared_null) {
        shared_null = new HbFrameBackgroundPrivate;
    }
    d = shared_null;
}

/*! Constructs a new frame background with the given \a frameGraphicsName and \a type .
*/
HbFrameBackground::HbFrameBackground(const QString &frameGraphicsName, HbFrameDrawer::FrameType type) :
    d(new HbFrameBackgroundPrivate(frameGraphicsName, type))
{
}

/*!
* Copy constructs a new frame background using the \a other frame background.
*/
HbFrameBackground::HbFrameBackground(const HbFrameBackground &other) :
    d(other.d)
{
}

/*!
* Destroys the frame background.
*/
HbFrameBackground::~HbFrameBackground()
{
}

/*!
* Assigns the \a other frame background to this frame background and returns a reference to
* this frame background. Copy-on-write semantics is used, so this only does a shallow copy.
*/
HbFrameBackground &HbFrameBackground::operator=(const HbFrameBackground &other)
{
    if (&other != this) {
        d = other.d;
    }
    return *this;
}

/*!
* Equality operator.
*/
bool HbFrameBackground::operator==(const HbFrameBackground &other) const
{
    return !(*this != other);
}

/*!
* Inequality operator.
*/
bool HbFrameBackground::operator!=(const HbFrameBackground &other) const
{
    if (d->frameGraphicsName != other.d->frameGraphicsName
            || d->frameGraphicsName.isNull() != other.d->frameGraphicsName.isNull()) {
        return true;
    }

    if (d->type != other.d->type) {
        return true;
    }

    return false;
}

/*!
* Returns true if the frame background has been constructed with the default constructor
* and the frame graphics name has not been set.
*/
bool HbFrameBackground::isNull() const
{
    return d->frameGraphicsName.isNull();
}

/*!
* Returns the frame graphics name.
* \sa HbFrameBackground::setFrameGraphicsName()
*/
QString HbFrameBackground::frameGraphicsName() const
{
    return d->frameGraphicsName;
}

/*!
* Sets the frame graphics name. See the HbFrameDrawer class description for the file name convention for different frame parts.
* \sa HbFrameBackground::frameGraphicsName()
*/
void HbFrameBackground::setFrameGraphicsName(const QString &frameGraphicsName)
{
    // Remove possible file extension
    QString nameWithoutExt = frameGraphicsName;
    int index = nameWithoutExt.lastIndexOf(QChar('.'));
    if (index > 0) {
        nameWithoutExt.resize(index);
    }

    if (d->frameGraphicsName != nameWithoutExt
            || d->frameGraphicsName.isNull() != nameWithoutExt.isNull()) {
        d.detach();
        d->frameGraphicsName = nameWithoutExt;
    }
}

/*!
* Returns the frame type.
* \sa HbFrameBackground::setFrameType()
*/
HbFrameDrawer::FrameType HbFrameBackground::frameType() const
{
    return d->type;
}

/*!
* Sets the frame type.
* \sa HbFrameBackground::frameType()
*/
void HbFrameBackground::setFrameType(HbFrameDrawer::FrameType type)
{
    if (d->type != type) {
        d.detach();
        d->type = type;
    }
}

/*!
* Returns the frame background as a QVariant.
*/
HbFrameBackground::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

// End of File
