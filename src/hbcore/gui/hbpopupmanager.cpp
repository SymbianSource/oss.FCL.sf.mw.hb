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

#include "hbpopupmanager_p.h"
#include "hbpopupmanager_p_p.h"
#include "hbpopup.h"
#include "hbpopup_p.h"
#include "hbnamespace_p.h"
#include "hbinstance.h"
#include "hbmainwindow_p.h"
#include "hbevent.h"
#include "hbwidget.h"
#include "hbgraphicsscene.h"
#include "hbmenu.h"
#include "hbmenu_p.h"

#include <QGraphicsWidget>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLayout>
#include <QDebug>
#include <QEvent>

HbPopupLayoutSpacer::HbPopupLayoutSpacer( QGraphicsItem *parent )
: HbWidgetBase( parent )
{
#if QT_VERSION >= 0x040600
    setFlag(QGraphicsItem::ItemHasNoContents, true);
#endif
    setAcceptedMouseButtons(Qt::NoButton);
}

QSizeF HbPopupLayoutSpacer::sizeHint(Qt::SizeHint which, const QSizeF &constraint ) const
{
    Q_UNUSED( constraint );

    if ( which == Qt::MaximumSize ) {
        return QSizeF( QWIDGETSIZE_MAX, QWIDGETSIZE_MAX );
    }
    return QSizeF(0.f,0.f);
}

/*
\reimp
*/
void HbPopupLayoutSpacer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Q_UNUSED(option)
    Q_UNUSED(widget);
    Q_UNUSED(painter);
}


HbPopupLayoutProxy::HbPopupLayoutProxy( HbPopup *popup, QGraphicsItem *parent )
: HbWidgetBase(parent), mPopup(popup)
{
#if QT_VERSION >= 0x040600
    setFlag(QGraphicsItem::ItemHasNoContents, true);
#endif
    setAcceptedMouseButtons(Qt::NoButton);
    popup->installEventFilter(this);
}

QSizeF HbPopupLayoutProxy::sizeHint(Qt::SizeHint which, const QSizeF &constraint ) const
{
    if (!mPopup.isNull()) {
        return mPopup->effectiveSizeHint(which, constraint);
    }
    return QSizeF(0, 0);
}

void HbPopupLayoutProxy::setGeometry(const QRectF &rect)
{
    if (!mPopup.isNull()) {
        const QSizeF popupPreferredSize = 
            mPopup->effectiveSizeHint(Qt::PreferredSize);
        const QSizeF usedSize( qMin(rect.width(),   popupPreferredSize.width() ),
                               qMin( rect.height(), popupPreferredSize.height() ) );
        QPointF usedPos(rect.left(), rect.top());
        mPreferredPos = ((HbPopupPrivate*)HbPopupPrivate::d_ptr(mPopup))->preferredPos;
        mPlacement = ((HbPopupPrivate*)HbPopupPrivate::d_ptr(mPopup))->placement;
        mPreferredPosSet = ((HbPopupPrivate*)HbPopupPrivate::d_ptr(mPopup))->preferredPosSet;
        qreal screenMargin = ((HbPopupPrivate*)HbPopupPrivate::d_ptr(mPopup))->mScreenMargin;
        if ( mPreferredPosSet ) { // user set preferred pos, ignore css
            const qreal pw(parentItem()->boundingRect().width() - screenMargin);
            const qreal ph(parentItem()->boundingRect().height() - screenMargin);
            const qreal uw(usedSize.width());
            const qreal uh(usedSize.height());
            const qreal x(mPreferredPos.x());
            const qreal y(mPreferredPos.y());
            qreal usedx = 0;
            qreal usedy = 0;
            switch( mPlacement  ) {
            case HbPopup::TopLeftCorner:
                usedx = qMin(x, pw-uw);
                usedy = qMin(y, ph-uh);
                break;
            case HbPopup::TopRightCorner:
                usedx = qMin(x-uw, pw-uw);
                usedy = qMin(y, ph-uh);
                break;
            case HbPopup::BottomLeftCorner:
                usedx = qMin(x, pw-uw);
                usedy = qMin(y-uh, ph-uh);
                break;
            case HbPopup::BottomRightCorner:
                usedx = qMin(x-uw, pw-uw);
                usedy = qMin(y-uh, ph-uh);
                break;
            case HbPopup::TopEdgeCenter:
                usedx = qMin(x-uw/2, pw-uw);
                usedy = qMin(y, ph-uh);
                break;
            case HbPopup::RightEdgeCenter:
                usedx = qMin(x-uw, pw-uw);
                usedy = qMin(y-uh/2, ph-uh);
                break;
            case HbPopup::BottomEdgeCenter:
                usedx = qMin(x-uw/2, pw-uw);
                usedy = qMin(y-uh, ph-uh);
                break;
            case HbPopup::LeftEdgeCenter:
                usedx = qMin(x, pw-uw);
                usedy = qMin(y-uh/2, ph-uh);
                break;
            case HbPopup::Center:
                usedx = qMin(x-uw/2, pw-uw);
                usedy = qMin(y-uh/2, ph-uh);
                break;
            default:
                //should not happen
                break;
            }
            if ( usedx < screenMargin ) usedx = screenMargin;
            if ( usedy < screenMargin ) usedy = screenMargin;

            usedPos = QPointF(usedx, usedy);
        }
        mPopup->setGeometry(QRectF(usedPos, usedSize));
    }
}

