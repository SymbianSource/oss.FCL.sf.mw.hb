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

#include "hbwidget.h"
#include "hbwidget_p.h"
#include "hbfocusgroup_p.h"
#include "hbevent.h"
#include "hbinstance.h"
#include "hbstyleoption.h"
#include "hbstyleparameters.h"
#include "hbgraphicsscene.h"
#include "hbgraphicsscene_p.h"
#include "hbdeviceprofile.h"
#include "hbspaceritem_p.h"
#include <QCoreApplication>
#include <QMetaType>
#include <QAction>
#include <QDebug>
#include <QDynamicPropertyChangeEvent>
#include <QGraphicsLayout>
#include <QInputContext>

#ifdef HB_TESTABILITY
/*!
    \internal
 */
static void testabilitySignalEnabledChange(bool enabled, void *param)
{
    HbWidget *widget = static_cast<HbWidget *>(param);
    if (widget) {
        // We need ItemTransformChanged notifications which must be enabled
        // manually by setting the ItemSendsGeometryChanges flag.
        // However this should only be done when the testability signal gets
        // enabled because it causes perf regression.
        widget->setFlag(QGraphicsItem::ItemSendsGeometryChanges, enabled);
    }
}
#endif //HB_TESTABILITY

HbWidgetPrivate::HbWidgetPrivate()
  : HbWidgetBasePrivate(),
    style(0),
    backgroundPrimitiveType(HbStyle::P_None),
    polished(false),
    polishPending(false),
    themingPending(true),
    repolishOutstanding(false),
    notifyScene(false),
    pluginBaseId(0),
    focusGroup(0),
    focusActiveType(HbStyle::P_None),
    focusResidualType(HbStyle::P_None),
    highlightExpired(false),
    backgroundItem(0),
    focusActiveItem(0),
    focusResidualItem(0)
{
    q_ptr = 0;
#ifdef HB_TESTABILITY 
	testabilitySignal = new HbTestabilitySignal_p();
#endif //HB_TESTABILITY 
}

HbWidgetPrivate::~HbWidgetPrivate()
{
    if (backgroundItem) {
        delete backgroundItem;
        backgroundItem = 0;
    }

    // need to delete the hash values
    QList<QGraphicsLayoutItem *> deletedSpacers = mSpacers.values();
    mSpacers.clear();
    qDeleteAll( deletedSpacers );            
}

/*!
    Creates a new spacer item with the given \a name.

    Returns a pointer to the newly created spacer item. Widget keeps the ownership.
*/
QGraphicsLayoutItem *HbWidgetPrivate::createSpacerItem( const QString &name )
{
#ifndef Q_OS_SYMBIAN
    Q_ASSERT( !name.isEmpty() );
    // check in desktop environments that the item does not exist already
    Q_ASSERT( !mSpacers.contains( name ) );
#endif

    HbSpacerItem *newSpacer = new HbSpacerItem();
    mSpacers.insert( name, newSpacer );
    return newSpacer;
}

/*
    
    \deprecated HbWidget::setBackgroundItem(HbStyle::Primitive, int)
    is deprecated. Use HbWidget::setBackgroundItem(QGraphicsItem *item, int zValue) instead.
    
    Creates background item to the widget from style primitive.
    
    Creates a new background item to the widget and the reparents it to
    be child of the widget. Also Z-value of the background item will be 
    changed to zValue. By default the zValue is -1, which should be 
    behind other widget content. Background item will be always resized 
    to be same size as the widgets bounding rect. 
    
    If type is HbStyle::P_None, the background item will be removed from
    widget.
    
    Previously set background item will be deleted.
 */
void HbWidget::setBackgroundItem(HbStyle::Primitive type, int zValue)
{
    Q_D(HbWidget);
    if(type!=d->backgroundPrimitiveType || type == HbStyle::P_None) {
        if (d->backgroundItem) {
            delete d->backgroundItem;
            d->backgroundItem = 0;
        }
        d->backgroundPrimitiveType = type;
        d->backgroundItem = style()->createPrimitive(d->backgroundPrimitiveType, const_cast<HbWidget*>(this));
        if(d->backgroundItem) {
            d->backgroundItem->setParentItem(this);
        }
        updatePrimitives();
    }
    if(d->backgroundItem && zValue != d->backgroundItem->zValue()) {
        d->backgroundItem->setZValue(zValue);
    }
}

/*
    Sets background item to the widget.
    
    The item will be reparented to be child of the widget. Also Z-value 
    of the background item will be changed to be zValue. By default the 
    zValue is -1, which should be  behind other widget content. 
    Background item will be always resized to be same size as widgets 
    bounding rect. If background item is widget, the size will 
    be changed with setGeometry to allow e.g. maintaining aspect ratio of 
    images. If item is not widget, the size will be changed with 
    transformations. 
    
    If item is 0, the background item will be removed from the widget.
    
    Previously set background item will be deleted.
 */
void HbWidget::setBackgroundItem(QGraphicsItem *item, int zValue)
{
    Q_D(HbWidget);
    if(d->backgroundItem) {
        delete d->backgroundItem;
        d->backgroundItem = 0;
    }
    d->backgroundPrimitiveType = HbStyle::P_None;
    d->backgroundItem = item;
    if(d->backgroundItem) {
        d->backgroundItem->setParentItem(this);
        d->backgroundItem->setZValue(zValue);
        d->updateBackgroundItemSize();
    }
}

/*
    Returns background item. 0 is returned if there isn't background
    item in the widget.
 */
