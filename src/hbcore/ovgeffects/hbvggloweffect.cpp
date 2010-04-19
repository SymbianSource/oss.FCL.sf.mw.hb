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

#include "hbvggloweffect_p.h"
#include "hbvgblureffect_p_p.h"
#include <QPainter>

/*!
 * \class HbVgGlowEffect
 *
 * \brief OpenVG-based inner glow filter effect.
 *
 * \internal
 */

HbVgGlowEffect::HbVgGlowEffect(QObject *parent)
    : HbVgBlurEffect(parent)
{
}

HbVgGlowEffect::HbVgGlowEffect(HbVgBlurEffectPrivate &dd, QObject *parent)
    : HbVgBlurEffect(dd, parent)
{
}

HbVgGlowEffect::~HbVgGlowEffect()
{
}

void HbVgGlowEffect::performEffect(QPainter *painter,
                                   const QPointF &offset,
                                   const QVariant &vgImage,
                                   const QSize &vgImageSize)
{
#ifdef HB_EFFECTS_OPENVG
    Q_D(HbVgBlurEffect);
    QPixmap blurred = makeBlur(vgImage, vgImageSize);
    painter->drawPixmap(offset, d->srcPixmap);
    vgSeti(VG_BLEND_MODE, VG_BLEND_ADDITIVE);
    painter->drawPixmap(offset, blurred);
    // Set the blend mode back to the default to make sure that it
    // does not affect other effects in the chain coming right after
    // this one.
    vgSeti(VG_BLEND_MODE, VG_BLEND_SRC_OVER);
#else
    Q_UNUSED(painter);
    Q_UNUSED(offset);
    Q_UNUSED(vgImage);
    Q_UNUSED(vgImageSize);
#endif
}
