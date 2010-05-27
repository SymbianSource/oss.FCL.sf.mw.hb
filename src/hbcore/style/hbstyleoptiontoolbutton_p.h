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

#ifndef HBSTYLEOPTIONTOOLBUTTON_H
#define HBSTYLEOPTIONTOOLBUTTON_H

#include <hbstyleoption_p.h>
#include <hbicon.h>


class HB_CORE_PRIVATE_EXPORT HbStyleOptionToolButton : public HbStyleOption
{
public:
    HbStyleOptionToolButton();
    HbStyleOptionToolButton(const HbStyleOptionToolButton &other);
    ~HbStyleOptionToolButton();

    enum StyleOptionType { Type = HbSO_ToolButton };
    enum StyleOptionVersion { Version = 1 };

    enum ToolBarPosition
    {
        TB_None,
        TB_OnlyOne,
        TB_Beginning,
        TB_Middle,
        TB_End
    };

    QString text;
    HbIcon icon;
    HbIcon customBackground;
    bool backgroundVisible;
    HbStyleOptionToolButton::ToolBarPosition toolBarPosition;
    Qt::Orientation orientation;
    bool isCheckable;
    bool useSecondaryGraphics;
    bool useTransparentGraphics;
    bool isToolBarExtension;
};

#endif // HBSTYLEOPTIONTOOLBUTTON_H
