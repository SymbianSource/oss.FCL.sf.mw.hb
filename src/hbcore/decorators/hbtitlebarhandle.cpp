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

#include "hbtitlebarhandle_p.h"

#include <QGraphicsWidget>
#include <QPainter>
#include <hbinstance.h>
#include <hbicon.h>

/*
    \class HbTitleBarHandle
    \brief HbTitleBarHandle represents a handle graphics inside indicatorgroup.
 */

static const char *TITLEBARHANDLE_BACKGROUND_IMAGE = "qtg_graf_titlebar_handle_normal";

/*
    Constructs a titlebar handle with \a parent.
    Also sets the default icon.
*/
HbTitleBarHandle::HbTitleBarHandle(QGraphicsItem *parent)
    : HbWidgetBase(parent),
      mAspectRatioMode(Qt::IgnoreAspectRatio),
      handleBackgroundIcon(new HbIcon(TITLEBARHANDLE_BACKGROUND_IMAGE))
{
    setAcceptedMouseButtons(Qt::LeftButton);
}

/*
    Destructor
 */
HbTitleBarHandle::~HbTitleBarHandle()
{
    delete handleBackgroundIcon;
}

/*
    \reimp
 */
void HbTitleBarHandle::paint(QPainter *painter,
    const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);
    QRectF indicatorRect(boundingRect());
    handleBackgroundIcon->setSize(indicatorRect.size());
    handleBackgroundIcon->paint(painter, indicatorRect, mAspectRatioMode);
}

