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

#include "hbiconanimation_p.h"
#include "hbimagetraces_p.h"
#include "hbiconloader_p.h"
#include "hbiconanimationmanager_p.h"
#include "hbiconanimator.h"
#include "hbiconanimator_p.h"
#include "hbtimer_p.h"


#include <QSvgRenderer>
#include <QImageReader>
#include <QPainter>
#include <QPixmap>
#include <QDebug>
#include <QStyleOption>
#include <QApplication>
#include <QPalette>

HbIconAnimation::HbIconAnimation(HbIconAnimator *animator, const QString &iconName) :
    mIconName(iconName),
    mMirrored(false),
    mResolutionCorrected(false),
    mAspectRatioMode(Qt::KeepAspectRatio),
    mMode(QIcon::Normal),
    mPlayMode(HbIconAnimationDefinition::PlayOnce),
    mStartSignalTimer(0),
    mFresh(true),
    mAnimMgrD(HbIconAnimationManagerPrivate::d_ptr(HbIconAnimationManager::global())),
    mColor(QColor()),
    mAnimator(animator),
    mView(0),
    mPaused(false),
    mPausedDueToBackground(false)
{
    Q_ASSERT(!(animator->d->animation));
    // Set the animation in the animator, it takes ownership of this object.
    animator->d->animation = this;

#ifdef HB_ICON_TRACES
    qDebug() << "HbIconAnimation created: " << mIconName;
#endif
}

HbIconAnimation::~HbIconAnimation()
{
#ifdef HB_ICON_TRACES
    qDebug() << "HbIconAnimation destroyed: " << mIconName;
#endif
    mAnimMgrD->animNotPlaying(this);
}

QString HbIconAnimation::iconName() const
{
    return mIconName;
}

QSizeF HbIconAnimation::size() const
{
    return mSize;
}

void HbIconAnimation::setSize(const QSizeF &size)
{
    // If size changed, invalidate calculated render size
    if (size != mSize) {
        mRenderSize = QSizeF();
    }

    mSize = size;
}

QColor HbIconAnimation::color() const
{
    return mColor;
}

void HbIconAnimation::setColor(const QColor &color)
{
    mColor = color;
}

Qt::AspectRatioMode HbIconAnimation::aspectRatioMode() const
{
    return mAspectRatioMode;
}

void HbIconAnimation::setAspectRatioMode(Qt::AspectRatioMode mode)
{
    // If aspect ratio mode changed, invalidate calculated render size
    if (mode != mAspectRatioMode) {
        mRenderSize = QSizeF();
    }

    mAspectRatioMode = mode;
}

QIcon::Mode HbIconAnimation::mode() const
{
    return mMode;
}

void HbIconAnimation::setMode(QIcon::Mode mode)
{
    mMode = mode;
}

QSizeF HbIconAnimation::defaultSize() const
{
    return mDefaultSize;
}

void HbIconAnimation::setDefaultSize(const QSizeF &size)
{
    mDefaultSize = size;
}

QSizeF HbIconAnimation::renderSize() const
{
    if (mRenderSize.isEmpty()) {
        const_cast<QSizeF &>(mRenderSize) = mDefaultSize;
        if (!mSize.isEmpty()) {
            const_cast<QSizeF &>(mRenderSize).scale(mSize, mAspectRatioMode);
        }
        // Apply resolution correction
        if (mResolutionCorrected) {
            HbIconLoader *loader = HbIconLoader::global();
            loader->applyResolutionCorrection(const_cast<QSizeF &>(mRenderSize));
        }
    }

    return mRenderSize;
}

void HbIconAnimation::setRenderSize(const QSizeF &size)
{
    mRenderSize = size;
}

void HbIconAnimation::setLoopCount(int loopCount)
{
    Q_UNUSED(loopCount);
    // Nothing to do here, some subclasses may want to reimplement this in case
    // they want to offer support to the user for manually controlling the loop
    // behavior.
}

bool HbIconAnimation::loopCountSet(int *loopCount)
{
    Q_UNUSED(loopCount);
    return false;
}

bool HbIconAnimation::mirrored() const
{
    return mMirrored;
}

void HbIconAnimation::setMirrored(bool mirrored)
{
    mMirrored = mirrored;
}

bool HbIconAnimation::resolutionCorrected() const
{
    return mResolutionCorrected;
}

void HbIconAnimation::setResolutionCorrected(bool corrected)
{
    mResolutionCorrected = corrected;
}

