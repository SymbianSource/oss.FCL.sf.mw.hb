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
#include "hbmarqueeitem.h"
#include "hbmarqueeitem_p.h"
#include "hbwidgetbase_p.h"
#include "hbtextutils_p.h"
#include "hbevent.h"

#ifdef HB_TEXT_MEASUREMENT_UTILITY
#include "hbtextmeasurementutility_p.h"
#include "hbfeaturemanager_r.h"
#endif //HB_TEXT_MEASUREMENT_UTILITY

#include "hbdeviceprofile.h"
#include "hbcolorscheme.h"
#include "hbnamespace_p.h"

#include <qmath.h>
#include <QPainter>
#include <QPropertyAnimation>

//#define HB_DEBUG_MARQUEE_DRAW_RECTS

namespace {
    // The bigger the value the slower the animation
    static const qreal ANIMATION_SPEED_FACTOR = 2.5;
    static const int ANIMATION_LEAD_TIME = 500;
    static const int ANIMATION_IDENT_BY_PIXEL = 5;

    static const QString DEFAULT_COLORGROUP = "qtc_view_normal";
}



HbMarqueeContent::HbMarqueeContent(HbMarqueeItem *parent) :
    QGraphicsObject(parent),
    parent(parent),
    mTextDirection(Qt::LeftToRight),
    mTextWidth(0),
    mAlpha(0),
    mFadeLength(0)
{
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}


QRectF HbMarqueeContent::boundingRect() const
{
    return mBoundingRect;
}

QPen HbMarqueeContent::pen()
{
    QColor fullColor = parent->textColor();
    QPen pen(fullColor);

    if(parent->contentsRect().width() < mTextWidth) {
        QColor fadeColor = fullColor;
        fadeColor.setAlpha(alpha());

        QLinearGradient gradient;
        gradient.setColorAt(0.0,fullColor);
        gradient.setColorAt(1.0,fadeColor);

        gradient.setStart(gradientStart);
        gradient.setFinalStop(gradientStop);
        pen.setBrush(QBrush(gradient));
    }
    return pen;
}

void HbMarqueeContent::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->save();

    painter->setPen(pen());

    painter->setLayoutDirection ( mTextDirection );
    painter->setFont(parent->font());
    painter->drawText(boundingRect(), Qt::TextDontClip, mText);

#ifdef HB_DEBUG_MARQUEE_DRAW_RECTS
    painter->setPen(Qt::green);
    painter->drawRect(boundingRect());
    painter->setOpacity(0.3);
#endif

    painter->restore();
}


void HbMarqueeContent::updateTextMetaData()
{
    QFontMetricsF metrics(parent->font());

    //calculate bounding rect
    prepareGeometryChange();

    mBoundingRect = metrics.boundingRect(mText);
    mBoundingRect.moveTopLeft(QPointF(0,0));

    // text direction
    bool rightToLeft = HbTextUtils::ImplicitDirectionalityIsRightToLeft(
        mText.utf16(), mText.length(), 0 );
    mTextDirection = rightToLeft ? Qt::RightToLeft : Qt::LeftToRight;

    // Update text width
    mTextWidth = mBoundingRect.width();

    // Update fade length from device profile
    mFadeLength = HbDeviceProfile::profile(this).unitValue()*HbPrivate::TextTruncationFadeWidth;

}


void HbMarqueeContent::setAlpha(int alpha)
{
    mAlpha = alpha;
    update();
}

int HbMarqueeContent::alpha() const
{
    return mAlpha;
}

HbMarqueeItemPrivate::HbMarqueeItemPrivate() :
    content(0),
    mAnimationPending(false)
    
{
}

