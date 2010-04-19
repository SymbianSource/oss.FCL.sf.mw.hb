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

#ifndef HBSTACKEDWIDGET
#define HBSTACKEDWIDGET

#include <QObject>
#include <hbglobal.h>
#include <hbnamespace.h>
#include <hbwidget.h>

class HbStackedWidgetPrivate;
class HbStackedLayout;

class HB_CORE_EXPORT HbStackedWidget : public HbWidget
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentChanged)

public:
    explicit HbStackedWidget(QGraphicsItem *parent = 0);
    ~HbStackedWidget();

public:

    int count() const;

    int addWidget(QGraphicsWidget *widget);
    int insertWidget(int index, QGraphicsWidget *widget);

    void removeWidget(QGraphicsWidget *widget);
    QGraphicsWidget *removeAt(int index);

    int indexOf(QGraphicsWidget *widget) const;
    int currentIndex() const;

    QGraphicsWidget *widgetAt(int index) const;
    QGraphicsWidget *currentWidget() const;

    void setCurrentWidget(QGraphicsWidget *widget, bool hideOld, bool showNew);

signals:
    void widgetAdded(int index);
    void widgetRemoved(int index);
    void widgetRemoved(QGraphicsWidget *widget);
    void currentChanged(int index);

public slots:
    void setCurrentIndex(int index);
    void setCurrentWidget(QGraphicsWidget *widget);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbStackedWidget)
    Q_DISABLE_COPY(HbStackedWidget)
};

#endif // HBSTACKEDWIDGET