HbIconAnimationDefinition::PlayMode HbIconAnimation::playMode() const
{
    return mPlayMode;
}

void HbIconAnimation::setPlayMode(HbIconAnimationDefinition::PlayMode playMode)
{
    mPlayMode = playMode;
}

void HbIconAnimation::delayedEmitStarted()
{
    if (!mStartSignalTimer) {
        mStartSignalTimer = new QTimer(this);
        connect(mStartSignalTimer, SIGNAL(timeout()), SLOT(notifyAnimationStarted()));
    }
    mStartSignalTimer->setSingleShot(true);
    mStartSignalTimer->start(0);
}

void HbIconAnimation::notifyAnimationStarted()
{
    mAnimMgrD->animPlaying(this);
    emit animationStarted();
}

void HbIconAnimation::notifyAnimationStopped()
{
    mAnimMgrD->animNotPlaying(this);
    emit animationStopped();
}

void HbIconAnimation::notifyAnimationFinished()
{
    mAnimMgrD->animNotPlaying(this);
    emit animationFinished();
}

// -----------------------------------------------------------------------------
// Class HbIconAnimationSvg
// -----------------------------------------------------------------------------
HbIconAnimationSvg::HbIconAnimationSvg(
    HbIconAnimator *animator,
    const QString &iconName,
    QSvgRenderer *renderer,
    const QString &iconPath) :

    HbIconAnimation(animator, iconName),
    mSvgRenderer(renderer),
    mIconPath(iconPath)
{
    connect(mSvgRenderer, SIGNAL(repaintNeeded()), this, SLOT(handleSvgAnimationUpdated()));
}

HbIconAnimationSvg::~HbIconAnimationSvg()
{
    delete mSvgRenderer;
}

int HbIconAnimationSvg::type() const
{
    return SVG;
}

void HbIconAnimationSvg::start()
{
    // TODO
}

void HbIconAnimationSvg::stop()
{
    // TODO
}

void HbIconAnimationSvg::pause()
{
    // TODO
}

void HbIconAnimationSvg::resume()
{
    // TODO
}

QPixmap HbIconAnimationSvg::currentFrame() const
{
    QSize pixelSize = renderSize().toSize();
    if (!pixelSize.isEmpty()) {
        QPixmap canvasPixmap(pixelSize);
        canvasPixmap.fill(Qt::transparent);
        QPainter painter(&canvasPixmap);
        mSvgRenderer->render(&painter, QRectF(QPointF(), pixelSize));
        painter.end();

        // Apply mirroring if required
        if (mMirrored) {
            QTransform t;
            t.scale(-1, 1);
            canvasPixmap = canvasPixmap.transformed(t);
        }

        // Apply mode
        if (mMode != QIcon::Normal) {
            QStyleOption opt(0);
            opt.palette = QApplication::palette();
            canvasPixmap = QApplication::style()->generatedIconPixmap(mMode, canvasPixmap, &opt);
        }

        return canvasPixmap;
    } else {
#ifdef HB_ICON_TRACES
        qDebug() << "Warning: HbIconAnimationSvg::currentFrame() - SVG default size is empty.";
#endif
        return QPixmap();
    }
}

void HbIconAnimationSvg::handleSvgAnimationUpdated()
{
#ifdef HB_ICON_TRACES
    qDebug() << "HbIconAnimation updated: " << iconName();
#endif

    emit animationUpdated();
}

// -----------------------------------------------------------------------------
// Class HbIconAnimationImage
// -----------------------------------------------------------------------------
HbIconAnimationImage::HbIconAnimationImage(
    HbIconAnimator *animator,
    const QString &iconName,
    const QString &iconFileName,
    QImageReader *renderer,
    int type) :

    HbIconAnimation(animator, iconName),
    mImageRenderer(renderer),
    mIconFileName(iconFileName),
    mType(type),
    mTimerEntry(0),
    mDoNotResetLoopCount(false)
{
    // This class supports these types
    Q_ASSERT(mType == MNG || mType == GIF);

    // Read the first frame. QImageReader::read() must be called before
    // QImageReader::nextImageDelay()
    QImage img = mImageRenderer->read();

    // Store the first frame in the current frame pixmap
    mCurrentFrame = QPixmap::fromImage(img);

    // Set default size based on the first frame
    setDefaultSize(mCurrentFrame.size());

    // Get the loop count. -1 means infinite loop.
    mLoopCount = mImageRenderer->loopCount();
    mCustomLoopCountSet = false;

    // Do not start the timer or initiate any signal emission here.
    // Do it in start() instead, since mFresh is true by default.
}

