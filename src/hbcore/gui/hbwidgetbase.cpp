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

#include <QApplication>
#include <QGraphicsWidget>
#include <QPainter>
#include <QDebug>
#include <hbtextitem.h>
#ifdef HB_EFFECTS
#include <hbeffect.h>
#include <hbeffectinternal_p.h>
#endif
#include "hbstyleoption_p.h"
#include "hbfontspec.h"
#include "hbwidgetbase.h"
#include "hbwidgetbase_p.h"
#include "hbevent.h"


/*!
    \class HbWidgetBase

    \brief HbWidgetBase is a common base for all Hb widgets and primitives. 
    It contains common functionality shared between these two types.

    Currently HbWidgetBase offers the following functionality:
        - Layout direction locking                 
*/

HbWidgetBasePrivate::HbWidgetBasePrivate() :
        mApiProtectionFlags(0),
        attributes(0)
{
    q_ptr = 0;
}

HbWidgetBasePrivate::~HbWidgetBasePrivate()
{
}

void HbWidgetBasePrivate::init()
{
    Q_Q( HbWidgetBase );
    QGraphicsItem *item = q->parentItem();
    if ( item ) {
        handleInsidePopup(item);
    }
}

void HbWidgetBasePrivate::handleInsidePopup(const QGraphicsItem *parent)
{
    Q_Q( HbWidgetBase );
    if(     parent
        &&  parent->isWidget()) {
        const HbWidgetBase *widgetBase = qobject_cast<const HbWidgetBase*>(static_cast<const QGraphicsWidget*>(parent));
        if (widgetBase) {
            if (q->testAttribute(Hb::InsidePopup) != widgetBase->testAttribute(Hb::InsidePopup)) {
                setInsidePopup(widgetBase->testAttribute(Hb::InsidePopup));
            }
        }
    }
}

void HbWidgetBasePrivate::setInsidePopup(bool insidePopup)
{
    Q_Q( HbWidgetBase );
    q->setAttribute(Hb::InsidePopup, insidePopup ? true : false);
    if (insidePopup) {
        q->setProperty("insidePopup", true);
    } else {
        q->setProperty("insidePopup", QVariant());
    }

    QList<QGraphicsItem *> items = q->childItems();
    int count = items.count();
    for (int i=0; i< count; i++) {
        QGraphicsItem *item = items.at(i);
        if (    item
            &&  item->isWidget()) {
            HbWidgetBase *childWidgetBase = qobject_cast<HbWidgetBase*>(static_cast<QGraphicsWidget*>(item));
            if (    childWidgetBase
                &&  q->testAttribute(Hb::InsidePopup) != childWidgetBase->testAttribute(Hb::InsidePopup)) {
                childWidgetBase->d_func()->setInsidePopup(insidePopup);
            }
        }
    }
}

/*
  Returns if the HW key navigation is enabled or not.
 */
bool HbWidgetBasePrivate::keyNavigation() const
{
    return false;
}

/*!
    Constructs an HbWidgetBase with the parent item and window flags.
    This constructor creates a new HbWidgetBasePrivate instance and 
    installs it to the widget.
*/
HbWidgetBase::HbWidgetBase( QGraphicsItem *parent, Qt::WindowFlags wFlags ):
    QGraphicsWidget(parent, wFlags), d_ptr( new HbWidgetBasePrivate )
{
    Q_D( HbWidgetBase );
    d->q_ptr = this;
    d->init();
}

/*!
    Constructs an HbWidgetBase with the given private object, parent item and window flags.
    This given HbWidgetBasePrivate instance is installed in the widget and deleted in its destructor. 
*/
HbWidgetBase::HbWidgetBase(HbWidgetBasePrivate &dd, QGraphicsItem *parent, Qt::WindowFlags wFlags):
    QGraphicsWidget(parent, wFlags), d_ptr( &dd )
{
    Q_D( HbWidgetBase );
    d->q_ptr = this;
    d->init();
}


/*!
 Destroys the widget.
*/
HbWidgetBase::~HbWidgetBase()
{
#ifdef HB_EFFECTS
    //Cancel the effect and do not send the callback notification
    HbEffect::enable(this); // remove from disabled list
    HbEffect::cancel(this, QString(), true, false, false);
#endif
    delete d_ptr;
}

/*!
 * \reimp
 * Handles font changes.
 * Remeber to call old implmentation when reimplementing this method.
 */
bool HbWidgetBase::event(QEvent *e)
{
    if (e->type() == HbEvent::ThemeChanged) {
        changeEvent(e);
    }

    // This class will be rebased to QGraphicsWidget
    // that is why direct ancestor is not called
    return QGraphicsWidget::event(e);
}


bool HbWidgetBase::sceneEvent(QEvent *event)
{
    bool result = QGraphicsWidget::sceneEvent(event);
    if(!result && event->type() == QEvent::Gesture &&
       !isBlockedByModalPanel() // workaround for missing panel support in qt
        ) {
        gestureEvent(static_cast<QGestureEvent *>(event));
        return true;
    }
    return result;
}

/*!
 * \reimp
 * Function handles attribute Hb::InsidePopup.
 */
QVariant HbWidgetBase::itemChange(GraphicsItemChange change, const QVariant &value)
{
    Q_D(HbWidgetBase);

    if( change == QGraphicsItem::ItemParentChange) {
        d->handleInsidePopup(value.value<QGraphicsItem *>());
    }
    return QGraphicsWidget::itemChange(change, value);
}

