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

#ifndef HBVGREFLECTIONEFFECT_P_H
#define HBVGREFLECTIONEFFECT_P_H

#include "hbvgframeeffect_p.h"

class HbVgReflectionEffectPrivate;

class HB_CORE_PRIVATE_EXPORT HbVgReflectionEffect : public HbVgFrameEffect
{
    Q_OBJECT
    Q_PROPERTY(QPointF offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(qreal fade READ fade WRITE setFade NOTIFY fadeChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    HbVgReflectionEffect(QObject *parent = 0);
    ~HbVgReflectionEffect();

    QRectF boundingRectFor(const QRectF &rect) const;

    QPointF offset() const;
    qreal fade() const;
    QColor color() const;

public slots:
    void setOffset(const QPointF &offset);
    void setOffset(qreal x, qreal y) {
        setOffset(QPointF(x, y));
    }
    void setFade(qreal fade);
    void setColor(const QColor &color);

signals:
    void offsetChanged(const QPointF &offset);
    void fadeChanged(qreal fade);
    void colorChanged(const QColor &color);

protected:
    HbVgReflectionEffect(HbVgReflectionEffectPrivate &dd, QObject *parent = 0);
    void performEffect(QPainter *painter, const QPointF &offset,
                       const QVariant &vgImage, const QSize &vgImageSize);

private:
    Q_DECLARE_PRIVATE(HbVgReflectionEffect)
    Q_DISABLE_COPY(HbVgReflectionEffect)
};

#endif
