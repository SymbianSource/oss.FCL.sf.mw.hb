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

#ifndef HB_EFFECTABSTRACT_P_H
#define HB_EFFECTABSTRACT_P_H

#include <hbglobal.h>
#include <hbtimer_p.h>

QT_BEGIN_NAMESPACE
class QGraphicsItem;
class QTransform;
QT_END_NAMESPACE

class HbEffectGroup;
class HbEffectAnimationData;

class HB_AUTOTEST_EXPORT HbEffectAbstract : public HbTimerEntry
{
public:
    HbEffectAbstract(int startTime, QGraphicsItem *item, HbEffectGroup *group);
    virtual ~HbEffectAbstract();

    virtual QString name() const = 0;
    virtual void init() = 0;
    virtual void setStartState(QTransform &transform) = 0;
    virtual void start() = 0;
    virtual void cancel(QTransform &transform, bool itemIsValid) = 0;
    virtual void updateItemTransform(QTransform &transform) = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;

protected:
    void timerFired();

    QGraphicsItem *item() const;
    HbEffectGroup *group() const;
    HbEffectAnimationData *mData;

private:
    QGraphicsItem *mItem;
    HbEffectGroup *mGroup;
};

#endif // HB_EFFECTABSTRACT_P_H
