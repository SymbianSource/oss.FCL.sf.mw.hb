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

#include "hbiconanimator.h"
#include "hbiconanimator_p.h"
#include "hbiconanimation_p.h"
#include "hbicon_p.h"
#include "hbiconengine_p.h"
#include "hbnamespace.h"
#include "hbview.h"
#include "hbmainwindow.h"
#include <QObject>
#include <QGraphicsWidget>
#include <QPainter>

/*!
  @stable
  @hbcore

  \class HbIconAnimator

  \brief HbIconAnimator is the animation interface for animated icons. Class HbIconItem
  uses it for built-in animation support. For more specific use cases, clients can also
  create their own HbIconAnimator instance and use it for driving an icon animation.
  
  If the icon is not animated, the animator does not do anything.
  
  The animator provides signals for observing the start, stop and finish of the animation and the
  animation progress.
*/

/*!
  \fn void HbIconAnimator::animationStarted()

  This signal is emitted whenever the icon animation is started from the
  beginning. By default an icon animation is started when the icon is painted
  for the first time. This behaviour can be disabled with the flag
  HbIcon::NoAutoStartAnimation.

  Note that the signal is not emitted when restarting due to
  looping. Also keep in mind that the animation will restart from the
  beginning if the icon properties, e.g. the size are changed. In this
  case the started signal will also be emitted again and this is not
  an error, it is normal.

  \sa startAnimation()
*/

/*!
  \fn void HbIconAnimator::animationStopped()
  
  This signal is emitted whenever the icon animation is stopped (by calling
  stopAnimation()) before the animation has finished.
  
  \sa stopAnimation()
*/

/*!
  \fn void HbIconAnimator::animationProgressed()
  
  This signal is emitted whenever the icon animation has progressed and the
  icon should be repainted to the screen.
*/

/*!
  \fn void HbIconAnimator::animationFinished()
  
  This signal is emitted whenever the icon animation is finished (and there was
  no stopAnimation() call). This can be used e.g. to synchronize subsequent icon
  animations. If looping is enabled then this signal is never emitted.
*/

/*!
  Creates an icon animator instance.
*/
HbIconAnimator::HbIconAnimator()
    : d(new HbIconAnimatorPrivate)
{
}

/*!
  Creates an icon animator instance and sets given graphics \a item in it.
*/
HbIconAnimator::HbIconAnimator(QGraphicsItem *item)
    : d(new HbIconAnimatorPrivate)
{
    setGraphicsItem(item);
}

/*!
  Destroys the icon animator instance.
*/
HbIconAnimator::~HbIconAnimator()
{
    if (d && d->icon.d->engine) {
        d->icon.d->engine->setAnimator(0);
    }
    delete d;
}

/*!
  Destroys the private instance.
 */
HbIconAnimatorPrivate::~HbIconAnimatorPrivate()
{
    delete animation;
}

/*!
  Returns the icon that has been set to the animator.
  The icon parameters can be changed and then updated in the animator
  with method HbIconAnimator::setIcon().

  \sa HbIconAnimator::setIcon()
*/
HbIcon HbIconAnimator::icon() const
{
    return d->icon;
}

/*!
  Sets the \a icon to the animator. If the icon is not animated, the
  animator draws it normally.

  \sa HbIconAnimator::icon()
*/
void HbIconAnimator::setIcon(const HbIcon &icon)
{
    // Delete animation if such parameters have changed which cannot be updated
    // in the animation dynamically or if the given icon is NULL.
    bool needToRecreateAnimation = false;

    if (d->animation) {
        // Mirroring change cannot be applied in FrameSet animation dynamically because
        // it may have different frame icons for the mirrored version.
        if (icon.mirroringMode() != d->icon.mirroringMode()) {
            if (d->animation->type() == HbIconAnimation::FrameSet) {
                needToRecreateAnimation = true;
            } else {
                // Update current mirroring in the animation
                d->animation->setMirrored(icon.d->engine->isMirrored());
            }
        }
        else if ((static_cast<int>(icon.flags()) & HbIcon::ResolutionCorrected) !=
                 (static_cast<int>(d->icon.flags()) & HbIcon::ResolutionCorrected) ||
                  icon.isNull()) {
            needToRecreateAnimation = true;
        }
    }

    if (needToRecreateAnimation) {
        delete d->animation;
        d->animation = 0;
    }

    d->icon = icon;

    if (d->icon.d->engine) {
        // The icon's private object could be detached here before setting the animator in its engine.
        // Without doing that, the HbIcon instance given to this method also starts to paint the animation frames
        // if the caller of this method tries to paint it.
        // On the other hand that causes a performance hit, so not doing that unless this is causing some problems.
        d->icon.d->engine->setAnimator(this);
    }
}

/*!
    Sets the graphics item which is automatically updated whenever the icon animation has progressed.
    This is a more light-weight way of handling icon animation updates than using the animationProgressed
    signal but the user must make sure that the given graphics item is not destroyed before the HbIcon
    instance owning this animator is destroyed.
*/
void HbIconAnimator::setGraphicsItem(QGraphicsItem *item)
{
    if (item != d->item) {
        d->item = item;

        // If there is animation already running, it may have resolved its owner view
        // based on the previously set graphics item (see HbIconAnimationManagerPrivate::animPlaying)
        // so in theory this graphics item might belong to other view, which needs to be updated in the animation.
        // Clearing the view information in the animation here so it gets updated next time the animation is started.
        if (d->animation && !d->view) {
            d->animation->setView(0);
        }
    }
}

/*!
    Paints the animated icon. This is a convenience method and does the same as calling paint() to the
    HbIcon instance returned by method HbIconAnimator::icon().
*/
void HbIconAnimator::paint(
    QPainter *painter,
    const QRectF &rect,
    Qt::AspectRatioMode aspectRatioMode,
    Qt::Alignment alignment,
    QIcon::Mode mode,
    QIcon::State state) const
{
    d->icon.paint(painter, rect, aspectRatioMode, alignment, mode, state);
}

/*!
* Starts the icon animation if the icon is animated. Otherwise this method has no effect.
* If the animation is already running or finished, it is restarted from the beginning.
* \sa flag HbIcon::NoAutoStartAnimation
*/
void HbIconAnimator::startAnimation()
{
    d->started = true;
    if (d->animation) {
        d->animation->start();
    }
}

/*!
* Stops the icon animation if the icon is animated. Otherwise this method has no effect.
* The icon shows the last frame of the animation.
*/
void HbIconAnimator::stopAnimation()
{
    d->started = false;
    if (d->animation) {
        d->animation->stop();
    }
}

/**
* Sets the \a view showing the animated icon that this animator instance is associated with.
* The animation is then paused automatically when the view is deactivated and resumed when it is activated.
* If the owner view is not set the animation does not care about view switches.
*
* \note Method HbIconAnimator::setGraphicsItem goes through the parent chain of the given graphics item
* and sets the owner view automatically if it finds a HbView instance in it. If that method is used,
* there is no need to call this method explicitly.
*/
void HbIconAnimator::setOwnerView(HbView *view)
{
    if (d->view != view) {
        d->view = view;

        // If there is existing animation object, update the owner view also in it.
        if (d->animation) {
            d->animation->setView(view);
        }
    }
}

// End of File
