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

#ifndef HBSLIDERCONTROL_H
#define HBSLIDERCONTROL_H

#include "hbglobal.h"
#include "hbnamespace_p.h"
#include "hbabstractslidercontrol.h"

class HbStyleOptionSlider;
class HbSliderControlPrivate;

QT_BEGIN_NAMESPACE
class QStyleOptionSlider;
QT_END_NAMESPACE

class HB_AUTOTEST_EXPORT HbSliderControl : public HbAbstractSliderControl
{
    Q_OBJECT

public:
    explicit HbSliderControl(QGraphicsItem *parent = 0);
    explicit HbSliderControl(Qt::Orientation orientation, QGraphicsItem *parent = 0);
    virtual  ~HbSliderControl();

    enum SnappingMode {
        NoSnapping,
        MajorTickSnapping,
        MinorTickSnapping
    };

    Hb::SliderTickPositions tickPosition() const;
    void setTickPosition(Hb::SliderTickPositions position);

    int majorTickInterval() const;
    void setMajorTickInterval(int interval);

    int minorTickInterval() const;
    void setMinorTickInterval(int interval);

    void setMajorTickLabels(const QStringList &majorTickLabels);
    QStringList majorTickLabels() const;

    void setMinorTickLabels(const QStringList &minorTickLabels);
    QStringList minorTickLabels() const;

    SnappingMode snappingMode() const;
    void setSnappingMode(SnappingMode mode);

    enum { Type = HbPrivate::ItemType_SliderControl };
    int type() const { return Type; }

    void setToolTipVisible(bool );
    bool isToolTipVisible()const;

    void setToolTipAlignment(Qt:: Alignment);
    Qt:: Alignment toolTipAlignment() const;
    void setSliderPosition(int);
    void setHandleIcon(const QString &handleIcon);
    QString handleIcon() const;

    void setHandleItem(QGraphicsItem *handleItem);
    QGraphicsItem *handleItem() const;

    void setHandleVisible(bool);
    bool handleVisible() const;


    void updateSliderPosToTick();

    void setPreviousValue(bool , int );

    void enableTrackEventHandling(bool enable = true);
    bool isTrackEventHandlingEnabled ( );

    void setTrackFilled(bool trackVisible );
    bool isTrackFilled() const;
    QSizeF getHandleSize() ;

public slots:
    void updateTheme();
    void updatePrimitives();
    void showToolTip();
 
private slots:

    void hideToolTip();

protected:
    HbSliderControl(HbSliderControlPrivate &dd,QGraphicsItem *parent);
    virtual void changeEvent(QEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
#ifdef HB_GESTURE_FW
    virtual void gestureEvent(QGestureEvent *event);
#endif 
    bool sceneEventFilter(QGraphicsItem *obj,QEvent *event);
    virtual void resizeEvent(QGraphicsSceneResizeEvent *event);
    virtual QGraphicsItem *primitive(HbStyle::Primitive primitive) const;
    void releasedOutside(bool);

    void initStyleOption(HbStyleOptionSlider *option) const;

    virtual void sliderChange(SliderChange change);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual bool sceneEvent(QEvent *event);
    virtual void polish( HbStyleParameters& params );

signals:
    void releasedOutside( );

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbSliderControl)
    Q_DISABLE_COPY(HbSliderControl)
    friend class HbSliderHandle;
    friend class HbSliderTickmarks;
    friend class HbSliderTickmarksLabel;
};

#endif // HBSLIDERCONTROL_H
