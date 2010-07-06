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

#ifndef HBBADGEICONINFO_P_H
#define HBBADGEICONINFO_P_H

#include <hbglobal.h>
#include <qnamespace.h>
#include <hbicon.h>
#include <QSizeF>

class HbBadgeIconInfoPrivate;

class HbBadgeIconInfo
{
public:
    HbBadgeIconInfo(const HbIcon &badge,
                    Qt::Alignment alignment,
                    int zValue,
                    const QSizeF &sizeFactor,
                    Qt::AspectRatioMode aspectRatio);

    bool operator==(const HbBadgeIconInfo &other) const;
    bool operator!=(const HbBadgeIconInfo &other) const;

    HbIcon icon() const { return mIcon; }
    void setIcon(const HbIcon &icon);

    int zValue() const { return mZValue; }
    void setZValue(int z);

    Qt::Alignment alignment() const { return mAlignment; }
    void setAlignment(Qt::Alignment align);

    QSizeF sizeFactor() const { return mSizeFactor; }
    void setSizeFactor(const QSizeF &sizeFactor);

    Qt::AspectRatioMode aspectRatio() const { return mAspectRatio; }
    void setAspectRatio(Qt::AspectRatioMode aspectRatio);

private:
    HbIcon mIcon;
    HbIcon mSizedIcon;
    Qt::Alignment mAlignment;
    int mZValue;
    QSizeF mSizeFactor;
    Qt::AspectRatioMode mAspectRatio;
};

#endif
