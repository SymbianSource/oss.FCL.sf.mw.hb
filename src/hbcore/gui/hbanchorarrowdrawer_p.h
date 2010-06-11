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

#ifndef HBANCHORARROWDRAWER_P_H
#define HBANCHORARROWDRAWER_P_H

#include <hbwidgetbase.h>
#include <hbglobal.h>
QT_FORWARD_DECLARE_CLASS(QGraphicsItem)
QT_FORWARD_DECLARE_CLASS(HbMeshLayout)

class HB_CORE_PRIVATE_EXPORT HbAnchorArrowDrawer : public HbWidgetBase
{
    Q_OBJECT

public:
    explicit HbAnchorArrowDrawer(HbMeshLayout* mesh, QGraphicsItem *parent = 0);
    virtual ~HbAnchorArrowDrawer();

public slots:
    void setDrawOutlines(bool enabled) { mDrawOutlines = enabled; };
    void setDrawArrows(bool enabled) { mDrawArrows = enabled; };
    void setDrawSpacers(bool enabled) { mDrawSpacers = enabled; };
    void updateFocusItem(const QGraphicsItem* item);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
    void changeEvent(QEvent *event);
    void updateColors();

private:
    HbMeshLayout* mLayout;
    bool mDrawOutlines;
    bool mDrawArrows;
    bool mDrawSpacers;
    QColor mValidColor;
    QColor mInvalidColor;
    QColor mBoxColor;
};

#endif // HBANCHORARROWDRAWER_P_H

