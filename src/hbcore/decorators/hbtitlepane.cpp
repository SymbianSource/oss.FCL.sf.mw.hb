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

#include "hbtitlepane_p.h"
#include "hbtitlepane_p_p.h"

#include <hbstyleoptiontitlepane.h>
#include <hbapplication.h>
#include <hbwidgetfeedback.h>
#include <hbmainwindow.h>
#include <hbview.h>
#include <hbmenu.h>

#include <QGraphicsSceneMouseEvent>

/*!
	@beta
    @hbcore
    \class HbTitlePane
    \brief HbTitlePane represents a title pane decorator.

    The HbTitlePane class represents title pane on each application. It provides an interface for
    setting text, font alignment and options menu.
*/

HbTitlePanePrivate::HbTitlePanePrivate( ) :
    mText(),
    mTextItem(0),
    mToggled(false),
    mIcon(0),
    mMode(QIcon::Normal)
{

}

void HbTitlePanePrivate::init()
{
    Q_Q(HbTitlePane);

    q->setAcceptedMouseButtons( Qt::LeftButton );
    q->setText( HbApplication::applicationName() );

    createPrimitives();
    updatePrimitives();
}

void HbTitlePanePrivate::toggle(bool on)
{
    mToggled = on;
}

void HbTitlePanePrivate::createPrimitives()
{
    Q_Q(HbTitlePane);

    q->setBackgroundItem(HbStyle::P_TitlePane_background);
    mTextItem = q->style()->createPrimitive(HbStyle::P_TitlePane_text, q);

    mIcon = q->style()->createPrimitive(HbStyle::P_TitlePane_icon, q);
}

void HbTitlePanePrivate::updatePrimitives()
{
    Q_Q(HbTitlePane);
    HbStyleOptionTitlePane option;

    if (q->backgroundItem() == 0 || mTextItem == 0) {
        return;
    }

    q->initStyleOption(&option);
    q->style()->updatePrimitive(q->backgroundItem(), HbStyle::P_TitlePane_background, &option);
    q->style()->updatePrimitive(mTextItem, HbStyle::P_TitlePane_text, &option);
    q->style()->updatePrimitive(mIcon, HbStyle::P_TitlePane_icon, &option);
}

// ======== MEMBER FUNCTIONS ========

/*
    Constructs a vertical title pane with \a parent.
 */

HbTitlePane::HbTitlePane(QGraphicsItem *parent)
    : HbWidget(*new HbTitlePanePrivate, parent)
{
    Q_D(HbTitlePane);
    d->init();
}

HbTitlePane::HbTitlePane(HbTitlePanePrivate &dd, QGraphicsItem *parent)
    : HbWidget(dd, parent)
{
    Q_D(HbTitlePane);
    d->init();
}

/*
    Destructor
 */
HbTitlePane::~HbTitlePane()
{
}

/*
    \property HbTitlePane::text
    \brief This property holds the text
 */

/*
    \brief  Returns the text used in the title pane.
    \return The title pane text.
 */
QString HbTitlePane::text() const
{
    Q_D(const HbTitlePane);
    return d->mText;
}

/*
    Resets icon mode
*/
void HbTitlePane::resetMode()
{
    Q_D(HbTitlePane);
    d->mMode = QIcon::Normal;
#ifdef HB_EFFECTS
    HbEffect::start(this, "decorator", "released");
#endif
}

/*
    \brief  This method sets the text of the title pane.
    \param  text The text to be used in the title pane.
 */
void HbTitlePane::setText(const QString &text)
{
    Q_D(HbTitlePane);

    QString tmp = text;
    if (tmp.isNull()) {
        tmp = HbApplication::applicationName();
    }

    if (tmp != d->mText) {
        d->mText = tmp;
        updatePrimitives();
    }
}

/*
    \reimp
 */
void HbTitlePane::updatePrimitives()
{
    Q_D(HbTitlePane);

    d->updatePrimitives();
}