QGraphicsItem *HbWidget::backgroundItem() const
{
    Q_D( const HbWidget );
    return d->backgroundItem;
}

/*
    Returns focusItem primitive items.
    Focus primitive is created if has not been created already.
 */
QGraphicsItem *HbWidgetPrivate::focusPrimitive(HbWidget::FocusHighlight highlightType) const
{
    Q_Q(const HbWidget);

    if (highlightType == HbWidget::FocusHighlightActive) {
        if (!focusActiveItem && focusActiveType != HbStyle::P_None) {
            focusActiveItem = q->style()->createPrimitive(
                                focusActiveType, const_cast<HbWidget*>(q));
            focusActiveItem->hide();
        }
        return focusActiveItem;
    } else if (highlightType == HbWidget::FocusHighlightResidual) {
        if (!focusResidualItem && focusResidualType != HbStyle::P_None) {
            focusResidualItem = q->style()->createPrimitive(
                                focusResidualType, const_cast<HbWidget*>(q));
            focusResidualItem->hide();
        }
        return focusResidualItem;
    }

    return 0;
}

/*
    Hides or shows focus primitive depending on the focus state of the widget.
*/
void HbWidgetPrivate::focusChangeEvent(HbWidget::FocusHighlight focusHighlight)
{
    Q_Q(HbWidget);
    bool activeHighlight(false);
    bool residualHighlight(false);

    HbFocusGroup *group = getFocusGroup();

    // Do not show focus if there is focus group and the flag is NeverOn.
    if (group && (group->focusParameters() == HbFocusGroup::NeverOn)) {
        return;
    }

    if (group) {
        group->stopHighlightTimer(q);
        }

    if (focusHighlight == HbWidget::FocusHighlightActive) {
        activeHighlight = true;

        if (group) {
           highlightExpired = false;
           group->startHighlightTimer(q);
        }

    } else if (focusHighlight == HbWidget::FocusHighlightResidual) {
        residualHighlight = true;
    }

    if (focusActiveItem) {
        focusActiveItem->setVisible(activeHighlight);
        focusActiveItem->update();         
    }

    if (focusResidualItem) {
        focusResidualItem->setVisible(residualHighlight);
        focusResidualItem->update();
    }

}

/*
    Find closest parent with focus group and update the focused child.
*/
void HbWidgetPrivate::updateCurrentFocusChild()
{
    Q_Q(HbWidget);

    HbFocusGroup *group = getFocusGroup();

    if (group) {
        group->updateCurrentFocusChild(q);
    }
}

/*
    Find and return the closest parent with focus group if any. If propagate
    is true then the closest parent with focus group and children is accepted as
    valid focus group e.g. used for a widget which has the key for changing the 
    focus mode set only.
*/
HbFocusGroup *HbWidgetPrivate::getFocusGroup(bool propagate) const {
    Q_Q(const HbWidget);
    HbFocusGroup *group(0);
    const QGraphicsItem* item(q);
    bool stop(false);

    const QGraphicsWidget *w = static_cast<const QGraphicsWidget *>(item);

    while (!stop && w) {
        const HbWidget* widget = qobject_cast<const HbWidget *>(w);

        if (widget)  {
            group = widget->d_func()->focusGroup;

            if (!propagate) {
                stop = true;
            }
            if (group && !stop) {
                stop = group->isChildrenRegistered();
            }

            if (!stop) {
                w = w->parentWidget();
            }
        } else {
            break;
        };
    }
    return (group) ? group : 0;
}

/*
    Set focus to child widget depending on the set focus delegation
    policy. 
*/
bool HbWidgetPrivate::delegateFocus(QFocusEvent *event) const
{
    return (focusGroup) ? focusGroup->delegateFocus(event) : false;
}

void HbWidgetPrivate::updateBackgroundItemSize()
{
    Q_Q(HbWidget);
    if(backgroundItem) {
        if(backgroundItem->isWidget()) {
            static_cast<QGraphicsWidget*>(backgroundItem)->setGeometry(q->boundingRect());
        } else {
            QRectF widgetRect = q->boundingRect();
            QRectF backgroundRect = backgroundItem->boundingRect();
            if(backgroundRect.width() != 0 && backgroundRect.height() != 0) {
                backgroundItem->setTransform(
                    QTransform().scale(widgetRect.width()/backgroundRect.width(), 
                                       widgetRect.height()/backgroundRect.height()));
            }
        }
    }
}

/*
    Test if some item in our parent hierarchy has
    the Hb::InputMethodNeutral flag set.
*/
bool HbWidgetPrivate::isInputMethodNeutral()
{
    Q_Q(HbWidget);

    // Test the widget itself...
    if(q->testAttribute(Hb::InputMethodNeutral)) {
        return true;
    }

    // and then all parents
    QGraphicsWidget* i = q->parentWidget();
    while(i) {
        HbWidget* w = qobject_cast<HbWidget *>(i);
        if(w && w->testAttribute(Hb::InputMethodNeutral)) {
            return true;
        }
        i = i->parentWidget();
    }
    return false;
}

