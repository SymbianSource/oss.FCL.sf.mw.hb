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

#ifndef HBICONANIMATION_H
#define HBICONANIMATION_H

#include <hbglobal.h>
#include <hbiconanimationdefinition.h>
#include <QtGlobal>
#include <QTimer>
#include <QObject>
#include <QIcon>

QT_BEGIN_NAMESPACE
class QSvgRenderer;
class QImageReader;
QT_END_NAMESPACE

class HbIconAnimationManagerPrivate;
class HbIconAnimator;
class HbView;
class HbTimerSignalEntry;

class HbIconAnimation : public QObject
{
    Q_OBJECT

public:
    HbIconAnimation(HbIconAnimator *animator, const QString &iconName);
    virtual ~HbIconAnimation();

    enum {
        SVG = 0,
        MNG = 1,
        GIF = 2,
        FrameSet = 3
    };

    virtual int type() const = 0;

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;

    virtual QPixmap currentFrame() const = 0;

    QString iconName() const;

    virtual QSizeF size() const;
    virtual void setSize(const QSizeF &size);

    virtual Qt::AspectRatioMode aspectRatioMode() const;
    virtual void setAspectRatioMode(Qt::AspectRatioMode mode);

    virtual QIcon::Mode mode() const;
    virtual void setMode(QIcon::Mode mode);

    virtual QSizeF defaultSize() const;
    virtual void setDefaultSize(const QSizeF &size);

    virtual QSizeF renderSize() const;
    virtual void setRenderSize(const QSizeF &size);

    bool mirrored() const;
    void setMirrored(bool mirrored);

    QColor color() const;
    void setColor(const QColor &color);

    bool resolutionCorrected() const;
    void setResolutionCorrected(bool corrected);

    HbIconAnimationDefinition::PlayMode playMode() const;
    void setPlayMode(HbIconAnimationDefinition::PlayMode playMode);

    inline HbView *view() const {
        return mView;
    }

    inline void setView(HbView *view) {
        mView = view;
    }

    inline HbIconAnimator *animator() const {
        return mAnimator;
    }

    inline bool paused() const {
        return mPaused;
    }

    inline bool pausedDueToBackground() const {
        return mPausedDueToBackground;
    }

    inline void setPausedDueToBackground(bool status) {
        mPausedDueToBackground = status;
    }


signals:
    void animationUpdated();
    void animationStarted();
    void animationStopped();
    void animationFinished();

public slots:
    void notifyAnimationStarted();
    void notifyAnimationStopped();
    void notifyAnimationFinished();

protected:
    void delayedEmitStarted();

    QString mIconName;
    bool mMirrored;
    bool mResolutionCorrected;
    // If size or aspectRatioMode are changed, it also recalculates the renderSize
    QSizeF mSize;
    Qt::AspectRatioMode mAspectRatioMode;
    QIcon::Mode mMode;

    QSizeF mDefaultSize;
    QSizeF mRenderSize;

    HbIconAnimationDefinition::PlayMode mPlayMode;

    QTimer *mStartSignalTimer;

    // Indicates that this is a newly constructed instance for which start() has never been called.
    bool mFresh;

    // dptr to the global animation manager instance
    HbIconAnimationManagerPrivate *mAnimMgrD;
    QColor mColor;

    HbIconAnimator *mAnimator;
    HbView *mView;

    bool mPaused;
    bool mPausedDueToBackground;
};

class HbIconAnimationSvg : public HbIconAnimation
{
    Q_OBJECT

public:
    HbIconAnimationSvg(HbIconAnimator *animator, const QString &iconName, QSvgRenderer *renderer, const QString &iconPath);
    ~HbIconAnimationSvg();

    int type() const;

    void start();
    void stop();
    void pause();
    void resume();

    QPixmap currentFrame() const;

private slots:
    void handleSvgAnimationUpdated();

private:
    QSvgRenderer *mSvgRenderer;
    QString mIconPath;
};

class HbIconAnimationImage : public HbIconAnimation
{
    Q_OBJECT

public:
    HbIconAnimationImage(HbIconAnimator *animator, const QString &iconName, const QString &iconFileName, QImageReader *renderer, int type);
    ~HbIconAnimationImage();

    int type() const;

    void start();
    void stop();
    void pause();
    void resume();

    QPixmap currentFrame() const;

private slots:
    void handleAnimationUpdated();

private:
    QImageReader *mImageRenderer;
    QString mIconFileName;
    int mType;
    int mTimerInterval;
    HbTimerSignalEntry *mTimerEntry;
    QPixmap mCurrentFrame;
    QPixmap mLastFrame;
};

class HbIconAnimationFrameSet : public HbIconAnimation
{
    Q_OBJECT

public:
    struct JumpData {
        int targetFrameIndex;
        int repeatCount; // max exec count
        int execCount; // current exec count
    };
    struct FrameData {
        QPixmap pixmap;
        int duration; // in milliseconds
        QList<JumpData> jumps;
        void assignJumps(const QList<HbIconAnimationDefinition::AnimationFrame::Jump> &jlist) {
            jumps.clear();
            for (int i = 0, ie = jlist.count(); i != ie; ++i) {
                JumpData jd;
                jd.targetFrameIndex = jlist.at(i).targetFrameIndex;
                jd.repeatCount = jlist.at(i).repeatCount;
                jd.execCount = 0;
                jumps.append(jd);
            }
        }
    };

    HbIconAnimationFrameSet(HbIconAnimator *animator, const QString &iconName, const QList<FrameData> &frames);
    ~HbIconAnimationFrameSet();

    int type() const;

    void start();
    void stop();
    void pause();
    void resume();

    QPixmap currentFrame() const;

public slots:
    void animationTimeout();

private:
    void moveToNextFrame();
    void resetJumpCount(FrameData &frame);
    void resetJumpCounts();

    QList<FrameData> mFrames;
    int mTimerInterval;
    int mCurrentFrameIndex;

    bool mMirrored;

    HbTimerSignalEntry *mTimerEntry;
};


#endif // HBICONANIMATION_H
