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

#ifndef HBICON_P_H
#define HBICON_P_H

#include "hbiconengine_p.h"
#include <QDataStream>

class HbIconAnimator;
class HbBadgeIcon;

class HbIconPrivate : public QSharedData
{
public:
    HbIconPrivate();
    HbIconPrivate(const QIcon &qicon);
    HbIconPrivate(const QString &iconName);
    HbIconPrivate(const HbIconPrivate &other);
    ~HbIconPrivate();

    void clear();
    void clearStoredIconContent();
    bool addBadge(Qt::Alignment alignment,
                  const HbIcon &badge,
                  int z,
                  const QSizeF &sizeFactor,
                  Qt::AspectRatioMode aspectRatio);
    bool removeBadge(const HbIcon &badge);
    void removeAllBadges();
    bool isBadged() const;
    void setThemedColor(const QColor &color);
    QColor themedColor() const;
    void setAsync(bool async, HbIconEngine::AsyncCallback callback, void *param);
    bool async() const;

private:
    // disabled
    HbIconPrivate &operator=(const HbIconPrivate &other);

public:
    static HbIconPrivate *d_ptr(HbIcon *icon) {
        return icon->d.data();
    }
    static HbIconPrivate *d_ptr_detached(HbIcon *icon) {
        icon->d.detach();
        return icon->d.data();
    }

    QSizeF size;
    HbIconEngine *engine; // this is 0 if HbIcon was copy constructed from QIcon.
    QIcon qicon;

    HbBadgeIcon *badgeInfo;

    HbIcon::Flags mQIconFlags;
    QPixmap mQIconPixmap;
    QIcon::Mode mQIconPixmapMode;
    QIcon::State mQIconPixmapState;
};

QDataStream &operator<<(QDataStream &stream, const HbIconPrivate &icon);
QDataStream &operator>>(QDataStream &stream, HbIconPrivate &icon);

#endif // HBICON_P_H
