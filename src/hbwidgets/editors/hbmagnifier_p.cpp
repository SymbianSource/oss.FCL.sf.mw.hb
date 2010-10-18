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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Hb API.  It exists purely as an
// implementation detail.  This file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include "hbmagnifier_p.h"
#include "hbwidget_p.h"
#include "hbframedrawer.h"
#include "hbframeitem.h"
#include "hbicon.h"
#include "hbeffect.h"
#include <QGraphicsItem>
#include <QGraphicsSceneResizeEvent>
#include <QPainter>
#include <QDebug>
#include <QPixmap>

/*
    @beta
    @hbwidgets
    \class HbMagnifier
    \brief HbMagnifier is a widget class for magnifying a content of another widget.

*/


class HbMagnifierPrivate :public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbMagnifier)

public:
    HbMagnifierPrivate();
    virtual ~HbMagnifierPrivate();
    void init();
    void updatePaintInfo();
    void drawBackground(QPainter * painter);
    void drawContent(QPainter * painter, const QStyleOptionGraphicsItem *option);
    void drawMask(QPainter * painter);
    void drawOverlay(QPainter * painter);
    void _q_hidingEffectFinished(const HbEffect::EffectStatus &status);


public:

    HbIcon background;
    HbMagnifierDelegate *delegate;
    HbFrameDrawer* maskDrawer;
    HbFrameDrawer* overlayDrawer;
    qreal contentScaleFactor;    
    QPointF centerOnContentPos;
    bool centerOnContentPosSet;
    // The offset from the content point to be displayed in the center of the magnifier
    QPointF centerOffset;
    QRectF  exposedContentRect;
    QPointF lockPos;
    bool lockPosSet;
    HbMagnifier::ContentLockStyle contentLockStyle;
    bool isHidingInProgress;
};

HbMagnifierPrivate::HbMagnifierPrivate():
        delegate(0),
        maskDrawer(0),
        overlayDrawer(0),
        contentScaleFactor(1.0),
        centerOnContentPosSet(false),
        lockPosSet(false),
        contentLockStyle(HbMagnifier::ContentLockStyleSmooth),
        isHidingInProgress(false)
{
}

HbMagnifierPrivate::~HbMagnifierPrivate()
{
    delete delegate;
    delete maskDrawer;  
}

void HbMagnifierPrivate::init()
{
    Q_Q(HbMagnifier);
    q->setFlag(QGraphicsItem::ItemClipsToShape);

    HbEffect::add("magnifier","magnifier_active","active");
    HbEffect::add("magnifier","magnifier_disappear","disappear");
}

void HbMagnifierPrivate::updatePaintInfo()
{
    Q_Q(HbMagnifier);

    // For performance set exposed rect to avoid rendering full content
    exposedContentRect = q->boundingRect();

    // Compensate for scaling
    exposedContentRect.setWidth(exposedContentRect.width()/contentScaleFactor);
    exposedContentRect.setHeight(exposedContentRect.height()/contentScaleFactor);
    QPointF  magnifierCenter = exposedContentRect.center();

    if (centerOnContentPosSet) {
        QPointF centerOfExposedContentRect = centerOnContentPos;
        if (lockPosSet) {
            
            QPointF centerMoveOffset = q->pos() - lockPos;
            if (contentLockStyle == HbMagnifier::ContentLockStyleFrozen) {
                centerMoveOffset /= contentScaleFactor;
            }

            centerOfExposedContentRect += centerMoveOffset;
        }
        exposedContentRect.moveCenter(centerOfExposedContentRect);

        centerOffset = magnifierCenter-centerOfExposedContentRect;
    }
}

void HbMagnifierPrivate::drawBackground(QPainter * painter)
{   
    Q_Q(HbMagnifier);
    background.paint(painter,q->boundingRect(),Qt::IgnoreAspectRatio);
}

void HbMagnifierPrivate::drawContent(QPainter * painter, const QStyleOptionGraphicsItem *option)
{
    if (delegate) {
        //QTransform oldTransform = painter->transform();
        //QPainter::RenderHints oldRenderHints = painter->renderHints();

        // We have to save the full painter state because of a Qt bug: QTBUG-7458 concerning clipping
        // region on raster backed paint device.
        // Once it is fixed it is enough save and restore transform and renderHints
        painter->save();

        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter->setRenderHint(QPainter::TextAntialiasing);

        QStyleOptionGraphicsItem tmpOption = *option;
        tmpOption.exposedRect = exposedContentRect;

        QTransform transform;
        transform.scale(contentScaleFactor,contentScaleFactor);
        transform.translate(centerOffset.x(),centerOffset.y());
        painter->setTransform(transform);
        delegate->drawContents(painter, &tmpOption);

        painter->restore();
        // Restore painter settings
        //painter->setTransform(oldTransform);
        //painter->setRenderHints(oldRenderHints);

    }
}

void HbMagnifierPrivate::drawMask(QPainter * painter)
{
    Q_Q(HbMagnifier);

    if (maskDrawer) {
        painter->setCompositionMode( QPainter::CompositionMode_DestinationIn );
        maskDrawer->paint(painter,q->boundingRect());
    }
}

void HbMagnifierPrivate::drawOverlay(QPainter * painter)
{
    Q_Q(HbMagnifier);

    if (overlayDrawer) {
        painter->setCompositionMode( QPainter::CompositionMode_SourceOver );
        overlayDrawer->paint(painter,q->boundingRect());
    }
}

