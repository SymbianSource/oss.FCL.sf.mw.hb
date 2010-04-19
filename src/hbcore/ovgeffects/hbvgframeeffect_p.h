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

#ifndef HBVGFRAMEEFFECT_P_H
#define HBVGFRAMEEFFECT_P_H

#include "hbvgeffect_p.h"

class HbVgFrameEffectPrivate;

class HB_CORE_PRIVATE_EXPORT HbVgFrameEffect : public HbVgEffect
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    enum Hint {
        ExcludeChildrenHint = 0x01,
        DrawSourceHint = 0x02,
        ForceFrameHint = 0x04
    };
    Q_DECLARE_FLAGS(Hints, Hint)

    HbVgFrameEffect(QObject *parent = 0);
    ~HbVgFrameEffect();

    QRectF boundingRectFor(const QRectF &rect) const;

    QColor color() const;

    void setHint(Hints hint);

public slots:
    void setColor(const QColor &color);

signals:
    void colorChanged(const QColor &color);

protected:
    HbVgFrameEffect(HbVgFrameEffectPrivate &dd, QObject *parent = 0);
    void performEffect(QPainter *painter, const QPointF &offset,
                       const QVariant &vgImage, const QSize &vgImageSize);

private:
    Q_DECLARE_PRIVATE(HbVgFrameEffect)
    Q_DISABLE_COPY(HbVgFrameEffect)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(HbVgFrameEffect::Hints)

#endif