/*!
    \class HbWidget

    \brief HbWidget is a base for all Hb widgets. It contains common functionality
    for widgets and offers virtual methods to be overridden in derived widgets.

    Currently HbWidget offers the following functionality:
        - Accessing style primitives
        - Accessing font specification property
        - Accessing platform style object and setting widget specific style
        - Handling focus in/out events (to support proper focus management)                   
*/
/*!
    \enum HbWidget::FocusDelegation
    Defines focus delegation policy for the focus group. Focus is delegated to child widgets
    according to the policy.

    \sa setFocusDelegation() focusDelagation()
*/
/*!
    \var HbWidget::FocusDelegationNone
    Focus is not delegated to child widgets. Instead the focus group itself has the focus.
*/
/*!
    \var HbWidget::FocusDelegationFirstChild
    Focus is delegated to first widget in the focus chain. Direction does not effect this.
*/
/*!
    \var HbWidget::FocusDelegationLastChild
    Focus is delegated to last widget in the focus chain. Direction does not effect this.
*/
/*!
    \var HbWidget::FocusDelegationBasedOnDirection
    Focus is given to first widget in focus chain, when focus movement is to next widget,
    and to last widget, when focus movement is to previous widget.
*/
/*!
    \var HbWidget::FocusDelegationFocusedChild
    Focus is given to the widget which was focused when focus group lost the focus
*/

/*!
    \enum HbWidget::FocusMode
    Focus group has focus mode attribute that can be used in situations where it would
    be impossible to determine whether focus should be moved from widget or should the
    widget handle the event.

    \sa setFocusMode() focusMode()
*/
/*!
    \var HbWidget::FocusModeNormal
    In normal mode only the focus change is performed, event is not passed to widget.

*/
/*!
    \var HbWidget::FocusModeEdit
    In edit mode event is passed to widget and the focus change is performed if widget
    did not accept the event.
*/
/*!
    \enum HbWidget::FocusHighlight
    Defines the highlight for the widget.

    \sa setFocusHighlight() focusHighlight()
*/


/*!
    \var HbWidget::FocusHighlightNone
    No focus highlight set for the widget. If active or residual highlight have alraedy been set,
    then FocusHightlightNone removes both highlights.
*/
/*!
    \var HbWidget::FocusHighlightActive
    Defines the active highlight for the widget.
*/
/*!
    \var HbWidget::FocusHighlightResidual
    Defines the residual highlight for the widget.
*/

/*!
    \fn int HbWidget::type() const

    Reimplemented from QGraphicsItem::type().
 */

/*!
    Constructs a widget with the parent item and window flags.
    This constructor creates a new HbWidgetPrivate instance and installs it to the widget.
*/
HbWidget::HbWidget( QGraphicsItem *parent, Qt::WindowFlags wFlags ):
    HbWidgetBase( *new HbWidgetPrivate, parent, wFlags )
{
    Q_D( HbWidget );
    d->q_ptr = this;

#ifdef HB_TESTABILITY 
	if(d->testabilitySignal) {
		d->testabilitySignal->setParent(this);
		d->testabilitySignal->notifySignalEnabled(testabilitySignalEnabledChange, this);
	}
#endif //HB_TESTABILITY
}

/*!
    Constructs a widget with the given private object, parent item and window flags.
    The given HbWidgetPrivate instance is installed in the widget and deleted in its destructor.
*/
HbWidget::HbWidget(HbWidgetPrivate &dd, QGraphicsItem *parent, Qt::WindowFlags wFlags):
    HbWidgetBase( dd, parent, wFlags )
{
    Q_D( HbWidget );
    d->q_ptr = this;

#ifdef HB_TESTABILITY 
	if (d->testabilitySignal) {
		d->testabilitySignal->setParent(this);
		d->testabilitySignal->notifySignalEnabled(testabilitySignalEnabledChange, this);
	}
#endif //HB_TESTABILITY
}

/*!
 Destroys the widget.
*/
HbWidget::~HbWidget()
{
}

/*! 
    FocusIn event hadler.
 */
void HbWidget::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED( event );
    Q_D(HbWidget);

    // Return if focus is delegated
    if (d->delegateFocus(event)) {
        return;
    }

    if (d->focusPrimitive(HbWidget::FocusHighlightActive)) {
        d->focusChangeEvent(HbWidget::FocusHighlightActive);
    }
    d->updateCurrentFocusChild();

    if (parentWidget()) {
        // It is must to notify parent widget that child widget has got focus
        //Start of snippet 1
        HbEvent event1( HbEvent::ChildFocusIn);
        QCoreApplication::sendEvent( parentWidget(), &event1 );        
        //End of snippet 1
    }

#if QT_VERSION >= 0x040600
    if(!d->isInputMethodNeutral() && !(flags() & QGraphicsItem::ItemAcceptsInputMethod)) {
        // Make sure the input panel is closed if this widget is not input method neutral or
        // it does not accept input method
        // Send close input panel event.
        QInputContext *ic = qApp->inputContext();
        if (ic) {
            QEvent *closeEvent = new QEvent(QEvent::CloseSoftwareInputPanel);
            ic->filterEvent(closeEvent);
            delete closeEvent;
        }
    }
#endif

}

/*! 
    FocusOut event hadler.
 */
void HbWidget::focusOutEvent( QFocusEvent *event )
{
    Q_UNUSED( event );
    Q_D(HbWidget);

    // Hide focus highlight.
    if (d->focusPrimitive(HbWidget::FocusHighlightResidual)) {
        d->focusChangeEvent(HbWidget::FocusHighlightResidual);
    } else {
        d->focusChangeEvent(HbWidget::FocusHighlightNone);
    }

    if ( parentWidget() ) {
        // It is must to notify parent widget that child widget has lost focus
        HbEvent event1( HbEvent::ChildFocusOut);
        QCoreApplication::sendEvent( parentWidget(), &event1 );
    }
}

