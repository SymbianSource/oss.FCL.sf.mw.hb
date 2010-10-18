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
#include "hbmainwindow.h"

#ifdef HB_TEXT_MEASUREMENT_UTILITY
#include "hbtextmeasurementutility_r.h"
#include "hbtextmeasurementutility_r_p.h"
#endif //HB_TEXT_MEASUREMENT_UTILITY

#include "hbdeviceprofile.h"
#include "hbcolorscheme.h"
#include "hbnamespace_p.h"
#include "hbforegroundwatcher_p.h"

#include <qmath.h>
#include <QPainter>
#include <QPropertyAnimation>
#include <QGraphicsLinearLayout>

//#define HB_DEBUG_MARQUEE_LOGS
#ifdef HB_DEBUG_MARQUEE_LOGS
    #include <QDebug>
    #define HB_MARQUEE_LOG(args) qDebug() << "HbMarqueeItem::" << __FUNCTION__ \
            << ", objectName:" << this->objectName() \
            << " " << args;
    #define HB_MARQUEE_PRIV_LOG(args) qDebug() << "HbMarqueeItemPrivate::" << __FUNCTION__ \
            << ", objectName:" << q_ptr->objectName() \
            << " " << args;
#else
    #define HB_MARQUEE_LOG(args)
    #define HB_MARQUEE_PRIV_LOG(args)
#endif

namespace {
    // The bigger the value the slower the animation
    static const int   ANIMATION_MAXIMUM_RETURN_TIME = 780;
    static const qreal ANIMATION_SPEED_METERS_PER_SEC = 0.012; // mm/ms
}



