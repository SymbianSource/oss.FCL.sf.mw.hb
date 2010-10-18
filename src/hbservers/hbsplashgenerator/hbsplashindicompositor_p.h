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

#ifndef HBSPLASHINDICOMPOSITOR_P_H
#define HBSPLASHINDICOMPOSITOR_P_H

#include "hbsplashcompositor_p.h"
#include "hbstatusbar_p.h"
#include <QObject>
#include <QImage>
#include <QRect>

class HbSplashGenerator;
class HbMainWindow;

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

#ifdef Q_OS_SYMBIAN
#include <e32property.h>
class CFbsBitmap;
#endif

class HbSplashIndicatorCompositor : public QObject, public HbSplashCompositorInterface
{
    Q_OBJECT

public:
    HbSplashIndicatorCompositor(HbSplashGenerator *gen);
    ~HbSplashIndicatorCompositor();
    void release();
    void composeToBitmap(void *bitmap, Qt::Orientation orientation, int splashExtraFlags);

private slots:
    void renderStatusBar();
    void handleStatusBarContentChange(HbStatusBar::ContentChangeFlags changeType);

private:
    void connectSignals();
    void queueRender(bool lazy = false);
    void doRender(HbMainWindow *mw, QImage *statusBarImage, QRect *statusBarRect);
    bool eventFilter(QObject *obj, QEvent *event);
#ifdef Q_OS_SYMBIAN
    void publishAsBitmap(const QImage &image, CFbsBitmap **bitmap, RProperty *prop);
#endif

    HbSplashGenerator *mGenerator;
    bool mSleeping;
    bool mSignalsConnected;
    QImage mStatusBarImagePrt;
    QImage mStatusBarImageLsc;
    QRect mStatusBarRectPrt;
    QRect mStatusBarRectLsc;
    QTimer *mRenderTimer;
#ifdef Q_OS_SYMBIAN
    CFbsBitmap *mStatusBarBitmapPrt;
    CFbsBitmap *mStatusBarBitmapLsc;
    RProperty mStatusBarBitmapHandlePropPrt;
    RProperty mStatusBarBitmapHandlePropLsc;
#endif
};

#endif