/*!
    \reimp
 */
void HbWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_D(HbWidget);
    HbWidgetBase::resizeEvent(event);
    d->updateBackgroundItemSize();
}


/*!
    Sets a custom style for the widget. 
	When a new style is set for the widget the widget's primitives need to recreated and primitives and layout updated.
	In order for the style change to work each widget needs to override recreatePrimitives() method and recreate the
	primitives. 
    \param style Custom style for the widget, note that the ownership is not transferred
*/
void HbWidget::setStyle( HbStyle *style )
{
    Q_D( HbWidget );
    d->style = style;
	recreatePrimitives();
	updatePrimitives();
	repolish();
}

/*!
    As a default returns the hbInstance's platform style. If style is set using
    the setStyle then the set style is returned.
*/
HbStyle *HbWidget::style() const
{
    Q_D( const HbWidget );

    if( d->style )
        return d->style;
    else return hbInstance->style();
}

/*!
    Removes all the widgets's actions. Actions owned by the widget and not
    shown in any other widget are deleted.

    \sa removeAction()
*/
void HbWidget::clearActions()
{
    QList<QAction*> acts = actions();
    for(int i = 0; i < acts.size(); i++) {
        removeAction(acts[i]);
        if (acts[i]->parent() == this && acts[i]->associatedGraphicsWidgets().isEmpty())
            delete acts[i];
    }
}

/*!
    This function returns the base id of the style plugin of the widget or
	\c 0 if the widget is not plugin based.
*/
int HbWidget::pluginBaseId() const
{
    Q_D(const HbWidget);
    return d->pluginBaseId;
}

/*!
    Sets the base id of the style plugin of the widget.
*/
void HbWidget::setPluginBaseId( int baseId )
{
    Q_D(HbWidget);
    d->pluginBaseId = baseId;
}

/*!
  This function returns the HbMainWindow of the widget or \c 0 if
  it doesn't exist.  Note that if the widget is not added to the scene
  then this function returns the first mainwindow (if there is one)
  which is not guaranteed to be the proper one in a multi-display
  environment.
*/
HbMainWindow *HbWidget::mainWindow() const
{
    QGraphicsScene *scene = this->scene();
    if (!scene) {
        QList<HbMainWindow *> mainWindows = hbInstance->allMainWindows();
        return mainWindows.isEmpty() ? 0 : mainWindows.at(0);
    }
    QList<QGraphicsView *> views = scene->views();
    foreach(QGraphicsView *view, views) {
        HbMainWindow *window = qobject_cast<HbMainWindow *>(view);
        if (window) {
            return window;
        }
    }
    return 0;
}

/*!
    A virtual slot to be called on the derived classes to notify in cases when the style
    primitives need to be recreated. Main use case for this is a style change. HbWidget::setStyle()
    causes a call to this method. If a widget wants to support style change it must re-implement this method
    and recreate primitives.
*/
void HbWidget::recreatePrimitives()
{
    Q_D(HbWidget);
    if(d->backgroundPrimitiveType != HbStyle::P_None) {
        if(d->backgroundItem) {
            delete d->backgroundItem;
            d->backgroundItem = 0;
        }
        d->backgroundItem = style()->createPrimitive(d->backgroundPrimitiveType, const_cast<HbWidget*>(this));
    }
}

/*!
    A virtual slot to be called on the derived classes to notify in cases when the style
    primitives need to be updated. For example the HbAbstractButton connects it's toggled
    signal to this slot to notify of checked state change.
*/
void HbWidget::updatePrimitives()
{
    Q_D(HbWidget);

    HbStyleOption option;
    initStyleOption(&option);

    if (backgroundItem()) {
        if(d->backgroundPrimitiveType != HbStyle::P_None) {
            style()->updatePrimitive(backgroundItem(), d->backgroundPrimitiveType, &option);
        }
        d->updateBackgroundItemSize();
    }

    if (d->focusPrimitive(HbWidget::FocusHighlightResidual)) {
        style()->updatePrimitive(d->focusPrimitive(HbWidget::FocusHighlightResidual),
            d->focusResidualType, &option);        
    }

    if (d->focusPrimitive(HbWidget::FocusHighlightActive)) {
        style()->updatePrimitive(d->focusPrimitive(HbWidget::FocusHighlightActive),
            d->focusActiveType, &option);
    }
   
}

/*!
    A virtual slot to be called on the derived classes to notify expiration of the
    highlight timeout. Used when the highlight is transient.
*/
void HbWidget::hideFocusHighlight()
{
    Q_D(HbWidget);
    d->highlightExpired = true;
    d->focusChangeEvent(HbWidget::FocusHighlightNone);
}

