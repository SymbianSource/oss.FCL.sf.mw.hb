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

#ifndef HBVGMASKEFFECT_P_H
#define HBVGMASKEFFECT_P_H

#include "hbvgeffect_p.h"

class HbVgMaskEffectPrivate;

class HB_CORE_PRIVATE_EXPORT HbVgMaskEffect : public HbVgEffect
{
    Q_OBJECT
    Q_PROPERTY(QRectF maskRect READ maskRect WRITE setMaskRect NOTIFY maskRectChanged)
    Q_PROPERTY(QRectF maskDeviceRect READ maskDeviceRect WRITE setMaskDeviceRect NOTIFY maskRectChanged)
    Q_PROPERTY(QPixmap mask READ mask WRITE setMask NOTIFY maskChanged)

public:
    HbVgMaskEffect(QObject *parent = 0);
    ~HbVgMaskEffect();

    QRectF boundingRectFor(const QRectF &rect) const;

    typedef QPixmap (*MaskCallback)(const QSize &, void *);

    QPixmap mask() const;
    QPixmap scaledMask() const;

    QRectF maskRect() const;
    QRectF maskDeviceRect() const;

    MaskCallback maskCallback() const;
    void *maskCallbackParam() const;

public slots:
    void clear();
    void setMask(const QPixmap &mask);
    void setMaskRect(const QRectF &rect);
    void setMaskDeviceRect(const QRectF &rect);
    inline void setMaskRect(int x, int y, int w, int h) { setMaskRect(QRectF(x, y, w, h)); }
    inline void setMaskDeviceRect(int x, int y, int w, int h) { setMaskDeviceRect(QRectF(x, y, w, h)); }
    void setMaskCallback(MaskCallback callback, void *param = 0);

signals:
    void maskChanged(const QPixmap &mask);
    void maskRectChanged(const QRectF &rect);
    void maskCallbackChanged(MaskCallback callback);

protected:
    HbVgMaskEffect(HbVgMaskEffectPrivate &dd, QObject *parent = 0);
    void performEffect(QPainter *painter, const QPointF &offset,
                       const QVariant &vgImage, const QSize &vgImageSize);

private:
    Q_DECLARE_PRIVATE(HbVgMaskEffect)
    Q_DISABLE_COPY(HbVgMaskEffect)
};

#endif
