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

#include <hbeffect.h>
#include <hbmainwindow.h>
#include <hbview.h>
#include <hbdevicedialog.h>
#include <hbaction.h>
#include <hbiconanimationmanager.h>
#include <hbiconitem.h>
#include <hbiconanimator.h>

#include "hbindicatorbutton_p.h"
#include "hbindicatorbutton_p_p.h"
#include "hbstyleoptionindicatorbutton_p.h"

#if defined(Q_OS_SYMBIAN)
#include <e32property.h>
#include "hbindicatorsym_p.h"
#include "hbdevicedialogsym_p.h"
const TUid PropertyCategoryUid = {0x20022FC5};
const TUint EventKey = 0x6e457674; // 'nEvt';
#else
#include "hbindicatorwin32_p.h"
#endif // defined(Q_OS_SYMBIAN)

static const char noteIndicatorType[] = {"com.nokia.hb.indicatormenu/1.0"};

#if defined(Q_OS_SYMBIAN)

class HbIndicatorButtonStatusPrivate  : public CActive
{
public:
    HbIndicatorButtonStatusPrivate(HbIndicatorButtonPrivate* indicatorButtonPrivate);
    ~HbIndicatorButtonStatusPrivate();

public: // from CActive
    void RunL();
    void DoCancel();

private:
    HbIndicatorButtonPrivate* mIndicatorButtonPrivate;
    RProperty mProperty;
};

HbIndicatorButtonStatusPrivate::HbIndicatorButtonStatusPrivate(
    HbIndicatorButtonPrivate* indicatorButtonPrivate) : CActive(CActive::EPriorityStandard),
    mIndicatorButtonPrivate(indicatorButtonPrivate)
{
    CActiveScheduler::Add(this);
    iStatus = KRequestPending;
    mProperty.Attach(PropertyCategoryUid, EventKey);
    mProperty.Subscribe(iStatus);
    SetActive();
}

HbIndicatorButtonStatusPrivate::~HbIndicatorButtonStatusPrivate()
{
    Cancel();
    mProperty.Close();
}

void HbIndicatorButtonStatusPrivate::RunL()
{
    int newValue(0);
    if (mProperty.Get(newValue) == KErrNone) {
        mIndicatorButtonPrivate->setNewEvent(newValue == 0 ? false : true);
    }
    mProperty.Subscribe(iStatus);
    SetActive();
}

void HbIndicatorButtonStatusPrivate::DoCancel()
{
    mProperty.Cancel();
}

#endif // defined(Q_OS_SYMBIAN)

HbIndicatorButtonPrivate::HbIndicatorButtonPrivate() :
    mHandleIcon(0), mDefaultAction(0), mNewEventAction(0), mProgressAction(0), mDeviceDialog(0), 
    mProgressAnimationFound(false), mNewEventIcon(0), mNewEvent(false), mStyle(0), mIndicatorMenuOpen(false),
    mTouchArea(0)
#if defined(Q_OS_SYMBIAN)
    ,mIndicatorButtonStatusPrivate(0)
#endif // defined(Q_OS_SYMBIAN)
{

}

HbIndicatorButtonPrivate::~HbIndicatorButtonPrivate()
{
    delete mDeviceDialog;

#if defined(Q_OS_SYMBIAN)    
    delete mIndicatorButtonStatusPrivate;
#endif // defined(Q_OS_SYMBIAN)
}

void HbIndicatorButtonPrivate::init()
{
    Q_Q(HbIndicatorButton);
    setBackgroundVisible(false);
    mProgressAnimationFound = HbIconAnimationManager::global()->addDefinitionFile(
        "qtg_anim_mono_loading.axml");
    
    // add default actions
    mDefaultAction = new HbAction(HbIcon("qtg_mono_options_menu"), "", q);
    mNewEventAction = new HbAction(HbIcon("qtg_mono_new_event"), "", q);

    QString iconName("qtg_anim_mono_loading_1");
    if (mProgressAnimationFound) {
        iconName = "qtg_anim_mono_loading";
    }
    HbIcon icon(iconName);
    icon.setFlags(HbIcon::Colorized);
    mProgressAction = new HbAction(icon, "", q);
}

void HbIndicatorButtonPrivate::showIndicatorMenu()
{
    QVariantMap parametersMap;
    QString noteType(noteIndicatorType);

    parametersMap.clear();
    mDeviceDialog->show(noteType, parametersMap);
    mNewEvent = false;
    mIndicatorMenuOpen = true;

    updateIcon();
   
#if defined(Q_OS_SYMBIAN) 
    RProperty::Set(PropertyCategoryUid, EventKey, 0);
#endif // defined(Q_OS_SYMBIAN)
}

