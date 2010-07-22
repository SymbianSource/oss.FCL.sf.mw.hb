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

#ifndef HBPIXMAPICONRENDERER_P_H
#define HBPIXMAPICONRENDERER_P_H

#include <hbglobal.h>
#include <QIcon>

#include "hbiconimpl_p.h"

class HbMaskableIconImpl;

class HB_CORE_PRIVATE_EXPORT HbPixmapIconRenderer
{
public:

    HbPixmapIconRenderer(const QPixmap &pixmap, HbIconImpl *impl);

    ~HbPixmapIconRenderer();

    void setColor(const QColor &color) {
        this->iconColor = color;
    }

    void setMode(QIcon::Mode mode) {
        this->iconMode = mode;
    }

    QColor color() {
        return iconColor;
    }

    QIcon::Mode mode() {
        return iconMode;
    }

    void draw(QPainter *painter,
              const QPointF &topLeft,
              const QPainterPath &clipPath,
              HbMaskableIconImpl *maskIconData);

    void destroyMaskedData(HbIconMaskedData *data);

private:

    void applyIconProperties();

    QPixmap getMaskedPixmap(HbMaskableIconImpl *maskIconData);

    void doDraw(QPainter *painter,
                const QPointF &topLeft,
                const QPixmap &finalPixmap,
                const QPainterPath &clipPath);

    QColor                  iconColor;
    QIcon::Mode             iconMode;
    bool                    iconPropertiesApplied;
    QPixmap                 pixmapData;
    HbIconImpl             *iconImpl;
};

#endif
