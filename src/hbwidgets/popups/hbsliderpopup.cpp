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

#include <hbslider.h>
#include "hbsliderpopup_p.h"
#include <hbstyle.h>
#include <QDebug>

#include <QTimeLine>
#include <QGraphicsLinearLayout>

/*!   
    internal- HbSliderPopupContentWidget class 
    creates content widget for slider popup
*/
class HbSliderPopupContentWidget : public HbWidget
{
    Q_OBJECT
public:
    HbSliderPopupPrivate *d;
    HbSlider *slider;
    
    HbSliderPopupContentWidget(HbSliderPopupPrivate *priv,
        QGraphicsItem* parent =0) : HbWidget(parent),d(priv),slider(0)
    {
        d->createSlider();
        slider = d->slider;
        slider->setParentItem(this);
        HbStyle::setItemName( slider,"slider" );
    }
    enum { Type = HbPrivate::ItemType_SliderPopupContentWidget };
    int type() const { return Type; }
};

/*!   
    internal- HbSliderPopupPrivate class ctor
*/
HbSliderPopupPrivate::HbSliderPopupPrivate():slider(0)
{
}

/*!   
    internal- HbSliderPopupPrivate class dtor
*/
HbSliderPopupPrivate::~HbSliderPopupPrivate()
{

}

/*!  
    This is virtual function for creating slider
    derived class can over ride it and return required type of slider
    ex HbVolumeSlider or HbZoomSlider
*/
void HbSliderPopupPrivate::createSlider() 
{
    slider= new HbSlider();
}

/*!   
    This slot will be called when ever value change happens
    This will reset time out interval of popup
*/
void HbSliderPopupPrivate ::_q_valueChanged(int val)
{
    Q_Q(HbSliderPopup);
    startTimeout();
    emit q->valueChanged(val);
}

/*!
   create slider and delegate all signals of slider
*/
void HbSliderPopupPrivate::init()
{
    Q_Q( HbSliderPopup);
    mSliderPopupContentWidget = new HbSliderPopupContentWidget( this );
    q->setContentWidget( mSliderPopupContentWidget );
    q->setBackgroundItem(HbStyle::P_SliderPopup_background);
    HbStyle::setItemName(q->backgroundItem(),QLatin1String("sliderpopup_background"));

    q->connect(mSliderPopupContentWidget->slider,SIGNAL(valueChanged(int)),q,SLOT(_q_valueChanged(int)));
    q->connect(mSliderPopupContentWidget->slider,SIGNAL(sliderPressed()),q,SIGNAL(sliderPressed()));
    q->connect(mSliderPopupContentWidget->slider,SIGNAL(sliderReleased()),q,SIGNAL(sliderReleased()));
    q->connect(mSliderPopupContentWidget->slider,SIGNAL(rangeChanged(int , int  )),
        q,SIGNAL(rangeChanged(int , int  )));
    q->connect(mSliderPopupContentWidget->slider,SIGNAL(sliderMoved(int)),q,SIGNAL(sliderMoved(int)));
    q->connect(mSliderPopupContentWidget->slider,SIGNAL(iconPressed()),q,SIGNAL(iconPressed()));
    q->connect(mSliderPopupContentWidget->slider,SIGNAL(iconReleased()),q,SIGNAL(iconReleased()));
    q->connect(mSliderPopupContentWidget->slider,SIGNAL(iconClicked()),q,SIGNAL(iconClicked()));
    q->connect(mSliderPopupContentWidget->slider,SIGNAL(textClicked()),q,SIGNAL(textClicked()));

    q->setDismissPolicy(HbDialog::TapOutside);
    q->setBackgroundFaded(false);
    q->setModal(false);
    mainLayout->setContentsMargins(0,0,0,0);
    mSliderPopupContentWidget->slider->setFlags(QGraphicsItem::ItemIsFocusable);
}

