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

#ifndef HBICONANIMATOR_H
#define HBICONANIMATOR_H

#include <hbglobal.h>
#include <QIcon>
#include <QObject>

class HbIconAnimatorPrivate;
class HbIcon;
class HbIconEngine;
class QGraphicsItem;
class QPainter;
class QRectF;
class HbView;

class HB_CORE_EXPORT HbIconAnimator : public QObject
{
    Q_OBJECT

public:
    HbIconAnimator();
    explicit HbIconAnimator(QGraphicsItem *item);
    ~HbIconAnimator();

    HbIcon icon() const;
    void setIcon(const HbIcon &icon);

    void setGraphicsItem(QGraphicsItem *item);
    void setOwnerView(HbView *view);

    void paint(
        QPainter *painter,
        const QRectF &rect,
        Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio,
        Qt::Alignment alignment = Qt::AlignCenter,
        QIcon::Mode mode = QIcon::Normal,
        QIcon::State state = QIcon::Off) const;

    void setLoopCount(int loopCount);

public slots:
    void startAnimation();
    void stopAnimation();

signals:
    void animationStarted();
    void animationStopped();
    void animationProgressed();
    void animationFinished();

private:
    Q_DISABLE_COPY(HbIconAnimator)
    friend class HbIconEngine;
    friend class HbIconAnimation;
    friend class HbIconLoader;
    friend class HbIconAnimationManagerPrivate;
    HbIconAnimatorPrivate *d;
};

#endif // HBICONANIMATOR_H
