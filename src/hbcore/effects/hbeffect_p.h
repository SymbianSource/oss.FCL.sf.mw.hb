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

#ifndef HBEFFECT_P_H
#define HBEFFECT_P_H

#include <hbeffect.h>
#include "hbeffectcontroller_p.h"
#include "hbeffectfactory_p.h"
#include <QMap>
#include <QList>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QGraphicsItem;
QT_END_NAMESPACE

class EffectMapKey;
class HbView;
class HbMainWindow;

class HbAnimatedItemGroup : public QObject
{
    Q_OBJECT

public:
    HbAnimatedItemGroup(
        const QList<QGraphicsItem *> &items,
        QObject *receiver,
        const char *member,
        const QVariant &userData);

public slots:
    void finished(const HbEffect::EffectStatus &status);
    void receiverDestroyed();

public: // data
    QList<QGraphicsItem *> mItems;
    int mFinishedCount;

    QObject *mReceiver;
    QString mFunction;
    QVariant mUserData;
};

class HbEffectPrivate : public QObject
{
    Q_OBJECT

public:
    HbEffectPrivate();
    ~HbEffectPrivate();

    void connectViewChanges();

private slots:
    void handleViewChanged(HbView *view);
    void handleWindowAdded(HbMainWindow *window);

public:
    HbEffectController mController;
    HbEffectFactory mFactory;

    QMap<EffectMapKey, HbEffectGroup *> mEventEffectList;
    bool mEnabled;
    QList<QGraphicsItem *> mDisabledItems;
    bool mViewChangeConnected;
    bool mEffectsEnabled;
};

#endif // HBEFFECT_P_H
