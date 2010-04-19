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

#include "hbcontentwidget_p.h"
#include "hbmainwindow.h"
#include "hbmainwindow_p.h"
#include "hbview.h"
#include "hbeffectinternal_p.h"
#include "hbwidgetfeedback.h"
#include "hbscreen_p.h"
#include <QEvent>
#include <QGraphicsSceneMouseEvent>

/*!
  \class HbContentWidget

  \brief Container for views in a mainwindow. Extends HbStackedWidget with view
  switch effects and other features.

  \internal
*/

HbContentWidget::HbContentWidget(HbMainWindow *mainWindow,QGraphicsItem *parent /*= 0*/):
    HbStackedWidget(parent),
    viewSwitchRunning(false),
    targetView(0),
    hidingView(0),
    mainWindow(mainWindow)
{
}

QSizeF HbContentWidget::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED( constraint );

    QSizeF size;    
    switch (which) {
    case Qt::MinimumSize:
        size = QSizeF(0, 0);
        break;
    
    case Qt::PreferredSize:
    {
        HbDeviceProfile profile(HbDeviceProfile::profile(this));
        size = profile.logicalSize();
        if (!size.isValid()) {
            size = QSizeF(1000, 1000);
        }
    }
    break;

    case Qt::MaximumSize:
    default:
        size = QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        break;
    }
    return size;
}

void HbContentWidget::delayedConstruction()
{
    setFocusPolicy(Qt::StrongFocus);
}

void HbContentWidget::setTargetView(HbView *view)
{
    targetView = view;
}

QString HbContentWidget::getEffectEvent(const char *baseName,
                                        Hb::ViewSwitchFlags flags,
                                        HbView *viewFrom,
                                        HbView *viewTo)
{
    QString event(baseName);
    if (flags & Hb::ViewSwitchUseAltEvent) {
        event.append(QLatin1String("_alt"));
    }
    bool backAnim = false;
    if (flags & Hb::ViewSwitchUseBackAnim) {
        backAnim = true;
    } else if (!(flags & Hb::ViewSwitchUseNormalAnim)) {
        // No view switch direction is explicitly specified, try to
        // guess from the context of the view switch, that is, the
        // ordering between the source and destination views (as there
        // is no other information available to us here).
        int fromIndex = indexOf(viewFrom);
        int toIndex = indexOf(viewTo);
        // If to > from use the baseName as it is (NormalAnim).
        // Otherwise add the suffix (BackAnim).
        if (toIndex < fromIndex) {
            backAnim = true;
        }
    }
    if (backAnim) {
        event.append(QLatin1String("_back"));
    }
    return event;
}

QGraphicsWidget *HbContentWidget::getEffectTarget(HbView *view, Hb::ViewSwitchFlags flags)
{
    // View switch effects ("show" and "hide") are always registered
    // for the HbView instance (not for its content widget). However
    // the graphics item to be animated may be something different
    // (the content widget, the "screen" item, etc.) so use the
    // special start() version from HbEffectInternal. Also, some views
    // may not have a content widget, in this case use the HbView as
    // the effect target.
    QGraphicsWidget *viewWidget = view->widget();
    QGraphicsWidget *effectTarget = viewWidget ? viewWidget : view;
    if (flags & Hb::ViewSwitchFullScreen) {
        effectTarget = HbMainWindowPrivate::d_ptr(mainWindow)->mClippingItem;
        if (!(flags & Hb::ViewSwitchSequential)) {
            // The Parallel+FullScreen combination does not make sense
            // (e.g. cannot animate the one and only titlebar
            // concurrently).
            qWarning("HbMainWindow: parallel fullscreen view switch is not supported");
            effectTarget = 0;
        }
    }
    return effectTarget;
}

void HbContentWidget::showTargetView()
{
    targetView->setVisible(true);
}

void HbContentWidget::hideEffectFinished(HbEffect::EffectStatus status)
{
    // Get rid of the old view and make sure the new one is ok in both
    // cases.  These need to be done also when the effect is canceled
    // otherwise we might end up with a previous view left visible in
    // the parallel case, etc.
    hidingView->setVisible(false);
    if (viewSwitchFlags & Hb::ViewSwitchSequential) {
        // Don't show targetView yet, make the call async to reduce
        // flickering on certain platforms (e.g. Linux). This is not a
        // perfect solution, though.
        QMetaObject::invokeMethod(this, "showTargetView", Qt::QueuedConnection);
        if (status.reason != Hb::EffectCancelled) {
            runViewSwitchEffectShow();
        }
    }
    if (status.reason == Hb::EffectCancelled) {
        viewSwitchRunning = false;
    }
}

void HbContentWidget::viewSwitchEffectsFinished(HbEffect::EffectStatus status)
{
    Q_UNUSED(status);
    viewSwitchRunning = false;
    if (HbMainWindowPrivate::d_ptr(mainWindow)->mDelayedConstructionHandled)
        HbMainWindowPrivate::d_ptr(mainWindow)->_q_viewReady();
}

/*!
  Starts the "hide" effect (the first phase of a view switch animation).
*/
void HbContentWidget::runViewSwitchEffectHide(HbView *viewToHide, Hb::ViewSwitchFlags flags)
{
    // Cancel all on-going effects. Relying on the cancel() calls made by the
    // start() functions would not be enough in all situations.
    HbEffectInternal::cancelAll(0, true); // ignore looping effects, those are not view switch effects and must not be stopped here
    viewSwitchRunning = true;

    // Make the new view the current one right away. This must be done asap to prevent
    // messed up state in mainwindow, the stack widget, etc. due to events coming during
    // the view switch animation.
    // 2nd param (hideOld): We still want to see the old view (normally setCurrentWidget would hide it).
    // 3rd param (showNew): The new view is not yet needed in the sequential case.
    setCurrentWidget(targetView, false, !(flags & Hb::ViewSwitchSequential));
    
    hidingView = viewToHide;
    viewSwitchFlags = flags;

    QGraphicsWidget *effectTarget = getEffectTarget(viewToHide, flags);
    if (effectTarget) {
        QString event = getEffectEvent("hide", flags, viewToHide, targetView);
        // The effect must not be persistent so pass 'true' as 3rd param (hideWhenFinished) to start().
        HbEffectInternal::start(viewToHide, effectTarget, true, "HB_view", event, this, "hideEffectFinished");
        if (!(flags & Hb::ViewSwitchSequential)) {
            runViewSwitchEffectShow();
        }
    }
}

/*!
  Starts the "show" effect. Must only be called from runViewSwitchEffectHide() or from
  the effect callback.
*/
void HbContentWidget::runViewSwitchEffectShow()
{
    QGraphicsWidget *effectTarget = getEffectTarget(targetView, viewSwitchFlags);
    if (effectTarget) {
        QString event = getEffectEvent("show", viewSwitchFlags, hidingView, targetView);
        HbEffectInternal::start(targetView, effectTarget, true, "HB_view", event, this, "viewSwitchEffectsFinished");
    }
}

void HbContentWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

bool HbContentWidget::event(QEvent *event)
{
    if (event->type() == QEvent::Polish) {
        // No need for any real polishing.
        static_cast<HbWidgetPrivate *>(d_ptr)->polished = true;
        return true;
    }
    return HbWidget::event(event);
}
