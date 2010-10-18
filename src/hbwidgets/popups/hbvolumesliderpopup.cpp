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

#include "hbvolumeslider_p.h"
#include <hbvolumesliderpopup.h>
#include <hbinstance.h>
#include "hbsliderpopup_p.h"
#include "hblayoutdirectionnotifier_p.h"
#include <hbmessagebox.h>
#include "hbdeviceprofile.h"

/*!
    @beta
    @hbwidgets
    \class HbVolumeSliderPopup
    \brief HbVolumeSliderPopup widget provides a vertical volume slider embedded in a popup.

    HbVolumeSliderPopup is provided for convenience. HbVolumeSlider sets appropriate
    icons and provides volume slider specific functionality like a
    checkable mute button and appropriate elements out of the box. HbVolumeSliderPopup
    consists of following slider elements by default:
    \li HbSlider::IncreaseElement
    \li HbSlider::TrackElement
    \li HbSlider::DecreaseElement
    \li HbSlider::IconElement

    \image html hbvolumeslider.png A volume slider.

    The elements can be changed by calling HbSlider::setElements() later at any time.

    \note order of element can not be changed

    Orientation of HbVolumeSliderPopup can not be changed. If orientation change is needed, 
    then first create HbSliderPopup and set needed elements.

    It is positioned at Right side of the screen in non mirrored layout.

    Example usage:
    \code
    HbVolumeSliderPopup *slider = new HbVolumeSliderPopup();
    connect(slider, SIGNAL(valueChanged(int)), receiver, SLOT(volumeSliderChanged(int)));
    connect(slider, SIGNAL(iconToggled(bool)), receiver, SLOT(volumeMuted(bool)));
    \endcode

    Note:: position and size of these elements cant be change.
    use HbSlider and set the elements instead if you want to change position or size

    Note:: if setElement is called on this slider , application is reponsible for
    inconsitent UI.
*/


/*
    internal
 */
class HbVolumeSliderPopupPrivate : public HbSliderPopupPrivate
{
    Q_DECLARE_PUBLIC(HbVolumeSliderPopup) 
protected:
    virtual void createSlider();
public:
    HbVolumeSliderPopupPrivate();
    virtual ~HbVolumeSliderPopupPrivate();
    void init();
};

/*
    internal
 */
void HbVolumeSliderPopupPrivate::init()
{
    Q_Q( HbVolumeSliderPopup);
    q->setTimeout(2000);
    HbVolumeSlider* volumeSlider = static_cast<HbVolumeSlider *>(slider);
    q->connect(volumeSlider,SIGNAL(iconToggled(bool)),q,SIGNAL(iconToggled(bool)));
     
}

/*
    internal
 */
HbVolumeSliderPopupPrivate::HbVolumeSliderPopupPrivate()
{
}

/*
    reimp
 */
void HbVolumeSliderPopupPrivate::createSlider() 
{
    slider = new HbVolumeSlider(true);
}

/*
    \internal
 */
HbVolumeSliderPopupPrivate::~HbVolumeSliderPopupPrivate()
{
}

/*!
    @beta
    Constructs a HbVolumeSliderPopup with \a parent.

    orientation will be vertical
 
*/
HbVolumeSliderPopup::HbVolumeSliderPopup( ) :
    HbSliderPopup(*new HbVolumeSliderPopupPrivate, 0)
{
    Q_D(HbVolumeSliderPopup);
    d->q_ptr = this;
    d->init();
}

/*!
    @beta
    \internal
 */
HbVolumeSliderPopup::HbVolumeSliderPopup(HbVolumeSliderPopupPrivate &dd, QGraphicsItem *parent) :
    HbSliderPopup(dd, parent)
{
    dd.init();
}

/*!
    Destructs the slider.
*/
HbVolumeSliderPopup::~HbVolumeSliderPopup()
{
}


/*!
    @beta
    Sets whether the text element is visible.

    \default is true

    \sa isCurrentValueVisible()
*/
void HbVolumeSliderPopup::setCurrentValueVisible(bool visible)
{
    Q_D(HbVolumeSliderPopup);
    if( d->slider ) {
        HbVolumeSlider* volumeSlider = static_cast<HbVolumeSlider *>(d->slider);
        volumeSlider->setCurrentValueVisible(visible);
    }
}

/*!
    @beta
    Returns \c true whether the text element is visible.

    The default value is \c false.

    \sa setCurrentValueVisible()
*/
bool HbVolumeSliderPopup::isCurrentValueVisible() const
{
    Q_D(const HbVolumeSliderPopup);
    if( d->slider ) {
        HbVolumeSlider* volumeslider = static_cast<HbVolumeSlider *>(d->slider);
        return volumeslider->isCurrentValueVisible() ;
    }
    return false;
}

/*!
    \reimp
    re position the slider popup
*/


/*!
    Reimplemented from QGraphicsItem::keyReleaseEvent().
 */
void HbVolumeSliderPopup::keyReleaseEvent(QKeyEvent *keyevent)
{
    Q_D( HbVolumeSliderPopup);
    switch (keyevent->key()) {
       //hide popup for following key press event
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Select:
         {
            HbMessageBox *note = new HbMessageBox( 
                tr("Volume Adjusted"), HbMessageBox::MessageTypeInformation);
         note->show();
        }
         hide();
    break;
    case Qt::Key_Left:
        if (d->keyNavigation()) {
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
    @proto

    Mutes or unmutes the volume output.
    \sa isMuted()
*/


void HbVolumeSliderPopup::setMuted( bool mute )
{
    Q_D( const HbVolumeSliderPopup );
    if( d->slider ) {
        HbVolumeSlider* volumeslider = static_cast<HbVolumeSlider *>( d->slider );
        volumeslider->setIconChecked( mute ) ;
    }
}

/*!
    @proto
    Returns whether volume is muted or unmuted.

    \sa setMute()
*/

bool HbVolumeSliderPopup::isMuted( ) const
{
    return( value( ) == minimum( ) ? true: false );
}


/*!
   \reimp
    Reimplemented from QGraphicsItem::keyPressEvent().
 */
void HbVolumeSliderPopup::keyPressEvent(QKeyEvent *keyevent)
{
    Q_D( HbVolumeSliderPopup);
    switch (keyevent->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
        if( d->keyNavigation() ) {
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


#include "moc_hbvolumesliderpopup.cpp"
