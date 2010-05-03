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

#include "hbindexfeedback.h"
#include "hbindexfeedback_p.h"

#include "hbabstractitemview.h"

#include <hbscrollbar.h>
#include <hbstyleoptionindexfeedback.h>
#include <hbstyleparameters.h>
#include <hbstyle.h>
#include <hbdeviceprofile.h>

#include <QEvent>
#include <QObject>
#include <QGraphicsScene>

/*!
    @alpha
    @hbwidgets
    \class HbIndexFeedback
    \brief HbIndexFeedback provides index feedback functionality when attached to an HbAbstractItemView.

    Index feedback functionality is provided when the vertical or horizontal scrollbars
    are interactive.

    To add index feedback functionality to an HbAbstractItemView, create the HbIndexFeedback and 
    use HbIndexFeedback::setItemView().

    Index feedback provides a non-interactive popup containing an index when the user taps
    or scrolls, the interactive scrollbar.  The data for the index feedback comes from
    Hb::IndexFeedbackRole in the item view's data model.  The index feedback will
    be displayed if that data can be interpreted as a string by QVariant.

    The sizing and number of characters to display are controlled by the IndexFeedbackPolicy.

    \sa HbAbstractItemView, HbIndexFeedback::IndexFeedbackPolicy.
*/

/*!
    \enum HbIndexFeedback::IndexFeedbackPolicy

    \brief Controls the number of characters to display, font and sizing of the index feedback.

    The default is IndexFeedbackSingleCharacter.  Other options are
    IndexFeedbackThreeCharacter, 
    IndexFeedbackString, and IndexFeedbackNone.
*/

/*!
    \var HbIndexFeedback::IndexFeedbackNone

    There is no index feedback provided.
*/

/*!
    \var HbIndexFeedback::IndexFeedbackSingleCharacter

    Index feedback will be the first letter of the contents of Hb::IndexFeedbackRole
    cast as a string.  It will be displayed using a larger font centered over the list.
    
    Recomended use is for alphabetically sorted lists.
*/

/*!
    \var HbIndexFeedback::IndexFeedbackThreeCharacter

    Index feedback will be the first three letters of the contents of 
    Hb::IndexFeedbackRole cast as a string.  It will be displayed using a larger font
    centered over the list.  
    
    Recomended us is for month abreviations.
*/

/*!
    \var HbIndexFeedback::IndexFeedbackString

    Index feedback will be the contents of Hb::IndexFeedbackRole cast as a string.
    It will be displayed using primary font and centered over the list.
*/

/*!
    Here are the main properties of the class:

    \li HbIndexFeedback::IndexFeedbackPolicy       : Controls the number of characters to display, font and sizing of the index feedback.
    \li HbIndexFeedback::ItemView                  : The item view which the index feedback is provided for.
*/

/* rather than magic numbers, let's define some constants. */
namespace {
/*
    string name from the index feedback .css for the single character height parameter.
*/
static const QString ONE_CHAR_HEIGHT = QLatin1String("one-char-height");

/*
    string name from the index feedback .css for the three character height parameter.
*/
static const QString THREE_CHAR_HEIGHT = QLatin1String("three-char-height");

/*
    string name from the index feedback .css for the three character width parameter.
*/
static const QString THREE_CHAR_WIDTH = QLatin1String("three-char-width");

/*
    string name from the index feedback .css for the string mode offest parameter.
*/
static const QString STRING_OFFSET = QLatin1String("string-offset");
}

/*!
    Constructs a new HbIndexFeedback with a parent.
*/
HbIndexFeedback::HbIndexFeedback(QGraphicsItem *parent)
    : HbWidget( *new HbIndexFeedbackPrivate, parent, 0)
    
{
    Q_D( HbIndexFeedback );
    d->q_ptr = this;

    d->init();
}

/*!
    Destructs the index feedback.
*/
HbIndexFeedback::~HbIndexFeedback()
{

}

/*!
    \brief sets the index feedback policy.

    \param policy - The HbIndexFeedback::IndexFeedbackPolicy to use.

    \sa HbIndexFeedback::IndexFeedbackPolicy
*/
void HbIndexFeedback::setIndexFeedbackPolicy(IndexFeedbackPolicy policy)
{
    Q_D( HbIndexFeedback );

    if (policy != d->mIndexFeedbackPolicy) {
        d->_q_hideIndexFeedbackNow();
        d->mIndexFeedbackPolicy = policy;
        d->calculatePopupRects();
        d->updatePrimitives();
    }
}

/*!
    \brief Returns the index feedback policy.

    \return The HbIndexFeedback::IndexFeedbackPolicy that's currently in use.
*/
HbIndexFeedback::IndexFeedbackPolicy HbIndexFeedback::indexFeedbackPolicy() const
{
    Q_D( const HbIndexFeedback );

    return d->mIndexFeedbackPolicy;
}

/*!
    \brief sets the item view for the index feedback.

    Disconnects from the existing item view, then connects to the specified one.

    If set to NULL the index feedback is simply disconnected.

    \param itemView The HbAbstractItemView* to provide index feedback for.
*/
void HbIndexFeedback::setItemView(HbAbstractItemView *itemView)
{
    Q_D( HbIndexFeedback );

    if (itemView == d->mItemView) {
        return;
    }

    d->disconnectItemView();

    d->mItemView = itemView;

    if (!d->mItemView) {
        return;
    }

    d->connectScrollBarToIndexFeedback(d->mItemView->horizontalScrollBar());
    d->connectScrollBarToIndexFeedback(d->mItemView->verticalScrollBar());

    connect(d->mItemView, SIGNAL(destroyed(QObject*)),
        this, SLOT(_q_itemViewDestroyed()));

    if (d->mItemView->scene()) {
        d->mItemView->scene()->addItem(this);
        d->mItemView->installSceneEventFilter(this);
    }

    d->calculatePopupRects();
    updatePrimitives();
}