/*
    \reimp
 */
void HbTitlePane::initStyleOption(HbStyleOptionTitlePane *option) const
{
    Q_D(const HbTitlePane);

    option->caption = d->mText;
    option->mode = d->mMode;
    if (mainWindow() && mainWindow()->currentView()) {
        if (mainWindow()->currentView()->viewFlags() & HbView::ViewTitleBarTransparent) {
            option->transparent = true;
        }
    }
}

/*
    \reimp
 */
void HbTitlePane::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbTitlePane);

    d->mMode = QIcon::Active;
    updatePrimitives();
#ifdef HB_EFFECTS
    if (boundingRect().contains(event->pos())) {
        HbEffect::start(this, "decorator", "pressed");
    }
#endif
    HbWidgetFeedback::triggered(this, Hb::InstantPressed);
    d->toggle(true);    
}

/*
    \reimp
 */
void HbTitlePane::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbTitlePane);

    if (boundingRect().contains(event->pos())) {
        if (d->mMode != QIcon::Active) {
            d->mMode = QIcon::Active;
            updatePrimitives();
        }
    } else {
        if (d->mMode != QIcon::Normal) {
            d->mMode = QIcon::Normal;
            updatePrimitives();
        }
    }
    if (boundingRect().contains(event->pos()) && !d->mToggled) {
        HbWidgetFeedback::triggered(this, Hb::InstantPressed);
        d->toggle(true);
    } else if (!boundingRect().contains(event->pos()) && d->mToggled) {
        HbWidgetFeedback::triggered(this, Hb::InstantReleased);
        d->toggle(false);
    }
}

/*
    \reimp
 */
void HbTitlePane::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbTitlePane);

    if (boundingRect().contains(event->pos())) {
        d->mMode = QIcon::Selected;
        updatePrimitives();
#ifdef HB_EFFECTS
        if (boundingRect().contains(event->pos())) {
            HbEffect::start(this, "decorator", "latched");
        }
#endif
    } else {
        d->mMode = QIcon::Normal;
        updatePrimitives();
#ifdef HB_EFFECTS
        if (boundingRect().contains(event->pos())) {
            HbEffect::start(this, "decorator", "released");
        }
#endif
    }
    if (d->mToggled) {
        HbWidgetFeedback::triggered(this, Hb::InstantReleased);
    }
    if (boundingRect().contains(event->pos())) {
        QPointF pos(scenePos().x() + boundingRect().width() / 2 + 3,
                    scenePos().y() + boundingRect().height());

        HbWidgetFeedback::triggered(this, Hb::InstantClicked);
        emit launchPopup(pos);
    }
}

/*
    \reimp
 */
void HbTitlePane::polish(HbStyleParameters &params)
{
    Q_D(HbTitlePane);
    HbWidget::polish(params);
    if (mainWindow() && mainWindow()->currentView()) {
        if (mainWindow()->currentView()->menu()->isEmpty()) {
            d->mIcon->setVisible(false);
        } else {
            d->mIcon->setVisible(true);
        }
    }
}

/*
    \reimp
 */
bool HbTitlePane::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);
    if (event->type() == QEvent::ActionAdded || event->type() == QEvent::ActionRemoved) {
       repolish();
    }
    return false;
}

/*
    \reimp
*/
QVariant HbTitlePane::itemChange(GraphicsItemChange change, const QVariant & value)
{
    switch (change) {
        case ItemVisibleHasChanged: 
            {
            Q_D(const HbTitlePane);
            if (d->polished)  {
                // Cannot emit signals directly from "visibilityChanged", since it might
                // lead to e.g. item deletion that's not allowed in "itemChange".
                // Using QMetaObject to emit the signal asynchronously.
                QMetaObject::invokeMethod(this, "visibilityChanged", Qt::QueuedConnection);
            }
            break;
            }      
        default:
            break;
    }
    return HbWidget::itemChange(change, value);
}