bool HbPopupLayoutProxy::event( QEvent *e ) 
{
    switch ( e->type() )
    {
    case QEvent::Polish:
        polishEvent();
        return QObject::event(e);
    default:
        return HbWidgetBase::event( e );
    }
}

bool HbPopupLayoutProxy::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED( obj );
    switch( event->type() ) {
        case QEvent::LayoutRequest: {
            updateGeometry();
            break;
        }
        case QEvent::ContextMenu: {
            updateGeometry();
            break;
        }
        default:
            break;
    }
    return false;
}


/*
\reimp
*/
void HbPopupLayoutProxy::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    Q_UNUSED(painter);
}

HbPopupLayoutManager::HbPopupLayoutManager( HbPopup *popup, QGraphicsScene *scene ) 
: HbWidget()
{        
#if QT_VERSION >= 0x040600
    setFlag(QGraphicsItem::ItemHasNoContents, true);
#endif
    setAcceptedMouseButtons(Qt::NoButton);
    scene->addItem( this );

    // create proxy
    HbPopupLayoutProxy *childItem = new HbPopupLayoutProxy( popup, this );

    // create spacers
    HbStyle::setItemName( new HbPopupLayoutSpacer(this), "spacerTop");
    HbStyle::setItemName( new HbPopupLayoutSpacer(this), "spacerBottom");
    HbStyle::setItemName( new HbPopupLayoutSpacer(this), "spacerLeft");
    HbStyle::setItemName( new HbPopupLayoutSpacer(this), "spacerRight");

    const QMetaObject *metaObject = popup->metaObject();
    if ( metaObject->className() == QLatin1String("HbMenu") ) {
        HbMenu *menu = qobject_cast<HbMenu*>(popup);
        if (menu && menu->menuType() == HbMenu::OptionsMenu) {
            HbStyle::setItemName( childItem, "options-menu" );
        } else {
            HbStyle::setItemName( childItem, "menu" );
        }
    } else if ( metaObject->className() == QLatin1String("HbToolBarExtension" ) ) {
        HbStyle::setItemName( childItem, "toolbar-extension" );
    } else {
        HbStyle::setItemName( childItem, "popup" );        
    }
    setZValue(popup->zValue() - 1);
    if ( metaObject->className() == QLatin1String("HbVolumeSliderPopup") ) {
        HbStyle::setItemName( childItem, "volumesliderpopup" );
        connect(scene,SIGNAL(sceneRectChanged(QRectF)),this,SLOT(orientationChanged(QRectF)));
    }
    if ( metaObject->className() == QLatin1String("HbZoomSliderPopup") ) {
        HbStyle::setItemName( childItem, "zoomsliderpopup" );
        connect(scene,SIGNAL(sceneRectChanged(QRectF)),this,SLOT(orientationChanged(QRectF)));
    }
}

