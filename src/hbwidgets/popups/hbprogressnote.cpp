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




#include <hbnamespace_p.h>
#include <hbiconitem.h>
#include <hbaction.h>
#include <hbprogressnote.h>
#include <hbprogressnote_p.h>
#include <hbstyleoptionprogressdialog.h>
#include <hbstyleoptionprogressbar.h>
#include <hbinstance.h>

#include <QGraphicsItem>
#include <QDebug>

class HbProgressNoteContentWidget:public HbWidget
{
    Q_OBJECT
	public:
	QGraphicsItem* mIconItem;
	HbProgressBar* mPb;
	HbLabel *mText;// This will be HbTextItem and creation will delegate to style when it fixes the layout issue...
	HbProgressNotePrivate *d;
    enum { Type = HbPrivate::ItemType_ProgressNoteContentWidget };
    int type() const { return Type; }
	HbProgressNoteContentWidget(HbProgressNotePrivate *priv,QGraphicsItem* parent =0 ):HbWidget(parent),
		mIconItem(0),mPb(0),mText(0),d(priv)
	{
       
	    if( !mPb ){
		    mPb = new HbProgressBar(HbProgressBar::SimpleProgressBar,this);
	    }
        
		// Unfortunately i cant create the textitem via Style...
		/*mTextItem = style()->createPrimitive(HbStyle::P_Note_text, this);
		*/

		mText     = new HbLabel(this); // This will be HbTextItem and creation will delegate to style 
		// when it fixes the layout issue...
		mIconItem = style()->createPrimitive(HbStyle::P_MessageBox_icon, this);

        mText->setTextWrapping(Hb::TextWrapAnywhere);
        HbStyle::setItemName(mText, "text");
        HbStyle::setItemName(mIconItem, "icon");
        HbStyle::setItemName(mPb, "pbar");
        HbStyle::setItemName(this, "this");

		setProperty("hasText",true);
        setProperty("hasIcon",true);
	    
	}
    void polish( HbStyleParameters& params ) {

		if( ((HbIconItem*)mIconItem)->isNull()){
			HbStyle::setItemName(mIconItem,QString());
			setProperty("hasIcon",false);
		 }
        HbWidget::polish(params);
        //hack for popups resize issue
        if(parentLayoutItem()->parentLayoutItem()) {
           HbWidget* parentWidget = (HbWidget*) parentLayoutItem()->parentLayoutItem();
           parentWidget->resize(parentWidget->preferredSize());
        }
    }
	
};

HbProgressNotePrivate::HbProgressNotePrivate() :
    mAction(0),mTimer(0),mContentWidget(0),mTextWrapping(true),mMinDuration(0),mDelayTime(0)
{
	
}

HbProgressNotePrivate::~HbProgressNotePrivate()
{
    
}


void HbProgressNotePrivate::init(HbProgressNote::ProgressNoteType type)
{
    Q_Q(HbProgressNote);
   	mContentWidget = new HbProgressNoteContentWidget(this);
    q->setContentWidget(mContentWidget);
	mNoteType = type ;
   	
	if(mNoteType == HbProgressNote::WaitNote){
		mContentWidget->mPb->setRange(0,0);
	}
	else{
		mContentWidget->mPb->setRange(0,100);
	}
	mAction = new HbAction(q->tr("Cancel"));
    QObject::connect(mAction, SIGNAL(triggered()), q, SLOT(_q_userCancel()));
    q->setPrimaryAction(mAction);
	mMinDuration = 1500;
	mDelayTime = 1000;

    mTimer = new QTimeLine(mDelayTime, q);
    mTimer->setFrameRange(0, 100);
   
	
    QObject::connect(mTimer, SIGNAL(finished()), q, SLOT(_q_finished()));
    q->setTimeout(HbPopup::NoTimeout);
    q->setDismissPolicy(HbPopup::NoDismiss);
    q->setModal(false);
    q->setBackgroundFaded(false);
    q->hide();
	

    
}

void HbProgressNotePrivate::_q_userCancel()
{
    Q_Q(HbProgressNote);
	flags &= ~HbProgressNotePrivate::Closetimer;
	mTimer->stop();
    q->cancel();
	
}

