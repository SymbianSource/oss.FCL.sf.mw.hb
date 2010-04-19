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

class HB_WIDGETS_EXPORT HbProgressSlider : public HbProgressBar
{
    Q_OBJECT
    Q_PROPERTY(int sliderValue READ sliderValue WRITE setSliderValue)
    Q_PROPERTY(HbIcon thumbIcon READ thumbIcon WRITE setThumbIcon)
    Q_PROPERTY(QString sliderToolTip READ sliderToolTip WRITE setSliderToolTip)
    Q_PROPERTY(SliderState state READ sliderState WRITE setSliderState)
    Q_ENUMS(SliderState)

    /* Deprecated */
    Q_PROPERTY(QString handleToolTip READ handleToolTip WRITE setHandleToolTip)
    Q_PROPERTY(HandleState state READ handleState WRITE setHandleState)    

public:

    /* Deprecated */
    enum HandleState
    {
        PlayNormal,
        PlayPressed,
        PauseNormal,
        PausePressed
    };

    enum SliderState
    {
        SliderStatePlayNormal,
        SliderStatePlayPressed,
        SliderStatePauseNormal,
        SliderStatePausePressed
    };

    HbProgressSlider(QGraphicsItem *parent = 0);
    ~HbProgressSlider();

    void setThumbIcon(const HbIcon &icon);
    HbIcon thumbIcon() const;

    enum { Type = Hb::ItemType_ProgressSlider };
    int type() const { return Type; }

    virtual void setInvertedAppearance(bool inverted);

    virtual void setGeometry(const QRectF & rect);

    int sliderValue() const;
    bool isSliderDown() const;

    /* Deprecated*/
    void setHandleToolTip(const QString &text);
    QString handleToolTip() const;

    /* Deprecated*/
    void setHandleState(HbProgressSlider::HandleState state);
    HbProgressSlider::HandleState handleState() const;

    void setSliderToolTip(const QString &text);
    QString sliderToolTip() const;

    void setSliderState(HbProgressSlider::SliderState state);
    HbProgressSlider::SliderState sliderState() const;

    QGraphicsItem* primitive(HbStyle::Primitive primitive) const; 

signals:
    void sliderPressed();
    void sliderReleased();
    void sliderMoved(int value);

public slots :
    void setSliderValue(int value);
    void updatePrimitives();

protected:
    HbProgressSlider(HbProgressSliderPrivate &dd,QGraphicsItem *parent = 0);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void showEvent( QShowEvent * event );
    virtual void initStyleOption(HbStyleOption *option) const;
    QVariant itemChange(GraphicsItemChange change,const QVariant & value);
    bool sceneEventFilter(QGraphicsItem *obj,QEvent *event);

private:
    Q_DECLARE_PRIVATE_D( d_ptr, HbProgressSlider)
    Q_DISABLE_COPY( HbProgressSlider )
};

#endif // HBPROGRESSSLIDER_H

