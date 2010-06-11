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

#ifndef HBVGEFFECT_P_H
#define HBVGEFFECT_P_H

#include <hbglobal.h>
#include <QGraphicsEffect>
#include <QVariant>

class HbVgEffectPrivate;
QT_BEGIN_NAMESPACE
class QGraphicsItem;
QT_END_NAMESPACE

class HB_CORE_PRIVATE_EXPORT HbVgEffect : public QGraphicsEffect
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(bool caching READ caching WRITE setCaching NOTIFY cachingChanged)

public:
    HbVgEffect(QObject *parent = 0);
    ~HbVgEffect();

    void install(QGraphicsItem *item);

    qreal opacity() const;

    virtual void performEffect(QPainter *painter,
                               const QPointF &offset,
                               const QVariant &vgImage,
                               const QSize &vgImageSize) = 0;

    void setChainRoot(HbVgEffect *effect);
    HbVgEffect *chainRoot() const;

    bool caching() const;

    static void releaseCachedResources();

public slots:
    void setOpacity(qreal opacity);
    void setCaching(bool caching);

signals:
    void opacityChanged(qreal opacity);
    void cachingChanged(bool caching);

protected:
    HbVgEffect(HbVgEffectPrivate &dd, QObject *parent = 0);

    void updateEffect();
    void updateEffectBoundingRect();

    QPixmap cached(const QSize &size) const;
    void tryCache(const QPixmap &pm);

    const QGraphicsItem *sourceItemForRoot() const;
    QRectF sourceBoundingRectForRoot() const;

    void draw(QPainter *painter);
    void sourceChanged(ChangeFlags flags);

    HbVgEffectPrivate *const d_ptr;
    Q_DECLARE_PRIVATE_D(d_ptr, HbVgEffect)
    Q_DISABLE_COPY(HbVgEffect)
};

#endif
