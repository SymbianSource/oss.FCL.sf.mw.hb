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

#ifndef HBRATINGSLIDER_H
#define HBRATINGSLIDER_H

#include <hbwidget.h>

class HbRatingSliderPrivate;

class HB_WIDGETS_EXPORT HbRatingSlider : public HbWidget
{
    Q_OBJECT
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
	Q_PROPERTY(int numberOfIcons READ numberOfIcons WRITE setNumberOfIcons)
	Q_PROPERTY(int stepCount READ stepCount WRITE setStepCount)
	Q_PROPERTY(int currentRating READ currentRating WRITE setCurrentRating)
	Q_PROPERTY(QString unRatedIconName READ unRatedIconName WRITE setUnRatedIconName)
	Q_PROPERTY(QString ratedIconName READ ratedIconName WRITE setRatedIconName)

public:
    explicit HbRatingSlider(QGraphicsItem *parent = 0);
    ~HbRatingSlider();

    enum { Type = Hb::ItemType_RatingSlider };
    int type() const { return Type; }
	
	void setNumberOfIcons(int number);
	int numberOfIcons() const;

	void setStepCount(int count);
	int stepCount()const;

    bool isReadOnly() const;
    void setReadOnly(bool value);

    void setCurrentRating(int currentRating);
    int currentRating() const ;

    void setUnRatedIconName(const QString name);
    QString unRatedIconName() const;

    void setRatedIconName(const QString name);
	QString ratedIconName() const;

	QGraphicsItem * primitive(HbStyle::Primitive primitive) const; 
	
	void setGeometry(const QRectF &rect);

public slots :
    void updatePrimitives();

signals:
    void ratingDone(int ratingValue);
    void ratingChanged(int ratingValue);

protected:
    HbRatingSlider(HbRatingSliderPrivate &dd,QGraphicsItem *parent = 0);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
#ifndef HB_GESTURE_FW
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent (QGraphicsSceneMouseEvent *event) ;    
#else
    virtual void gestureEvent(QGestureEvent *event);
#endif
    void initStyleOption(HbStyleOption *option) const;
    void changeEvent(QEvent *event);
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    Q_DECLARE_PRIVATE_D( d_ptr, HbRatingSlider)
    Q_DISABLE_COPY( HbRatingSlider )
};



#endif // HBRATINGSLIDER_H

