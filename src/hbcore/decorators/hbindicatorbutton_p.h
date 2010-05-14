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

#ifndef HBINDICATORBUTTON_P_H
#define HBINDICATORBUTTON_P_H

#include <hbtoolbutton.h>
#include "hbnamespace_p.h"

class HbIndicatorButtonPrivate;
class HbStyleOptionIndicatorButton;
struct IndicatorClientInfo;

class HB_CORE_PRIVATE_EXPORT HbIndicatorButton : public HbToolButton
{
    Q_OBJECT

public:
    explicit HbIndicatorButton(QGraphicsItem *parent = 0);
    virtual ~HbIndicatorButton();

    void delayedConstruction();

    enum { Type = HbPrivate::ItemType_IndicatorButton };
    int type() const { return Type; }

    void showHandleIndication(bool show);
    bool handleVisible() const;

public slots:
    virtual void createPrimitives();
    virtual void updatePrimitives();
    void activate(const QList<IndicatorClientInfo> &clientInfo);
    void deactivate(const QList<IndicatorClientInfo> &clientInfo);

protected:
    virtual void initStyleOption(HbStyleOptionIndicatorButton *option) const;
    virtual void changeEvent(QEvent* event);

private slots:
    void handlePress();
    void handleRelease();

private:  
    Q_DECLARE_PRIVATE_D(d_ptr, HbIndicatorButton)
    Q_DISABLE_COPY(HbIndicatorButton)
};

#endif // HBINDICATORBUTTON_P_H