/*!
    @beta
    @hbwidgets
    \class HbSliderPopup
    \brief HbSliderPopup widget provides a generic vertical or horizontal 
    slider embeeded in a popup.

    HbSliderPopup is a classic widget for controlling a bounded value.
    It lets the user move a slider handle along a horizontal or vertical
    track and translates the handle's position into an integer value
    within the legal range.

    The most important functions are setValue() to set the slider directly
    to some value; and setMinimum() and setMaximum() to define the range
    of the slider.

    HbSliderPopup consists of elements. The available slider elements are
    defined by the Hb::SliderElement enumeration. A set of elements
    can be set by setSliderElements() and currently existing elements can be
    queried by sliderElements().

    HbSliderPopup provides methods for controlling tickmarks.  You can use
    setTickPosition() to indicate where you want the tickmarks to be,
    setMajorTickInterval() and setMinorTickInterval() to indicate how
    many of them you want. The current set tick position and intervals
    can be queried using the tickPosition(), majorTickInterval() and
    minorTickInterval() functions, respectively.

    HbSliderPopup provides a comprehensive set of signals:
    \li valueChanged() Emitted when the slider's value has changed. The tracking()
       determines whether this signal is emitted during user
       interaction.
    \li sliderPressed() Emitted when the user starts to drag the slider.
    \li sliderMoved() Emitted when the user drags the slider.
    \li sliderReleased() Emitted when the user releases the slider.
    \li rangeChanged() Emitted when the range of the slider changes.
    \li iconPressed() Emitted when the user presses the associated slider icon.
    \li iconReleased() Emitted when the user releases the associated slider icon.
    \li iconClicked() Emitted when the user clicks the associated slider icon.

    HbSliderPopup provides only integer ranges. Note that although
    HbSliderPopup handles very large numbers, it becomes difficult for users
    to use a slider accurately for very large ranges.

    The slider handles keys in the following way:
    \li Left/Right moves a horizontal slider by one single step.
    \li Up/Down moves a vertical slider by one single step.

    Example usage:
    \code
    HbSliderPopup *slider = new HbSliderPopup();
    connect(slider, SIGNAL(valueChanged(int)), receiver, SLOT(volumeSliderChanged(int)));
    connect(slider, SIGNAL(iconToggled(bool)), receiver, SLOT(volumeMuted(bool)));
    \endcode

    \sa HbZoomSliderPopup, HbVolumeSliderPopup
*/

/*!
    \fn void HbSliderPopup::valueChanged(int value)

    This signal is emitted when the slider value has changed.

    \sa hasTracking()
*/

/*!
    \fn void HbSliderPopup::sliderPressed()

    This signal is emitted when the slider thumb has been pressed.
*/

/*!
    \fn void HbSliderPopup::sliderReleased()

    This signal is emitted when the slider thumb has been released.
*/

/*!
    \fn void HbSliderPopup::sliderMoved(int value)

    This signal is emitted when the slider position has changed.

    \sa hasTracking()
*/

/*!
    \fn void HbSliderPopup::rangeChanged(int min, int max)

    This signal is emitted when the slider range has changed.
*/

/*!
    \fn void HbSliderPopup::iconPressed()

    This signal is emitted when the slider icon has been pressed.
*/

/*!
    \fn void HbSliderPopup::iconReleased()

    This signal is emitted when the slider icon has been released.
*/

/*!
    \fn void HbSliderPopup::iconClicked()

    This signal is emitted when the slider icon has been clicked.
*/


/*!
    \reimp
    \fn int HbSliderPopup::type() const
 */

/*!
    \enum HbSliderPopup::SliderElement

    This enum defines available slider elements.
 */

/*!
    \var HbSliderPopup::IncreaseElement

    A button to increase the slider value. Usually a plus sign.
 */

/*!
    \var HbSliderPopup::TrackElement

    The slider track that consists of a groove, ticks and handle.
 */

/*!
    \var HbSliderPopup::DecreaseElement

    A button to decrease the slider value. Usually a minus sign.
 */

/*!
    \var HbSliderPopup::IconElement

    An icon element associated to the slider.
 */

/*!
    \var HbSliderPopup::TextElement

    A text element associated to the slider.
 */



