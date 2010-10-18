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

#ifndef HBPROGRESSSLIDER_H
#define HBPROGRESSSLIDER_H

#include <hbprogressbar.h>
#include <hbicon.h>

class HbProgressSliderPrivate;
class HbStyleOptionProgressSlider;

class HB_WIDGETS_EXPORT HbProgressSlider : public HbProgressBar
{
    Q_OBJECT
    Q_PROPERTY(int sliderValue READ sliderValue WRITE setSliderValue)
    Q_PROPERTY(QString sliderToolTip READ sliderToolTip WRITE setSliderToolTip)
    Q_PROPERTY(QString handleIcon READ handleIcon WRITE setHandleIcon)

public:

    HbProgressSlider(QGraphicsItem *parent = 0);
    ~HbProgressSlider();

    enum { Type = Hb::ItemType_ProgressSlider };
    int type() const { return Type; }

    virtual void setInvertedAppearance(bool inverted);

    virtual void setGeometry(const QRectF & rect);

    int sliderValue() const;
    bool isSliderDown() const;

    void setSliderToolTip(const QString &text);
    QString sliderToolTip() const;

    void setHandleIcon(const QString& handlePath );
    QString handleIcon() const;

    QGraphicsItem *primitive(const QString &itemName) const;

signals:
    void sliderPressed();
    void sliderReleased();
    void sliderMoved(int value);
    void trackPressed();
    void trackReleased();

public slots :
    void setSliderValue(int value);
    void updatePrimitives();

protected:
    HbProgressSlider(HbProgressSliderPrivate &dd,QGraphicsItem *parent = 0);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void gestureEvent(QGestureEvent *event);
    void showEvent( QShowEvent *event );

    QVariant itemChange(GraphicsItemChange change,const QVariant & value);
    bool sceneEventFilter(QGraphicsItem *obj,QEvent *event);
    void initStyleOption( HbStyleOptionProgressSlider *option ) const;
    void initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive);

private:
    Q_DECLARE_PRIVATE_D( d_ptr, HbProgressSlider)
    Q_DISABLE_COPY( HbProgressSlider )
};

#endif // HBPROGRESSSLIDER_H

