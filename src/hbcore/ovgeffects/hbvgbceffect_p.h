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

#ifndef HBVGBCEFFECT_P_H
#define HBVGBCEFFECT_P_H

#include "hbvgeffect_p.h"

class HbVgBcEffectPrivate;

class HB_CORE_PRIVATE_EXPORT HbVgBcEffect : public HbVgEffect
{
    Q_OBJECT
    Q_PROPERTY(qreal brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    Q_PROPERTY(qreal contrast READ contrast WRITE setContrast NOTIFY contrastChanged)

public:
    HbVgBcEffect(QObject *parent = 0);
    ~HbVgBcEffect();

    QRectF boundingRectFor(const QRectF &rect) const;

    qreal brightness() const;
    qreal contrast() const;

public slots:
    void setBrightness(qreal brightness);
    void setContrast(qreal contrast);

signals:
    void brightnessChanged(qreal brightness);
    void contrastChanged(qreal contrast);

protected:
    HbVgBcEffect(HbVgBcEffectPrivate &dd, QObject *parent = 0);
    void performEffect(QPainter *painter, const QPointF &offset,
                       const QVariant &vgImage, const QSize &vgImageSize);

private:
    Q_DECLARE_PRIVATE(HbVgBcEffect)
    Q_DISABLE_COPY(HbVgBcEffect)
};

#endif