void HbMarqueeItemPrivate::init()
{
    Q_Q(HbMarqueeItem);
    content = new HbMarqueeContent(q);

    q->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
    QObject::connect(&mAnimGroup, SIGNAL(stateChanged(QAbstractAnimation::State,QAbstractAnimation::State)) ,q,SLOT(_q_stateChanged()));

#ifdef HB_TEXT_MEASUREMENT_UTILITY
    if ( HbFeatureManager::instance()->featureStatus( HbFeatureManager::TextMeasurement ) ) {
        q->setProperty( HbTextMeasurementUtilityNameSpace::textMaxLines, 1 );
    }
#endif
}


void HbMarqueeItemPrivate::updateTextMetaData()
{
    Q_Q(HbMarqueeItem);

    q->update();
    q->updateGeometry();
    content->updateTextMetaData();
    initContentPosition();
    initAnimations();
    content->update();
}

/*
  Initializes the position of the text so that it can be drawn to the screen
  in the initial position, either elided or not
*/
void HbMarqueeItemPrivate::initContentPosition()
{
    Q_Q(HbMarqueeItem);

    QRectF contentRect = content->boundingRect();
    QRectF rect = q->contentsRect();
    contentRect.moveCenter(rect.center());

    if (q->layoutDirection() == Qt::RightToLeft) {
        contentRect.moveRight(rect.right());
    } else {
        contentRect.moveLeft(rect.left());
    }

    if(rect.width() < content->mTextWidth) {
        if (content->mTextDirection == Qt::RightToLeft) {
            contentRect.moveRight(rect.right());

        } else {
            contentRect.moveLeft(rect.left());
        }
    }
    content->setPos(contentRect.topLeft());

    // Calculate the gradient point only after the content was positioned
    initGradient();

}

void HbMarqueeItemPrivate::initGradient()
{
    Q_Q(HbMarqueeItem);
    QRectF rect = q->contentsRect();

    if(rect.width() < content->mTextWidth) {
        if (content->mTextDirection == Qt::RightToLeft) {
            content->gradientStop = q->mapToItem(content, rect.topRight());
            content->gradientStop.rx() -= rect.width();
            content->gradientStart = content->gradientStop;
            content->gradientStart.rx() += content->mFadeLength;

        } else {
            content->gradientStop = q->mapToItem(content,rect.topLeft());
            content->gradientStop.rx() += rect.width();
            content->gradientStart = content->gradientStop;
            content->gradientStart.rx() -= content->mFadeLength;
        }
        content->setAlpha(0);
    }
}

void HbMarqueeItemPrivate::initAnimations()
{
    Q_Q(HbMarqueeItem);

    bool oldAnimationPending = mAnimationPending;
    mAnimGroup.clear();
    mAnimationPending = oldAnimationPending;

    if (q->contentsRect().width() < content->mTextWidth) {

        // get pixel per millimeter value to ensure same animation speed on each device
        qreal ppmValue = HbDeviceProfile::profile(q).ppmValue();

        // Calculate the offset for scrolling
        qreal scrollOffsetX = content->mTextWidth+ANIMATION_IDENT_BY_PIXEL - q->contentsRect().width();

        // animation duration depends on the length of the scrolled text and is not linear
        int duration = (int)((qSqrt(scrollOffsetX)*1000*ANIMATION_SPEED_FACTOR)/ppmValue);

        if (content->mTextDirection != Qt::LeftToRight) {
            scrollOffsetX = -scrollOffsetX;
        }

        QPointF scrolledOutPos(content->pos().x() - scrollOffsetX, content->pos().y());

        mAnimGroup.addPause(ANIMATION_LEAD_TIME);

        QPropertyAnimation *anim = 0;

        anim = new QPropertyAnimation;
        anim->setEasingCurve(QEasingCurve::Linear);
        anim->setTargetObject(content);
        anim->setPropertyName("alpha");
        anim->setStartValue(0);
        anim->setEndValue(0xFF);
        anim->setDuration(1000);
        mAnimGroup.addAnimation(anim);

        anim = new QPropertyAnimation;
        anim->setEasingCurve(QEasingCurve::SineCurve);
        anim->setTargetObject(content);
        anim->setPropertyName("pos");
        anim->setStartValue(content->pos());
        anim->setEndValue(scrolledOutPos);
        anim->setDuration(duration);
        mAnimGroup.addAnimation(anim);

        anim = new QPropertyAnimation;
        anim->setEasingCurve(QEasingCurve::Linear);
        anim->setTargetObject(content);
        anim->setPropertyName("alpha");
        anim->setEndValue(0);
        anim->setDuration(1000);
        mAnimGroup.addAnimation(anim);

        if(mAnimationPending) {
            q->startAnimation();
        }
    }
}


