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
#ifndef HBVKBHOSTCONTAINERWIDGET_P_H
#define HBVKBHOSTCONTAINERWIDGET_P_H

#include <QGraphicsObject>
#include <QGraphicsView>
#include <QPointer>
#include <QPointF>
#include <QRectF>
#include <QSizeF>

#include <hbglobal.h>

class QWidget;

class HbInputFocusObject;

class HB_CORE_PRIVATE_EXPORT HbVkbHostContainerWidget
{
public:
    explicit HbVkbHostContainerWidget(QObject *containterWidget);
    void setPos(QPointF newPosition);
    QPointF pos() const;
    QRectF sceneBoundingRect() const;
    QObject *widgetObject() const {
        if (mGraphicsObject) {
            return mGraphicsObject.data();
        }
        return mWidget.data();
    }
    void connectSignals(QObject *receiver);
    void disconnectSignals(QObject *receiver);   
    QPointF fixedContainerMovement() const;

private:
    QPointer<QWidget> mWidget;
    QPointer<QGraphicsObject> mGraphicsObject;
    mutable QGraphicsItem *mTopLevelItem;
};

#endif // HBVKBHOSTCONTAINERWIDGET_P_H

// End of file
