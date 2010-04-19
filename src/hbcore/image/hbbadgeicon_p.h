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

#ifndef HBBADGEICON_P_H
#define HBBADGEICON_P_H

#include <QList>
#include <QIcon>

#include "hbbadgeiconinfo_p.h"

class HbBadgeIcon
{
public:
    HbBadgeIcon();
    ~HbBadgeIcon();

public:
    void addBadge(Qt::Alignment alignment,
                      const HbIcon& badge,
                      int zValue=0);
    bool removeBadge(const HbIcon& badge);
    void removeAllBadges();
    bool isBadged() const;

    const QList<HbBadgeIconInfo> badges() const;

    void paint( QPainter *painter,
                const QRectF& rect,
                QIcon::Mode mode,
                QIcon::State state,
                bool mirror);
    void externalize(QDataStream& stream);
    void internalize(QDataStream& stream);


private:
    QRectF badgeLocation(const HbBadgeIconInfo& iconInfo,
                         const QRectF& parentRect,
                         bool mirror);
    QList<HbBadgeIconInfo> mBadgeIcons;
};

#endif /* HBBADGEICON_P_H */

