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

#ifndef HBEFFECTINTERNAL_P_H
#define HBEFFECTINTERNAL_P_H

#include <hbglobal.h>
#include <hbnamespace.h>
#include <QStringList>

const int HbGVWrapperItemType = Hb::ItemType_Last + 10000; // just some value after the last one

class HbMainWindow;

// Graphics View wrapper item, to enable transforming graphics view using effects engine
class HB_CORE_PRIVATE_EXPORT HbGVWrapperItem: public QGraphicsItem
{
public:
    HbGVWrapperItem();
    int type() const {
        return HbGVWrapperItemType;
    }
    void setMainWindow(HbMainWindow &mainWindow);
    HbMainWindow *mainWindow() const;
    qreal transformDegrees;
private:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    HbMainWindow *mMainWindow;
};

class HB_CORE_PRIVATE_EXPORT HbEffectInternal
{

public:
    enum EffectFlag {
        Normal = 0,
        ClearEffectWhenFinished = 1,
        ShowItemOnFirstUpdate = 2,
        HideRegItemBeforeClearingEffect = 4,
        HideTargetItemBeforeClearingEffect = 8,
        UpdateAtEachStep = 16
    };
    Q_DECLARE_FLAGS(EffectFlags, EffectFlag)

    static bool start(QGraphicsItem *registrationItem,
                      QGraphicsItem *targetItem,
                      EffectFlags flags,
                      const QString &itemType,
                      const QString &effectEvent,
                      QObject *receiver = 0,
                      const char *member = 0,
                      const QVariant &userData = QVariant(),
                      const QRectF &extRect = QRectF());

    static bool start(QGraphicsItem *registrationItem,
                      QGraphicsItem *targetItem,
                      EffectFlags flags,
                      const QString &effectEvent,
                      QObject *receiver = 0,
                      const char *member = 0,
                      const QVariant &userData = QVariant(),
                      const QRectF &extRect = QRectF());

    static bool add(const QString &itemType, const QString &filePath, const QString &effectEvent = QString());
    static bool add(const QStringList &itemType, const QStringList &filePath, const QStringList &effectEvent = QStringList());
    static bool add(const char *itemType[], const char *filePath[], const char *effectEvent[], int count);
    static bool add(QGraphicsItem *item, const QString &filePath, const QString &effectEvent = QString());
    static bool add(QGraphicsItem *item, const QStringList &filePath, const QStringList &effectEvent = QStringList());

    static void remove(const QString &itemType, const QString &filePath, const QString &effectEvent);
    static void remove(QGraphicsItem *item, const QString &filePath, const QString &effectEvent);
    static void remove(QGraphicsItem *item);

    static void reloadFxmlFiles();

    static void cancelAll(const QList<QGraphicsItem *> *exceptionList = 0, bool ignoreLooping = false);
    static void safeCancelAll(bool clear = false);

    static void stopEffects();
    static void resumeEffects();

    static void setEffectsEnabled(bool enabled = true);

private:
    HbEffectInternal();
    HbGVWrapperItem mGVWrapper;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(HbEffectInternal::EffectFlags)

#endif // HBEFFECTINTERNAL_P_H