/*!
    Constructs a slider embeeded in a popup with \a parent.

    The orientation is \b Qt::Vertical and the slider contains only track element by default.

    \sa setOrientation(), setElements()
*/
HbSliderPopup::HbSliderPopup(QGraphicsItem *parent) :
    HbDialog(*new HbSliderPopupPrivate, parent)
{
    Q_UNUSED( parent);
    Q_D(HbSliderPopup);
    d->q_ptr = this;
    d->init();
}

/*!
Constructs a sliderPopup with a \a orientation and \a parent.

The slider contains only track element by default.

\sa setElements()
*/
HbSliderPopup::HbSliderPopup(Qt::Orientation orientation, QGraphicsItem *parent) :
    HbDialog(*new HbSliderPopupPrivate, parent)
{
    Q_D(HbSliderPopup);
    d->q_ptr = this;
    d->init();
    setOrientation(orientation);
}

/*!
    \internal
 */
HbSliderPopup::HbSliderPopup(HbSliderPopupPrivate &dd, QGraphicsItem *parent) :
    HbDialog(dd, parent)
{
    Q_UNUSED( parent);
    Q_D(HbSliderPopup);
    d->q_ptr = this;
    d->init();
}

/*!
    Destructs the slider.
*/
HbSliderPopup::~HbSliderPopup()
{
}