void HbIndicatorButtonPrivate::addIndicators(const QList<IndicatorClientInfo> &clientInfo)
{
    for (int i = 0; i < clientInfo.size(); ++i) {
        if (clientInfo.at(i).hasMenuData 
            || clientInfo.at(i).category == HbIndicatorInterface::ProgressCategory) {
            mIndicators.prepend(clientInfo.at(i));
            if (clientInfo.at(i).category == HbIndicatorInterface::NotificationCategory) {
                mNewEvent = true;
            }
        }
    }

    updateIcon();
}

void HbIndicatorButtonPrivate::removeIndicators(const QList<IndicatorClientInfo> &clientInfo)
{
    for (int i = 0; i < clientInfo.size(); ++i) {
        int index = findIndicator(clientInfo.at(i));
        if (index >= 0) {
            mIndicators.removeAt(index);
        }
    }

    updateIcon();
}

void HbIndicatorButtonPrivate::updateIndicators(const QList<IndicatorClientInfo> &clientInfo)
{
    for (int i = 0; i < clientInfo.size(); ++i) {
        int index = findIndicator(clientInfo.at(i));
        if (index >= 0) {
            if (mIndicators[index].category == HbIndicatorInterface::NotificationCategory) {
                mNewEvent = true;
            }
        }
    }

    updateIcon();
}

int HbIndicatorButtonPrivate::findIndicator(const IndicatorClientInfo &indicator) const
{
    int index = -1;
    for (int i = 0; i < mIndicators.size(); ++i) {
        if (mIndicators.at(i).type == indicator.type) {
            index = i;
            break;
        }
    }
    return index;
}

void HbIndicatorButtonPrivate::updateIcon()
{
    Q_Q(HbIndicatorButton);

    setStyle();
    switch (mStyle)
    {
    case 0:
        q->setProperty("layout_type", 1);
        q->setAction(mDefaultAction);
        break;
    case 1:
        q->setProperty("layout_type", 1);
        q->setAction(mNewEventAction);
        break;
    case 2:
        q->setProperty("layout_type", 1);
        q->setAction(mProgressAction);
        break;
    case 3:
        q->setProperty("layout_type", 2);
        q->setAction(mProgressAction);
        break;
    default:
        q->setProperty("layout_type", 1);
        q->setAction(mDefaultAction);
        break;
    }
    q->repolish();
}

void HbIndicatorButtonPrivate::setStyle()
{
    bool newEvent(false);
    bool progress(false);
    for (int i = 0; i < mIndicators.size(); ++i) {
        if (mIndicators.at(i).category == HbIndicatorInterface::NotificationCategory && mNewEvent) {
            newEvent = true;
        }
        if (mIndicators.at(i).category == HbIndicatorInterface::ProgressCategory) {
            progress = true;
        }
    }

    if (!newEvent && !progress) {
        mStyle = 0;
    } else if (newEvent && !progress){
        mStyle = 1;
    } else if (!newEvent && progress){
        mStyle = 2;
    } else {
        mStyle = 3;
    }
}

void HbIndicatorButtonPrivate::setNewEvent(bool newEvent)
{
    mNewEvent = newEvent;
    updateIcon();
}

HbIndicatorButton::HbIndicatorButton(QGraphicsItem *parent) 
    : HbToolButton(*new HbIndicatorButtonPrivate, parent)
{
    Q_D(HbIndicatorButton);
    setProperty("layout_type", 1);
    d->init(); 

    setAction(d->mDefaultAction);

    createPrimitives();
}

HbIndicatorButton::~HbIndicatorButton()
{

}

void HbIndicatorButton::delayedConstruction()
{
    Q_D(HbIndicatorButton);

    connect(this, SIGNAL(pressed()), this, SLOT(handlePress()));
    connect(this, SIGNAL(released()), this, SLOT(handleRelease()));
    
    
    d->mDeviceDialog = new HbDeviceDialog();
#if defined(Q_OS_SYMBIAN)
    ((HbDeviceDialogPrivate*)HbDeviceDialogPrivate::d_ptr(d->mDeviceDialog))->connect();
#endif
    connect(d->mDeviceDialog, SIGNAL(deviceDialogClosed()), this, SLOT(resetBackground()));

    connect(d->mDefaultAction, SIGNAL(triggered()), this, SLOT(showIndicatorMenu()));
    connect(d->mNewEventAction, SIGNAL(triggered()), this, SLOT(showIndicatorMenu()));
    connect(d->mProgressAction, SIGNAL(triggered()), this, SLOT(showIndicatorMenu()));
   
#if defined(Q_OS_SYMBIAN)
    d->mIndicatorButtonStatusPrivate = new HbIndicatorButtonStatusPrivate(d);
#endif // defined(Q_OS_SYMBIAN)
}

void HbIndicatorButton::showHandleIndication(bool show)
{
    Q_D(HbIndicatorButton);
    d->mHandleIcon->setVisible(show);
}

