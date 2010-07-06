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

#ifndef HBGRAPHICSSCENE_P_H
#define HBGRAPHICSSCENE_P_H

#include <QBasicTimer>
#include <QPointF>
#include <QPoint>
#include <QGraphicsItem>
#include "hbgraphicsscene.h"

class HbPopupManager;
class HbToolTipLabel;
class HbInputMethod;
class HbPopup;

class HbGraphicsScenePrivate
{
    Q_DECLARE_PUBLIC(HbGraphicsScene)
public:
    HbGraphicsScenePrivate();
    virtual ~HbGraphicsScenePrivate();

    void setInputFocus(QGraphicsItem *focusingWidget);
    void clearInputFocus();

    bool focusChangeEvent(const QKeyEvent *event);

    void showPopup(HbPopup *popup);
    void hidePopup(HbPopup *popup);

    HbPopupManager *popupManager();

public:
    HbGraphicsScene *q_ptr;
    HbPopupManager *mPopupManager;
    HbToolTipLabel *mToolTip;
    bool mInputFocusSet;
    bool mPolishWidgets;
    bool mRepolishWidgets;
    int mPolishItemsIndex;
    // fps counter
    static bool fpsCounterEnabled;
    int mDrawCount;
    qreal mFPS;
    QTime *mFPSTime;
    qreal mMaxFPS;

private:

    class GraphicsObject: public QGraphicsObject
    {
    public:
        using QGraphicsObject::inputMethodEvent;
        using QGraphicsObject::inputMethodQuery;
    };

    static HbGraphicsScenePrivate *d_ptr(HbGraphicsScene *scene) {
        Q_ASSERT(scene);
        return scene->d_func();
    }
    friend class HbWidget;
};

#endif // HBGRAPHICSSCENE_P_H
