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

#ifndef HBABSTRACTSLIDERCONTROL_H
#define HBABSTRACTSLIDERCONTROL_H

#include <hbwidget.h>
#include <hbnamespace.h>

class HbAbstractSliderControlPrivate;

class HB_WIDGETS_EXPORT HbAbstractSliderControl : public HbWidget
{
    Q_OBJECT
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(int singleStep READ singleStep WRITE setSingleStep)
    Q_PROPERTY(int pageStep READ pageStep WRITE setPageStep)
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged USER true)
    Q_PROPERTY(int sliderPosition READ sliderPosition WRITE setSliderPosition NOTIFY sliderMoved)
    Q_PROPERTY(bool tracking READ hasTracking WRITE setTracking)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(bool invertedAppearance READ invertedAppearance WRITE setInvertedAppearance)
    Q_PROPERTY(bool invertedControls READ invertedControls WRITE setInvertedControls)
    Q_PROPERTY(bool sliderDown READ isSliderDown WRITE setSliderDown DESIGNABLE false)

public:
    explicit HbAbstractSliderControl(QGraphicsItem *parent = 0);
    explicit HbAbstractSliderControl(Qt::Orientation orientation, QGraphicsItem *parent = 0);
    ~HbAbstractSliderControl();

    int value() const;
    Qt::Orientation orientation() const;

    void setMinimum(int min);
    int minimum() const;

    void setMaximum(int max);
    int maximum() const;

    void setRange(int min, int max);

    void setSingleStep(int step);
    int singleStep() const;

    void setPageStep(int step);
    int pageStep() const;

    void setTracking(bool enable);
    bool hasTracking() const;

    void setSliderDown(bool down);
    bool isSliderDown() const;

    void setSliderPosition(int pos);
    int sliderPosition() const;

    void setInvertedAppearance(bool inverted);
    bool invertedAppearance() const;

    void setInvertedControls(bool inverted);
    bool invertedControls() const;

    enum SliderAction {
        SliderNoAction,
        SliderSingleStepAdd,
        SliderSingleStepSub,
        SliderPageStepAdd,
        SliderPageStepSub,
        SliderToMinimum,
        SliderToMaximum,
        SliderMove
    };
    void triggerAction(SliderAction action);

    SliderAction repeatAction() const;
    void setRepeatAction(SliderAction action, int pressValue = -1, int thresholdTime = 500, int repeatTime = 50);

    enum { Type = Hb::ItemType_AbstractSlider };
    int type() const { return Type; }

public slots:
    void setValue(int value);
    void setOrientation(Qt::Orientation orientation);

signals:
    void valueChanged(int value);
    void sliderPressed();
    void sliderMoved(int pos);
    void sliderReleased();

    void rangeChanged(int min, int max);

    void actionTriggered(int action);

protected:
    HbAbstractSliderControl(HbAbstractSliderControlPrivate &dd, QGraphicsItem *parent);

    enum SliderChange {
        SliderRangeChange,
        SliderOrientationChange,
        SliderStepsChange,
        SliderValueChange,
        SliderAppearanceChange
    };
    virtual void sliderChange(SliderChange change);

    void changeEvent(QEvent *event);
    void focusInEvent(QFocusEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void timerEvent(QTimerEvent *event);
 
private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbAbstractSliderControl)
    Q_DISABLE_COPY(HbAbstractSliderControl)
};

#endif // HBABSTRACTSLIDERCONTROL_H
