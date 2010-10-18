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

#ifndef HB_REPEATITEM_P_H
#define HB_REPEATITEM_P_H

#include <hbicon.h>
#include <hbwidgetbase.h>
#include <QPixmap>

class HbRepeatItem : public HbWidgetBase
{
    Q_OBJECT
public:
    HbRepeatItem(QGraphicsItem *parent = 0);
    HbRepeatItem(const QString &name, QGraphicsItem *parent = 0);
    ~HbRepeatItem();

    void setName(const QString& name);  
    void setRepeatingNumber(int number);
    void setAspectRatioMode(Qt::AspectRatioMode mode);
    void paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    void setOrientation(Qt::Orientation orientation);
    void setInverted(bool inversion);
    void createPixmap(QRectF rect);

protected:
    void resizeEvent ( QGraphicsSceneResizeEvent * event );

public:
    HbIcon mIcon;
    Qt::AspectRatioMode mMode;
    int mOffset;
    QPixmap* mRepeatPixmap;
    int mNumberOfRepeating;   
    Qt::Orientation mOrientation;
    bool inverted;
    int mRectWidth;
    int mRectHeight;
    qreal mBaseIconWidth;
    qreal mBaseIconHeight;
    int mBaseResizeWidth;
    int mBaseResizeHeight;
    qreal multiFactor;
};

class HbRepeatMaskItem : public HbRepeatItem
{
public:
    HbRepeatMaskItem(QGraphicsItem *parent = 0);
    HbRepeatMaskItem(const QString &name, QGraphicsItem *parent = 0);
    void paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    void setMaskValue(int maskValue);
    void setMaximum(int max);

protected:
    void resizeEvent ( QGraphicsSceneResizeEvent * event );

public:
    int mMaskValue;
    int mMaximum;
};

#endif // HB_REPEATITEM_P_H

