/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbWidgets module of the UI Extensions for Mobile.
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

#include <QPainter>

#include "hbtransparentwindow.h"

/*!
    @beta
    @hbwidgets
    \class HbTransparentWindow
    \brief The HbTransparentWindow class provides a easy way to create 
    transparent area to UI.

    A transparent window is transparent area in the UI which will show
    underlying content through. The HbMainWindow needs to be constructed
    with Hb::WindowFlagTransparent attribute to get the hole working. 
    
    Transparent window will erase all the pixels in the UI underneath the 
    widget. You can place content on top of transparent window normally.
    
    This widget is meant to be used when developer has own window surface
    running underneath the UI and this surface should be show through the 
    hole in the UI e.g. video playback through own surface or HW accelerated 
    3D content in own surface.

    \sa HbWidget, HbMainWindow, HbMainWindow::nativeBackgroundWindow
*/

/*!
    \reimp
    \fn int HbToolButton::type() const
 */

/*!
    Constructs a new HbTransparentWindow with \a parent.
*/
HbTransparentWindow::HbTransparentWindow(QGraphicsItem *parent) :
    HbWidget(parent)
{
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);
}


/*!
    Destructs the transparent window.
 */
HbTransparentWindow::~HbTransparentWindow()
{
}

void HbTransparentWindow::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    QPainter::CompositionMode compositionMode = painter->compositionMode();
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->fillRect(option->exposedRect, QColor(0,0,0,0));
    painter->setCompositionMode(compositionMode);
}

#include "moc_hbtransparentwindow.cpp"
