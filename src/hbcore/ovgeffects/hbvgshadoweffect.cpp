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

#include "hbvgshadoweffect_p.h"
#include "hbvgoutlineeffect_p_p.h"
#include <QPainter>

/*!
 * \class HbVgShadowEffect
 *
 * \brief OpenVG-based dropshadow filter effect.
 *
 * \internal
 */

HbVgShadowEffect::HbVgShadowEffect(QObject *parent)
    : HbVgOutlineEffect(parent)
{
}

HbVgShadowEffect::HbVgShadowEffect(HbVgOutlineEffectPrivate &dd, QObject *parent)
    : HbVgOutlineEffect(dd, parent)
{
}

HbVgShadowEffect::~HbVgShadowEffect()
{
}

void HbVgShadowEffect::performEffect(QPainter *painter,
                                   const QPointF &offset,
                                   const QVariant &vgImage,
                                   const QSize &vgImageSize)
{
#ifdef HB_EFFECTS_OPENVG
    Q_D(HbVgOutlineEffect);
    QPixmap outlinePm = makeOutline(vgImage, vgImageSize);
    // Draw the shadow. Note that this paints also outside of our bounding
    // rectangle but all those pixels (that fall outside of our rect) are
    // transparent. This should be more effective than trying to get the shadow
    // translated already in outlinePm.
    painter->drawPixmap(offset + d->mapOffset(d->offset), outlinePm);
    // Draw the source.
    painter->drawPixmap(offset, d->srcPixmap);
#else
    Q_UNUSED(painter);
    Q_UNUSED(offset);
    Q_UNUSED(vgImage);
    Q_UNUSED(vgImageSize);
#endif
}
