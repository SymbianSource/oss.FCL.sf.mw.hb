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

#ifndef HBMARQUEEITEM_P_H
#define HBMARQUEEITEM_P_H

#include "hbmarqueeitem.h"
#include "hbwidgetbase_p.h"

#include <QSequentialAnimationGroup>
#include <QGraphicsObject>


class HbMarqueeContent : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(int alpha READ alpha WRITE setAlpha)
public:
    explicit HbMarqueeContent(HbMarqueeItem *parent = 0);
    virtual ~HbMarqueeContent(){};

    QRectF boundingRect() const;
    QPen pen();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget =0);

    void updateTextMetaData();
    void setAlpha(int alpha);
    int alpha() const;

    HbMarqueeItem *parent;
    QString mText;
    Qt::LayoutDirection mTextDirection;
    qreal mTextWidth;
    QPointF gradientStart;
    QPointF gradientStop;
    int mAlpha;
    qreal mFadeLength;
    QRectF mBoundingRect;
};

class HbMarqueeItemPrivate : public HbWidgetBasePrivate
{
    Q_DECLARE_PUBLIC(HbMarqueeItem)

public:

    HbMarqueeItemPrivate();
    void init();
    void updateTextMetaData();
    void initContentPosition();
    void initGradient();
    void initAnimations();
    void _q_stateChanged();

    HbMarqueeContent *content;
    mutable QColor mDefaultColor;
    QColor mColor;
    bool mAnimationPending;

    QSequentialAnimationGroup mAnimGroup;
};

#endif // HBMARQUEEITEM_P_H
