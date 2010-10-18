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

#include "hbemptyviewwidget_p.h"
#include "hbstyletextprimitivedata.h"
#include "hbtextitem.h"
#include "hbeffect.h"

#include <HbVkbHostBridge>
#include <QGraphicsSceneResizeEvent>

HbEmptyViewWidget::HbEmptyViewWidget(QGraphicsItem *parent) :
    HbWidget(parent),
    mTextItem(0)
{
    if (parent) {
        if (parent->isWidget()) {
            static_cast<QGraphicsWidget *>(parent)->installEventFilter(this);
        }

        resize(parent->boundingRect().size());
    }

    mTextItem = style()->createPrimitive(HbStyle::PT_TextItem, QLatin1String("text"), this);
    setEmptyViewText(mText);

    connect(HbVkbHostBridge::instance(), SIGNAL(aboutToOpen ()), this, SLOT(vkbStateAboutToChange()));
    connect(HbVkbHostBridge::instance(), SIGNAL(aboutToClose ()), this, SLOT(vkbStateAboutToChange()));
    connect(HbVkbHostBridge::instance(), SIGNAL(keypadOpened ()), this, SLOT(vkbStateChanged()));
    connect(HbVkbHostBridge::instance(), SIGNAL(keypadClosed ()), this, SLOT(vkbStateChanged()));
}

HbEmptyViewWidget::~HbEmptyViewWidget()
{
}

void HbEmptyViewWidget::setEmptyViewText(const QString &emptyViewText)
{
    mText = emptyViewText;
    HbStyleTextPrimitiveData textPrimitiveData;
    HbWidgetBase::initPrimitiveData(&textPrimitiveData, mTextItem);
    textPrimitiveData.text = emptyViewText;
    textPrimitiveData.alignment = Qt::AlignCenter;
    textPrimitiveData.textWrapping = Hb::TextNoWrap;
    textPrimitiveData.minimumLines = 1;
    textPrimitiveData.maximumLines = 1;
    style()->updatePrimitive(mTextItem,&textPrimitiveData,this);

}

QString HbEmptyViewWidget::emptyViewText()
{
    return mText;
}

bool HbEmptyViewWidget::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);

    if (event->type() == QEvent::GraphicsSceneResize) {
        QGraphicsSceneResizeEvent *resizeEvent = static_cast<QGraphicsSceneResizeEvent *>(event);
        resize(resizeEvent->newSize());
    }

    return false;
}

bool HbEmptyViewWidget::event(QEvent *event)
{
    if (event->type() == QEvent::ParentAboutToChange) {
        QGraphicsItem *parent = parentItem();
        if (parent && parent->isWidget()) {
            static_cast<QGraphicsWidget *>(parent)->removeEventFilter(this);
        }
    } else if (event->type() == QEvent::ParentChange) {
        QGraphicsItem *parent = parentItem();
        if (parent && parent->isWidget()) {
            static_cast<QGraphicsWidget *>(parent)->installEventFilter(this);
        }
    }

    return HbWidget::event(event);
}

void HbEmptyViewWidget::vkbStateChanged()
{
    if (isVisible()) {
        repolish();
        mTextItem->show();
    }
}

void HbEmptyViewWidget::vkbStateAboutToChange()
{
    if (isVisible()) {
        mTextItem->hide();
    }
}

void HbEmptyViewWidget::polish(HbStyleParameters& params)
{
    
    HbVkbHost *vkbHost = HbVkbHost::activeVkbHost();
    if (vkbHost && vkbHost->keypadStatus() == HbVkbHost::HbVkbStatusOpened) {
        setProperty("virtualKeyboardOpen", true);
    } else {
        setProperty("virtualKeyboardOpen", false);
    }
    
    HbWidget::polish(params);
}