void HbMarqueeItemPrivate::_q_stateChanged()
{
    Q_Q(HbMarqueeItem);
    if (mAnimGroup.state() == QAbstractAnimation::Running) {
        emit q->animationStarted();
    } else if (mAnimGroup.state() == QAbstractAnimation::Stopped) {
        initContentPosition();
        mAnimationPending = false;
        emit q->animationStopped();
    } else {
        // Other states are irrelevant
    }
}

/*!
  @alpha
  @hbcore
 \class HbMarqueeItem
 \brief HbMarqueeItem is a lightweight item for showing a single line marqueed text.


 This is mainly used as a primitive in widgets.
 It derives from HbWidgetBase so it can be layouted.

 */

/*!
 Constructor for the class.
 */

HbMarqueeItem::HbMarqueeItem(QGraphicsItem *parent) :
    HbWidgetBase(*new HbMarqueeItemPrivate, parent)
{
    Q_D(HbMarqueeItem);
    d->init();
}

/*!
 Text can be set with \a text attribute.
 */
HbMarqueeItem::HbMarqueeItem(const QString &text, QGraphicsItem *parent) :
    HbWidgetBase(*new HbMarqueeItemPrivate, parent)
{
    Q_D(HbMarqueeItem);
    d->init();
    setText(text);
}

HbMarqueeItem::HbMarqueeItem(HbMarqueeItemPrivate &dd, QGraphicsItem *parent) :
    HbWidgetBase(dd, parent)
{
    Q_D(HbMarqueeItem);
    d->init();
}

/*!
    Destructor for the class.
 */
HbMarqueeItem::~HbMarqueeItem()
{

}

/*!
    Returns the text

    \sa HbMarqueeItem::setText()
 */
QString HbMarqueeItem::text () const
{
    Q_D( const HbMarqueeItem );
    return d->content->mText;
}


/*!
    Sets the text into \a text.
 */
void HbMarqueeItem::setText(const QString &text)
{
    Q_D(HbMarqueeItem);

    QString txt(text);

#ifdef HB_TEXT_MEASUREMENT_UTILITY
    if (HbFeatureManager::instance()->featureStatus(HbFeatureManager::TextMeasurement)) {
        if (text.endsWith(QChar(LOC_TEST_END))) {
            int index = text.indexOf(QChar(LOC_TEST_START));
            setProperty(HbTextMeasurementUtilityNameSpace::textIdPropertyName, 
                text.mid(index + 1, text.indexOf(QChar(LOC_TEST_END)) - index - 1));
            //setProperty( HbTextMeasurementUtilityNameSpace::textMaxLines, 1);
            txt = text.left(index);
        } else {
            setProperty(HbTextMeasurementUtilityNameSpace::textIdPropertyName,  QVariant::Invalid);
        }
    }
#endif // HB_TEXT_MEASUREMENT_UTILITY

    if (d->content->mText != txt) {
        d->content->mText = txt;
        d->updateTextMetaData();
        d->content->update();
    }
}

/*!
    Returns if the text is currently animating.
 */
bool HbMarqueeItem::isAnimating() const
{
    Q_D(const HbMarqueeItem);
    return (d->mAnimGroup.state()==QAbstractAnimation::Running);
}