void  HbPopupLayoutManager::orientationChanged(const QRectF& rect)
{
    Q_UNUSED(rect);
    repolish();
}

/*!
    \reimp
*/
QSizeF HbPopupLayoutManager::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(which);
    Q_UNUSED(constraint);
    return HbDeviceProfile::profile(this).logicalSize();
}

/*
\reimp
*/
void HbPopupLayoutManager::changeEvent(QEvent *event)
{
    HbWidget::changeEvent(event);
    if (event && event->type() == QEvent::LayoutDirectionChange) {
        repolish();
    }

}

/*
\reimp
*/
void HbPopupLayoutManager::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Q_UNUSED(option)
    Q_UNUSED(widget);
    Q_UNUSED(painter);
}


/*
    \class HbPopupManagerPrivate
    \brief HbPopupManagerPrivate private data class for HbPopupManager.
*/
HbPopupManagerPrivate::HbPopupManagerPrivate(HbGraphicsScene *scene, QObject * parent):
    QObject(parent),
    topLevelFocusablePopup(0),
    initialFocusedItem(0),
    scene(scene)
{
    connect(&deviceFadeControl, SIGNAL(fadeChange(bool)), this,
        SLOT(deviceFadeChange(bool)));
}

HbPopupManagerPrivate::~HbPopupManagerPrivate()
{
}

/*
    Deliver mouseEvent to all HbPopupBackGround object for non-modal popups until event blocking
    item is found.
*/
void HbPopupManagerPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    foreach (QGraphicsItem *item, scene->items(mouseEvent->scenePos())) {
        if (eventBlockItem(item)) {
            break;
        }
        if (item->type() == HbPrivate::ItemType_PopupBackGround) {
            static_cast<GraphicsItem *>(item)->sceneEvent(mouseEvent);
            // Finish loop if popup of background is modal
            if (static_cast<HbPopupBackGround *>(item)->isModal()) {
                break;
            }
        }
    }
}

/*
  Returns true if item is an event blocking item.
*/
bool HbPopupManagerPrivate::eventBlockItem(QGraphicsItem *item)
{
    if (item->type() == HbPrivate::ItemType_FadeItem) {
        return true;
    }

    return false;
}

qreal HbPopupManagerPrivate::popupZValueForPriority( int priority, int popupCountWaterMark )
{
    qreal ret = (qreal)(HbPrivate::PopupZValueDefaultOffset) 
                + (qreal)(priority * HbPrivate::PopupWithDifferentPriorityZValueUnit)
                + (qreal)(popupCountWaterMark * HbPrivate::PopupWithSamePriorityZValueUnit);

    Q_ASSERT (HbPrivate::PopupZValueRangeStart <= ret && ret <= HbPrivate::PopupZValueRangeEnd);

    return ret;
}

qreal HbPopupManagerPrivate::nextPopupZValueForPriority( int priority )
{
    // The priorities are converted to Z values so that the resulting Z value is
    // smaller than HbPrivate::PopupZValue + 1 this ensures that all the popup
    // Z values fit in [HbPrivate::PopupZValue, HbPrivate::PopupZValue + 1] range
    PopupCountRegister popupCountRegister = popupCountRegisters.value(priority, PopupCountRegister());
    popupCountRegister.count++;
    popupCountRegister.waterMark++;
    popupCountRegisters[priority] = popupCountRegister;

    return popupZValueForPriority( priority, popupCountRegister.waterMark );
}

void HbPopupManagerPrivate::updateHash( int priority, qreal zValueOfRemovedPopup )
{
    PopupCountRegister popupCountRegister = popupCountRegisters.value(priority);

    popupCountRegister.count--;
    // If there are no more popups for priority then reset water mark to 0
    if (!popupCountRegister.count) {
        popupCountRegister.waterMark = 0;
    }
    // If the popup that removed had the highest Z value for priority decrease the water mark
    else if (qFuzzyCompare(popupZValueForPriority(priority, popupCountRegister.waterMark), zValueOfRemovedPopup)) {
        popupCountRegister.waterMark--;
    }
    popupCountRegisters[priority] = popupCountRegister;
}