/*!
    Returns the slider text.

    returns empty text if text element does not exist

    \sa setText()
*/
QString HbSliderPopup::text() const
{
    Q_D(const HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->text();
}

/*!
    Sets the slider \a text for text element 

    Slider will not take care of localization.

    \warning Setting text to a non-existing text element has no effect.

    \sa text()
*/
void HbSliderPopup::setText(const QString &text)
{
    Q_D(HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setText(text);
}

/*!
    Sets whether the tooltip is visible .

    \sa isToolTipVisible()
*/
void HbSliderPopup::setToolTipVisible(bool value)
{
    Q_D(const HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setToolTipVisible(value);
}

/*!
    Returns \c true if tooltip is visible.

    The default value is \c false.

    \sa setToolTipVisible()
*/
bool HbSliderPopup::isToolTipVisible()
{
    Q_D(const HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->isToolTipVisible();
}

/*!
   sets the toolTip Alignment with respect to the thumb.

    \sa toolTipAlignment()
*/
void HbSliderPopup::setToolTipAlignment(Qt::Alignment alignment)
{
    Q_D( HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setToolTipAlignment(alignment);
}

/*!
    returns the tooltip Alignment with respect to the thumb

    \sa setToolTipAlignment()
*/
Qt::Alignment HbSliderPopup::toolTipAlignment() const
{
    Q_D(const HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->toolTipAlignment();
}


/*!
    Returns the major ticklabels of the slider popup.
  
    \sa setMajorTickLabels()
*/
QStringList HbSliderPopup::majorTickLabels() const
{
    Q_D(const HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->majorTickLabels();
}

/*!
    Sets the major ticklabels of the slider popup.

    Detailed description:

    Strings from the stringlist will be taken sequentially for drawing
    LabelItem,there exists a one-to-one mapping between tickmarkitem &
    tickmarklabelitem.

    case 1: if strings provided > tickmarks, labels are drawn for the
    strings which maps the tickmarkitems.If there are extra strings 
    available in the list beyond 1-1 mapping, those strings will be left out.

    case 2: if strings provided < tickmarks, labels are drawn only for strings
    provided.Rest of the tickmarkitems will be drawn without labelItem.

    case 3: If some tickmarkitems to be drawn without labelItem in the middle,
    specify empty string ("") for that item  in the string list.

    case 4: if both major & minor tickinterval are same,then only majortickLabel
    strings are taken into account for drawing the labelItem

    \sa majorTickLabels()
*/
void HbSliderPopup::setMajorTickLabels(const QStringList &majorTickLabels)
{
    Q_D(HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->setMajorTickLabels(majorTickLabels);
}

/*!
    Returns the minor ticklabels of the slider popup.
  
    \sa setMajorTickLabels()
*/
QStringList HbSliderPopup::minorTickLabels() const
{
    Q_D(const HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->minorTickLabels();
}

/*!
    Sets the minor ticklabels of the slider popup.
    see setMajorTickLabels for detailed description

    \sa minorTickLabels(),setMajorTickLabels()
*/
void HbSliderPopup::setMinorTickLabels(const QStringList &minorTickLabels)
{
    Q_D(HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setMinorTickLabels(minorTickLabels);
}

/*!
    @beta
    Returns the icon of the \a element.

    returns NULL if element does not exist or icon was not set for this element

    \sa setElementIcon()
*/
HbIcon HbSliderPopup::elementIcon(HbSlider::SliderElement element) const
{
    Q_D(const HbSliderPopup);
    return(d->mSliderPopupContentWidget->slider->elementIcon(element));
}

/*!
    @beta
    Sets the \a icon if the \a element.

    Supported elements:
    \li HbSlider::IncreaseElement
    \li HbSlider::DecreaseElement
    \li HbSlider::IconElement

    \warning Setting icon to a non-existing element has no effect.

    \sa elementIcon() setElements()
*/
void HbSliderPopup::setElementIcon(HbSlider::SliderElement element, const HbIcon &icon)
{
    Q_D( HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setElementIcon(element,icon);
}



/*!
    @proto
    Returns the map , which consist of element name as key and icon name  as value

    returns NULL map if none of the element has icon

    \sa setElementIcons()
*/
QMap<QString,QVariant> HbSliderPopup::elementIcons( )const
{
    Q_D(const HbSliderPopup);
    return(d->mSliderPopupContentWidget->slider->elementIcons());
}


/*!
    @proto
    Sets the icons for elements

    key of \a elements is element name  QString)  and value is icon
    name to set on given element
    Use following string for specifying elements
    "IncreaseElement" for increase element
    "DecreaseElement" for decrease element
    "IconElement" for icon element
    
    Eg Usage:
    HbSlider slider;
    QMap<QString , QVariant> map;
    map.insert(  QString( "IncreaseElement", QVariant(":\icon\small.jpg"));
    map.insert(  QString( "IconElement", QVariant(":\icon\small.jpg"));
    slider.setElementIcons( map );

    \warning Setting icon to a non-existing element has no effect.

    \sa elementIcons() setSliderElements() setElementIcon()
*/

void HbSliderPopup::setElementIcons(const QMap<QString,QVariant> &elements)
{
    Q_D( HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setElementIcons(elements);
}


/*!
    Returns \c true whether the slider track is inverted.

    The default value is \c false.

    If this property is \c false, the minimum and maximum will
    be shown in its classic position. If the value is \c true,
    the minimum and maximum appear at their opposite location.

    \sa setInvertedAppearance()
*/
bool HbSliderPopup::invertedAppearance() const
{
    Q_D(const HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->invertedAppearance();
}

/*!
    Sets whether the slider track is \a inverted.

    \sa invertedAppearance()
*/
void HbSliderPopup::setInvertedAppearance(bool inverted)
{
    Q_D(HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setInvertedAppearance(inverted);
 }

/*!
    Returns the list of slider elements as QVariant ( can be type-casted to HbSlider::SliderElement ).   

    The slider contains only track element by default.

    \sa setSliderElements()
*/
QList<QVariant> HbSliderPopup::sliderElements() const
{
    Q_D(const HbSliderPopup);
    return (d->mSliderPopupContentWidget->slider->sliderElements());
}

/*!
    Sets the elements of the slider.

    \note Duplicate elements will be ignored.

    \note element order cant be changed

    Eg Usages:
    HbSliderPopup slider;
    QList<QVariant> list;
    list.append( QVariant( HbSlider::IncreaseElement));
    list.append( QVariant( HbSlider::DecreaseElement));
    list.append( QVariant( HbSlider::IconElement));
    slider.setElements( list);

    The available slider elements are
    defined by the HbSlider::SliderElement enumeration

    \sa sliderElements()

*/
void HbSliderPopup::setSliderElements(const QList<QVariant>&elements)
{
    Q_D(HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setSliderElements(elements);

}


/*!
    Returns the maximum value of the slider.

    The default value is \c 100.

    \sa setMaximum()
*/
int HbSliderPopup::maximum() const
{
    Q_D(const HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->maximum();
}

/*!
    Sets the maximum value of the slider.

    \note When setting this property, the minimum is adjusted if

    necessary to ensure that the range remains valid.  Also the

    slider's current value is adjusted to be within the new range.

    \sa maximum()
*/
void HbSliderPopup::setMaximum(int max)
{
    Q_D(HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setMaximum(max);
}

/*!
    Returns the minimum value of the slider.

    The default value is \c 0.

    \sa setMinimum()
*/
int HbSliderPopup::minimum() const
{
    Q_D(const HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->minimum();
}

/*!
    Sets the minimum value of the slider.

    \note When setting this property, the maximum is adjusted if
    necessary to ensure that the range remains valid. Also the
    slider's current value is adjusted to be within the new range.

    \sa minimum()
*/
void HbSliderPopup::setMinimum(int min)
{
    Q_D(HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setMinimum(min);
}

/*!
    This function is provided for convenience.

    Sets the slider's minimum to \a min and its maximum to \a max.

    If maximum is smaller than minimum, minimum becomes the only legal value.

    \sa setMinimum(), setMaximum()
*/
void HbSliderPopup::setRange(int min, int max)
{
    Q_D(HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setRange(min, max);
}

/*!
    Returns the page step of the slider.

    The default value is \c 10.

    The larger of two natural steps that a slider provides
    and typically corresponds to the user pressing PageUp or PageDown.

    \sa setPageStep()
*/
int HbSliderPopup::pageStep() const
{
    Q_D(const HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->pageStep();
}

/*!
    Sets the page \a step of the slider.

    \sa pageStep()
*/
void HbSliderPopup::setPageStep(int step)
{
    Q_D(HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setPageStep(step);
}

/*!
    Returns the single step of the slider.

    The default value is \c 1.

    The smaller of two natural steps that an
    abstract sliders provides and typically corresponds to the user
    pressing an arrow key.

    \sa setSingleStep()
*/
int HbSliderPopup::singleStep() const
{
    Q_D(const HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->singleStep();
}

/*!
    Sets the single \a step of the slider.

    \sa singleStep()
*/
void HbSliderPopup::setSingleStep(int step)
{
    Q_D(HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setSingleStep(step);
}

/*!
    Returns the current slider position.

    If tracking is enabled (the default), this is identical to the value.

    \sa setSliderPosition()
*/
int HbSliderPopup::sliderPosition() const
{
    Q_D(const HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->sliderPosition();
}

/*!
    Sets the current slider position.

    \sa sliderPosition()
*/
void HbSliderPopup::setSliderPosition(int pos)
{
    Q_D(HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setSliderPosition(pos);
}

/*!
    Returns \c true whether slider tracking is enabled.

    The default value is \c true.

    If tracking is enabled, the slider emits the
    valueChanged() signal while the slider is being dragged. If
    tracking is disabled, the slider emits the valueChanged() signal
    only when the user releases the slider.

    \sa setTracking()
*/
bool HbSliderPopup::hasTracking() const
{
    Q_D(const HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->hasTracking();
}

/*!
    Sets whether the slider tracking is enabled.

    \sa hasTracking()
*/
void HbSliderPopup::setTracking(bool enable)
{
    Q_D(HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setTracking(enable);
}

/*!
    Returns the current value of the slider.

    The default value is \c 0.
*/
int HbSliderPopup::value() const
{
    Q_D(const HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->value();
}

/*!
    Sets the current value of the slider.

    The slider forces the value to be within the legal range: \b
    minimum <= \c value <= \b maximum.

    Changing the value also changes the \b sliderPosition.

    \sa value()
*/
void HbSliderPopup::setValue(int value)
{
    Q_D(HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setValue(value);
}

/*!
    Returns the tickmark position of the slider.

    The default value is \c HbSlider::NoTicks.

    \sa setTickPosition()
*/
Hb::SliderTickPositions HbSliderPopup::tickPosition() const
{
    Q_D(const HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->tickPosition();
}

/*!
    Sets the tickmark position of the slider.

    \sa tickPosition()
 */
void HbSliderPopup::setTickPosition(Hb::SliderTickPositions  position)
{
    Q_D(HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setTickPosition(position);
}

/*!
    Returns the SnappingMode of the slider.

    The default value is \c HbSlider::MinorTickSnapping.

    \sa setSnappingMode()
*/
HbSlider::SnappingMode HbSliderPopup::snappingMode() const  
{
    Q_D(const HbSliderPopup);
    HbSlider::SnappingMode mode = d->mSliderPopupContentWidget->slider->snappingMode();
    return mode;
}

/*!
    Sets the snappingMode of the slider.

    \sa snappingMode()
*/
void HbSliderPopup::setSnappingMode(HbSlider::SnappingMode mode)   
{
    Q_D(HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setSnappingMode(mode);
}

/*!
    Sets the \a orientation of the sliderpopup.

    \sa orientation()
*/
void HbSliderPopup::setOrientation(Qt::Orientation orientation)
{
    Q_D(HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setOrientation(orientation);
}

/*!
    Returns the orientation of the sliderpopup.

    The default value is \c Qt::Vertical.

    \sa setOrientation()
 */
Qt::Orientation HbSliderPopup::orientation() const
{
    Q_D(const HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->orientation();
}

/*!
    Returns the interval between major tickmarks.

    The default value is \c 0.

    \sa setMajorTickInterval()
*/
int HbSliderPopup::majorTickInterval() const
{
    Q_D(const HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->majorTickInterval();
}

/*!
    Sets the interval between major tickmarks.

    Special values:
    \li a negative value means no major ticks will be drawn
    \li value \c 0 means that the interval is automatically calculated

    \sa majorTickInterval()
*/
void HbSliderPopup::setMajorTickInterval(int interval)
{
    Q_D(HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setMajorTickInterval(interval);
}

/*!
    Returns the interval between minor tickmarks.

    The default value is \c 0.

    \sa setMinorTickInterval()
*/
int HbSliderPopup::minorTickInterval() const
{
    Q_D(const HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->minorTickInterval();
}

/*!
    Sets the interval between minor tickmarks.

    Special values:
    \li a negative value means no minor ticks will be drawn
    \li value \c 0 means that the interval is automatically calculated

    \sa minorTickInterval()
*/
void HbSliderPopup::setMinorTickInterval(int interval)
{
    Q_D(HbSliderPopup);
    d->mSliderPopupContentWidget->slider->setMinorTickInterval(interval);
}

/*!
    \reimp
 */
void HbSliderPopup::updatePrimitives()
{
    Q_D(const HbSliderPopup);
    d->mSliderPopupContentWidget->slider->updatePrimitives();
    HbDialog::updatePrimitives();
}

/*!
    @proto
    Sets whether to display progress track or not

    \default value is true

    \sa isTrackFilled( )
*/
void HbSliderPopup::setTrackFilled(bool trackVisible )
{
    
    Q_D( const HbSliderPopup );
    return d->mSliderPopupContentWidget->slider->setTrackFilled( trackVisible );

}

/*!
    @proto
    returns whether progress track is visible or not

    \sa setTrackFilled( )
     
*/
bool HbSliderPopup::isTrackFilled() const
{
    Q_D( const HbSliderPopup );
    return d->mSliderPopupContentWidget->slider->isTrackFilled();

}


/*!

    \deprecated HbSliderPopup::primitive(HbStyle::Primitive)
        is deprecated.

    \reimp
 */
QGraphicsItem * HbSliderPopup::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbSliderPopup);
    return d->mSliderPopupContentWidget->slider->primitive( primitive );
}

#include "moc_hbsliderpopup.cpp"
#include "hbsliderpopup.moc"
