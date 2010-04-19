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
#ifndef HBTOOLBAR_P_H
#define HBTOOLBAR_P_H

#include <QObject>
#include <QPointer>
#include "hbtoolbar.h"
#include "hbwidget_p.h"
#ifdef HB_EFFECTS
#include "hbeffect.h"
#endif // HB_EFFECTS

class HbToolButton;
QT_FORWARD_DECLARE_CLASS(QActionEvent)
QT_FORWARD_DECLARE_CLASS(QGraphicsLinearLayout)

class HbActionManager;

class HbToolBarPrivateCore : public QObject
{
    Q_OBJECT

    signals:
        void visibilityChanged();
        void valueChanged( qreal value );
        void orientationChanged();

    friend class HbToolBarPrivate;
    friend class HbToolBar;
    friend class TestHbToolbar;
};

class HB_AUTOTEST_EXPORT HbToolBarPrivate : public HbWidgetPrivate
{

    Q_DECLARE_PUBLIC(HbToolBar)

public:
    HbToolBarPrivate();
    virtual ~HbToolBarPrivate();

    void init();
    void doLazyInit();

public:
    void setOrientation( Qt::Orientation orientation );
    void delayedConstruction();
    void calculateMaximumButtons();
    void calculateButtonMinimumSize();
    void updateButtonStyle(HbToolButton *button,bool inExtension);
    void updateButtons(int maxToolBarButtons);
    void updateState();
    void actionAdded( QActionEvent *event );
    void actionRemoved( QActionEvent *event );
    void initToolBarExtension(HbToolBarExtension *extension);
    void updateToolBarExtensions();
    void doLayout();
    void _q_orientationChanged( Qt::Orientation orientation );

    void startAppearEffect();
    void startDisappearEffect();

    void startAppearOrientEffect();
    void startDisappearOrientEffect();

    void setExtensionLayout(HbToolButton *button, bool extensionLayout);
    void prepareButtonForExtension(HbToolButton *button);
    void prepareButtonForToolbar(HbToolButton *button,bool handlePolish = true);
    void resetVisibleButtonsList();
    void createToolButton(QAction *Action,bool update = false);
    void actionAdded(int index);
    void actionRemoved(int index,HbToolButton *button);
    void updateToolBarForSizeChange();
    bool fullUpdateNeeded();
    bool fullUpdateNeeded(int index);
    void updateExtension(bool moreButtonNeeded);
    void suppressNextAppearEffect(bool suppress = true);

#ifdef HB_EFFECTS
public:
    void _q_delayedHide(HbEffect::EffectStatus status);
    //void _q_delayedShow(HbEffect::EffectStatus status);
#endif // HB_EFFECTS

private:

    void updateButtonsLayoutDirection();
    void setButtonLayoutDirection( HbToolButton& button );

public:
    QList<QPointer<HbToolButton> > mToolButtons;
    QList<QPointer<HbToolButton> > mVisibleToolButtons;
    QGraphicsLinearLayout *mLayout;

    HbToolButton *mPressedDownButton;
    HbToolButton *mPreviouslyPressedDownButton;
    HbToolBarExtension *mToolBarExtension;

    Qt::Orientation mOrientation;

    HbToolBarPrivateCore core;

    HbActionManager *actionManager;
    bool hasEffects;
    static bool effectsLoaded;

    bool hidingInProgress;
    bool delayedHide;
    bool delayedStartEffects;
    bool emitVisibilityChangeSignal;
    bool polishButtons;
    QSizeF minimumToolButtonSize;
    int maxToolBarButtons;
    QPointer<HbToolBarExtension> moreExtension;
    QPointer<HbToolButton> moreExtensionButton;

    bool mDialogToolBar;
    bool mDoLayout;    
    bool mDoLayoutPending;
    bool mOrientationEffectsRunning;
    bool mSuppressNextAppearEffect;

private:
    static HbToolBarPrivate *d_ptr( HbToolBar *toolbar ) {
        Q_ASSERT(toolbar);
        return toolbar->d_func();
    }
    friend class HbMainWindow;
    friend class HbMainWindowPrivate;
    friend class TestHbToolbar;
    friend class ContentWidget;
    friend class ToolbarView;
    friend class HbToolBarExtensionPrivate;
    friend class HbScreen;
    friend class HbDialog;
};

#endif // HBTOOLBAR_P_H