/*!
    Returns the HbAbstractItemView which the index feedback currently monitors.

    \return HbAbstractItemView*.
*/
HbAbstractItemView* HbIndexFeedback::itemView() const
{
    Q_D( const HbIndexFeedback );

    return d->mItemView;
}

/*!

    \deprecated HbIndexFeedback::primitive(HbStyle::Primitive)
        is deprecated.

    Returns the primitives used in HbIndexFeedback.

    \param primitive The primitive type requested.

    \return A pointer for the primitive requested.
*/
QGraphicsItem* HbIndexFeedback::primitive(HbStyle::Primitive primitive) const
{
    Q_D( const HbIndexFeedback );

    QGraphicsItem* retVal = HbWidget::primitive(primitive);

    switch (primitive) {
        case HbStyle::P_IndexFeedback_popup_text:
            retVal = d->mTextItem;
            break;
            
        case HbStyle::P_IndexFeedback_popup_background:
            retVal = d->mPopupItem;
            break;

        default:
            qt_noop();
            break;
    }

    return retVal;
}


/*
    A scene event filter.  It's purpose is to call calculatePopupRects on
    a resize event for the item view.
*/
bool HbIndexFeedback::sceneEventFilter(QGraphicsItem *watched, QEvent *ev)
{
    Q_D( HbIndexFeedback );

    if (ev->type() == QEvent::GraphicsSceneResize) {
        d->calculatePopupRects();
    }
    
    return QGraphicsItem::sceneEventFilter(watched, ev);
}

/*
    Rather than adding signals to HbScrollBar specifically to implement 
    index feedback, an event filter is used.

    Specifically, if a scrollbar which is interactive is pressed or released
    this function will call the appropriate function in HbIndexFeedbackPrivate.
*/
bool HbIndexFeedback::eventFilter(QObject *obj, QEvent *ev)
{
    Q_D( HbIndexFeedback );
    HbScrollBar* scrollBar = qobject_cast<HbScrollBar*>(obj);

    if (d->mIndexFeedbackPolicy != HbIndexFeedback::IndexFeedbackNone
        && scrollBar) {
        switch (ev->type()) {
            case QEvent::GraphicsSceneMousePress:
            case QEvent::MouseButtonPress:
                if (scrollBar->isInteractive()) {
                    d->scrollBarPressed();
                }
                break;

            case QEvent::GraphicsSceneMouseRelease:
            case QEvent::MouseButtonRelease:
                if (scrollBar->isInteractive()) {
                    d->scrollBarReleased();
                }
                break;
        
            case QEvent::GraphicsSceneResize:
            case QEvent::Resize:
                    d->_q_hideIndexFeedbackNow();
                    d->calculatePopupRects();
                    d->updatePrimitives();
                break;

            default:
                // do nothing, ignore other events.
                break;
        }
    }

    return QObject::eventFilter(obj, ev);
}

/*
    For use with HbStyle.

    Provide the correct data to use in the 'model.'
*/
void HbIndexFeedback::initStyleOption(HbStyleOptionIndexFeedback *option) const
{
    Q_D( const HbIndexFeedback );

    HbWidget::initStyleOption(option);

    if (!d->mItemView) {
        return;
    }

    HbFontSpec fontSpec;
    qreal margin = 0;

    style()->parameter(QLatin1String("hb-param-margin-gene-popup"), margin);

    switch (d->mIndexFeedbackPolicy) {
        case IndexFeedbackSingleCharacter:
            {
                fontSpec = HbFontSpec(HbFontSpec::Primary);
                fontSpec.setTextHeight(d->textHeight());
            }
            break;
            
        case IndexFeedbackThreeCharacter:
            {
                fontSpec = HbFontSpec(HbFontSpec::Primary);
                fontSpec.setTextHeight(d->textHeight());
            }
            break;

        case IndexFeedbackString:
            {
                fontSpec = HbFontSpec(HbFontSpec::Primary);
                qreal textHeight = 0;
                style()->parameter(QLatin1String("hb-param-text-height-primary"), textHeight);
                fontSpec.setTextHeight( textHeight );
            }
            break;

        case IndexFeedbackNone:
            // leave the HbStyleOption uninitialized
            return;
    }

    option->text = d->mPopupContent;
    option->fontSpec = fontSpec;
    option->textRect = d->mPopupTextRect;
    option->popupRect = d->mPopupBackgroundRect;
}

void HbIndexFeedback::polish(HbStyleParameters& params)
{
    Q_D( HbIndexFeedback );

    params.addParameter( ONE_CHAR_HEIGHT );
    params.addParameter( THREE_CHAR_HEIGHT );
    params.addParameter( THREE_CHAR_WIDTH );
    params.addParameter( STRING_OFFSET );

    HbWidget::polish( params );

    d->mOneCharHeight = params.value( ONE_CHAR_HEIGHT ).toDouble();
    d->mThreeCharHeight = params.value( THREE_CHAR_HEIGHT ).toDouble();
    d->mThreeCharWidth = params.value( THREE_CHAR_WIDTH ).toDouble();
    d->mStringOffset = params.value( STRING_OFFSET ).toDouble();

    d->calculatePopupRects();
}

#include "moc_hbindexfeedback.cpp"
