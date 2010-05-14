/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbInput module of the UI Extensions for Mobile.
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

#include <QGraphicsGridLayout>

#include <hbaction.h>
#include <hbinputmethod.h>
#include <hbinpututils.h>
#include <hbinputvkbhost.h>
#include <hbinputsettingproxy.h>
#include <hbinputcommondialogs.h>
#include <hbmainwindow.h>
#include <hbview.h>
#include <hbdataform.h>

#include "hbinputvkbwidget_p.h"
#include "hbinputhwtoolcluster.h"
#include "hbinputmodeindicator.h"

const QString HbCustomButtonObjName = "Mini VKB custom button ";

const int HbMiniVirtualKeypadNumberOfColumn = 5;
const qreal HbMiniVirtualKeypadReductionFactor = 4.5;

/*!
@proto
@hbinput
\class HbHwToolCluster
\brief Implementation of hardware tool cluster.

Implementation of hardware tool cluster.

\sa HbHwToolCluster
\sa HbTouchKeypadButton
*/

/// @cond

class HbHwToolClusterPrivate : HbInputVkbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbHwToolCluster)

public:
    HbHwToolClusterPrivate();
    ~HbHwToolClusterPrivate();
public:
};

HbHwToolClusterPrivate::HbHwToolClusterPrivate()
{
}

HbHwToolClusterPrivate::~HbHwToolClusterPrivate()
{
}

/// @endcond

/*!
Constructs the object.
*/
HbHwToolCluster::HbHwToolCluster(HbInputMethod* owner,
                                       QGraphicsItem* parent)
                                       : HbInputVkbWidget(*new HbHwToolClusterPrivate, parent)
{
    if (0 == owner) {
        return;
    }
    Q_D(HbHwToolCluster);
    d->q_ptr = this;
    d->mOwner = owner;
}

/*!
Creates the layout of the mini touch keypad for hardware qwerty.
*/
void HbHwToolCluster::createLayout()
{
}


/*!
Creates the tools cluster for mini VKB layout.
*/
void HbHwToolCluster::setupToolCluster()
{
}

/*!
Returns keyboard type.
*/
HbKeyboardType HbHwToolCluster::keyboardType() const
{
    return HbKeyboardQwerty;
}

/*!
Destructs the object.
*/
HbHwToolCluster::~HbHwToolCluster()
{
}

/*!
This is called right before the keypad is about to open. The layout of the
keypad happens here.
*/
void HbHwToolCluster::aboutToOpen(HbVkbHost *host)
{
    Q_D(HbHwToolCluster);
    HbInputVkbWidget::aboutToOpen(host);

    QSizeF size = preferredKeyboardSize();
    qreal height = size.height() - HbCloseHandleHeight;

    qreal width = size.width() / (qreal)HbMiniVirtualKeypadNumberOfColumn;
    for (int i=0; i < HbMiniVirtualKeypadNumberOfColumn ;i++) {
        d->mButtonLayout->setColumnFixedWidth(i, width);
    }
    //There is only one row
    d->mButtonLayout->setRowFixedHeight(0, height);
}

/*!
Returns preferred keyboard size. HbVkbHost uses this information when it opens the keyboard.
*/
QSizeF HbHwToolCluster::preferredKeyboardSize()
{
    Q_D(HbHwToolCluster);
    QSizeF ret = QSizeF(0.0, 0.0);

    if (d->mCurrentHost) {
        QSizeF ret;
        //Get the available keypad area from the VKB host
        ret = d->mCurrentHost->keyboardArea();
        //Since this is a mini VKB, it has a lesser size than the available
        //area for the keypad.
        ret.setHeight(ret.height()/HbMiniVirtualKeypadReductionFactor);
        return ret;
    }

    return ret;
}

/*!
Shows the input method selection dialog.
*/
void HbHwToolCluster::showMethodDialog()
{
    Q_D(HbHwToolCluster);

    HbInputMethodDescriptor method
        = HbInputCommonDialogs::showCustomInputMethodSelectionDialog(HbInputSettingProxy::instance()->globalInputLanguage());
    if (!method.isEmpty() && d->mOwner) {
        d->mOwner->activateInputMethod(method);
    }
}

/*!
Shows the language selection dialog.
*/
void HbHwToolCluster::showLanguageDialog()
{
    HbInputLanguage language =
        HbInputCommonDialogs::showLanguageSelectionDialog(HbInputSettingProxy::instance()->globalInputLanguage().language());
    HbInputSettingProxy::instance()->setGlobalInputLanguage(language);
}

// End of file

