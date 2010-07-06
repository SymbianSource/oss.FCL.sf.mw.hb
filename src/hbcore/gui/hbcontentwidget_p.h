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

#ifndef HBCONTENTWIDGET_P_H
#define HBCONTENTWIDGET_P_H

#include <hbstackedwidget.h>
#include <hbeffect.h>

class HbMainWindow;
class HbView;

class HbContentWidget : public HbStackedWidget
{
    Q_OBJECT

public:
    explicit HbContentWidget(HbMainWindow *mainWindow, QGraphicsItem *parent = 0);
    ~HbContentWidget() {}

    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;
    void setTargetView(HbView *view);
    void runViewSwitchEffectHide(HbView *viewToHide, Hb::ViewSwitchFlags flags);
    bool isSwitchingViews() const {
        return mViewSwitchRunning;
    }

private slots:
    void hideEffectFinished(HbEffect::EffectStatus status);
    void viewSwitchEffectsFinished(HbEffect::EffectStatus status);

private:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    bool event(QEvent *event);

    QString getEffectEvent(const char *baseName,
                           Hb::ViewSwitchFlags flags,
                           HbView *viewFrom,
                           HbView *viewTo);
    QGraphicsWidget *getEffectTarget(HbView *view, Hb::ViewSwitchFlags flags);
    void runViewSwitchEffectShow();

    bool mViewSwitchRunning;
    HbView *mTargetView;
    HbView *mHidingView;
    Hb::ViewSwitchFlags mViewSwitchFlags;
    HbMainWindow *mMainWindow;
};

#endif // HBCONTENTWIDGET_P_H