void HbProgressNotePrivate::_q_finished()
{
    Q_Q(HbProgressNote);

	if(flags.testFlag(HbProgressNotePrivate::Showtimer)){
            mTimer->stop();
                        q->HbDialog::show();
			mTimer->setDuration(mMinDuration);
            mTimer->setCurrentTime(0);
			mTimer->start();
			flags &= ~HbProgressNotePrivate::Showtimer;
			flags |= HbProgressNotePrivate::Closetimer;
	}
	else if(flags.testFlag(HbProgressNotePrivate::Closetimer)){
		flags &= ~HbProgressNotePrivate::Closetimer;
		if(flags.testFlag(HbProgressNotePrivate::Closepending)){
			q->close();
		}
	}
}

void HbProgressNotePrivate::_q_progressValueChanged(int value)
{
	 Q_Q(HbProgressNote);
	 if(value >= mContentWidget->mPb->maximum() && q->autoClose()){
		 if(flags.testFlag(HbProgressNotePrivate::Closetimer)){
			flags |= HbProgressNotePrivate::Closepending;
			flags &= ~HbProgressNotePrivate::Showtimer;
		 }
		 else if(!flags.testFlag(HbProgressNotePrivate::Showtimer)){
			 q->close();
		 }
		 else{
			 flags &= ~HbProgressNotePrivate::Showtimer;
		 }		 
	 }
}


/*!
    @deprecated
    @hbwidgets

    \this is deprecated class, use HbPrgoressDialog.

    \class HbProgressNote
    \brief HbProgressNote is a notification widget that combines test, icon, and a progress bar.
    
    \image html hbprogressnote.png A progress note.

    HbProgressNote provides several types of notifications. The supported  types are:
    
    \code
    enum ProgressNoteType { ProgressNote, WaitNote };
    \endcode

    \sa HbDialog

    Progress note has a number of use cases. Depending on the use case, either the value of the progress bar
    is updated  manually or waiting progress bar will be shown. The former case is used when displaying a download note.
    The latter case is used when connecting to network or opening a web page. The examples below demonstrate the use
    cases.

    The first example is a note  displayed when the application is connecting
    to network. As the delay is unknown the waiting progressbar will be shown indefinitely. 
    The note disappears if the user clicks Cancel button or
    after the connection is established in which case the application closes the progress note.

    Here is an example of using the infinite progress note:
    
    \code
    HbProgressNote *note = new HbProgressNote(HbProgressNote::WaitNote);
    note->show();
    \endcode

    The progress note is closed explicitly when the user clicks Cancel button or the application calls cancel().

    Another use case is an application downloading a file. 
    \code
    HbProgressNote *note = new HbProgressNote(HbProgressNote::ProgressNote);
    note->setMinimum(0);
    note->setMaximum(1000);
    note->show();
    for (int i=0;i<1000;i+=100) 
    {
      note->setProgressValue(i);
      note->setText(QString("Downloaded %1/1000 KB").arg(i));
    }

    \endcode
*/

/*!
    Constructor.

    \param type Must be one of the defined ProgressNoteType enumerations.
    \param parent An optional parameter.

    \deprecated HbProgressNote::HbProgressNote(HbProgressNote::ProgressNoteType, QGraphicsItem*)
        is deprecated.
*/

HbProgressNote::HbProgressNote(ProgressNoteType type, QGraphicsItem *parent) :
    HbDialog(*new HbProgressNotePrivate, parent)
{
    Q_D(HbProgressNote);
    d->init(type);
	setAutoClose(true);
}

/*!
    Constructor.

    \param parent An optional parameter.
    
    \deprecated HbProgressNote::HbProgressNote(QGraphicsItem*)
        is deprecated.

*/
HbProgressNote::HbProgressNote(QGraphicsItem *parent) :
    HbDialog(*new HbProgressNotePrivate, parent)
{
    Q_D(HbProgressNote);
    d->init(ProgressNote);
    setAutoClose(true);
}

/*!
    Returns the maximum value of the progressbar within the note.

    The default value is \c 100.

    \sa setMaximum()
*/
int HbProgressNote::maximum() const
{
    Q_D(const HbProgressNote);

    return d->mContentWidget->mPb->maximum();
}


/*!
    Sets the maximum value of the progressbar within the note.

    \sa maximum()
*/
void HbProgressNote::setMaximum(int max)
{
    Q_D(HbProgressNote);

    d->mContentWidget->mPb->setMaximum(max);
}

