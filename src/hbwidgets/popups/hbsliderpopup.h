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

#ifndef HB_SLIDERPOPUP_H
#define HB_SLIDERPOPUP_H

#include <hbslider.h>
#include <hbdialog.h>
#include <QList>

class HbSliderPopupPrivate;

class HB_WIDGETS_EXPORT HbSliderPopup : public HbDialog
{
    Q_OBJECT
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(int  minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(int pageStep READ pageStep WRITE setPageStep)
    Q_PROPERTY(int singleStep READ singleStep WRITE setSingleStep)
    Q_PROPERTY(int sliderPosition READ sliderPosition WRITE setSliderPosition)
    Q_PROPERTY(bool tracking READ hasTracking WRITE setTracking)
    Q_PROPERTY(int value READ value WRITE setValue USER true)
    Q_PROPERTY(Hb::SliderTickPositions tickPosition  READ tickPosition WRITE setTickPosition)
    Q_PROPERTY(int majorTickInterval READ majorTickInterval WRITE setMajorTickInterval)
    Q_PROPERTY(int minorTickInterval READ minorTickInterval WRITE setMinorTickInterval)
    Q_PROPERTY(QStringList majorTickLabels READ majorTickLabels WRITE setMajorTickLabels)
    Q_PROPERTY(QStringList minorTickLabels READ minorTickLabels WRITE setMinorTickLabels)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(bool invertedAppearance READ invertedAppearance WRITE setInvertedAppearance)
    Q_PROPERTY(bool toolTipVisible READ isToolTipVisible WRITE setToolTipVisible)
    Q_PROPERTY(Qt::Alignment toolTipAlignment READ toolTipAlignment WRITE setToolTipAlignment)
    Q_PROPERTY(bool trackFilled READ isTrackFilled WRITE setTrackFilled)
    Q_PROPERTY(QList<QVariant> sliderElements READ sliderElements WRITE setSliderElements)
    Q_PROPERTY(QMap elementIcons READ elementIcons WRITE setElementIcons)
 
 public:
    explicit HbSliderPopup(QGraphicsItem *parent = 0);
    explicit HbSliderPopup(Qt::Orientation orientation, QGraphicsItem *parent = 0);
    virtual ~HbSliderPopup();

    bool invertedAppearance() const;
    void setInvertedAppearance(bool inverted);

    int maximum() const;
    void setMaximum(int max);

    int minimum() const;
    void setMinimum(int min);

    void setRange(int min, int max);
    Qt::Orientation orientation() const;

    int pageStep() const;
    void setPageStep(int step);

    int singleStep() const;
    void setSingleStep(int step);

    int sliderPosition() const;
    void setSliderPosition(int pos);

    bool hasTracking() const;
    void setTracking(bool enable);

    Hb::SliderTickPositions tickPosition() const;
    void setTickPosition(Hb::SliderTickPositions position);

    HbSlider::SnappingMode snappingMode() const;  
    void setSnappingMode(HbSlider::SnappingMode mode);

    HbIcon icon(HbSlider::SliderElement element) const;
    void setIcon(HbSlider::SliderElement element, const HbIcon &icon);

    void setElementIcon(HbSlider::SliderElement element, const HbIcon &icon);
    HbIcon elementIcon(HbSlider::SliderElement element) const; 

    QList<HbSlider::SliderElement> elements() const;
    void setElements(const QList<HbSlider::SliderElement> &elements);

    QMap<QString,QVariant> elementIcons( )const;
    void setElementIcons(const QMap<QString,QVariant> &elements);

    void setSliderElements(const QList<QVariant> &elements);
    QList<QVariant> sliderElements() const;

    int majorTickInterval() const;
    void setMajorTickInterval(int interval);

    int minorTickInterval() const;
    void setMinorTickInterval(int interval);

    QStringList majorTickLabels() const;
    void setMajorTickLabels(const QStringList &majorTickLabels);

    QStringList minorTickLabels() const;    
    void setMinorTickLabels(const QStringList &minorTickLabels);

    int value() const;
    QString text() const;

    void setToolTipAlignment(Qt:: Alignment);
    Qt:: Alignment toolTipAlignment() const;

    virtual void setToolTipVisible(bool);
    bool isToolTipVisible();

    void setTrackFilled(bool trackVisible = true );
    bool isTrackFilled() const;

    virtual QGraphicsItem * primitive(HbStyle::Primitive primitive) const;

    enum { Type = Hb::ItemType_SliderPopup };
    int type() const { return Type; }

public slots:
    void setValue(int value);
    void setText(const QString &text);
    void setOrientation(Qt::Orientation orientation);
    void updatePrimitives();

signals:
    void valueChanged(int value);
    void sliderPressed();
    void sliderReleased();
    void sliderMoved(int value);
    void rangeChanged(int min, int max);
    void iconPressed();
    void iconReleased();
    void iconClicked();
    void textClicked();

protected:
    HbSliderPopup(HbSliderPopupPrivate &dd, QGraphicsItem *parent);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbSliderPopup)
    Q_DISABLE_COPY(HbSliderPopup)
    Q_PRIVATE_SLOT(d_func(), void _q_valueChanged(int val ))

};


#endif // HB_SLIDERPOPUP_H
