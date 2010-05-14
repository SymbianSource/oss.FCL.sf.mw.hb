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

#ifndef HBTITLEBAR_H
#define HBTITLEBAR_H

#include "hbnamespace_p.h"
#include <hbwidget.h>
#ifdef HB_EFFECTS
#include <hbeffect.h>
#endif // HB_EFFECTS

class HbIndicatorButton;
class HbTitleBarPrivate;
class HbTitlePane;
class HbView;
class HbAction;
struct IndicatorClientInfo;

class HB_CORE_PRIVATE_EXPORT HbTitleBar : public HbWidget
{
    Q_OBJECT
    Q_PROPERTY(bool minimizable READ minimizable)

public:

    enum Position {
        Original = 0, // original position from layout
        Minimized = 1, // only handle visible
        DismissedUp = 2 // titlebar moved up
    };

    explicit HbTitleBar(HbMainWindow *mainWindow, QGraphicsItem *parent = 0);
    virtual ~HbTitleBar();

    void delayedConstruction();

    enum { Type = HbPrivate::ItemType_TitleBar };
    int type() const { return Type; }

    HbTitlePane *titlePane() const;

    Position position() const;

    bool minimizable() const;

    void propertiesChanged();

    HbAction *navigationAction() const;
    void setNavigationAction(HbAction *action);
    void setDefaultNavigationAction();
    QGraphicsItem *primitive(const QString &itemName) const;

signals:
    void titleBarStateChanged();
    void activated(const QList<IndicatorClientInfo> &clientInfo);
    void deactivated(const QList<IndicatorClientInfo> &clientInfo);

public slots:
    void gestureRight();
    void gestureLeft();
    void currentViewChanged(HbView *view);
#ifdef HB_EFFECTS
    void effectFinished(const HbEffect::EffectStatus &status);
#endif // HB_EFFECTS

protected:
    HbTitleBar(HbTitleBarPrivate &dd, HbMainWindow *mainWindow, QGraphicsItem *parent=0);
    void polish(HbStyleParameters &params);
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbTitleBar)
    Q_DISABLE_COPY(HbTitleBar)
};

#endif // HBTITLEBAR_H