HbMarqueeContent::HbMarqueeContent(HbMarqueeItem *parent) :
    HbTextItem(parent)
{
    // setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

void HbMarqueeContent::setAlpha(qreal alpha)
{
    setFadeLengths(alpha, 0);
}

qreal HbMarqueeContent::alpha() const
{
    return fadeLengths().x();
}

qreal HbMarqueeContent::xOffset() const
{
    Q_D(const HbTextItem);
    return d->mOffsetPos.x();
}

void HbMarqueeContent::setXOffset(qreal newOffset)
{
    Q_D(HbTextItem);
    if (!qFuzzyCompare(d->mOffsetPos.x(), newOffset)) {
        d->mOffsetPos.setX(newOffset);
        d->mPaintFaded = d->fadeNeeded(contentsRect());
        update();
    }
}

HbMarqueeItemPrivate::HbMarqueeItemPrivate() :
    content(0),
    mUserRequestedAnimation(false),
    mOffsetAnimation(0),
    mBackAnimation(0),
    mAnimationIsNeeded(false),
    mLastPrefWidth(0)
{
}

void HbMarqueeItemPrivate::init()
{
    Q_Q(HbMarqueeItem);

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(q);
    content = new HbMarqueeContent(q);
    layout->setContentsMargins(0,0,0,0);
    layout->addItem(content);
    content->setFont(QFont());
    content->setTextWrapping(Hb::TextNoWrap);
    content->setMaximumLines(1);

    q->setFlag(QGraphicsItem::ItemHasNoContents);

    QObject::connect(&mAnimGroup, SIGNAL(stateChanged(QAbstractAnimation::State,QAbstractAnimation::State)) ,q,SLOT(_q_stateChanged()));

    connectToMainWidow();
#ifdef Q_OS_SYMBIAN
    // this feature has sence only on phone device
    HbForegroundWatcher *fgWatcher = HbForegroundWatcher::instance();
    if (fgWatcher) {
        //this has similar funcionality like mainWindow obscured reveled signals
        // but it is used just in case if HbMarqueItem is used in application
        // without HbMainWindow
        QObject::connect(fgWatcher, SIGNAL(foregroundLost()), q, SLOT(_q_temporaryStopAnimation()));
        QObject::connect(fgWatcher, SIGNAL(foregroundGained()), q, SLOT(_q_tryToResumeAnimation()));
    }
#endif // Q_OS_SYMBIAN

#ifdef HB_TEXT_MEASUREMENT_UTILITY
    if (HbTextMeasurementUtility::instance()->locTestMode()) {
        q->setProperty( HbTextMeasurementUtilityNameSpace::textMaxLines, 1 );
    }
#endif
}

void HbMarqueeItemPrivate::createAnimation()
{
    Q_ASSERT(content);

    mOffsetAnimation = new QPropertyAnimation;
    mOffsetAnimation->setEasingCurve(QEasingCurve::Linear);
    mOffsetAnimation->setTargetObject(content);
    mOffsetAnimation->setPropertyName("xOffset");
    mOffsetAnimation->setStartValue(0);
    mAnimGroup.addAnimation(mOffsetAnimation);

    mAnimGroup.addPause(1000);

    mBackAnimation = new QPropertyAnimation;
    mBackAnimation->setEasingCurve(QEasingCurve::Linear);
    mBackAnimation->setTargetObject(content);
    mBackAnimation->setPropertyName("xOffset");
    mBackAnimation->setEndValue(0);
    mAnimGroup.addAnimation(mBackAnimation);

    mAnimGroup.addPause(2000);
}

void HbMarqueeItemPrivate::updateAnimation()
{
    Q_Q(HbMarqueeItem);
    mAnimationIsNeeded = mLastPrefWidth>q->size().width();

    if (!mAnimationIsNeeded) {
        // nothing to update
        return;
    }

    if (mAnimGroup.animationCount()==0) {
        // create animation only when it is needed
        createAnimation();
    }

    Q_ASSERT(mOffsetAnimation);
    Q_ASSERT(mBackAnimation);

    qreal ppmValue = HbDeviceProfile::profile(content).ppmValue();

    // Calculate the offset for scrolling
    const qreal MaxOffset = mLastPrefWidth-q->size().width();

    qreal v = ANIMATION_SPEED_METERS_PER_SEC*ppmValue; // pisxels per milisecond
    int duration = qRound(MaxOffset/v);  // t = s/v in miliseconds

    if (content->layoutDirection()!=Qt::RightToLeft) {
        mOffsetAnimation->setEndValue(-MaxOffset);
    } else {
        mOffsetAnimation->setEndValue(MaxOffset);
    }
    mOffsetAnimation->setDuration(duration);
    mBackAnimation->setDuration(qMin(duration, ANIMATION_MAXIMUM_RETURN_TIME));

    // resume animation if it is needed
    _q_tryToResumeAnimation();
}

void HbMarqueeItemPrivate::connectToMainWidow()
{
    Q_Q(HbMarqueeItem);

    if (q->scene()) {
        QGraphicsView *view=q->scene()->views().front();
        if (qobject_cast<HbMainWindow *>(view)) {
            HB_MARQUEE_PRIV_LOG("Connecting to main window")
            QObject::connect(view, SIGNAL(obscured()),
                             q, SLOT(_q_temporaryStopAnimation()));

            QObject::connect(view, SIGNAL(revealed()),
                             q, SLOT(_q_tryToResumeAnimation()));
        } else {
            HB_MARQUEE_PRIV_LOG("FAIL to connect: " << view)
        }
    } else {
        HB_MARQUEE_PRIV_LOG("No Scene")
    }
}

void HbMarqueeItemPrivate::_q_stateChanged()
{
    Q_Q(HbMarqueeItem);

    HB_MARQUEE_PRIV_LOG("New State: " << mAnimGroup.state())

    if (mAnimGroup.state() == QAbstractAnimation::Running) {
        emit q->animationStarted();
    } else if (mAnimGroup.state() == QAbstractAnimation::Stopped) {
        mUserRequestedAnimation = false;
        emit q->animationStopped();
    } else {
        // Other states are irrelevant
    }
}

/*
    Starts animation if:
    1. user what to animate it
    2. item is visible
    3. item is scene
    4. application is in foreground TODO: missing feature in HbForegroundWatcher how to read current state
 */
void HbMarqueeItemPrivate::_q_tryToResumeAnimation()
{
    Q_Q(HbMarqueeItem);
    HB_MARQUEE_PRIV_LOG(mUserRequestedAnimation << mAnimationIsNeeded
                        << q->isVisible() << (q->scene()!=0))

    if (mUserRequestedAnimation
        && mAnimationIsNeeded
        && q->isVisible()
        && (q->scene()!=0)) {
        HbMainWindow *mainWin = qobject_cast<HbMainWindow *>(q->scene()->views().front());
        if (mainWin && mainWin->isObscured()) {
            HB_MARQUEE_PRIV_LOG("Preventing animation main window is obscured")
            return;
        }

        mAnimGroup.start();
    }
}

void HbMarqueeItemPrivate::_q_temporaryStopAnimation()
{
    // store old state
    bool oldUserRequest = mUserRequestedAnimation;
    q_func()->stopAnimation();

    // restore old state
    mUserRequestedAnimation = oldUserRequest;
}

void HbMarqueeItemPrivate::toggleAnimation(bool startAnimate)
{
    if (startAnimate) {
        if (mAnimGroup.state()!=QAbstractAnimation::Running) {
            _q_tryToResumeAnimation();
        }
    } else {
        _q_temporaryStopAnimation();
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
    Q_D(const HbMarqueeItem);
    return d->content->text();
}

/*!
    Sets the text into \a text.
 */
void HbMarqueeItem::setText(const QString &text)
{
    Q_D(HbMarqueeItem);

    QString txt(text);

#ifdef HB_TEXT_MEASUREMENT_UTILITY
    if (HbTextMeasurementUtility::instance()->locTestMode()) {
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

    d->content->setText(text);
}

/*!
    Returns if the text is currently animating.

    Note that if marquee item is not visible (isVisible is false or
    it is not in scene or phone in in sleep mode) then animation is stoped and
    this method will return false even when startAnimation() was called.
 */
bool HbMarqueeItem::isAnimating() const
{
    Q_D(const HbMarqueeItem);
    return (d->mAnimGroup.state()==QAbstractAnimation::Running);
}

/*!
    Starts or restarts the animation of the text when animation is needed,
    marquee item became/is visible and is inside of scene.
 */
void HbMarqueeItem::startAnimation()
{
    Q_D(HbMarqueeItem);

    d->mUserRequestedAnimation = true;
    d->_q_tryToResumeAnimation();
}

/*!
    Stops the animation of the text if it is ongoing.
 */
void HbMarqueeItem::stopAnimation()
{
    Q_D(HbMarqueeItem);
    d->mAnimGroup.setCurrentTime(0);
    d->mAnimGroup.stop();
    d->mUserRequestedAnimation = false;
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
    return HbWidgetBase::sizeHint(which, constraint);
}

 /*!
    \reimp
 */
void HbMarqueeItem::changeEvent(QEvent *event)
{
    Q_D(HbMarqueeItem);

    if(event->type() == HbEvent::FontChange ||
       event->type() == HbEvent::LayoutDirectionChange) {
        d->updateAnimation();
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
    d->updateAnimation();
    d->toggleAnimation(d->mAnimationIsNeeded);
}

/*!
    \reimp
 */
QVariant HbMarqueeItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    Q_D(HbMarqueeItem);

    switch (change) {
    case QGraphicsItem::ItemVisibleHasChanged: {
            HB_MARQUEE_LOG("Visible changed event:" << value.toBool())
            d->toggleAnimation(value.toBool());
        }
        break;

    case QGraphicsItem::ItemSceneHasChanged: {
            HB_MARQUEE_LOG("Sceane changed event")
            d->toggleAnimation(0!=scene());
            d->connectToMainWidow();
        }
        break;

    default:
        ;// nothing to do
    }
    return HbWidgetBase::itemChange(change, value);
}

/*!
    \reimp
*/
bool HbMarqueeItem::event(QEvent *e)
{
    Q_D(HbMarqueeItem);
    if (e->type() == HbEvent::SleepModeEnter) {
        HB_MARQUEE_LOG("Sleep Enter")
        d->_q_temporaryStopAnimation();
    } else if (e->type() == HbEvent::SleepModeExit) {
        HB_MARQUEE_LOG("Sleep Exit")
        d->_q_tryToResumeAnimation();
    }
    return HbWidgetBase::event(e);
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
    d->content->setTextColor(color);
}

/*!
    Returns the text color used for paiting text.
    If no color was set it returns color based on theme.

    \sa HbTextItem::setTextColor()
 */
QColor HbMarqueeItem::textColor() const
{
    Q_D( const HbMarqueeItem );
    return d->content->textColor();
}

void HbMarqueeItem::setGeometry(const QRectF &rect)
{
    Q_D(HbMarqueeItem);

    if (!qFuzzyCompare(d->mLastPrefWidth, d->content->preferredWidth())) {
        d->mLastPrefWidth = d->content->preferredWidth();
        HbWidgetBase::setGeometry(rect);

        d->updateAnimation();
        d->toggleAnimation(d->mAnimationIsNeeded);
        return;
    }

    HbWidgetBase::setGeometry(rect);
}

#include "moc_hbmarqueeitem.cpp"
// end of file