/*!
    Starts or restarts the animation of the text.
 */
void HbMarqueeItem::startAnimation()
{
    Q_D(HbMarqueeItem);
    if (isVisible()) {
        d->mAnimGroup.start();
    }
    d->mAnimationPending = true;
}

/*!
    Stops the animation of the text if it is ongoing.
 */
void HbMarqueeItem::stopAnimation()
{
    Q_D(HbMarqueeItem);
    d->mAnimGroup.stop();
}

/*!
    Returns the loop count of the animation.

    \sa setLoopCount
 */
int HbMarqueeItem::loopCount() const
{
    Q_D(const HbMarqueeItem);
    return d->mAnimGroup.loopCount();
}

/*!
    Sets the loop count of the animation.
    The loop count determines how many times the animation is repeated
    after the animation started.
    Default value: 1
    Value -1 makes the animation continues and only stops when stopAnimation is called

    \sa loopCount
 */
void HbMarqueeItem::setLoopCount(int count)
{
    Q_D(HbMarqueeItem);
    d->mAnimGroup.setLoopCount(count);
}

/*!
    \reimp
 */
QSizeF HbMarqueeItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_D(const HbMarqueeItem);

    QSizeF size;

    switch(which) {
    case Qt::MinimumSize: {
            if (d->content->mText.isEmpty()) {
                return QSizeF(0.f, 0.f);
            }

            size = d->content->boundingRect().size();
            size.setWidth(qMin( size.width() , size.height()));
            size.setHeight(size.height());

            break;
        }

    case Qt::PreferredSize: {
            size = d->content->boundingRect().size();
            break;
        }

    default:
        size = HbWidgetBase::sizeHint(which, constraint);
    }

    return size;
}


 /*!
    \reimp
 */
void HbMarqueeItem::changeEvent(QEvent *event)
{
    Q_D(HbMarqueeItem);

    if(event->type() == HbEvent::FontChange ||
       event->type() == HbEvent::LayoutDirectionChange) {
        d->updateTextMetaData();
    }
    if (event->type() == HbEvent::ThemeChanged) {
        d->mDefaultColor = QColor();
        if(!d->mColor.isValid()) {
           update();
           d->content->update();
        }
    }
    HbWidgetBase::changeEvent(event);
}

/*!
    \reimp
 */
void HbMarqueeItem::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_D(HbMarqueeItem);
    HbWidgetBase::resizeEvent(event);
    d->updateTextMetaData();
}


/*!
    \reimp
 */
QVariant HbMarqueeItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    Q_D(HbMarqueeItem);

    if (change == QGraphicsItem::ItemVisibleHasChanged) {
        if (!value.toBool()) {
            bool oldAnimationPending = d->mAnimationPending;
            stopAnimation();
            d->mAnimationPending = oldAnimationPending;
        } else if (d->mAnimationPending) {
            startAnimation();
        }
    }
    return HbWidgetBase::itemChange(change, value);
}

/*!
    Sets the text color into \a color.
    If invalid color is used color from theme will be used.

    \sa HbTextItem::textColor()
 */
void HbMarqueeItem::setTextColor(const QColor &color)
{
    Q_D(HbMarqueeItem);
    d->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor, true);
    if (d->mColor != color) {
        d->mColor = color;
        update();
        d->content->update();
    }
}

/*!
    Returns the text color used for paiting text.
    If no color was set it returns color based on theme.

    \sa HbTextItem::setTextColor()
 */
QColor HbMarqueeItem::textColor() const
{
    Q_D( const HbMarqueeItem );

    if (d->mColor.isValid()) { // Means user has set text color
        return d->mColor;
    }
    if (!d->mDefaultColor.isValid()) {
        d->mDefaultColor = HbColorScheme::color(DEFAULT_COLORGROUP);
    }
    return d->mDefaultColor;
}


#include "moc_hbmarqueeitem.cpp"
// end of file
