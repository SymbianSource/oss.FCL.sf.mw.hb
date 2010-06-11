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

#include <hbinputvirtualkeyboard.h>
#include <hbinputmethod.h>

#include "hbwidget.h"
#include "hbmainwindow.h"
#include "hbmainwindow_p.h"

/*!
\proto
\class HbShrinkingVkbHost
\brief A virtual keyboard host that doesn't move the active mainwindow view but shrinks it.

The default virtual keyboard host moves the editor container widget in order to keep the
cursor line visible. In some situations that doesn't work and the container should be shrunk
and relayouted instead.

This virtual keyboard host does that. It works with editors that live inside in main window's
active view and shrinks the view instead of moving it around when the virtual keyboard comes up.
*/

/// @cond

class HbShrinkingVkbHostPrivate : public HbAbstractVkbHostPrivate
{
    Q_DECLARE_PUBLIC(HbShrinkingVkbHost)

public:
    HbShrinkingVkbHostPrivate(HbAbstractVkbHost *myHost, HbWidget *widget);
    bool prepareContainerAnimation(HbVkbHost::HbVkbStatus status);
    void closeKeypad();
    void closeKeypadWithoutAnimation();
    void openKeypadWithoutAnimation();
    void minimizeKeypadWithoutAnimation();

    void shrinkView();
    void resetViewSize();

public:
    QSizeF mContainerOriginalSize;
};

HbShrinkingVkbHostPrivate::HbShrinkingVkbHostPrivate(HbAbstractVkbHost *myHost, HbWidget *widget)
: HbAbstractVkbHostPrivate(myHost, widget)
{
}

bool HbShrinkingVkbHostPrivate::prepareContainerAnimation(HbVkbHost::HbVkbStatus status)
{
    Q_UNUSED(status);

    // This host doesn't move the container, only the keypad.
    return false;
}

void HbShrinkingVkbHostPrivate::closeKeypad()
{ 
    resetViewSize();
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

void HbShrinkingVkbHostPrivate::minimizeKeypadWithoutAnimation()
{
    HbAbstractVkbHostPrivate::minimizeKeypadWithoutAnimation();
    shrinkView();
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
        HbMainWindowPrivate::d_ptr(mainWin)->setViewportSize(q->applicationArea().size());
    }
}

/// @endcond

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
int HbShrinkingVkbHost::priority() const
{   
    return 0;
}

/*!
\reimp
*/
void HbShrinkingVkbHost::animationFinished()
{
    Q_D(HbShrinkingVkbHost);


    HbAbstractVkbHost::animationFinished();

    if (d->mKeypadStatus == HbVkbHost::HbVkbStatusOpened ||
        d->mKeypadStatus == HbVkbHost::HbVkbStatusMinimized) {
        d->shrinkView();
    }
}

// End of file

