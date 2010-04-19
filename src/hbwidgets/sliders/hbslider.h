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

#ifndef HBSLIDER_H
#define HBSLIDER_H

#include <hbwidget.h>

class HbSliderPrivate;

class HB_WIDGETS_EXPORT HbSlider : public HbWidget
{
    Q_OBJECT
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(int pageStep READ pageStep WRITE setPageStep)
    Q_PROPERTY(int singleStep READ singleStep WRITE setSingleStep)
    Q_PROPERTY(int sliderPosition READ sliderPosition WRITE setSliderPosition)
    Q_PROPERTY(bool tracking READ hasTracking WRITE setTracking) 
    Q_PROPERTY(int value READ value WRITE setValue USER true)
    Q_PROPERTY(Hb::SliderTickPositions tickPosition READ tickPosition WRITE setTickPosition) // does not work in armv5
    Q_PROPERTY(int majorTickInterval READ majorTickInterval WRITE setMajorTickInterval)
    Q_PROPERTY(int minorTickInterval READ minorTickInterval WRITE setMinorTickInterval)
    Q_PROPERTY(QStringList majorTickLabels READ majorTickLabels WRITE setMajorTickLabels)
    Q_PROPERTY(QStringList minorTickLabels READ minorTickLabels WRITE setMinorTickLabels)
    Q_PROPERTY(SnappingMode snappingMode READ snappingMode WRITE setSnappingMode)
    Q_PROPERTY(bool iconCheckable READ isIconCheckable WRITE setIconCheckable)
    Q_PROPERTY(bool iconChecked READ isIconChecked WRITE setIconChecked)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(bool invertedAppearance READ invertedAppearance WRITE setInvertedAppearance)
    Q_PROPERTY(bool toolTipVisible READ isToolTipVisible WRITE setToolTipVisible)
    Q_PROPERTY(Qt::Alignment toolTipAlignment READ toolTipAlignment WRITE setToolTipAlignment)
    Q_PROPERTY(bool iconEnabled READ isIconEnabled WRITE setIconEnabled)
    Q_PROPERTY(QString handleIcon READ handleIcon WRITE setHandleIcon)
    Q_PROPERTY(bool trackFilled READ isTrackFilled WRITE setTrackFilled)
    Q_PROPERTY(QList<QVariant> sliderElements READ sliderElements WRITE setSliderElements)
    Q_PROPERTY(QMap elementIcons READ elementIcons WRITE setElementIcons)

    Q_ENUMS(SliderElement SnappingMode)

public:
    explicit HbSlider(QGraphicsItem *parent = 0);
    explicit HbSlider(Qt::Orientation orientation, QGraphicsItem *parent = 0);
    virtual ~HbSlider();

    enum SliderElement {
        IncreaseElement,
        TrackElement,
        DecreaseElement,
        IconElement,
        TextElement
    };

    enum SnappingMode {
        NoSnapping,
        MajorTickSnapping,
        MinorTickSnapping
    };

    void setMaximum(int max);
    int maximum() const;

    void setMinimum(int min);
    int minimum() const;

    Qt::Orientation orientation() const;

    void setPageStep(int step);
    int pageStep() const;

    void setSingleStep(int step);
    int singleStep() const;

    void setSliderPosition(int pos);
    int sliderPosition() const;

    void setTracking(bool enable);
    bool hasTracking() const;

    int value() const;

    void setMajorTickInterval(int interval);
    int majorTickInterval() const;

    void setMinorTickInterval(int interval);
    int minorTickInterval() const;

    void setMajorTickLabels(const QStringList &majorTickLabels);
    QStringList majorTickLabels() const;

    void setMinorTickLabels(const QStringList &minorTickLabels);
    QStringList minorTickLabels() const;

    void setSnappingMode(SnappingMode mode);
    SnappingMode snappingMode() const;

    void setIconCheckable(bool checkable);
    bool isIconCheckable() const;

    void setIconChecked(bool checked);
    bool isIconChecked() const;

    QString text() const;

    void setInvertedAppearance(bool inverted);
    bool invertedAppearance() const;

    virtual void setToolTipVisible(bool);
    bool isToolTipVisible() const;

    Qt::Alignment toolTipAlignment() const;
    void setToolTipAlignment(Qt::Alignment);

    void setRange(int min, int max);

    void setTickPosition(Hb::SliderTickPositions position);
    Hb::SliderTickPositions tickPosition() const;

    void setElements(const QList<SliderElement> &elements);
    QList<SliderElement> elements() const;

    void setIcon(SliderElement element, const HbIcon &icon);
    HbIcon icon(SliderElement element) const;   

    void setElementIcon(SliderElement element, const HbIcon &icon);
    HbIcon elementIcon(SliderElement element) const; 
    
    void setElementIcons(const QMap<QString,QVariant> &elements);
    QMap<QString,QVariant> elementIcons( )const;

    void setSliderElements(const QList<QVariant> &elements);
    QList<QVariant> sliderElements() const;


    bool isIconEnabled() const;
    void setIconEnabled(bool enabled);

    enum { Type = Hb::ItemType_Slider };
    int type() const { return Type; }

    void setHandleIcon(const QString& handlePath );
    QString handleIcon() const;

     void setTrackFilled(bool trackVisible = true );
     bool isTrackFilled() const;

    virtual QGraphicsItem * primitive(HbStyle::Primitive primitive) const;

public slots:
    void setOrientation(Qt::Orientation orientation);
    void setValue(int value);
    void setText(const QString &text);
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
    void iconToggled(bool checked);
    void textClicked(); 

protected:
    HbSlider(HbSliderPrivate &dd, QGraphicsItem *parent);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);    
    virtual void focusInEvent(QFocusEvent *event); 



private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbSlider)
    Q_DISABLE_COPY(HbSlider)
#ifdef HB_EFFECTS
    Q_PRIVATE_SLOT(d_func(), void  _q_startIconPressedEffect()) 
    Q_PRIVATE_SLOT(d_func(), void  _q_startIconReleasedEffect()) 
    Q_PRIVATE_SLOT(d_func(), void  _q_startTextClickEffect()) 
#endif
};

#endif // HBSLIDER_H
