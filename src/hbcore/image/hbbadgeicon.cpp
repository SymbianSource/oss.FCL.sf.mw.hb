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

#include "hbbadgeicon_p.h"
#include "hbicon.h"
#include "hbbadgeiconinfo_p.h"
#include <QPainter>

/*!
  @beta
  @hbcore
  \internal
  \class HbBadgeIcon
  \brief HbBadgeIcon represents zero or more badge icons to be drawn
  over an HbIcon.

  This class uses a QList<HbBadgeIconInfo> in z-order to store the badges
  that must be applied when rendering an HbIcon.

  This class is to be used only internally within HbCore. Furthermore,
  This class is not intended to be a superclass for anything else.
*/

/*!
  Default constructor; needs to do nothing.
  \internal
*/
HbBadgeIcon::HbBadgeIcon()
{
    // This space intentionally left blank
}


/*!
  Destructor; no explicit work needs to be done here.
\internal
*/
HbBadgeIcon::~HbBadgeIcon()
{
    // This space intentionally left blank
}

/*!
  Add an icon to be drawn at the specified alignment in the
  given z-plane after scaling a predetermined amount. The
  badges are stored in increasing z-order value.

  Note that scaling is here for API compatibility later, but this
  implementation provides no scaling of the badge icon.
  \internal
 */
void HbBadgeIcon::addBadge(Qt::Alignment alignment,
                           const HbIcon &icon,
                           int zValue)
{
    HbBadgeIconInfo info(icon, alignment, zValue);
    int size = mBadgeIcons.size();
    if (size == 0) {
        mBadgeIcons.append(info);
    } else {
        bool added = false;
        //Find a spot to insert the badgeinfo in the list.
        for (int i = size - 1; i >= 0; i--) {
            if (mBadgeIcons[i].zValue() > zValue) {
                mBadgeIcons.insert(i + 1, info);
                added = true;
                break;
            }
        }
        //Not added anywhere so put it at the top.
        if (!added) {
            mBadgeIcons.prepend(info);
        }
    }
}

/*!
  Removes all badges with a specific \a _iconName as the HbIcon::iconName().
  \internal
 */
bool HbBadgeIcon::removeBadge(const HbIcon &badge)
{
    bool result = false;
    // Could make this a binary search, but complexity isn't worth it for normal use cases.
    int count = mBadgeIcons.count();
    for (int i = count - 1; i >= 0; --i) {
        const HbBadgeIconInfo &info = mBadgeIcons.at(i);
        if (info.icon() == badge) {
            mBadgeIcons.removeAt(i);
            result = true;
        }
    }
    return result;
}

/*!
  Indicate whether or not this icon is badged.
  \internal
*/
bool HbBadgeIcon::isBadged() const
{
    return mBadgeIcons.size() > 0;
}

/*!
  Remove all badge icons from this icon
  \internal
*/
void HbBadgeIcon::removeAllBadges()
{
    mBadgeIcons.clear();
}

/*!
    Returns the list of all badge icons
*/
const QList<HbBadgeIconInfo> HbBadgeIcon::badges() const
{
    return mBadgeIcons;
}

/*!
  \internal
  Paint all badges in z-order.
*/
void HbBadgeIcon::paint(QPainter *painter,
                        const QRectF &rect,
                        QIcon::Mode mode,
                        QIcon::State state,
                        bool mirror)
{
    int count = mBadgeIcons.count();

    for (int i = count - 1; i >= 0; i--) {
        HbBadgeIconInfo aIcon = mBadgeIcons[i];
        Qt::Alignment align = aIcon.alignment();
        Qt::Alignment absAlign = align;
        if (mirror) {
            absAlign = align & ~(Qt::AlignRight | Qt::AlignLeft);

            if (align & Qt::AlignLeft) {
                absAlign |= Qt::AlignRight;
            }
            if (align & Qt::AlignRight) {
                absAlign |= Qt::AlignLeft;
            }
        }
        // ... and then draw at the specified location.
        aIcon.icon().paint(painter,
                           rect,
                           Qt::KeepAspectRatio,
                           absAlign,
                           mode,
                           state);
    }
}

/*!
  \internal
 */
void HbBadgeIcon::externalize(QDataStream &stream)
{
    int size = mBadgeIcons.size();
    // Write out how many badges we'll save first
    stream << size;
    // And write each item
    for (int i = 0; i < size; i++) {
        HbBadgeIconInfo aIcon = mBadgeIcons[i];
        stream << aIcon.icon();
        stream << (qint32)(aIcon.alignment());
        stream << aIcon.zValue();
    }
}


/*!
\Internal
*/
void HbBadgeIcon::internalize(QDataStream &stream)
{
    int howManyBadges;
    stream >> howManyBadges;
    for (int i = 0; i < howManyBadges; ++i) {
        qint32 align;
        qint32 zValue;
        HbIcon icon;
        stream >> icon;
        stream >> align;
        stream >> zValue;

        HbBadgeIconInfo info(icon, (Qt::Alignment)align, zValue);
        mBadgeIcons.append(info);
    };
}
