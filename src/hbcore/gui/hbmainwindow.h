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

#ifndef HBMAINWINDOW_H
#define HBMAINWINDOW_H

#include <QGraphicsView>

#include <hbglobal.h>
#include <hbnamespace.h>

QT_BEGIN_NAMESPACE
class QGraphicsWidget;
QT_END_NAMESPACE

class HbAction;
class HbMainWindowPrivate;
class HbView;

class HB_CORE_EXPORT HbMainWindow : public QGraphicsView
{
    Q_OBJECT

    Q_PROPERTY(Qt::Orientation orientation
               READ orientation
               WRITE setOrientation
               RESET unsetOrientation
               NOTIFY orientationChanged)

    Q_PROPERTY(bool obscuredState READ isObscured)

public:
    explicit HbMainWindow(QWidget *parent = 0, Hb::WindowFlags windowFlags = Hb::WindowFlagNone);
    ~HbMainWindow();

    HbView *addView(QGraphicsWidget *widget = 0);
    HbView *insertView(int index, QGraphicsWidget *widget = 0);
    void removeView(QGraphicsWidget *widget);
    QList<HbView *> views() const;
    HbView *currentView() const;
    void setCurrentView(HbView *view, bool animate = true, Hb::ViewSwitchFlags flags = Hb::ViewSwitchDefault);

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation orientation, bool animate = true);
    void unsetOrientation(bool animate = true);

    WId nativeBackgroundWindow();
    void resetNativeBackgroundWindow();

    QRectF layoutRect() const;

    void setBackgroundImageName(Qt::Orientation orientation, const QString &name);
    QString backgroundImageName(Qt::Orientation orientation) const;

    void setBackgroundImageMode(Hb::BackgroundImageMode mode);
    Hb::BackgroundImageMode backgroundImageMode() const;

    void setAutomaticOrientationEffectEnabled(bool enabled = true);
    bool automaticOrientationEffectEnabled() const;

    bool isObscured() const;

public slots:
    void broadcastEvent(int eventType);

signals:
    void viewReady();
    void aboutToChangeView(HbView *oldView, HbView *newView);
    void currentViewChanged(HbView *view);
    void aboutToChangeOrientation();
    void aboutToChangeOrientation(Qt::Orientation newOrientation, bool animated);
    void orientationChanged(Qt::Orientation orientation);
    void obscured();
    void revealed();

protected:
    void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);
    void customEvent(QEvent *event);
    void scrollContentsBy(int dx, int dy);
    void paintEvent(QPaintEvent *event);
    void showEvent(QShowEvent *event);
    bool event(QEvent *event);

    HbMainWindowPrivate *const d_ptr;

private:
    Q_DISABLE_COPY(HbMainWindow)
    Q_DECLARE_PRIVATE_D(d_ptr, HbMainWindow)
    Q_PRIVATE_SLOT(d_func(), void rootItemFirstPhaseDone(const HbEffect::EffectStatus &status))
    Q_PRIVATE_SLOT(d_func(), void rootItemFinalPhaseDone(const HbEffect::EffectStatus &status))
    Q_PRIVATE_SLOT(d_func(), void orientationEffectFinished(const HbEffect::EffectStatus &status))
    Q_PRIVATE_SLOT(d_func(), void _q_viewChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_viewRemoved(QGraphicsWidget *))
    Q_PRIVATE_SLOT(d_func(), void _q_viewTitleChanged(const QString &))
    Q_PRIVATE_SLOT(d_func(), void _q_viewToolBarChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_viewDockWidgetChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_launchMenu(const QPointF &))
    Q_PRIVATE_SLOT(d_func(), void _q_viewVisibleItemsChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_contentFullScreenChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_themeChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_continueBroadcasting(int))
    Q_PRIVATE_SLOT(d_func(), void _q_restoreTitlePane())
    Q_PRIVATE_SLOT(d_func(), void _q_delayedConstruction())
    Q_PRIVATE_SLOT(d_func(), void _q_viewReady())
};

#endif // HBMAINWINDOW_H
