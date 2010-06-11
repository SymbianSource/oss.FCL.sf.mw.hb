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

#ifndef HBTHETESTWIDGET_H
#define HBTHETESTWIDGET_H

#include <hbtoolbutton.h>
#include <hbwidget.h>

class HbTheTestWidgetPrivate;
class HbTheTestWidget;

class HbTheTestButton : public HbToolButton
{
    Q_OBJECT
public:
    HbTheTestButton(QGraphicsItem *parent = 0);
    ~HbTheTestButton();

    int type() const { return Hb::ItemType_Last+1; };

public slots:
    void updatePrimitives();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;

private:
    bool mPressedDown;
    bool mMoved;
    QPointF mPressedPos;
    QPointF mPosition;
    QSize mScreenSize;
    friend class HbTheTestWidget;
};


class HbTheTestWidget : public HbWidget
{
    Q_OBJECT

public:
    ~HbTheTestWidget();
    explicit HbTheTestWidget(HbMainWindow *mainWindow, QGraphicsItem *parent = 0);
    
    HbTheTestButton *button1();
    HbTheTestButton *button2();
    HbTheTestButton *button3();
    HbTheTestButton *button4();

public slots:
    void textLayoutMeasure();
    void textLayoutWriteReport();
    void toggleOrientation();
    void toggleMirroring();
    void setApplicationBackground(); // only for S60
    void showThemeServerMemoryInfo();
    void screenCapture(); // not working on HW
    void createSharedMemoryReport() const;

protected:
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;

private slots:
    void doScreenCapture();
    void orientationChanged(Qt::Orientation newOrientation);

private:
    void showWarning(QString text);

private:
    HbTheTestWidgetPrivate *d;
};


#endif