bool HbIndicatorButton::handleVisible() const
{
    bool handleVisible = false;
    if (mainWindow() && mainWindow()->currentView()) {
        handleVisible = mainWindow()->currentView()->viewFlags() & HbView::ViewTitleBarMinimizable;
    }
    return handleVisible;
}

int HbIndicatorButton::buttonStyle() const
{
    Q_D(const HbIndicatorButton);
    return d->mStyle;
}

void HbIndicatorButton::currentViewChanged(HbView *view)
{
    Q_D(HbIndicatorButton);
    HbIconItem *item = qgraphicsitem_cast<HbIconItem *>(d->iconItem);
    if (item) {
        item->animator().setOwnerView(view);
    }
}

void HbIndicatorButton::createPrimitives()
{
    Q_D(HbIndicatorButton);
    d->mHandleIcon = HbStylePrivate::createPrimitive(HbStylePrivate::P_IndicatorButton_handleindication, this);
    d->mHandleIcon->setVisible(false);
    d->mNewEventIcon = HbStylePrivate::createPrimitive(HbStylePrivate::P_IndicatorButton_eventindication, this);
    d->mTouchArea = HbStylePrivate::createPrimitive(HbStylePrivate::P_IndicatorButton_toucharea, this);
    ungrabGesture(Qt::TapGesture);
    d->setBackgroundItem(HbStylePrivate::P_IndicatorButton_background);
}

void HbIndicatorButton::updatePrimitives()
{
    Q_D(HbIndicatorButton);
    HbStyleOptionIndicatorButton option;
    initStyleOption(&option);
    HbStylePrivate::updatePrimitive(backgroundItem(), HbStylePrivate::P_IndicatorButton_background, &option);
    HbStylePrivate::updatePrimitive(d->mHandleIcon, HbStylePrivate::P_IndicatorButton_handleindication, &option);
    HbStylePrivate::updatePrimitive(d->mNewEventIcon, HbStylePrivate::P_IndicatorButton_eventindication, &option);
    HbStylePrivate::updatePrimitive(d->mTouchArea, HbStylePrivate::P_IndicatorButton_toucharea, &option);
    HbToolButton::updatePrimitives();
}

void HbIndicatorButton::activate(const QList<IndicatorClientInfo> &clientInfo)
{
    Q_D(HbIndicatorButton);
    d->addIndicators(clientInfo);
}

void HbIndicatorButton::deactivate(const QList<IndicatorClientInfo> &clientInfo)
{
    Q_D(HbIndicatorButton);
    d->removeIndicators(clientInfo);
}

void HbIndicatorButton::activateAll(const QList<IndicatorClientInfo> &clientInfo)
{
    Q_D(HbIndicatorButton);
    d->mIndicators.clear();
    d->addIndicators(clientInfo);
}

void HbIndicatorButton::update(const QList<IndicatorClientInfo> &clientInfo)
{
    Q_D(HbIndicatorButton);
    d->addIndicators(clientInfo);
}

void HbIndicatorButton::resetBackground()
{
    Q_D(HbIndicatorButton);
    d->mIndicatorMenuOpen = false;
    updatePrimitives();
}

void HbIndicatorButton::initStyleOption(HbStyleOptionIndicatorButton *option) const
{
    Q_D(const HbIndicatorButton);
    if (isDown()) {
        option->mode = QIcon::Active;
    } else if (d->mIndicatorMenuOpen) {
        option->mode = QIcon::Selected;
    } else {
        option->mode = QIcon::Normal;
    }
    if (mainWindow() && mainWindow()->currentView()) {
        if (mainWindow()->currentView()->viewFlags() & HbView::ViewTitleBarTransparent) {
            option->transparent = true;
        }
    }
    option->twoIcons = (d->mStyle == 3);
}

void HbIndicatorButton::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LayoutDirectionChange) {
        updatePrimitives();
    }
    HbToolButton::changeEvent(event);
}

/*
  Overloaded hit detection to include touch area
 */
bool HbIndicatorButton::hitButton(const QPointF &pos) const
{
    Q_D(const HbIndicatorButton);
    QRectF compRect = d->mTouchArea->boundingRect();
    compRect.translate(d->mTouchArea->pos());
    return compRect.contains(pos);
}

void HbIndicatorButton::handlePress()
{
#ifdef HB_EFFECTS
    HbEffect::start(this, "decorator", "pressed");
#endif
    updatePrimitives();
}

void HbIndicatorButton::handleRelease()
{
#ifdef HB_EFFECTS
    HbEffect::start(this, "decorator", "released");
#endif
    updatePrimitives();
}

void HbIndicatorButton::showIndicatorMenu()
{
    Q_D(HbIndicatorButton);
    d->showIndicatorMenu();
}

