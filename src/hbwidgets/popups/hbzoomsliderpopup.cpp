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

#include "hbzoomslider_p.h"
#include <hbzoomsliderpopup.h>
#include <hbinstance.h>
#include "hbsliderpopup_p.h"
#include "hblayoutdirectionnotifier_p.h"
#include <hbmessagebox.h>
#include "hbdeviceprofile.h"

/*!
    @beta
    @hbwidgets
    \class HbZoomSliderPopup
    \brief HbZoomSliderPopup widget provides a vertical zoom slider embedded in a popup.

    HbZoomSliderPopup is provided for convenience. HbZoomSliderPopup sets appropriate
    icons, properties and provides zoom slider specific functionality like a
    zoom level indicator label and appropriate elements out of the box.
    HbZoomSlider sets HbSlider::text  and consists of following
    slider elements by default:
    \li HbSlider::IncreaseElement
    \li HbSlider::TrackElement
    \li HbSlider::DecreaseElement
    \li HbSlider::TextElement

    \image html hbzoomslider.png A vertical zoom slider.

    The elements can be changed by calling HbSlider::setElements() later at any time.

    Orientation of HbZoomSliderPopup can not be changed. If orientation change is needed, 
    then first create HbSlider and set needed elements.

    It is positioned at Right side of the screen in non mirrored layout.

    Example usage:
    \code
    HbZoomSliderPopup *slider = new HbZoomSliderPopup();
    connect(slider, SIGNAL(valueChanged(int)), receiver, SLOT(zoomSliderChanged(int)));
    \endcode

    
    Note:: position and size of these elements cant be change.
    use HbSlider and set the appropriate elements instead if you want to change position or size

    Note:: if setElement is called on this slider ,  application is reponsible for
    inconsitent UI.
*/

/*!
    \reimp
    \fn int HbZoomSlider::type() const
 */

/*
    internal
 */
class HbZoomSliderPopupPrivate : public HbSliderPopupPrivate
{
 Q_DECLARE_PUBLIC(HbZoomSliderPopup) 
protected:
    virtual void createSlider();
public:
    HbZoomSliderPopupPrivate();
    virtual ~HbZoomSliderPopupPrivate();
    void init();
  
};

/*
    internal
 */
void HbZoomSliderPopupPrivate::init()
{
    Q_Q(HbZoomSliderPopup);
    q->setTimeout(0);
}



/*
    internal
 */
HbZoomSliderPopupPrivate::HbZoomSliderPopupPrivate() 
{
 
}

/*
    reimp
 */
void HbZoomSliderPopupPrivate::createSlider() 
{
    slider = new HbZoomSlider();
}

/*
   internal
 */
HbZoomSliderPopupPrivate::~HbZoomSliderPopupPrivate()
{
}

  
/*!
    @beta
    Constructs a slider with \a parent.

    The orientation is \b Qt::Vertical and the slider contains only track element by default.

    \sa setOrientation(), setElements()
*/
HbZoomSliderPopup::HbZoomSliderPopup() :
    HbSliderPopup(*new HbZoomSliderPopupPrivate, NULL)
{
    Q_D(HbZoomSliderPopup);
    d->q_ptr = this;
    d->init();
}

/*!
    @beta
    \internal
 */
HbZoomSliderPopup::HbZoomSliderPopup(HbZoomSliderPopupPrivate &dd, QGraphicsItem *parent) :
    HbSliderPopup(dd, parent)
{
    dd.init();    
}

/*!
    Destructs the slider.
*/
HbZoomSliderPopup::~HbZoomSliderPopup()
{
}

/*!
    @beta
    Returns the default Zoom Level of the zoom slider if set by application
    otherwise returns minimum value. 

    \sa setDefaultZoomLevel()

*/
int HbZoomSliderPopup::defaultZoomLevel( ) const
{
    Q_D( const HbZoomSliderPopup);
    if( d->slider ) {
        HbZoomSlider *zoomSlider= static_cast<HbZoomSlider *> (d->slider);
        return(zoomSlider->defaultZoomLevel());
    }
    return 0;
}

/*!
    @beta
    sets the default Zoom Level of the zoom slider with \a value.
    if not set minimum() value will be used for default value.

    \sa defaultZoomLevel()
*/
void HbZoomSliderPopup::setDefaultZoomLevel(int value)
{
    Q_D(HbZoomSliderPopup);
    if( d->slider ) {
        HbZoomSlider *zoomSlider= static_cast<HbZoomSlider *> (d->slider);
        zoomSlider->setDefaultZoomLevel(value);
    }
}



/*!
   \reimp
    Reimplemented from QGraphicsItem::keyReleaseEvent().
 */
void HbZoomSliderPopup::keyReleaseEvent(QKeyEvent *keyevent)
{
    Q_D( HbZoomSliderPopup);
    switch (keyevent->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Select:
         {
         HbMessageBox *note = new HbMessageBox( 
                tr("Zoom Adjusted"), HbMessageBox::MessageTypeInformation);
         note->show();
        }
         hide();
    break;
    case Qt::Key_Left:
        if ( d->keyNavigation() ) {
            hide();
            keyevent->accept();
            break;
        }
    case Qt::Key_Backspace:
    case Qt::Key_Back:
         hide();
         keyevent->accept();
         break;
    default:
        HbSliderPopup::keyReleaseEvent(keyevent);
         break;
    }
}

/*!
    \reimp
    Reimplemented from QGraphicsItem::keyPressEvent().
 */
void HbZoomSliderPopup::keyPressEvent(QKeyEvent *keyevent)
{
    Q_D( HbZoomSliderPopup );
    switch (keyevent->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
        if ( d->keyNavigation() )
        {
            hide();
            keyevent->accept();
            break;
        }
    case Qt::Key_Back:
    case Qt::Key_Backspace:
         hide();
         keyevent->accept();
         break;
    default:
        HbSliderPopup::keyPressEvent(keyevent);
         break;
    }
}


#include "moc_hbzoomsliderpopup.cpp"