void HbMagnifierPrivate::_q_hidingEffectFinished(const HbEffect::EffectStatus &status)
{
    Q_UNUSED(status);

    Q_Q(HbMagnifier);

    HbEffect::cancel(q, QString(), true);
    q->hide();
    isHidingInProgress = false;
}


/*
* Constructs a magnifier with given  \a parent graphics item.
*/
HbMagnifier::HbMagnifier(QGraphicsItem *parent) :
    HbWidget(*new HbMagnifierPrivate,parent)
{
    Q_D(HbMagnifier);
    d->q_ptr = this;
    d->init();
}


/*
    \internal
 */
HbMagnifier::HbMagnifier(HbMagnifierPrivate &dd, QGraphicsItem *parent) :
    HbWidget(dd, parent)
{
    Q_D(HbMagnifier);
    d->q_ptr = this;
    d->init();
}
/*
* Destroys the magnifier.
*/
HbMagnifier::~HbMagnifier()
{
}

void HbMagnifier::setBackground(const QString& graphicsName)
{
    Q_D(HbMagnifier);
    d->background.setIconName(graphicsName);
    d->background.setSize(boundingRect().size());
}

void HbMagnifier::setMask(const QString& graphicsName)
{
    Q_D(HbMagnifier);
    if (!d->maskDrawer) {
        d->maskDrawer = new HbFrameDrawer();
    }
    if (d->maskDrawer->frameGraphicsName() != graphicsName) {
        d->maskDrawer->setFrameGraphicsName(graphicsName);
        // TODO: Is this line needed?
        d->maskDrawer->setFrameType(HbFrameDrawer::NinePieces);
        update();

    }
}

void HbMagnifier::setOverlay(const QString& graphicsName)
{
    Q_D(HbMagnifier);
    if (!d->overlayDrawer) {
        d->overlayDrawer = new HbFrameDrawer();
    }
    if (d->overlayDrawer->frameGraphicsName() != graphicsName) {
        d->overlayDrawer->setFrameGraphicsName(graphicsName);
        // TODO: Is this line needed?
        d->overlayDrawer->setFrameType(HbFrameDrawer::NinePieces);
        update();
    }
}


void HbMagnifier::setContentDelegate(HbMagnifierDelegate * delegate)
{
    Q_D(HbMagnifier);
    d->delegate = delegate;
}

HbMagnifierDelegate * HbMagnifier::contentDelegate() const
{
    Q_D(const HbMagnifier);
    return d->delegate;
}

void HbMagnifier::setContentScale(qreal factor)
{
    Q_D(HbMagnifier);
    d->contentScaleFactor = factor;

    d->updatePaintInfo();
    update();
}

qreal HbMagnifier::contentScale() const
{
    Q_D(const HbMagnifier);
    return d->contentScaleFactor;
}

void HbMagnifier::centerOnContent ( const QPointF & pos )
{    
    Q_D(HbMagnifier);

    d->centerOnContentPos = pos;
    d->centerOnContentPosSet = true;
    d->updatePaintInfo();
    update();
}

bool HbMagnifier::isContentPositionLocked() const
{
    Q_D(const HbMagnifier);
    return d->lockPosSet;
}

void HbMagnifier::lockContentPosition()
{
    Q_D(HbMagnifier);
    d->lockPos = pos();
    d->lockPosSet = true;
    update();
}

void HbMagnifier::unlockContentPosition()
{
    Q_D(HbMagnifier);
    d->lockPosSet = false;
}

void HbMagnifier::setContentLockstyle(ContentLockStyle contentLockStyle)
{
    Q_D(HbMagnifier);
    d->contentLockStyle = contentLockStyle;
}

HbMagnifier::ContentLockStyle HbMagnifier::contentLockstyle() const
{
    Q_D(const HbMagnifier);
    return d->contentLockStyle;
}


/*
    \reimp
 */
void HbMagnifier::paint(QPainter *painter_res, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    Q_D(HbMagnifier);

    QPixmap contentPixmap(boundingRect().size().toSize());
    contentPixmap.fill(Qt::transparent);
    QPainter painter(&contentPixmap);

    d->drawBackground(&painter);
    d->drawContent(&painter,option);
    d->drawMask(&painter);
    d->drawOverlay(&painter);

    painter.end();

    painter_res->drawPixmap(0,0,contentPixmap);
};

/*
   Triggers the delayed hiding of the magnifier using an effect.
*/
void HbMagnifier::hideWithEffect()
{
    Q_D(HbMagnifier);

    if (isVisible() && !d->isHidingInProgress) {

        d->isHidingInProgress = true;
        HbEffect::start(this, "magnifier", "disappear", this,"_q_hidingEffectFinished");
    }
}

/*
   Shows the magnifier using an effect.
*/
void HbMagnifier::showWithEffect()
{
    if (!isVisible()) {
        show();
        HbEffect::start(this, "magnifier", "active");
    }
}



/*
    \reimp
 */
QVariant HbMagnifier::itemChange ( GraphicsItemChange change, const QVariant & value )
{
    Q_D(HbMagnifier);

    if (change == QGraphicsItem::ItemPositionHasChanged && d->lockPosSet) {
        d->updatePaintInfo();
    }
    return HbWidget::itemChange(change, value);
}


/*
    \reimp
 */
void HbMagnifier::resizeEvent( QGraphicsSceneResizeEvent * event )
{
    Q_D(HbMagnifier);

    HbWidget::resizeEvent(event);

    d->background.setSize(boundingRect().size());
    d->updatePaintInfo();
}


#include "moc_hbmagnifier_p.cpp"