/*! 
    Event handler.
*/
//Start of snippet 2
bool HbWidget::event(QEvent *event)
{
    Q_D(HbWidget);

    switch (event->type()) {
        case QEvent::InputMethod:
            inputMethodEvent(static_cast<QInputMethodEvent*>(event));
            return true;

        case QEvent::KeyPress:
            event->ignore();

            if (d->highlightExpired) {
                // Update focus active primitive.
                d->highlightExpired = false;
                d->focusChangeEvent(HbWidget::FocusHighlightActive);
                event->accept();
                return true;
            }

            // if no keys set for focus mode let the widget decide what to
            // do next.
            if (!keyFocusMode().isEmpty()) {
                if (focusMode() == HbWidget::FocusModeEdit) {
                    keyPressEvent(static_cast<QKeyEvent *>(event));
                }
            } else {
                keyPressEvent(static_cast<QKeyEvent *>(event));
            }
            return true;

        case QEvent::KeyRelease:
            keyReleaseEvent(static_cast<QKeyEvent *>(event));
            return true;

        case QEvent::DynamicPropertyChange:             
            if (!qstrcmp(static_cast<QDynamicPropertyChangeEvent *>(event)->propertyName(), "state")) {
                if(!d->themingPending){
                    style()->updateThemedParams(this);                    
                }
            }
            return true;

        default:
            if (event->type() == HbEvent::ChildFocusIn) {
                QGraphicsWidget *parentW = parentWidget();
                //Update last focus child in parent widget's focus group
                if (parentW) {
                    HbWidget *parentHbWidget = qobject_cast<HbWidget *>(parentW);
                    if (parentHbWidget) {
                        HbWidgetPrivate *parentPrivate = static_cast<HbWidgetPrivate *>(parentHbWidget->d_ptr);
                        if (parentPrivate->focusGroup) {
                            parentPrivate->focusGroup->updateCurrentFocusChild(this);
                        }
                    }
                    // Notify to the parent of this widget too
                    QCoreApplication::sendEvent(parentW, event);
                }
                return true;
            } else if (event->type() == HbEvent::ChildFocusOut) {
                // Notify to the parent of this widget too
                if (parentWidget()) {
                    QCoreApplication::sendEvent(parentWidget(), event);
                }
                return true;
            } else if (event->type() == HbEvent::ThemeChanged) {
                changeEvent(event);
                return true;
            } else if (event->type() == HbEvent::DeviceProfileChanged) {
                HbDeviceProfileChangedEvent* dpEvent = static_cast<HbDeviceProfileChangedEvent*>(event);
                if ( dpEvent->profile().alternateProfileName() == dpEvent->oldProfile().name() ) {
                    // This is an orientation change event.
                    if (style()->hasOrientationSpecificStyleRules(this)) {
                        polishEvent();
                    }
                } else {
                    // Some other device profile event (e.g. resolution change)
                    polishEvent();
                }
            }
            break;
    }

    return HbWidgetBase::event(event);
}
//End of snippet 2


/*!
    \reimp
 */
void HbWidget::changeEvent(QEvent *event)
{
    if ( event->type() == HbEvent::ThemeChanged ) {
        style()->updateThemedParams(this);
    }
    HbWidgetBase::changeEvent( event );
}

/*!
    \reimp
 */
QVariant HbWidget::itemChange ( GraphicsItemChange change, const QVariant & value )
{
    Q_D(HbWidget);

#ifdef HB_TESTABILITY
    if (d->testabilitySignal && d->testabilitySignal->signalEnabled()) {
    	emit d->testabilitySignal->propertyChanges(change, value);
    }
#endif //HB_TESTABILITY

	if (change == QGraphicsItem::ItemVisibleHasChanged) {
        if (!d->polished && !value.toBool()) {

            // temporary solution starts -->>
            // bypass QT defect: 251309
            // when the defect is fixed, this needs to be removed to prevent calling the polish
            // twice.
#if (QT_VERSION < QT_VERSION_CHECK(4, 6, 1))            
			if ( !scene() ) {
                d->polishPending = true;
            }
#endif
            // <<-- temporary solution ends


            // Don't call baseclass imp to prevent hideEvent()
            // to be sent when the widget is not polished yet.
            return value;
        }
        if (d->polishPending && value.toBool()) {
            d->polishPending = false;
            HbStyleParameters params;
            polish( params );
        }
    }
    else if (change == QGraphicsItem::ItemChildAddedChange) {
        if (d->focusGroup) {
            QGraphicsItem *child = qVariantValue<QGraphicsItem *>(value);
            d->focusGroup->registerItem(child);
        }
    }
    else if (change == QGraphicsItem::ItemChildRemovedChange) {
        if (d->focusGroup) {
            QGraphicsItem *child = qVariantValue<QGraphicsItem *>(value);
            d->focusGroup->unregisterItem(child);
        }
    }
    else if (change == QGraphicsItem::ItemSceneChange){
        // The widget receives ItemSceneHasChanged in two cases: when added to
        // scene and when removed from it.  we want to notify hbgraphicsscene to
        // handle polish and layoutrequests only when item is being added to
        // scene and not when being removed.
        d->notifyScene = true;
    }
    else if (change == QGraphicsItem::ItemSceneHasChanged) {
        if (d->focusGroup) {
            d->focusGroup->registerChildren(this);
        }
        if (d->notifyScene && scene()) {
            // The widget has been added to a new scene and this would result in
            // polishing widget. As we set layout to widget in polishEvent,
            // inform scene to handle polish and any resulting layoutrequest
            // events before drawing.
            d->notifyScene = false;
            HbGraphicsScene *hbscene = qobject_cast<HbGraphicsScene*>(scene());
            if (hbscene) {
                HbGraphicsScenePrivate::d_ptr(hbscene)->mPolishWidgets = true;
            }
        }
    }
    else if( change == QGraphicsItem::ItemEnabledHasChanged) {
         if(!d->themingPending){
            style()->updateThemedParams(this);
         }
    }
    return HbWidgetBase::itemChange(change, value);
}

/*!
    \reimp

    Calls polish(HbStyleParameters& params).

    Normally you shouldn't need to override this method. If you want to do something
    special in polish override polish(HbStyleParameters& params) instead. 

    \sa polish(HbStyleParameters& params), repolish()
 */