HbIconAnimationImage::~HbIconAnimationImage()
{
    delete mImageRenderer;
}

int HbIconAnimationImage::type() const
{
    return mType;
}

inline void stopTimer(HbTimerSignalEntry *&entry)
{
    // entry may be non-null and may already be deleted, however this is not a
    // problem because we avoid double deletion by checking the return value of
    // unregisterEntry().
    if (entry && HbTimer::instance()->unregisterEntry(entry)) {
        delete entry;
        entry = 0;
    }
}

void HbIconAnimationImage::start()
{
    if (mStartSignalTimer && mStartSignalTimer->isActive()) {
        return;
    }
    if (mFresh) {
        mFresh = false;
        mTimerInterval = mImageRenderer->nextImageDelay();
        mTimerEntry = HbTimer::instance()->addTimeout(mTimerInterval, this, SLOT(handleAnimationUpdated()));
        // Emit the signal later as nobody is connected at this point (if the
        // animation is auto-started, because the start() comes straight after
        // construction in that case).
        delayedEmitStarted();
    } else {
        stopTimer(mTimerEntry);

        // Recreate the image reader. It's slow but QImageReader::jumpToImage does not work.
        delete mImageRenderer;
        mImageRenderer = 0;
        mImageRenderer = new QImageReader(mIconFileName, mType == MNG ? "MNG" : "GIF");

        if (mDoNotResetLoopCount) {
            mDoNotResetLoopCount = false;
        } else {
            // Reset the loop count.
            if (mCustomLoopCountSet) {
                mLoopCount = mCustomLoopCount;
            } else {
                mLoopCount = mImageRenderer->loopCount();
            }
        }

        // New image reader starts from the first frame. Handle animation update.
        notifyAnimationStarted();
        handleAnimationUpdated();
    }
}

void HbIconAnimationImage::stop()
{
    // If there is still a pending 'started' signal then drop it.
    if (mStartSignalTimer && mStartSignalTimer->isActive()) {
        mStartSignalTimer->stop();
    }

    // Stop the animation and go to the last frame.
    stopTimer(mTimerEntry);

    // Has the last frame been read already?
    if (!mLastFrame.isNull()) {
        mCurrentFrame = mLastFrame;
    } else {
        // Read all frames until the last one.
        // QImageReader::jumpToImage does not work so cannot jump to the last frame if it has not been read yet.
        while (true) {
            QImage img = mImageRenderer->read();
            // Reached last frame?
            if (!mImageRenderer->canRead()) {
                // Must get rid of the old pixmap first, in case of very large frames
                // keeping both the old and new frame in memory would not succeed.
                mCurrentFrame = QPixmap(); // This call is not superfluous.
                mCurrentFrame = QPixmap::fromImage(img);
                mLastFrame = mCurrentFrame;
                break;
            }
        }
    }

    // Inform client to update display
    emit animationUpdated();

    notifyAnimationStopped();
}

void HbIconAnimationImage::pause()
{
    stopTimer(mTimerEntry);
}

void HbIconAnimationImage::resume()
{
    if (!mTimerEntry) {
        mTimerEntry = HbTimer::instance()->addTimeout(mTimerInterval, this, SLOT(handleAnimationUpdated()));
    }
}

QPixmap HbIconAnimationImage::currentFrame() const
{
    QSize pixelSize = renderSize().toSize();
    QPixmap canvasPixmap;

    if (!pixelSize.isEmpty()) {
        if (pixelSize != mCurrentFrame.size()) {
            canvasPixmap = mCurrentFrame.scaled(pixelSize, mAspectRatioMode, Qt::SmoothTransformation);
        } else {
            canvasPixmap = mCurrentFrame;
        }

        // Apply mirroring if required
        if (mMirrored) {
            QTransform t;
            t.scale(-1, 1);
            canvasPixmap = canvasPixmap.transformed(t);
        }

        // Apply mode
        if (mMode != QIcon::Normal) {
            QStyleOption opt(0);
            opt.palette = QApplication::palette();
            canvasPixmap = QApplication::style()->generatedIconPixmap(mMode, canvasPixmap, &opt);
        }

        return canvasPixmap;
    } else {
#ifdef HB_ICON_TRACES
        qDebug() << "Warning: HbIconAnimationImage::currentFrame() - default size is empty.";
#endif
        return QPixmap();
    }
}