/*!
    Returns the minimum value of the progressbar within the note.

    The default value is \c 0.

    \sa setMinimum()
*/
int HbProgressNote::minimum() const
{
    Q_D(const HbProgressNote);
    return d->mContentWidget->mPb->minimum();
}

/*!
    Sets the minimum value of the progressbar within the note.

    \sa minimum()
*/
void HbProgressNote::setMinimum(int min)
{
    Q_D(HbProgressNote);

    d->mContentWidget->mPb->setMinimum(min);
}

/*!
    Sets the minimum and maximum value of the progressbar within the note.

    \sa minimum()
    \sa maximum()
*/
void HbProgressNote::setRange(int min,int max)
{
    Q_D(HbProgressNote);
    d->mContentWidget->mPb->setRange(min,max);
}

/*!
  Returns the value of the progressbar within the note.

  This value is constrained as follows:
  \b minimum <= \c value <= \b maximum.

  \sa setValue()

 */
int HbProgressNote::progressValue() const
{
    Q_D(const HbProgressNote);

    return d->mContentWidget->mPb->progressValue();
}

/*!
  Sets the value of the progressbar within the note.
  After the timeline has been started the value is updated automatically.
  Use this function only if the timer is not used.

  This value is constrained as follows:
  \b minimum <= \c value <= \b maximum.

 \sa value()

*/
void HbProgressNote::setProgressValue(int value)
{
    Q_D(HbProgressNote);

    d->mContentWidget->mPb->setProgressValue(value);
	d->_q_progressValueChanged(value);
}

/*!
  Closes the popup while emitting the cancelled() signal. This function is called when 
  user presses the Cancel button or then the timer expires.

 */
void HbProgressNote::cancel() 
{
   // Q_D(HbProgressNote);

    emit cancelled();
    close();
}

QGraphicsItem* HbProgressNote::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbProgressNote);
    switch (primitive) {
        case HbStyle::P_Popup_background:
            return HbDialog::primitive(primitive);
        case HbStyle::P_ProgressDialog_icon:
            return d->mContentWidget->mIconItem;
        default:
            return 0;
        }    
}

/*!
  Sets the progressnote type. 
  \sa progressNoteType()
 */
void HbProgressNote::setProgressNoteType(HbProgressNote::ProgressNoteType type)
{
    Q_D(HbProgressNote);
	if(d->mNoteType != type) {
		d->mNoteType = type;
		if(type == WaitNote){
			d->mContentWidget->mPb->setRange(0,0);
		}
		else {
			d->mContentWidget->mPb->setRange(0,100);
		}
	}
}

/*!
  returns progressnote type;
   \sa setProgressNoteType()
 */
HbProgressNote::ProgressNoteType HbProgressNote::progressNoteType() const
{
    Q_D(const HbProgressNote);
    return d->mNoteType;
}
  

void HbProgressNote::showEvent(QShowEvent *event)
{
    Q_D(HbProgressNote);
	d->mContentWidget->mPb->show();
        HbDialog::showEvent(event);
}

/*!
  Shows the Note after a delay(say 1sec). This is a convenient slot
  if user cancels note before delay expired note wont be shown at all 
 */
void HbProgressNote::delayedShow()
{
		Q_D(HbProgressNote);
		d->flags |= HbProgressNotePrivate::Showtimer;
		d->mTimer->setDuration(d->mDelayTime);
		d->mTimer->start();		
}

bool HbProgressNote::autoClose () const 
{
	Q_D(const HbProgressNote);	
	return d->flags.testFlag(HbProgressNotePrivate::Autoclose);
}

void HbProgressNote::setAutoClose ( bool b )
{
    Q_D(HbProgressNote);
	b?d->flags 
		|= HbProgressNotePrivate::Autoclose : d->flags &= ~HbProgressNotePrivate::Autoclose;
}

void HbProgressNote::initStyleOption(HbStyleOption *option) const
{
	Q_D(const HbProgressNote);
	HbStyleOptionProgressDialog* progressNoteOption 
		= qstyleoption_cast< HbStyleOptionProgressDialog *>(option);
	
        HbDialog::initStyleOption(progressNoteOption);
	progressNoteOption->progressBarSize = d->mContentWidget->mPb->minimumSize();
    progressNoteOption->icon = d->mIcon;
    progressNoteOption->iconAlignment = d->mIconAlignment;
}
void HbProgressNote::closeEvent ( QCloseEvent * event )
{
    Q_D(HbProgressNote);
	d->mAction->setToolTip("");
    if(d->flags.testFlag(HbProgressNotePrivate::Closetimer)){
        d->flags |= HbProgressNotePrivate::Closepending;
        event->setAccepted(false);
        return;
    }
    if(d->flags.testFlag(HbProgressNotePrivate::Showtimer)){
        d->mTimer->stop();
        d->flags &= ~HbProgressNotePrivate::Showtimer;
    }
    d->mContentWidget->mPb->close();
    HbDialog::closeEvent(event);
	
}