void HbWidget::polishEvent()
{   
    HbWidgetBase::polishEvent();
    HbStyleParameters params;
    polish( params );   
}

/*!
    Polishes the widget.

    Calls HbStyle::polish and sets polished flag to true.

    Override this method if you want to do something special in polish
    (e.g. fetch some custom style parameters to your widget from style
    backend). When overriding, always call the base classes 
    impelentation.

    If you are implementing a custom widget that has a style plugin
    you must make sure that pluginBaseId is set.

    \param params, For querying (custom) style parameters from HbStyle.

    \sa polish(), pluginBaseId()
*/
void HbWidget::polish( HbStyleParameters& params )
{
    Q_D(HbWidget);
    if (isVisible()) {
        style()->polish(this, params);
        d->polished = true;
        d->repolishOutstanding = false;
        if (d->themingPending) {
            style()->updateThemedParams(this);
            d->themingPending = false;
        }
    } else {
        d->polishPending = true;
    }
}

/*!
    Convenience method for polishing.

    Calls polish(HbStyleParameters& params) asynchronously.

    \sa polish(HbStyleParameters& params)
*/
void HbWidget::repolish()
{
    Q_D(HbWidget);
    // "d->polished" check removed because there's a bug in QGraphicsScene (Qt bug id 251309):
    // "polishEvent" is never sent to items that were invisible when added to scene.
#if (QT_VERSION < QT_VERSION_CHECK(4, 6, 1))       
	if (!d->repolishOutstanding) {
#else
	if (d->polished && !d->repolishOutstanding) {
#endif
        d->repolishOutstanding = true;
        QEvent* polishEvent = new QEvent( QEvent::Polish );
        QCoreApplication::postEvent(this, polishEvent);
    }
}

/*!

    \deprecated HbWidget::primitive(HbStyle::Primitive)
        is deprecated. Use HbWidget::primitive(const QString) instead.
*/
QGraphicsItem *HbWidget::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbWidget);
    if (primitive == d->backgroundPrimitiveType) {
        return backgroundItem();
    }
    if (primitive == d->focusActiveType) {
        return d->focusPrimitive(HbWidget::FocusHighlightActive);
    }
    if (primitive == d->focusResidualType) {
        return d->focusPrimitive(HbWidget::FocusHighlightResidual);
    }

    return 0;
}

/*!
    This is the default implementation of style primitive accessing. The method iterates through all widget's child items
    and returns the child QGraphicsItem matching to \a itemName parameter. The \a itemName parameter matches to the 
    item names set by the HbStyle::setItemName(). The item name should match with widget's WidgetML CSS definition.

    Hb widgets can reimplement this method to do optimized lookup of primitives meaning that widget's implementation of this 
    method will return correct primitives without going through all child items.

    The returned QGraphicsItem can be used for example to perform an application-specific effect for widget's primitive.

    \param itemName The string of the itemName used to search for the primitive.
    \return QGraphicsItem* pointer to QGraphicsItem that matches the itemName. Returns 0 if not found.

    \sa HbStyle::setItemName(QGraphicsItem *, const QString)
    \sa HbStyle::itemName()
*/
QGraphicsItem *HbWidget::primitive(const QString &itemName) const
{
    if ( itemName == "" ) {
        return 0;
    } else {
        QList<QGraphicsItem*> list = childItems();
        for ( int i = 0 ; i < list.count() ; i++  ) {
            QGraphicsItem* item = list.at(i);
            if ( HbStyle::itemName(item) == itemName ) {
                return item;
            }
        }
    }
    return 0;
}

/*!
    Returns primitive which HbStyle::itemName equals to \a itemName.
    
    If the \a itemName is empty, the layout is returned. 
    
    If the \a itemName does not match any primitive, \a itemName is mapped to owned spacers.

    If the \a itemName cannot be mapped to any of the above, returns 0.

    \sa HbStyle::itemName()
*/
QGraphicsLayoutItem *HbWidget::layoutPrimitive(const QString &itemName) const
{
    Q_D(const HbWidget);
    if ( itemName == "" ) {
        return layout();
    } else {
        QList<QGraphicsItem*> list = childItems();
        for ( int i = 0 ; i < list.count() ; i++  ) {
            QGraphicsItem* item = list.at(i);
            if ( HbStyle::itemName(item) == itemName ) {
                return item->isWidget() ? static_cast<QGraphicsWidget*>(item) : 0;
            }
        }
    }

    if ( d->mSpacers.contains(itemName) ) {
        return d->mSpacers.value(itemName); 
    }
 
    return 0;
}

