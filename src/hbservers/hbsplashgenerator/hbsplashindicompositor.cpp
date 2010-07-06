/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbServers module of the UI Extensions for Mobile.
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

#include "hbsplashindicompositor_p.h"
#include "hbsplashgenerator_p.h"
#include "hbsplashdefs_p.h"
#include "hbmainwindow.h"
#include "hbmainwindow_p.h"
#include "hbsleepmodelistener_p.h"
#include "hbevent.h"
#include <QTimer>
#include <QApplication>

#ifdef Q_OS_SYMBIAN
#include <fbs.h>
#endif

// The indicator compositor renders a part of the mainwindow (the
// statusbar) into an image from time to time. This pixel data is then
// copied over to the splash screen bitmap whenever a client requests
// a screen (except if the screen is marked having a non-standard
// (hidden or transparent) statusbar).
//
// This ensures that there will be relatively up-to-date indicators in
// the splash screens.

HbSplashIndicatorCompositor::HbSplashIndicatorCompositor(HbSplashGenerator *gen)
    : mGenerator(gen), mSleeping(false), mSignalsConnected(false)
{
    // When the splash screens are regenerated the statusbar must be rendered
    // again too because the theme or the splashml files may have changed.
    connect(mGenerator, SIGNAL(finished()), SLOT(renderStatusBar()), Qt::QueuedConnection);

    // Regenerate once using a singleshot timer (to have a little delay) but
    // then start listening to change notifications from the statusbar instead.
    mRenderTimer = new QTimer(this);
    mRenderTimer->setSingleShot(true);
    connect(mRenderTimer, SIGNAL(timeout()), SLOT(renderStatusBar()));
    mRenderTimer->start(5000); // 5 sec

    // There must be no activity while the device is sleeping so listen to sleep
    // mode events too.
    HbSleepModeListener::instance(); // just to make sure it is created
    QApplication::instance()->installEventFilter(this);
}

void HbSplashIndicatorCompositor::release()
{
    delete this;
}

void HbSplashIndicatorCompositor::connectSignals()
{
    HbStatusBar *sb = HbMainWindowPrivate::d_ptr(mGenerator->ensureMainWindow())->mStatusBar;
    connect(sb, SIGNAL(contentChanged(HbStatusBar::ContentChangeFlags)),
            SLOT(handleStatusBarContentChange(HbStatusBar::ContentChangeFlags)));
    mSignalsConnected = true;
}

void HbSplashIndicatorCompositor::handleStatusBarContentChange(
    HbStatusBar::ContentChangeFlags changeType)
{
    // No need to rush when battery level changes while charging
    // because it is not the real level, just the animation.
    queueRender(changeType.testFlag(HbStatusBar::BatteryCharging));
}

void HbSplashIndicatorCompositor::queueRender(bool lazy)
{
    // Compress subsequent change notifications into one update.
    if (!mRenderTimer->isActive() && !mSleeping) {
        mRenderTimer->start(lazy ? 10000 : 100); // 10 sec or 0.1 sec
    }
}

void HbSplashIndicatorCompositor::renderStatusBar()
{
    // Do nothing in sleep mode.
    if (mSleeping) {
        return;
    }
    // Try again later if a screen is just being generated. We share the same
    // mainwindow and our changes done here (orientation, statusbar visibility)
    // could possibly ruin the output.
    if (!mGenerator->lockMainWindow()) {
        mRenderTimer->start(1000); // 1 sec
        return;
    }
    try {
        if (!mSignalsConnected) {
            connectSignals();
            // The first rendering may be wrong due to the deferred
            // polish/layout handling.  So issue a regenerate request.
            queueRender();
        }
        HbMainWindow *mw = mGenerator->ensureMainWindow();
        HbSplashGenerator::setStatusBarElementsVisible(mw, true);
        mw->setOrientation(Qt::Vertical, false);
        doRender(mw, &mStatusBarImagePrt, &mStatusBarRectPrt);
        mw->setOrientation(Qt::Horizontal, false);
        doRender(mw, &mStatusBarImageLsc, &mStatusBarRectLsc);
    } catch (const std::bad_alloc &) {
        mStatusBarImagePrt = mStatusBarImageLsc = QImage();
    }
    mGenerator->unlockMainWindow();
}

void HbSplashIndicatorCompositor::doRender(HbMainWindow *mw,
        QImage *statusBarImage,
        QRect *statusBarRect)
{
    *statusBarRect = mw->mapFromScene(HbMainWindowPrivate::d_ptr(mw)->mStatusBar->geometry())
                     .boundingRect().intersected(QRect(QPoint(0, 0), mw->size()));
    *statusBarImage = QImage(statusBarRect->size(), QImage::Format_ARGB32_Premultiplied);
    statusBarImage->fill(QColor(Qt::transparent).rgba());
    QPainter painter(statusBarImage);
    mw->render(&painter, statusBarImage->rect(), *statusBarRect);
}

void HbSplashIndicatorCompositor::composeToBitmap(void *bitmap,
        Qt::Orientation orientation,
        int splashExtraFlags)
{
#ifdef Q_OS_SYMBIAN
    if (!(splashExtraFlags & HbSplashNonStandardStatusBar)) {
        const QImage *srcImg = orientation == Qt::Horizontal ? &mStatusBarImageLsc
                               : &mStatusBarImagePrt;
        const QRect *sbRect = orientation == Qt::Horizontal ? &mStatusBarRectLsc
                              : &mStatusBarRectPrt;
        if (!srcImg->isNull()) {
            CFbsBitmap *bmp = static_cast<CFbsBitmap *>(bitmap);
            uchar *dst = reinterpret_cast<uchar *>(bmp->DataAddress());
            const int dstBpl = CFbsBitmap::ScanLineLength(bmp->SizeInPixels().iWidth,
                               bmp->DisplayMode());
            const uchar *src = srcImg->bits();
            const int srcBpl = srcImg->bytesPerLine();
            const int dstLineStartOffset = sbRect->left() * 4;
            const int y0 = sbRect->top();
            const int y1 = sbRect->bottom();
            for (int y = y0; y <= y1; ++y) {
                int dstOffset = y * dstBpl + dstLineStartOffset;
                int srcOffset = (y - y0) * srcBpl;
                qMemCopy(dst + dstOffset, src + srcOffset, srcBpl);
            }
        }
    }
#else
    Q_UNUSED(bitmap);
    Q_UNUSED(orientation);
    Q_UNUSED(splashExtraFlags);
#endif
}

bool HbSplashIndicatorCompositor::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == HbEvent::SleepModeEnter && !mSleeping) {
        mSleeping = true;
    } else if (event->type() == HbEvent::SleepModeExit && mSleeping) {
        mSleeping = false;
        queueRender();
    }
    return QObject::eventFilter(obj, event);
}