void HbPopupManagerPrivate::eventHook(QEvent *event)
{
    if (event && event->type() == QEvent::GraphicsSceneMouseRelease && popupList.count() && scene) {
        mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
    }
}

void HbPopupManagerPrivate::showPopup(HbPopup *popup)
{
    if (popup) {
        addPopup(popup);
    } else {
        qWarning() << "HbPopupManager::showPopup(): null pointer passed!";
    }
}

void HbPopupManagerPrivate::hidePopup(HbPopup *popup)
{
    if (popup) {
        removePopup(popup);
    }
}

void HbPopupManagerPrivate::addPopup(HbPopup *popup)
{
    if (!popupList.contains(popup)) {
        popupList.append(popup);
        qreal popupZvalue = nextPopupZValueForPriority(HbPopupPrivate::d_ptr(popup)->priority());

        // Set popup and its background item to correct Z value
        popup->setZValue(popupZvalue);
        HbPopupBackGround *backgroundItem = HbPopupPrivate::d_ptr(popup)->backgroundItem;
        if (backgroundItem) {
            backgroundItem->setZValue(popupZvalue - HbPrivate::PopupBackgroundItemZValueUnit);
        }

        if ( !popup->parentItem() ) {
            if ( popup->metaObject()->className() != QLatin1String("HbInputCustomButtonList") &&
                 popup->metaObject()->className() != QLatin1String("HbCharPreviewPane") &&
                 popup->metaObject()->className() != QLatin1String("HbCandidateList") &&
                 popup->metaObject()->className() != QLatin1String("HbExactWordPopup") &&
                 popup->metaObject()->className() != QLatin1String("HbInputSmileyPicker") &&
                 popup->metaObject()->className() != QLatin1String("Hb12KeyCustomKeypad") &&
				 popup->metaObject()->className() != QLatin1String("HbInputThaiSpecialPopup") &&
                 !popup->inherits("HbInputVkbWidget")) {
                setGeometryForPopup( popup );
            }
        }

        // Check if popup is top level popup
        if((!topLevelFocusablePopup || topLevelFocusablePopup->zValue() < popup->zValue()) &&
             popup->flags().testFlag(QGraphicsItem::ItemIsFocusable) ) {

            topLevelFocusablePopup = popup;

            // If this is the first popup in the stack being launched
            if(!initialFocusedItem && scene) {
                initialFocusedItem = scene->focusItem();
            }

            topLevelFocusablePopup->setFocus();
        }

        updateFading();
    }
}

void HbPopupManagerPrivate::removePopup(HbPopup *popup)
{
    //delete the layoutproxy and remove the popup from parentItems list
    //only if its in its destruction
    bool popupInDestruction = (HbPopupPrivate::d_ptr(popup))->inDestruction;
    if ( parentItems.contains( popup ) && (popupInDestruction)) {
        //HbPopupLayoutManager *parentItem = parentItems[popup];        
        parentItems.remove( popup );
        //delete parentItem;
    }

    // Only execute if close if popup was in popupList before
    if (popupList.removeOne(popup)) {
        // Update popup count registers
        updateHash(HbPopupPrivate::d_ptr(popup)->priority(), popup->zValue());

        // Update fading
        updateFading(true);

        // Find new top level popup
        if (topLevelFocusablePopup && topLevelFocusablePopup == popup) {
            topLevelFocusablePopup = 0;
            foreach (HbPopup *popup, popupList) {
                if ((!topLevelFocusablePopup || topLevelFocusablePopup->zValue() < popup->zValue()) &&
                    popup->flags().testFlag(QGraphicsItem::ItemIsFocusable)){
                    topLevelFocusablePopup = popup;
                }
            }
        }

        bool wasActivePopup = true;
        // If the previous popup was an inactive panel, then it would not have
        // gained focused. So no need to set the focus to the next topmost popup.
        if (popup->isPanel() && !popup->isActive()) {
            wasActivePopup = false;
        }
        if (topLevelFocusablePopup && wasActivePopup ) {
            topLevelFocusablePopup->setFocus();
        }
        //TODO: this is not very efficient way of deciding if has already been deleted or not
        else if (initialFocusedItem && scene &&
                 scene->items().contains(initialFocusedItem)) {

            // Move the focus to the initial focus item if there is no current focus item or
            // the ancestor of the current fucus item is popup
            if( !scene->focusItem() || popup->hasFocus() || popup->isAncestorOf(scene->focusItem())) {
                initialFocusedItem->setFocus();
                initialFocusedItem = 0;
            }
        }
    }
}

