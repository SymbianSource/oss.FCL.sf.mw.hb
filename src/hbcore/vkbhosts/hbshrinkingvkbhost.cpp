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
#include "hbshrinkingvkbhost.h"
#include "hbabstractvkbhost_p.h"
#include "hbvkbhostcontainerwidget_p.h"

#include "hbinputvirtualkeyboard.h"
#include "hbinputmethod.h"

#include "hbwidget.h"
#include "hbmainwindow.h"
#include "hbmainwindow_p.h"

/*!
\proto
@hbcore
\class HbShrinkingVkbHost
\brief A virtual keyboard host that doesn't move the active mainwindow view but shrinks it.

The default virtual keyboard host moves the editor container widget in order to keep the
cursor line visible. In some situations that doesn't work and the container should be shrunk
and relayouted instead.

The shrinking virtual keyboard host does that. It works with editors that live inside in main window's
active view and shrinks the view instead of moving it around when the virtual keyboard comes up.

See \ref vkbHandling "virtual keyboard handling guide" for more information

\sa HbVkbHost
\sa HbAbstractVkbHost
\sa HbStaticVkbHost
*/

class HbShrinkingVkbHostPrivate : public HbAbstractVkbHostPrivate
{
    Q_DECLARE_PUBLIC(HbShrinkingVkbHost)

public:
    HbShrinkingVkbHostPrivate(HbAbstractVkbHost *myHost, HbWidget *widget);
    ~HbShrinkingVkbHostPrivate();
    bool prepareContainerAnimation(HbVkbHost::HbVkbStatus status);
    void closeKeypad();
    void closeKeypadWithoutAnimation();
    void openKeypadWithoutAnimation();
    void cancelAnimationAndHideVkbWidget();

    void shrinkView();
    void resetViewSize();

public:
    QSizeF mContainerOriginalSize;
    QSizeF mKeyboardSize;
    QSizeF mActiveViewSize;
};

HbShrinkingVkbHostPrivate::HbShrinkingVkbHostPrivate(HbAbstractVkbHost *myHost, HbWidget *widget)
    : HbAbstractVkbHostPrivate(myHost, widget)
{
}

HbShrinkingVkbHostPrivate::~HbShrinkingVkbHostPrivate()
{
    resetViewSize();
}

bool HbShrinkingVkbHostPrivate::prepareContainerAnimation(HbVkbHost::HbVkbStatus status)
{
    if (status == HbVkbHost::HbVkbStatusOpened) {
        if (mContainerWidget &&
            (mKeypadStatus == HbVkbHost::HbVkbStatusClosed)) {
            mContainerMovementVector = mContainerWidget->fixedContainerMovement();
            return true;
        }
    } else if (status == HbVkbHost::HbVkbStatusClosed) {
        if (mContainerMovementStartingPoint != mOriginalContainerPosition) {
           mContainerMovementVector = mOriginalContainerPosition - mContainerMovementStartingPoint;
           return true;
        }
    }

    return false;
}

void HbShrinkingVkbHostPrivate::closeKeypad()
{
    HbAbstractVkbHostPrivate::closeKeypad();
}

void HbShrinkingVkbHostPrivate::closeKeypadWithoutAnimation()
{
    resetViewSize();
    HbAbstractVkbHostPrivate::closeKeypadWithoutAnimation();
}

void HbShrinkingVkbHostPrivate::openKeypadWithoutAnimation()
{
    HbAbstractVkbHostPrivate::openKeypadWithoutAnimation();
    shrinkView();
}

void HbShrinkingVkbHostPrivate::cancelAnimationAndHideVkbWidget()
{
    resetViewSize();
    HbAbstractVkbHostPrivate::cancelAnimationAndHideVkbWidget();
}

void HbShrinkingVkbHostPrivate::resetViewSize()
{
    HbMainWindow *mainWin = mainWindow();
    if (mainWin && mContainerOriginalSize.isValid()) {
        HbMainWindowPrivate::d_ptr(mainWin)->setViewportSize(mContainerOriginalSize);
        mContainerOriginalSize = QSizeF();
    }
}

void HbShrinkingVkbHostPrivate::shrinkView()
{
    Q_Q(HbShrinkingVkbHost);

    HbMainWindow *mainWin = mainWindow();
    if (mainWin) {
        if (!mContainerOriginalSize.isValid()) {
            mContainerOriginalSize = HbMainWindowPrivate::d_ptr(mainWin)->viewPortSize();
        }
        if (mTimeLine.state() != QTimeLine::Running) {
            QSizeF newViewportSize = q->applicationArea().size();
            newViewportSize.setHeight(newViewportSize.height() - mContainerWidget->fixedContainerMovement().y());
            HbMainWindowPrivate::d_ptr(mainWin)->setViewportSize(newViewportSize);
        } else {
            HbMainWindowPrivate::d_ptr(mainWin)->setViewportSize(mActiveViewSize);
        }
    }
}

/*!
Constructs the object.
*/
HbShrinkingVkbHost::HbShrinkingVkbHost(HbWidget *widget) : HbAbstractVkbHost(new HbShrinkingVkbHostPrivate(this, widget))
{
    setParent(widget);
}

/*!
Destructs the object.
*/
HbShrinkingVkbHost::~HbShrinkingVkbHost()
{
}

/*!
\reimp
*/
void HbShrinkingVkbHost::animValueChanged(qreal value)
{
    Q_D(HbShrinkingVkbHost);

    HbAbstractVkbHost::animValueChanged(value);

    QSizeF vpSize = d->screenSize();
    QRectF viewport = QRectF(QPointF(0.0, 0.0), QPointF(vpSize.width(), vpSize.height()));

    if (!d->mKeyboardSize.isValid()) {
        d->mKeyboardSize = confirmedKeyboardSize();
    }
    
    // Calculate available space above the keyboard
    if (d->mKeypadStatus == HbVkbHost::HbVkbStatusOpened) {
        viewport.setHeight(viewport.height() - d->mKeyboardSize.height() * value);
    } else {
        viewport.setHeight(viewport.height() - d->mKeyboardSize.height() * (1 - value));
    }

    if (d->mContainerWidget) {
        viewport.setHeight(viewport.height() - d->mContainerWidget->fixedContainerMovement().y());
    }

    d->mActiveViewSize = viewport.size();

    d->shrinkView();
    d->ensureVisibilityInsideScrollArea();
}

/*!
\reimp
*/
void HbShrinkingVkbHost::animationFinished()
{
    Q_D(HbShrinkingVkbHost);

    HbAbstractVkbHost::animationFinished();

    if (d->mKeypadStatus == HbVkbHost::HbVkbStatusOpened) {
        d->shrinkView();
    } else {
        d->resetViewSize();
    }

    d->mKeyboardSize = QSizeF();
    d->ensureVisibilityInsideScrollArea();
}

// End of file