/*!
    Defines the orientation to move focus from focused widget to previous or next
    widget. Keys used to move focus are determined from the orientation. In horizontal
    orientation the key to move focus to previous widget is Qt::Key_Left and to next widget
    Qt::Key_Right. In vertical orientation the key to move focus to previous widget is
    Qt::Key_Up and to next widget Qt::Key_Down. Both orientations can be used. Default:
    QKeySequence.

    \param previous determines orientation to move focus to previous child.
    \param next determines orientation to move focus to next child.
*/
void HbWidget::setFocusOrientation(const Qt::Orientations previous, const Qt::Orientations next)
{
    Q_D(HbWidget);
    if (!d->focusGroup) {
        d->focusGroup = new HbFocusGroup(this);
        d->focusGroup->registerChildren(this);
    }

    QKeySequence seq;

    if (previous.testFlag(Qt::Horizontal) && previous.testFlag(Qt::Vertical)) {
        seq = QKeySequence(Qt::Key_Left, Qt::Key_Up);
    } else if (previous.testFlag(Qt::Horizontal)) {
        seq = QKeySequence(Qt::Key_Left);

    } else if (previous.testFlag(Qt::Vertical)) {
        seq = QKeySequence(Qt::Key_Up);
    }

    d->focusGroup->setKeyForFocusPreviousChild(seq);

    seq = QKeySequence();
    if (next.testFlag(Qt::Horizontal) && next.testFlag(Qt::Vertical)) {
        seq = QKeySequence(Qt::Key_Right, Qt::Key_Down);
    } else if (next.testFlag(Qt::Horizontal)) {
        seq = QKeySequence(Qt::Key_Right);

    } else if (next.testFlag(Qt::Vertical)) {
        seq = QKeySequence(Qt::Key_Down);
    }

    d->focusGroup->setKeyForFocusNextChild(seq);
}

/*!
    Returns key (or keys) for focusing next child.

    \sa setFocusOrientation()
*/
QKeySequence HbWidget::keyForFocusNextChild() const
{
    Q_D(const HbWidget);
    return d->focusGroup?d->focusGroup->keyForFocusNextChild():QKeySequence();
}

/*!
    Returns key (or keys) for focusing previous child.

    \sa setFocusOrientation()
*/
QKeySequence HbWidget::keyForFocusPreviousChild() const
{
    Q_D(const HbWidget);
    return d->focusGroup?d->focusGroup->keyForFocusPreviousChild():QKeySequence();
}

/*!
    Set key that serve as shortcut to change the used focus mode. Focus mode
    toggles between FocusNavigationMode and FocusEditMode.

    \param key deterimines the key used to toggle focus mode property.
*/
void HbWidget::setKeyFocusMode(const QKeySequence& key)
{
    Q_D(HbWidget);
    if (!d->focusGroup) {
        d->focusGroup = new HbFocusGroup(this);
        d->focusGroup->registerChildren(this);
    }
    d->focusGroup->setKeyForFocusMode(key);
}

/*!
  Returns the key used for changing the focus mode.

  \sa setKeyFocusMode()
*/
QKeySequence HbWidget::keyFocusMode() const
{
    Q_D( const HbWidget);
    HbFocusGroup *group = d->getFocusGroup(false);
    return group?group->keyFocusMode():QKeySequence();
}

/*!
    Returns the last focus child if there is any or 0.
*/
QGraphicsItem *HbWidget::lastFocusedChild() const
{
    Q_D(const HbWidget);
    return d->focusGroup?d->focusGroup->currentFocusChild():0;
}

/*!
    Moves the \a second item around the ring of focus items so that
    keyboard focus moves from the \a first item to the \a second item when
    the focus next key is pressed.

    If \a first is 0, this indicates that \a second should be the first item
    to receive input focus should this widget gain next focus with focus reason other than Qt::BacktabFocusReason.
    If \a second is 0, this indicates that \a first should be the first widget to gain focus if the
    scene gained focus with focus reason Qt::BacktabFocusReason.

    By default, focus order is defined implicitly using item creation order.

    \sa focusDelegation()
*/
void HbWidget::setFocusOrder(QGraphicsItem *first, QGraphicsItem *second)
{
    Q_D(HbWidget);

    if (!first && !second) {
        qWarning("HbWidget::setFocusOrder(0, 0) is undefined");
        return;
    }

    if ((first && second) && first->scene() != second->scene()) {
        qWarning("HbWidget::setFocusOrder: scenes %p and %p are different",
        first->scene(), second->scene());
        return;
    }

    QGraphicsScene *scene = first ? first->scene() : second->scene();
    if (!scene && (!first || !second)) {
        qWarning("HbWidget::setFocusOrder: assigning focus order from/to the"
                 " scene requires the item to be in a scene.");
        return;
    }

    if (!d->focusGroup) {
        d->focusGroup = new HbFocusGroup(this);
        d->focusGroup->registerChildren(this);
    }

    if (first && !d->focusGroup->isItemRegistered(first)) {
        qWarning("HbWidget::setFocusOrder(first,second): first %p is not focusable or not a child of %p", first, this);
        return;
    }

    if (second && !d->focusGroup->isItemRegistered(second)) {
        qWarning("HbWidget::setFocusOrder(first,second): second %p is not focusable or not a child of %p", second, this);
        return;
    }

    d->focusGroup->setFocusOrder(first,second);
}

/*!
    Sets the focus delegation policy property of the widget.

    \param focusDelegation determines if the focus is delegated further and to which
    graphics item upon this widget receiving the focus.
    Default: HbWidget::FocusDelegationFirstChild

    \sa focusDelegation()
*/
void HbWidget::setFocusDelegation(HbWidget::FocusDelegation focusDelegation)
{
    Q_D(HbWidget);
    if (!d->focusGroup) {
        d->focusGroup = new HbFocusGroup(this);
        d->focusGroup->registerChildren(this);
    }
    d->focusGroup->setFocusDelegation(focusDelegation);
}

/*!
    Returns focus delegation policy property of the widget.

    \sa setFocusDelegation()
*/
HbWidget::FocusDelegation HbWidget::focusDelegation() const
{
    Q_D(const HbWidget);
    return d->focusGroup?d->focusGroup->focusDelegation():FocusDelegationFirstChild;
}

