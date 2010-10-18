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
#include "hbtextitem.h"
#include "hbtextitem_p.h"

class QPropertyAnimation;

class HbMarqueeContent : public HbTextItem
{
    Q_OBJECT
    Q_PROPERTY(qreal alpha READ alpha WRITE setAlpha)
    Q_PROPERTY(qreal xOffset READ xOffset WRITE setXOffset)

public:
    explicit HbMarqueeContent(HbMarqueeItem *parent = 0);
    virtual ~HbMarqueeContent(){}

    void setAlpha(qreal alpha);
    qreal alpha() const;

    qreal xOffset() const;
    void setXOffset(qreal newOffset);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbTextItem)
};

class HbMarqueeItemPrivate : public HbWidgetBasePrivate
{
    Q_DECLARE_PUBLIC(HbMarqueeItem)

public:

    HbMarqueeItemPrivate();
    void init();
    void createAnimation();
    void updateAnimation();
    void connectToMainWidow();
    void _q_stateChanged();

    void _q_tryToResumeAnimation();
    void _q_temporaryStopAnimation();
    void toggleAnimation(bool toStart);

    HbMarqueeContent *content;
    mutable QColor mDefaultColor;
    QColor mColor;
    bool mUserRequestedAnimation;

    QSequentialAnimationGroup mAnimGroup;
    QPropertyAnimation *mOffsetAnimation; // this animation should be updated on resize
    QPropertyAnimation *mBackAnimation; // this animation should be updated on resize
    bool mAnimationIsNeeded;
    qreal mLastPrefWidth;
};

#endif // HBMARQUEEITEM_P_H
