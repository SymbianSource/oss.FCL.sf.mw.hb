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

#include <hbstyleoptionnotificationdialog_p.h>
#include <hbdeviceprofile.h>

#include "hbnotificationdialogcontent_p.h"

HbNotificationDialogContent::HbNotificationDialogContent(QGraphicsWidget *parent) :
    HbWidget(parent), mTitleTextWrapping(Hb::TextNoWrap),
    mIsTouchActivating(false), mTextItem(0), mTitleItem(0), mIconItem(0)
{
}

void HbNotificationDialogContent::setIcon(const HbIcon &icon)
{
    delete mIconItem;
    mIconItem = 0;

    if (!icon.isNull()) {    
        mIconItem = style()->createPrimitive(
                HbStyle::P_NotificationDialog_icon, this);
        Q_CHECK_PTR(mIconItem);
    }
    mIcon = icon;
    updatePrimitives();
    if (isVisible()) {
        polishEvent();
    }
}

void HbNotificationDialogContent::setText(const QString &text)
{
    if (!mTextItem) {
        mTextItem = style()->createPrimitive(
                HbStyle::P_NotificationDialog_text, this);
        Q_CHECK_PTR(mTextItem);
    }
    mText = text;
    updatePrimitives();
    if (isVisible()) {
        polishEvent();
    }
}

void HbNotificationDialogContent::setTitle(const QString &title)
{
    if (!mTitleItem) {
        mTitleItem = style()->createPrimitive(
                HbStyle::P_NotificationDialog_title, this);
        Q_CHECK_PTR(mTitleItem);
    }

    mTitle = title;
    updatePrimitives();
    if (isVisible()) {
        polishEvent();
    }
}

void HbNotificationDialogContent::setTitleTextWrapping(Hb::TextWrapping wrapping)
{
    if (mTitleTextWrapping != wrapping) {
        mTitleTextWrapping = wrapping;
        updatePrimitives();
        if (isVisible()) {
            polishEvent();
        }
    }
}

Hb::TextWrapping HbNotificationDialogContent::titleTextWrapping() const
{
    return mTitleTextWrapping;
}

void HbNotificationDialogContent::enableTouchActivation(bool enabled)
{
    if (mIsTouchActivating != enabled) {
        mIsTouchActivating = enabled;
        updatePrimitives();
    }
}

bool HbNotificationDialogContent::isTouchActivating() const
{
    return mIsTouchActivating;
}

void HbNotificationDialogContent::polish( HbStyleParameters& params )
{
    HbStyleOptionNotificationDialog option;
    initStyleOption(&option);
    int textFields(0);
    bool iconSet(!option.icon.isNull());

    // Check that content is filled properly
    if (!mTitleItem) {
        qWarning("HbNotificationDialog Warning: Title not set");
        setTitle(QString(""));
    }

    if (option.title.isEmpty()) {
        if (!option.text.isEmpty()) {
            textFields = 2; //secondary text (uses:  primary + secondary text.)
        } else if (iconSet) {
            textFields = 1;
        }
    } else if (option.text.isEmpty()) {
        textFields = 1; // primary text
    } else {
        textFields = 2; // primary + secondary text
    }
    setProperty("icon", iconSet);
    setProperty("textFields", textFields);
    setProperty("link", option.isLink);
    setProperty("titleWrapping", (option.titleTextWrapping != Hb::TextNoWrap));
    HbWidget::polish(params);
}

void HbNotificationDialogContent::initStyleOption(HbStyleOptionNotificationDialog *option) const
{
    HbWidget::initStyleOption(option);
    option->title = mTitle;
    option->text = mText;
    option->icon = mIcon;
    option->textTextWrapping = Hb::TextNoWrap;
    option->titleTextWrapping = mTitleTextWrapping;
    option->isLink = mIsTouchActivating;
}

QSizeF HbNotificationDialogContent::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    switch (which) {
    case Qt::PreferredSize: {
        QSizeF preferred = HbWidget::sizeHint(which, constraint);
        HbDeviceProfile dp(HbDeviceProfile::profile(this));
        int widthInPixels = (dp.orientation() == Qt::Vertical)
                            ? dp.logicalSize().width()
                            : dp.logicalSize().height();

        //width is always portrait screen width - 9un.
        preferred.setWidth(widthInPixels - dp.unitValue() * 9 );
        return preferred;
    }
    default:
        break;
    }
    return HbWidget::sizeHint(which, constraint);
}

void HbNotificationDialogContent::updatePrimitives()
{
    HbStyleOptionNotificationDialog option;
    initStyleOption(&option);

    if (mTextItem) {
        style()->updatePrimitive(mTextItem,
            HbStyle::P_NotificationDialog_text, &option);
    }
    if (mTitleItem) {
        style()->updatePrimitive(mTitleItem,
            HbStyle::P_NotificationDialog_title, &option);
    }
    if (!mIcon.isNull()) {
        style()->updatePrimitive(mIconItem,
            HbStyle::P_NotificationDialog_icon, &option);
    }
}

/*!
    \reimp
*/
QGraphicsItem *HbNotificationDialogContent::primitive(const QString &itemName) const
{
    if (itemName == "") {
        return 0;
    } else {
        if (itemName == "text") {
            return mTextItem;
        } else if (itemName == "title") {
            return mTitleItem;
        } else if (itemName == "icon") {
            return mIconItem;
        } else {
            return 0;
        }
    }
}
