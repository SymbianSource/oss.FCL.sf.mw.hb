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
#include "hbstatusbar_p.h"
#include "hbsleepmodelistener_p.h"
#include "hbevent.h"
#include <QTimer>
#include <QApplication>
#include <QDebug>

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

#define PRE "[hbsplashgenerator] [indicompositor]"

HbSplashIndicatorCompositor::HbSplashIndicatorCompositor(HbSplashGenerator *gen)
    : mGenerator(gen)
{
#ifdef HB_SPLASH_STATUSBAR_ENABLED
    // When the splash screens are regenerated the statusbar must be rendered
    // again too because the theme or the splashml files may have changed.
    connect(mGenerator, SIGNAL(finished()), SLOT(renderStatusBar()), Qt::QueuedConnection);

    // Regenerate every minute to have an up-to-date clock.
    // ### replace with notifications from indicator fw
    mRenderTimer = new QTimer(this);
    connect(mRenderTimer, SIGNAL(timeout()), SLOT(renderStatusBar()));
    mRenderTimer->setInterval(60000); // 1 min
    mRenderTimer->start();

    // There must be no activity while the device is sleeping so listen to sleep
    // mode events too.
    HbSleepModeListener::instance(); // just to make sure it is created
    QApplication::instance()->installEventFilter(this);
#endif
}

void HbSplashIndicatorCompositor::release()
{
    delete this;
}

void HbSplashIndicatorCompositor::renderStatusBar()
{
    // Try again later if a screen is just being generated. We share the same
    // mainwindow and our changes done here (orientation, statusbar visibility)
    // could possibly ruin the output.
    if (!mGenerator->lockMainWindow()) {
        QTimer::singleShot(1000, this, SLOT(renderStatusBar()));
        return;
    }
    try {
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
    qDebug() << PRE << "rendering status bar" << *statusBarRect;
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
            qDebug() << PRE << "composeToBitmap" << bitmap << orientation << splashExtraFlags;
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
    if (event->type() == HbEvent::SleepModeEnter && mRenderTimer->isActive()) {
        qDebug() << PRE << "entering sleep mode";
        mRenderTimer->stop();
    } else if (event->type() == HbEvent::SleepModeExit && !mRenderTimer->isActive()) {
        qDebug() << PRE << "leaving sleep mode";
        mRenderTimer->start();
    }
    return QObject::eventFilter(obj, event);
}
