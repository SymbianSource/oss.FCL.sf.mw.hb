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

#ifndef HBSTATUSBAR_H
#define HBSTATUSBAR_H

#include "hbnamespace_p.h"
#include <hbwidget.h>

class HbStatusBarPrivate;
class HbStyleOptionStatusBar;
class HbView;
struct IndicatorClientInfo;

class HB_CORE_PRIVATE_EXPORT HbStatusBar : public HbWidget
{
    Q_OBJECT

public:
    explicit HbStatusBar(HbMainWindow *mainWindow, QGraphicsItem *parent = 0);
    virtual ~HbStatusBar();

    void delayedConstruction();

    enum { Type = HbPrivate::ItemType_StatusBar };
    int type() const { return Type; }

    void propertiesChanged();
    QGraphicsItem *primitive(const QString &itemName) const;

public slots:
    virtual void createPrimitives();
    virtual void updatePrimitives();
    void currentViewChanged(HbView *view);

signals:
    void activated(const QList<IndicatorClientInfo> &clientInfo);
    void deactivated(const QList<IndicatorClientInfo> &clientInfo);
    void allActivated(const QList<IndicatorClientInfo> &clientInfo);

protected:
    void initStyleOption(HbStyleOptionStatusBar *option) const;
    void timerEvent(QTimerEvent *event);
    void gestureEvent(QGestureEvent* e);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbStatusBar)
    Q_DISABLE_COPY(HbStatusBar)
};

#endif // HBSTATUSBAR_H
