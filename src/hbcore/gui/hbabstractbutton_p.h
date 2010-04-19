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

#ifndef HBABSTRACTBUTTON_P_H
#define HBABSTRACTBUTTON_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QBasicTimer>
#include "hbwidget_p.h"

class HbGestureSceneFilter;

class HB_CORE_PRIVATE_EXPORT HbAbstractButtonPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC( HbAbstractButton )

public:
    HbAbstractButtonPrivate( QSizePolicy::ControlType type = QSizePolicy::DefaultType );
    virtual ~HbAbstractButtonPrivate();

    //QString text;
    //HbIcon icon;
    QSize iconSize;
    QKeySequence shortcut;
    int shortcutId;
    uint checkable :1;
    uint checked :1;
    uint autoRepeat :1;
    uint autoExclusive :1;
    uint down :1;
    uint blockRefresh :1;
    uint longPress :1;

    QBasicTimer repeatTimer;
    QBasicTimer animateTimer;
    int autoRepeatDelay, autoRepeatInterval;

    QSizePolicy::ControlType controlType;
    mutable QSize sizeHint;
    mutable bool mSizeHintPolish;
    mutable bool mRepolishRequested;

public:
    void init();
    void click();
    void refresh();

    QList<HbAbstractButton *>queryButtonList() const;
    HbAbstractButton *queryCheckedButton() const;
    void notifyChecked();
    void moveFocus(int key);
    void fixFocusPolicy();

    void emitPressed();
    void emitReleased();
    void emitClicked();
};

#endif // HBABSTRACTBUTTON_P_H
