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

#ifndef HBVGOUTLINEEFFECT_P_H
#define HBVGOUTLINEEFFECT_P_H

#include "hbvgeffect_p.h"

class HbVgOutlineEffectPrivate;

class HB_CORE_PRIVATE_EXPORT HbVgOutlineEffect : public HbVgEffect
{
    Q_OBJECT

    Q_PROPERTY(QPointF outline READ outline WRITE setOutline NOTIFY outlineChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(qreal steepness READ steepness WRITE setSteepness NOTIFY steepnessChanged)
    Q_PROPERTY(QPointF offset READ offset WRITE setOffset NOTIFY offsetChanged)

public:
    HbVgOutlineEffect(QObject *parent = 0);
    ~HbVgOutlineEffect();

    QRectF boundingRectFor(const QRectF &rect) const;

    QPointF outline() const;
    QColor color() const;
    qreal steepness() const;
    QPointF offset() const;

public slots:
    void setOutline(const QPointF &outline);
    inline void setOutline(qreal x, qreal y) { setOutline(QPointF(x, y)); }
    void setColor(const QColor &color);
    void setSteepness(qreal steepness);
    void setOffset(const QPointF &offset);
    inline void setOffset(qreal x, qreal y) { setOffset(QPointF(x, y)); }

signals:
    void outlineChanged(const QPointF &outline);
    void colorChanged(const QColor &color);
    void steepnessChanged(qreal steepness);
    void offsetChanged(const QPointF &offset);

protected:
    HbVgOutlineEffect(HbVgOutlineEffectPrivate &dd, QObject *parent = 0);
    QPixmap makeOutline(const QVariant &vgImage, const QSize &vgImageSize);
    void performEffect(QPainter *painter, const QPointF &offset,
                       const QVariant &vgImage, const QSize &vgImageSize);

private:
    Q_DECLARE_PRIVATE(HbVgOutlineEffect)
    Q_DISABLE_COPY(HbVgOutlineEffect)
};

#endif