void HbPopupManagerPrivate::updateFading(bool unfadeFirst /* false */)
{
    HbPopup *topFadingPopup = 0;
    // Find the top fading popup
    foreach (HbPopup *popup, popupList) {
        if (popup->isBackgroundFaded()) {
            if (!topFadingPopup || topFadingPopup->zValue() < popup->zValue()) {
                topFadingPopup = popup;
            }
        }
    }

    // To avoid double fading, device dialog server may ask applications to
    // set background fading off
    if (deviceFadeControl.fadeOff()) {
        topFadingPopup = 0;
        unfadeFirst = true;
    }
    // Device dialog server commands other applications to set background fading off.
    deviceFadeControl.setFadeOff(topFadingPopup != 0);

    HbMainWindow *mainWindow = 0;
    if (!scene->views().isEmpty()) {
        // in the Hb architecture, there is only one view per scene
        mainWindow = qobject_cast<HbMainWindow *>(scene->views().at(0));
    }
    if (mainWindow) {
        if (unfadeFirst) {
            HbMainWindowPrivate::d_ptr(mainWindow)->unfadeScreen();
        }

        if (topFadingPopup) {
            HbMainWindowPrivate::d_ptr(mainWindow)->fadeScreen(
                    HbPopupPrivate::d_ptr(topFadingPopup)->backgroundItem->zValue() - HbPrivate::FadingItemZValueUnit );
        }
    }
}

// Device wide fading change is requested
void HbPopupManagerPrivate::deviceFadeChange(bool fadeOff)
{
    updateFading(fadeOff);
}

void HbPopupManagerPrivate::setGeometryForPopup(HbPopup *popup)
{
    // clear the old one if deleted
    HbPopupLayoutManager *parentItem = 0;
    HbPopupLayoutProxy *childItem = 0;
    if ( parentItems.contains( popup ) ) {
        parentItem = parentItems[popup];
        childItem = static_cast<HbPopupLayoutProxy*>(parentItem->childItems().at(0));
        if ( childItem->mPopup.isNull() ) {
            parentItems.remove( popup );
            delete parentItem;
        }
    }
    parentItem = 0;
    childItem = 0;
    if ( parentItems.contains( popup ) ) {
        parentItem = parentItems[popup];        
    } else {
        parentItem = new HbPopupLayoutManager( popup, scene );
        parentItems.insert( popup, parentItem );
    }    

    childItem = static_cast<HbPopupLayoutProxy*>(parentItem->childItems().at(0));
    childItem->setSizePolicy( popup->sizePolicy() );
}


/*
    \class HbPopupManager
    \brief HbPopupManager is managing the priority order of HbPopup objects and fading of
           the screen.
*/

HbPopupManager::HbPopupManager(HbGraphicsScene *scene, QObject *parent): QObject(parent)
{
    d = new HbPopupManagerPrivate(scene, this);
}

/*
    Destructor.
*/
HbPopupManager::~HbPopupManager()
{
// no need to delete d because it is child of this object
}

void HbPopupManager::eventHook(QEvent *event)
{
    d->eventHook(event);
}

void HbPopupManager::showPopup(HbPopup *popup)
{
    d->showPopup(popup);
}

void HbPopupManager::hidePopup(HbPopup *popup)
{
    d->hidePopup(popup);
}

