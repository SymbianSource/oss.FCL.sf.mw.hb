/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbInput module of the UI Extensions for Mobile.
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

#ifndef HB_INPUT_SCREENSHOT_WIDGET_H
#define HB_INPUT_SCREENSHOT_WIDGET_H

#include <QPixmap>

#include <hbwidget.h>

class HbInputScreenshotWidgetPrivate;

class HbInputScreenshotWidget : public HbWidget
{
    Q_OBJECT

public:

    HbInputScreenshotWidget(QGraphicsItem *parent = 0);
    virtual ~HbInputScreenshotWidget();

    void setScreenshot(QPixmap &pixmap);

protected: // From QGraphicsItem
    virtual QPainterPath shape() const;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual int type() const {
        return Hb::ItemType_InputVkbWidget;
    }

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputScreenshotWidget)
    Q_DISABLE_COPY(HbInputScreenshotWidget)
};

#endif // HB_INPUT_SCREENSHOT_WIDGET_H

// End of file