void HbIconAnimationImage::setLoopCount(int loopCount)
{
    // By default we use whatever value QImageReader::loopCount() returned.
    // However we also offer the possibility to override it manually at any
    // time.
    mLoopCount = mCustomLoopCount = loopCount;
    mCustomLoopCountSet = true;
}

bool HbIconAnimationImage::loopCountSet(int *loopCount)
{
    if (loopCount && mCustomLoopCountSet) {
        *loopCount = mCustomLoopCount;
    }
    return mCustomLoopCountSet;
}

void HbIconAnimationImage::handleAnimationUpdated()
{
    bool finished = false;

    // Read the new frame. QImageReader::read() must be called before
    // QImageReader::nextImageDelay()
    QImage img = mImageRenderer->read();

    int delay = mImageRenderer->nextImageDelay();
    // If there is next image, restart the timer
    if (!img.isNull() && delay > 0) {
        // Restart timer
        mTimerInterval = delay;
        mTimerEntry = HbTimer::instance()->addTimeout(mTimerInterval, this, SLOT(handleAnimationUpdated()));
    }

    // Store the new frame in the current frame pixmap
    if (!img.isNull()) {
        // Must get rid of the old pixmap first, in case of very large frames
        // keeping both the old and new frame in memory would not succeed.
        // (with the OpenVG paint engine QPixmap will create a new QImage if
        // the pixel format is different and it will be different here...)
        mCurrentFrame = QPixmap(); // This call is not superfluous.
        mCurrentFrame = QPixmap::fromImage(img);
    }
    // Reached the last frame. Store it so it can be used by stop().
    else {
        mLastFrame = mCurrentFrame;
        finished = true;
    }

#ifdef HB_ICON_TRACES
    qDebug() << "HbIconAnimation updated: " << iconName();
#endif

    // Inform client
    emit animationUpdated();

    if (finished) {
        if (mLoopCount == 0) {
            notifyAnimationFinished();
        } else {
            finished = false;
            if (mLoopCount > 0) {
                --mLoopCount;
            }
            mDoNotResetLoopCount = true;
            start();
        }
    }
}

HbIconAnimationFrameSet::HbIconAnimationFrameSet(
    HbIconAnimator *animator, const QString &iconName, const QList<FrameData> &frames) :
    HbIconAnimation(animator, iconName),
    mFrames(frames),
    mCurrentFrameIndex(0),
    mTimerEntry(0),
    mManualLoopingSet(false),
    mManualLooping(false)
{
    // Do not start the timer or initiate any signal emission here.
    // Do it in start() instead, since mFresh is true by default.
}

HbIconAnimationFrameSet::~HbIconAnimationFrameSet()
{
    mFrames.clear(); // This destroys the pixmaps
}

int HbIconAnimationFrameSet::type() const
{
    return FrameSet;
}

void HbIconAnimationFrameSet::resetJumpCount(FrameData &frame)
{
    for (int i = 0, ie = frame.jumps.count(); i != ie; ++i) {
        // Note that the default value of execCount is 0 because frames are
        // shown once anyway so if there is a loop element with count="1"
        // then the frames need to be shown twice (because it "loops once").
        frame.jumps[i].execCount = 0;
    }
}

void HbIconAnimationFrameSet::resetJumpCounts()
{
    for (int i = 0, ie = mFrames.count(); i != ie; ++i) {
        resetJumpCount(mFrames[i]);
    }
}

void HbIconAnimationFrameSet::start()
{
    if (mStartSignalTimer && mStartSignalTimer->isActive()) {
        return;
    }
#ifdef HB_ICON_TRACES
    qDebug("HbIconAnimationFrameSet::start() mFresh=%d", mFresh);
#endif
    if (mFresh) {
        mFresh = false;
        mTimerInterval = mFrames.at(0).duration;
        mTimerEntry = HbTimer::instance()->addTimeout(mTimerInterval, this, SLOT(animationTimeout()));
        // Emit the signal later as nobody is connected at this point (if the
        // animation is auto-started, because the start() comes straight after
        // construction in that case).
        delayedEmitStarted();
    } else {
        stopTimer(mTimerEntry);
        resetJumpCounts();
        // Go to first frame and handle animation update
        mCurrentFrameIndex = -1;
        notifyAnimationStarted();
        animationTimeout();
    }
}

