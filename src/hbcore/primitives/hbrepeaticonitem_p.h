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

#ifndef HB_REPEATICONITEM_H
#define HB_REPEATICONITEM_H

#include <hbicon.h>
#include <hbwidgetbase.h>
#include <QPixmap>

class QTimer;

class HbRepeatIconItem : public HbWidgetBase
{
    Q_OBJECT
public:
    HbRepeatIconItem(QGraphicsItem *parent = 0);
    HbRepeatIconItem(const QString &name, QGraphicsItem *parent = 0);
    ~HbRepeatIconItem();

    void setName(const QString& name);    
    void setIconHeight(qreal height);
    void setIconWidth(int width);
    void setMask(QPixmap mask);
    void setAspectRatioMode(Qt::AspectRatioMode mode);
    void paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    void stopAnimation();
    void setOrientation(Qt::Orientation orientation);

protected:
    void resizeEvent ( QGraphicsSceneResizeEvent * event );

public slots:
     void updateAnimation();

public:
    HbIcon mIcon;
    Qt::AspectRatioMode mMode;
    QPixmap mMask;
    QPixmap mCopy;
    int mOffset;
    static QTimer mTimer;
    QPixmap* mRepeatPixmap;
    QRectF mParentRect;
    Qt::Orientation mOrientation;
	bool mConnected;
};

#endif // HB_REPEATICONITEM_H

