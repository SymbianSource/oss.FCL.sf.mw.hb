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

#ifndef HBTITLEPANE_P_H
#define HBTITLEPANE_P_H

#include <hbwidget.h>
#include "hbnamespace_p.h"

class HbTitlePanePrivate;
class HbStyleOptionTitlePane;

class HB_AUTOTEST_EXPORT HbTitlePane : public HbWidget
{
    Q_OBJECT

public:
    HbTitlePane(QGraphicsItem *parent = 0);
    virtual ~HbTitlePane();

    QString text() const;

    enum { Type = HbPrivate::ItemType_TitlePane };
    int type() const { return Type; }

    void resetMode();

signals:
    void launchPopup(const QPointF &pos);
    void visibilityChanged();

public slots:
    void setText(const QString &text);
    void updatePrimitives();

protected:
    HbTitlePane(HbTitlePanePrivate &dd, QGraphicsItem *parent = 0);

    void initStyleOption(HbStyleOptionTitlePane *option) const;
	
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);	
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void polish(HbStyleParameters &params);
    bool eventFilter(QObject *object, QEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbTitlePane)
    Q_DISABLE_COPY(HbTitlePane)

    friend class HbTitleBar;
};

#endif // HBTITLEPANE_P_H