void HbIconAnimationFrameSet::stop()
{
    stopTimer(mTimerEntry);
    // Go to last frame and handle animation update
    mCurrentFrameIndex = mFrames.count() - 1;
    emit animationUpdated();
    notifyAnimationStopped();
}

void HbIconAnimationFrameSet::pause()
{
    stopTimer(mTimerEntry);
    mPaused = true;
    mPausedDueToBackground = false;
}

void HbIconAnimationFrameSet::resume()
{
    if (!mTimerEntry) {
        mTimerEntry = HbTimer::instance()->addTimeout(mTimerInterval, this, SLOT(animationTimeout()));
    }
    mPaused = false;
    mPausedDueToBackground = false;
}

QPixmap HbIconAnimationFrameSet::currentFrame() const
{
    if (mFrames.count()) {
        QPixmap pm = mFrames.at(mCurrentFrameIndex).pixmap;

        // Mirroring is not needed here. Frames are mirrored if needed when they are loaded.

        // Apply mode
        if (mMode != QIcon::Normal) {
            QStyleOption opt(0);
            opt.palette = QApplication::palette();
            pm = QApplication::style()->generatedIconPixmap(mMode, pm, &opt);
        }

        return pm;
    } else {
        return QPixmap();
    }
}

void HbIconAnimationFrameSet::setLoopCount(int loopCount)
{
    // Here we only support manually enabling/disabling infinite looping.
    // (so effectively loop count should be -1 or 0) Positive values are
    // treated as do-not-loop, i.e. zero. This is because explicit loop
    // counts are not supported by axml.
    mManualLoopingSet = true;
    mManualLooping = loopCount < 0; // -1 means loop for ever
}

bool HbIconAnimationFrameSet::loopCountSet(int *loopCount)
{
    if (loopCount && mManualLoopingSet) {
        *loopCount = mManualLooping ? -1 : 0;
    }
    return mManualLoopingSet;
}

void HbIconAnimationFrameSet::moveToNextFrame()
{
    if (mCurrentFrameIndex >= 0 && mCurrentFrameIndex < mFrames.count()) {
        for (int i = 0, ie = mFrames[mCurrentFrameIndex].jumps.count(); i != ie; ++i) {
            JumpData &jumpData(mFrames[mCurrentFrameIndex].jumps[i]);
            if (jumpData.execCount < jumpData.repeatCount) {
                ++jumpData.execCount;
                // Before returning, the exec counts of all previous jumps in
                // this frame must be reset, because they were caused by
                // <loop> elements that were embedded into the <loop> that
                // generated this jump.
                for (int j = 0; j < i; ++j) {
                    mFrames[mCurrentFrameIndex].jumps[j].execCount = 0;
                }
                // And similarly, all jumps in frames that fall between the
                // target (incl.) and the current frame (excl.) must be
                // reset. Note that jumping forward is not supported and such
                // jumps are never generated by the animation xml parser.
                for (int j = jumpData.targetFrameIndex; j < mCurrentFrameIndex; ++j) {
                    resetJumpCount(mFrames[j]);
                }
                mCurrentFrameIndex = jumpData.targetFrameIndex;
                return;
            }
        }
    }

    ++mCurrentFrameIndex;
}

void HbIconAnimationFrameSet::animationTimeout()
{
    // Frame delay has passed, pick the next frame index.
    moveToNextFrame();

    bool finished = false;

    // Go back to the beginning if indexing goes beyond frame count
    // and looping is enabled. Change to the last valid frame and
    // finish if looping is disabled.
    if (mCurrentFrameIndex >= mFrames.count()) {
        bool looping = playMode() == HbIconAnimationDefinition::Loop;
        if (mManualLoopingSet) {
            looping = mManualLooping;
        }
        if (looping) {
            mCurrentFrameIndex = 0;
        } else {
            finished = true;
            mCurrentFrameIndex = mFrames.count() - 1;
        }
        resetJumpCounts();
    }

    // If there are more frames to come or the animation is looping,
    // restart the timer.
    if (!finished) {
        mTimerInterval = mFrames.at(mCurrentFrameIndex).duration;
        mTimerEntry = HbTimer::instance()->addTimeout(mTimerInterval, this, SLOT(animationTimeout()));
    }

#ifdef HB_ICON_TRACES
    //qDebug() << "HbIconAnimation updated: " << iconName();
#endif

    // Inform client.
    if (finished) {
        notifyAnimationFinished();
    } else {
        emit animationUpdated();
    }
}

// End of File
