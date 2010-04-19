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

#ifndef HBPROGRESSIVESLIDER_H
#define HBPROGRESSIVESLIDER_H

#include <hbprogressbar.h>
#include <hbicon.h>

class HbProgressiveSliderPrivate;

QT_BEGIN_NAMESPACE
class QGraphicsLayoutItem;
QT_END_NAMESPACE

class HB_WIDGETS_EXPORT HbProgressiveSlider : public HbProgressBar
{
    Q_OBJECT
    Q_PROPERTY(int sliderValue READ sliderValue WRITE setSliderValue)
    Q_PROPERTY(QString handleToolTip READ handleToolTip WRITE setHandleToolTip)
    Q_PROPERTY(HbIcon thumbIcon READ thumbIcon WRITE setThumbIcon)
public:

    HbProgressiveSlider(QGraphicsItem *parent = 0);
    ~HbProgressiveSlider();

    void setThumbIcon(const HbIcon &icon);
    HbIcon thumbIcon() const;

    void setThumbItem(QGraphicsWidget* thumbItem);
    QGraphicsWidget* thumbItem() const;
   
    enum { Type = Hb::ItemType_ProgressiveSlider };
    int type() const { return Type; }

    virtual void setInvertedAppearance(bool inverted);

    virtual void setGeometry(const QRectF & rect);

    int sliderValue() const;
    bool isSliderDown() const;

    void setHandleToolTip(const QString &text);
    QString handleToolTip() const;

    void setTracking( bool enable );
    bool hasTracking( ) const;

signals:
    void sliderValueChanged(int value);
    void sliderPressed();
    void sliderReleased();
    void sliderMoved(int value);

public slots :
    void setSliderValue(int value);
    void updatePrimitives();

protected:
    HbProgressiveSlider(HbProgressiveSliderPrivate &dd,QGraphicsItem *parent = 0);
    virtual void resizeEvent(QGraphicsSceneResizeEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void showEvent( QShowEvent * event );
    //virtual void initStyleOption(HbStyleOption *option) const;//progbar initStyleOption is called, that's fine
    QVariant itemChange(GraphicsItemChange change,const QVariant & value);

private:
    Q_DECLARE_PRIVATE_D( d_ptr, HbProgressiveSlider)
    Q_DISABLE_COPY( HbProgressiveSlider )
};

#endif // HBPROGRESSIVESLIDER_H

