/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbWidgets module of the UI Extensions for Mobile.
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

#ifndef HBPUSHBUTTON_P_H
#define HBPUSHBUTTON_P_H

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

#include "hbabstractbutton_p.h"
#include "hbicon.h"

#define HB_PUSHBUTTON_TYPE "HB_PUSHBUTTON"

QT_BEGIN_NAMESPACE
class QTimer;
class QGraphicsItem;
QT_END_NAMESPACE

class HbFrameDrawer;
class QGraphicsItem;

class HbPushButtonPrivate : public HbAbstractButtonPrivate
{
    Q_DECLARE_PUBLIC( HbPushButton )

public:
    HbPushButtonPrivate();
    virtual ~HbPushButtonPrivate();

    void createPrimitives();
    void initialize();
    void _q_handleLongKeyPress( );

    QGraphicsItem *textItem;
    QGraphicsItem *additionalTextItem;
    QGraphicsItem *touchArea;
    QGraphicsItem *iconItem;
    QGraphicsItem *frameItem;
    QGraphicsItem *focusItem;

    QString text;
    QString additionalText;
    HbIcon icon;
    HbIcon background;
    HbFrameDrawer *backgroundFrameDrawer;
    QTimer *longPressTimer;
    Qt::Alignment textAlignment;
    Qt::Alignment additionalTextAlignment;
    bool stretched;
    bool navigationKeyPress;
    bool hasTextAlignment;
    bool hasAdditionalTextAlignment;
};

#endif // HBPUSHBUTTON_P_H
