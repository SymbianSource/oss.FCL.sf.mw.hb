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

#ifndef HBNAVIGATIONBUTTON_P_H
#define HBNAVIGATIONBUTTON_P_H

#include <hbtoolbutton.h>
#include "hbnamespace_p.h"

class HbNavigationButtonPrivate;
class HbStyleOptionNavigationButton;

class HB_CORE_PRIVATE_EXPORT HbNavigationButton : public HbToolButton
{
    Q_OBJECT

public:
    explicit HbNavigationButton(QGraphicsItem *parent = 0);
    virtual ~HbNavigationButton();

    enum { Type = HbPrivate::ItemType_NavigationButton };
    int type() const { return Type; }

public slots:
    virtual void createPrimitives();
    virtual void updatePrimitives();

protected:
    virtual void initStyleOption(HbStyleOptionNavigationButton *option) const;
    virtual void changeEvent(QEvent* event);

private slots:
    void handlePress();
    void handleRelease();

private:  
    Q_DECLARE_PRIVATE_D(d_ptr, HbNavigationButton)
    Q_DISABLE_COPY(HbNavigationButton)
};

#endif // HBNAVIGATIONBUTTON_P_H
