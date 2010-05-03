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

#ifndef HBTOOLBAREXTENSION_P_H
#define HBTOOLBAREXTENSION_P_H

#include <hbglobal.h>
#include <hbnamespace.h>
#include <hbdialog_p.h>

class HbToolButton;
class QGraphicsGridLayout;

class HbToolBarExtensionPrivate : public HbDialogPrivate
{
    Q_DECLARE_PUBLIC(HbToolBarExtension)

public:
    HbToolBarExtensionPrivate();
    virtual ~HbToolBarExtensionPrivate();

    void initialiseContent();
    void doLayout();
    void placeToolBarExtension();
    void actionAdded( QActionEvent *event );
    void actionRemoved( QActionEvent *event );
    void actionChanged();

    void setAlignment(Qt::Alignment alignment);
    void setExtensionAction(HbAction *action);

    void _q_orientationChanged();
    void _q_animateButtonClicked();

public:
    QList<HbToolButton *> mToolButtons;
    QGraphicsGridLayout *mLayout;
    HbAction *extensionAction;
    Qt::Alignment mAlignment;
    bool mDefaultContentWidget;
    qreal mMargins;
    int   mRowsPortrait;
    int   mRowsLandscape;
    int   mColsPortrait;
    int   mColsLandscape;
    bool  lazyInitDone;
    bool  orientationConnectDone;

private:
    HbToolButton *mExtendedButton;
    HbToolBar *mToolBar;

protected:
    void init();
    void doLazyInit();
private:
    static bool extensionEffectsLoaded;
    static HbToolBarExtensionPrivate *d_ptr(HbToolBarExtension *tbe) {
        Q_ASSERT(tbe);
        return tbe->d_func();
    }
    friend class TestHbToolBarExtension;
    friend class HbToolButton;
    friend class HbToolBarPrivate;
    friend class HbAction;
};

#endif // HBTOOLBAREXTENSION_P_H
