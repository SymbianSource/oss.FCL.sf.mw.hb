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

#include "hblongpressvisualizer.h"
#include "hblongpressvisualizer_p.h"
#include <hbiconitem.h>
#include <hbmainwindow.h>
#include <hbwidget.h>
#include <hbinstance.h>
#include <hbiconanimationmanager.h>
#include <hbiconanimator.h>
#include <hbwidgetfeedback.h>
#include <hbnamespace_p.h>
#include <QGraphicsScene>

/*!
  @stable
  @hbcore
  \class HbLongPressVisualizer

  \brief Displays the long press animation, that is, the small animated icon
  indicating that the tap being held may potentially become a long press.

  Widgets or applications should use the functions of this class to show and
  hide the standard, themed long press animation.
*/

Q_GLOBAL_STATIC(HbLongPressVisualizerPrivate, visualizer)

/*!
  Shows the animated icon at (or near) position \a pos. If \a delayMs is non-zero then
  the animation is only shown after the specified number of milliseconds.

  \a pos is treated to be a scene position. In typical usage scenarios the scene
  position will be retrieved from a gesture event and passed as it is in \a
  pos. Do not make any assumptions about the exact position of the icon, the
  visualizer may decide to position it a bit differently in order to make it
  more visible to the user (e.g. to prevent being obscured by the user's
  finger).

  If \a widget is not 0 then the icon is added to the scene of the widget's
  mainwindow.  Otherwise the first main window is used. If no main windows were
  instantiated before calling start() then it will return immediately.

  The widget is also used for tactile feedback, if it is 0 then no feedback
  effect will be started. If the widget is given then the instant LongPressed
  feedback effect will be started automatically.
 */
void HbLongPressVisualizer::start(const QPointF &pos, int delayMs, const HbWidget *widget)
{
    visualizer()->start(pos, delayMs, widget);
}

/*!
  Hides the animated icon if it is visible. Has no effect if the animation is
  not currently visible.
 */
void HbLongPressVisualizer::stop()
{
    visualizer()->stop();
}

HbLongPressVisualizerPrivate::HbLongPressVisualizerPrivate()
    : mInited(false), mWidget(0), mIconItem(0)
{
}

void HbLongPressVisualizerPrivate::start(const QPointF &pos, int delayMs, const HbWidget *widget)
{
    // Multiple mainwindow support is in place below, however currently there is
    // only one icon item so the icon is only shown in one mainwindow at a time.
    stop();
    mWidget = widget;
    HbMainWindow *mainWindow = widget ? widget->mainWindow() : 0;
    if (!mainWindow) {
        QList<HbMainWindow *> mainWindows(hbInstance->allMainWindows());
        if (!mainWindows.isEmpty()) {
            mainWindow = mainWindows.at(0);
        } else {
            return;
        }
    }
    if (!mInited) {
        mInited = true;
        mTimer.setSingleShot(true);
        connect(&mTimer, SIGNAL(timeout()), SLOT(showIcon()));
        HbIconAnimationManager::global()->addDefinitionFile("qtg_anim_longtap.axml");
        mIconItem = new HbIconItem("qtg_anim_longtap");
        mIconItem->hide();
        mIconItem->setSize(mIconItem->defaultSize());
        mIconItem->setZValue(HbPrivate::PopupZValueRangeEnd + 5000);
    }
    QGraphicsScene *targetScene = mainWindow->scene();
    QGraphicsScene *oldScene = mIconItem->scene();
    if (targetScene != oldScene) {
        if (oldScene) {
            oldScene->removeItem(mIconItem);
        }
        targetScene->addItem(mIconItem); // takes ownership
    }
    prepareIcon(pos);
    if (delayMs > 0) {
        mTimer.start(delayMs);
    } else {
        showIcon();
    }
}

void HbLongPressVisualizerPrivate::stop()
{
    mTimer.stop();
    if (mIconItem) {
        mIconItem->animator().stopAnimation();
        mIconItem->hide();
    }
}

void HbLongPressVisualizerPrivate::prepareIcon(const QPointF &pos)
{
    mPos = pos;
    QSizeF iconSize = mIconItem->size();
    mPos -= QPointF(iconSize.width() / 2, iconSize.height());
    mIconItem->setPos(mPos);
}

void HbLongPressVisualizerPrivate::showIcon()
{
    mIconItem->animator().startAnimation();
    mIconItem->show();
    if (mWidget) {
        HbWidgetFeedback::triggered(mWidget, Hb::InstantLongPressed);
    }
}