/*!
    Returns the fontSpec property of the widget.

    The font specification defines the font with a font role and optional other
    parameters.

    \sa setFontSpec(), effectiveFontSpec()
*/
HbFontSpec HbWidgetBase::fontSpec() const
{
    Q_D(const HbWidgetBase);
    return d->fontSpec;
}

/*!
     Sets the \a fontSpec property of the widget.

     The font specification defines the font with a font role and optional other
     parameters. Setting fontSpec property also sets the font property to a font
     matching the specification.

     \note If the font property of the widget is changed directly, the fontSpec
     property is cleared.

    \sa fontSpec(), effectiveFontSpec()
*/
void HbWidgetBase::setFontSpec(const HbFontSpec &fontSpec)
{
    Q_D(HbWidgetBase);
    d->fontSpec = fontSpec;

    QFont oldFont = font();
    setFont(fontSpec.font());

    // Work-around for Qt "setFont over-optimized" problem.
    if ( oldFont != font() ) {
        QGraphicsLayoutItem::updateGeometry();
    }
}

/*!
    Returns the effective fontSpec of the widget. This may be different
    than the set fontSpec.
    
    FontSpec may be inherited from the parent item if it's not explicitly
    set to this widget. 

    Font may be overriden by setFont in which case the originally set 
    fontSpec may not be effective anymore. In this case Undefined fontSpec
    is returned.

    \sa fontSpec(), setFontSpec()
*/
HbFontSpec HbWidgetBase::effectiveFontSpec() const
{
    Q_D(const HbWidgetBase);
    HbFontSpec spec = d->fontSpec;
    QGraphicsItem* parent = parentItem();
    while (spec.role() == HbFontSpec::Undefined && parent) {
        if (parent->isWidget()) {
            QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(parent);
            HbWidgetBase *widgetBase = qobject_cast<HbWidgetBase *>(widget);
            if ( widgetBase ) {
                spec = widgetBase->fontSpec();
            }
        }
        parent = parent->parentItem();
    }

    if (spec.role() != HbFontSpec::Undefined) {
        QFont fontSpecFont = spec.font().resolve(QApplication::font());
        if (font() != fontSpecFont) {
            spec = HbFontSpec();
        }
    }

    return spec;
}

/*!
    See the class documentation for QGraphicsWidget for a complete list of
    which attributes are supported, and what they are for.

    \sa testAttribute()
*/
void HbWidgetBase::setAttribute(Qt::WidgetAttribute attribute, bool on)
{
    QGraphicsWidget::setAttribute(attribute, on);
}

/*!
    See the class documentation for QGraphicsWidget.

    \sa setAttribute()
*/
bool HbWidgetBase::testAttribute(Qt::WidgetAttribute attribute) const
{
    return QGraphicsWidget::testAttribute(attribute);
}

/*!
    If \a on is true, this function enables \a attribute; otherwise
    \a attribute is disabled.

    See Hb namespace reference for a complete list of
    which attributes are supported, and what they are for.

    \sa testAttribute()
*/
void HbWidgetBase::setAttribute(Hb::WidgetAttribute attribute, bool on)
{
    Q_D(HbWidgetBase);
    int bit = d->attributeToBitIndex(attribute);
    if (bit == -1) {
        return;
    }

    int k = (1 << bit);
    Q_UNUSED(k);
    if ( on )
        d->attributes |= (1 << bit);
    else
        d->attributes &= ~(1 << bit);
}

/*!
    Returns true if \a attribute is enabled for this widget; otherwise,
    returns false.

    \sa setAttribute()
*/
bool HbWidgetBase::testAttribute(Hb::WidgetAttribute attribute) const
{
    Q_D(const HbWidgetBase);
    bool res;
    int bit = d->attributeToBitIndex(attribute);
    if (bit == -1)
        return false;
    res = (d->attributes & (1 << bit)) != 0;
    return res;
}

/*
 * \reimp
 * Initializes the style options common for all widgets within the library. 
 * 
 * This function sets the following style \a option parameters; state, direction, rect 
 * and boundingRect. 
 * 
 * The state style option consists of flags. If this widget is enabled the flag 
 * QStyle::State_Enabled is set. If this widget is active the flag QStyle::State_Active
 * will be set. 
 * 
 * Derived implementations should create a base call to this method.
 * This function does not call the base class implementation.
*/
void HbWidgetBase::initStyleOption(HbStyleOption *option) const
{
    // This function does not call the base class implementation because it is doing
    // time consuming operations (isUnderMouse() and hasFocus()) that are not needed 
    // in this library.
    Q_ASSERT(option);

    option->state = QStyle::State_None;
    if (isEnabled())
        option->state |= QStyle::State_Enabled;
    
    if (QGraphicsWidget *w = window()) {
        if (w->isActiveWindow())
            option->state |= QStyle::State_Active;
    }

    option->direction = layoutDirection();
    option->rect = rect().toRect(); 
    option->boundingRect = boundingRect();
}

/*!
    This event handler, for \a event, receives gesture events. Its base
    implementation ignores all gestures delivered in the \a event.

    You can reimplement this handler in a subclass of HbWidgetBase to
    provide your own custom gesture handling.

    \sa event()
*/
void HbWidgetBase::gestureEvent(QGestureEvent *event)
{
    event->ignore();
    foreach(QGesture *g, event->gestures()) {
        event->ignore(g);
    }
}