/*!
    Enables or disables focus looping. If the focus looping is enabled the focus is moved to first child
    from the last child when a key defined in setFocusOrientation pressed or is moved to last child
    from the firt child when a key defined in setFocusOrientation pressed.
    Note: The first and last child in this context refers the first and last child in
        the widget's focus chain defined by setFocusOrder.
    Default: true

    \sa hasFocusLooping(), focusDelegation(), setFocusOrientation()
*/
void HbWidget::setFocusLooping(bool enable)
{
    Q_D(HbWidget);
    if (!d->focusGroup) {
        d->focusGroup = new HbFocusGroup(this);
        d->focusGroup->registerChildren(this);
    }
    d->focusGroup->setFocusLooping(enable);
}

/*!
    Returns the focus looping property of the widget.

    \sa setFocusLooping(), setFocusOrder()
*/
bool HbWidget::hasFocusLooping() const
{
    Q_D(const HbWidget);
    return d->focusGroup?d->focusGroup->hasFocusLooping():true;
}

/*!
    Sets focus navigation mode of the widget.
    Note: If the key for changing the focus mode is not set, focus is not moved if widget uses the
        event
    Default: HbWidget::FocusModeNormal

    \param \a mode determines how moving the focus is done. In FocusMode focus traversal is
    not performed.

    \sa focusMode(), setKeyFocusMode()
*/
void HbWidget::setFocusMode(HbWidget::FocusMode mode) {
    Q_D(HbWidget);

    if (!d->focusGroup) {
        d->focusGroup = new HbFocusGroup(this);
        d->focusGroup->registerChildren(this);
    }
    d->focusGroup->setFocusMode(mode);
}

/*!
    Returns the focus mode property of the widget. If widget is not
    a container (does not have focus group) property value is fetched
    from the focus group widget belongs to.

    \sa setFocusMode(), keyFocusMode()
*/
HbWidget::FocusMode HbWidget::focusMode() const
{
    Q_D( const HbWidget);
    HbFocusGroup *group = d->getFocusGroup(false);
    return group?group->focusMode():HbWidget::FocusModeNormal;
}

/*

    \deprecated HbWidget::setFocusHighlight(HbStyle::Primitive, HbWidget::FocusHighlight)
    is deprecated. This method will be made private and should not be used by public API users.

    Sets focus primitives defined in HbStyle. If FocusHighlightNone is set both active
    and residual focus primitives are deleted.

    \param type Defines the primitive for the highlight.
    \param focusHighlight Defines the highlight the primitive is used.
*/
void HbWidget::setFocusHighlight(HbStyle::Primitive type, HbWidget::FocusHighlight focusHighlight)
{
    Q_D(HbWidget);
    if (focusHighlight == HbWidget::FocusHighlightNone) {
        d->focusActiveType = HbStyle::P_None;
        d->focusResidualType = HbStyle::P_None;

        delete d->focusActiveItem;
        d->focusActiveItem = 0;
        delete d->focusResidualItem;
        d->focusResidualItem = 0;
    } else if (focusHighlight == HbWidget::FocusHighlightActive) {
        d->focusActiveType = type;
    } else {
        d->focusResidualType = type;
    }
}

/*!

    \deprecated HbWidget::focusHighlight(HbWidget::FocusHighlight)
    is deprecated. This method will be made private and should not be used by public API users.

    Returns identifier of the focus primitive defined in HbStyle for focus highlight.
    \param highlightType defines the highlight type.
*/
HbStyle::Primitive HbWidget::focusHighlight(HbWidget::FocusHighlight highlightType)
{
    Q_D(HbWidget);
    HbStyle::Primitive primitive(HbStyle::P_None);

    if (highlightType == HbWidget::FocusHighlightActive) {
        primitive = d->focusActiveType;
    } else if (highlightType == HbWidget::FocusHighlightResidual) {
        primitive = d->focusResidualType;
    }
    return primitive;
}

/*! 
    
    \deprecated HbWidget::overrideFeedback(Hb::InstantInteraction) const
        is deprecated. Effect overriding is done via properties.

    Default implementation of a virtual method for overriding instant feedback effects.
    Returns HbFeedback::NoOverride by default.
    Widgets can reimplement this method to override the default feedback effects (if
    there is a strong need not to use the default ones).

    \param interaction the interaction event triggered by the widget
*/
HbFeedback::InstantEffect HbWidget::overrideFeedback(Hb::InstantInteraction interaction) const {
    Q_UNUSED(interaction);

    return HbFeedback::NoOverride;
}

/*! 

    \deprecated HbWidget::overrideContinuousFeedback(Hb::ContinuousInteraction, int*) const
        is deprecated. Effect overriding is done via properties.

    Default implementation of a virtual method for overriding continuous feedback effects.
    Returns HbFeedback::NoContinuousOverride by default.
    Widgets can reimplement this method to override the default continuous feedbacks
    and/or their intensity (if there is a strong need not to use the default ones).

    \param interaction  the interaction event triggered by the widget
    \param intensity    the default intensity of the effect; can be overriden to any value
                        between HbFeedback::IntensityZero, HbFeedback::IntensityFull
*/
HbFeedback::ContinuousEffect HbWidget::overrideContinuousFeedback(Hb::ContinuousInteraction interaction, int *intensity) const {
    Q_UNUSED(interaction);
    Q_UNUSED(intensity);

    return HbFeedback::NoContinuousOverride;
}