/*!
    Sets text of the note.
    \sa text()
*/
void HbProgressNote::setText(const QString &text)
{
    Q_D(HbProgressNote);
    if ( text != d->mContentWidget->mText->plainText() ) {
        d->mContentWidget->mText->setPlainText(text);
	}
}
/*!
    Returns text of the note.
    \sa setText()
*/
QString HbProgressNote::text() const
{
    Q_D(const HbProgressNote);
    return d->mContentWidget->mText->plainText();
}


/*!
    Sets icon for the note.
    \sa icon()
*/
void HbProgressNote::setIcon(const HbIcon &icon)
{
    Q_D(HbProgressNote);
    if (icon != d->mIcon){
        d->mIcon = icon;
        if (d->mContentWidget->mIconItem) {
            HbStyleOptionProgressDialog progressNoteOption;
            initStyleOption(&progressNoteOption);
            style()->updatePrimitive(d->mContentWidget->mIconItem, HbStyle::P_ProgressDialog_icon, &progressNoteOption);
        }
	}
}


/*!
    Returns icon of the note.
    \sa setIcon()
*/
HbIcon HbProgressNote::icon() const
{
    Q_D(const HbProgressNote);
    return d->mIcon;
}


/*!
    Sets the text alignment.
    \param align Qt defined alignment options can used.

    The default value is Qt::AlignLeft|Qt::AlignVCenter

    \sa mTextAlignment()
*/
void HbProgressNote::setTextAlignment( Qt::Alignment align )
{
    Q_D(HbProgressNote);

    if (align != d->mContentWidget->mText->alignment()) {
		d->mContentWidget->mText->setAlignment(align);
       
    }
}


/*!
    Returns the text alignment.

    \sa setTextAlignment()
*/
Qt::Alignment HbProgressNote::textAlignment() const
{
    Q_D(const HbProgressNote);
	return d->mContentWidget->mText->alignment();
}


/*!
    Sets the icon alignment.

    \param align Qt defined alignment options can used.

    The default value is Qt::AlignCenter.

    \sa mIconAlignment()
*/
void HbProgressNote::setIconAlignment( Qt::Alignment align )
{
    Q_D(HbProgressNote);
    if (align != d->mIconAlignment){
        d->mIconAlignment = align;
        if (d->mContentWidget->mIconItem) {
            HbStyleOptionProgressDialog progressNoteOption;
            initStyleOption(&progressNoteOption);
            style()->updatePrimitive(d->mContentWidget->mIconItem, HbStyle::P_ProgressDialog_icon, &progressNoteOption);
        }
    }
}


/*!
    Returns the icon alignment.

    \sa setIconAlignment()
*/
Qt::Alignment HbProgressNote::iconAlignment() const
{
    Q_D(const HbProgressNote);
    return d->mIconAlignment;
}


/*!
    Sets the text wrapping.
    \param wrap When set, the text is drawn with Qt::TextWordWrap enabled
     meaning that lines breaks are at appropriate point,
     e.g. at word boundaries.

    The default value is true;

    \sa textWrapping()
*/
void HbProgressNote::setTextWrapping(bool wrap)
{
    Q_D(HbProgressNote);
    if (wrap != d->mTextWrapping){
			d->mTextWrapping = wrap;
			if(wrap) {
				d->mContentWidget->mText->setTextWrapping(Hb::TextWordWrap);
			}
			else{
				d->mContentWidget->mText->setTextWrapping(Hb::TextNoWrap);
			}
    }
}

/*!
    Returns the text wrapping setting.

    \sa setTextWrapping()
*/
bool HbProgressNote::textWrapping() const
{
    Q_D(const HbProgressNote);
    return d->mTextWrapping;
}


#include "moc_hbprogressnote.cpp"
#include "hbprogressnote.moc"

