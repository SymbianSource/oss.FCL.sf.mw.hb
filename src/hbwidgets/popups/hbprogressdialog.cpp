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
#include <hbprogressdialog.h>
#include <hbprogressdialog_p.h>
#include <hbstyleoptionprogressdialog.h>
#include <hbstyleoptionprogressbar.h>
#include <hbinstance.h>
#include <hbtextitem.h>
#include <QGraphicsItem>
#include <QDebug>

class HbProgressDialogContentWidget : public HbWidget
{
    Q_OBJECT
	public:
	QGraphicsItem* mIconItem;
	HbProgressBar* mPb;
	QGraphicsItem *mText;// This will be HbTextItem and creation will delegate to style when it fixes the layout issue...
	HbProgressDialogPrivate *d;
    enum { Type = HbPrivate::ItemType_ProgressDialogContentWidget };
    int type() const { return Type; }
	HbProgressDialogContentWidget(HbProgressDialogPrivate *priv,QGraphicsItem* parent =0 ):HbWidget(parent),
		mIconItem(0),mPb(0),mText(0),d(priv)
	{
       
	    if( !mPb ){
		    mPb = new HbProgressBar(HbProgressBar::SimpleProgressBar,this);
	    }
        
		mText = style()->createPrimitive(HbStyle::P_ProgressDialog_text,this);
        mIconItem = style()->createPrimitive(HbStyle::P_ProgressDialog_icon, this);
		mIconItem->hide();
        HbStyle::setItemName(mText, "text");
        HbStyle::setItemName(mIconItem, "icon");
        HbStyle::setItemName(mPb, "pbar");
    
		setProperty("hasText",true);
        setProperty("hasIcon",false);
	    
	}

	void repolishContent()
	{
		repolish();
	}
};



HbProgressDialogPrivate::HbProgressDialogPrivate() :
mAction(0),mTimer(0),mContentWidget(0),mTextWrapping(true),mMinDuration(0),mDelayTime(0),mTextString(QString()),mAlign(Qt::AlignTop|Qt::AlignLeft)
{
	
}

HbProgressDialogPrivate::~HbProgressDialogPrivate()
{
    
}


void HbProgressDialogPrivate::init(HbProgressDialog::ProgressDialogType type)
{
    Q_Q(HbProgressDialog);
   	mContentWidget = new HbProgressDialogContentWidget(this);
    q->setContentWidget(mContentWidget);
	mNoteType = type ;
   	
	if(mNoteType == HbProgressDialog::WaitDialog){
		mContentWidget->mPb->setRange(0,0);
	}
	else{
		mContentWidget->mPb->setRange(0,100);
	}
	mAction = new HbAction(q->tr("Cancel"),q);
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

void HbProgressDialogPrivate::_q_userCancel()
{
    Q_Q(HbProgressDialog);
	flags &= ~HbProgressDialogPrivate::Closetimer;
	mTimer->stop();
    q->cancel();
	
}

void HbProgressDialogPrivate::_q_finished()
{
    Q_Q(HbProgressDialog);

	if(flags.testFlag(HbProgressDialogPrivate::Showtimer)){
            mTimer->stop();
                        q->HbDialog::show();
			mTimer->setDuration(mMinDuration);
            mTimer->setCurrentTime(0);
			mTimer->start();
			flags &= ~HbProgressDialogPrivate::Showtimer;
			flags |= HbProgressDialogPrivate::Closetimer;
	}
	else if(flags.testFlag(HbProgressDialogPrivate::Closetimer)){
		flags &= ~HbProgressDialogPrivate::Closetimer;
		if(flags.testFlag(HbProgressDialogPrivate::Closepending)){
			q->close();
		}
	}
}

void HbProgressDialogPrivate::_q_progressValueChanged(int value)
{
	 Q_Q(HbProgressDialog);
	 if(value >= mContentWidget->mPb->maximum() && q->autoClose()){
		 if(flags.testFlag(HbProgressDialogPrivate::Closetimer)){
			flags |= HbProgressDialogPrivate::Closepending;
			flags &= ~HbProgressDialogPrivate::Showtimer;
		 }
		 else if(!flags.testFlag(HbProgressDialogPrivate::Showtimer)){
			 q->close();
		 }
		 else{
			 flags &= ~HbProgressDialogPrivate::Showtimer;
		 }		 
	 }
}


/*!
    @beta
    @hbwidgets
    \class HbProgressDialog
    \brief HbProgressDialog is a notification widget that combines test, icon, and a progress bar.
    
    \image html hbprogressdialog.png A progress note.

    HbProgressDialog provides several types of notifications. The supported  types are:
    
    \code
    enum ProgressDialogType { ProgressDialog, WaitDialog };
    \endcode

    \sa HbDialog

    Progress dialog has a number of use cases. Depending on the use case, either the value of the progress bar
    is updated  manually or waiting progress bar will be shown. The former case is used when displaying a download note.
    The latter case is used when connecting to network or opening a web page. The examples below demonstrate the use
    cases.

    The first example is a dialog  displayed when the application is connecting
    to network. As the delay is unknown the waiting progressbar will be shown indefinitely. 
    The note disappears if the user clicks Cancel button or
    after the connection is established in which case the application closes the progress note.

    Here is an example of using the infinite progress note:
    
    \code
    HbProgressDialog *note = new HbProgressDialog(HbProgressDialog::WaitDialog);
    note->show();
    \endcode

    The progress note is closed explicitly when the user clicks Cancel button or the application calls cancel().

    Another use case is an application downloading a file. 
    \code
    HbProgressDialog *note = new HbProgressDialog(HbProgressDialog::ProgressDialog);
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
    @beta
    Constructor.

    \param type Must be one of the defined ProgressDialogType enumerations.
    \param parent An optional parameter.

*/

HbProgressDialog::HbProgressDialog(ProgressDialogType type, QGraphicsItem *parent) :
    HbDialog(*new HbProgressDialogPrivate, parent)
{
    Q_D(HbProgressDialog);
    d->init(type);
	setAutoClose(true);
}

/*!
    @beta
    Constructor.

    \param parent An optional parameter.

*/
HbProgressDialog::HbProgressDialog(QGraphicsItem *parent) :
    HbDialog(*new HbProgressDialogPrivate, parent)
{
    Q_D(HbProgressDialog);
    d->init(ProgressDialog);
    setAutoClose(true);
}

/*!
    @beta
    Returns the maximum value of the progressbar within the note.

    The default value is \c 100.

    \sa setMaximum()
*/
int HbProgressDialog::maximum() const
{
    Q_D(const HbProgressDialog);

    return d->mContentWidget->mPb->maximum();
}


/*!
    @beta
    Sets the maximum value of the progressbar within the note.

    \sa maximum()
*/
void HbProgressDialog::setMaximum(int max)
{
    Q_D(HbProgressDialog);

    d->mContentWidget->mPb->setMaximum(max);
}

/*!
    @beta
    Returns the minimum value of the progressbar within the note.

    The default value is \c 0.

    \sa setMinimum()
*/
int HbProgressDialog::minimum() const
{
    Q_D(const HbProgressDialog);
    return d->mContentWidget->mPb->minimum();
}

/*!
    @beta
    Sets the minimum value of the progressbar within the note.

    \sa minimum()
*/
void HbProgressDialog::setMinimum(int min)
{
    Q_D(HbProgressDialog);

    d->mContentWidget->mPb->setMinimum(min);
}

/*!
    @beta
    Sets the minimum and maximum value of the progressbar within the note.

    \sa minimum()
    \sa maximum()
*/
void HbProgressDialog::setRange(int min,int max)
{
    Q_D(HbProgressDialog);
    d->mContentWidget->mPb->setRange(min,max);
}

/*!
    @beta
  Returns the value of the progressbar within the note.

  This value is constrained as follows:
  \b minimum <= \c value <= \b maximum.

  \sa setValue()

 */
int HbProgressDialog::progressValue() const
{
    Q_D(const HbProgressDialog);

    return d->mContentWidget->mPb->progressValue();
}

/*!
    @beta
  Sets the value of the progressbar within the note.
  After the timeline has been started the value is updated automatically.
  Use this function only if the timer is not used.

  This value is constrained as follows:
  \b minimum <= \c value <= \b maximum.

 \sa value()

*/
void HbProgressDialog::setProgressValue(int value)
{
    Q_D(HbProgressDialog);

    d->mContentWidget->mPb->setProgressValue(value);
	d->_q_progressValueChanged(value);
}

/*!
    @beta
  Closes the popup while emitting the cancelled() signal. This function is called when 
  user presses the Cancel button or then the timer expires.

 */
void HbProgressDialog::cancel() 
{
    emit cancelled();
    close();
}

QGraphicsItem* HbProgressDialog::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbProgressDialog);
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
    @beta
  Sets the progressdialog type. 
  \sa progressDialogType()
 */
void HbProgressDialog::setProgressDialogType(HbProgressDialog::ProgressDialogType type)
{
    Q_D(HbProgressDialog);
	if(d->mNoteType != type) {
		d->mNoteType = type;
		if(type == WaitDialog){
			d->mContentWidget->mPb->setRange(0,0);
		}
		else {
			d->mContentWidget->mPb->setRange(0,100);
		}
	}
}

/*!
    @beta
  returns progressDialog type;
   \sa setProgressDialogType()
 */
HbProgressDialog::ProgressDialogType HbProgressDialog::progressDialogType() const
{
    Q_D(const HbProgressDialog);
    return d->mNoteType;
}
  

void HbProgressDialog::showEvent(QShowEvent *event)
{
    Q_D(HbProgressDialog);
	d->mContentWidget->mPb->show();
        HbDialog::showEvent(event);
}

/*!
    @beta
  Shows the Note after a delay(say 1sec). This is a convenient slot
  if user cancels note before delay expired note wont be shown at all 
 */
void HbProgressDialog::delayedShow()
{
		Q_D(HbProgressDialog);
		d->flags |= HbProgressDialogPrivate::Showtimer;
		d->mTimer->setDuration(d->mDelayTime);
		d->mTimer->start();		
}

/*!
    @beta
  Returns the auto close flag.
 */
bool HbProgressDialog::autoClose () const 
{
	Q_D(const HbProgressDialog);	
	return d->flags.testFlag(HbProgressDialogPrivate::Autoclose);
}

/*!
    @beta
  Sets the auto close flag.
 */
void HbProgressDialog::setAutoClose ( bool b )
{
    Q_D(HbProgressDialog);
	b?d->flags 
		|= HbProgressDialogPrivate::Autoclose : d->flags &= ~HbProgressDialogPrivate::Autoclose;
}

void HbProgressDialog::initStyleOption(HbStyleOption *option) const
{
	Q_D(const HbProgressDialog);
	HbStyleOptionProgressDialog* progressDialogOption 
		= qstyleoption_cast< HbStyleOptionProgressDialog *>(option);
	
    HbDialog::initStyleOption(progressDialogOption);
	progressDialogOption->progressBarSize = d->mContentWidget->mPb->minimumSize();
    progressDialogOption->icon = d->mIcon;
    progressDialogOption->iconAlignment = d->mIconAlignment;
	progressDialogOption->text = d->mTextString;
	progressDialogOption->wrap = d->mTextWrapping;
	progressDialogOption->textAlignment = d->mAlign;

}
void HbProgressDialog::closeEvent ( QCloseEvent * event )
{
    Q_D(HbProgressDialog);
	d->mAction->setToolTip("");
    if(d->flags.testFlag(HbProgressDialogPrivate::Closetimer)){
        d->flags |= HbProgressDialogPrivate::Closepending;
        event->setAccepted(false);
        return;
    }
    if(d->flags.testFlag(HbProgressDialogPrivate::Showtimer)){
        d->mTimer->stop();
        d->flags &= ~HbProgressDialogPrivate::Showtimer;
    }
    d->mContentWidget->mPb->close();
    HbDialog::closeEvent(event);
	
}

/*!
    @beta
    Sets text of the note.
    \sa text()
*/
void HbProgressDialog::setText(const QString &text)
{
    Q_D(HbProgressDialog);
	if ( text != d->mTextString) {
		d->mTextString = text;
		HbStyleOptionProgressDialog progressDialogOption;
        initStyleOption(&progressDialogOption);
		style()->updatePrimitive(d->mContentWidget->mText, HbStyle::P_ProgressDialog_text, &progressDialogOption);
	}
}
/*!
    @beta
    Returns text of the note.
    \sa setText()
*/
QString HbProgressDialog::text() const
{
    Q_D(const HbProgressDialog);
	return d->mTextString;
}


/*!
    @beta
    Sets icon for the note.
    \sa icon()
*/
void HbProgressDialog::setIcon(const HbIcon &icon)
{
    Q_D(HbProgressDialog);
    if (icon != d->mIcon){
        d->mIcon = icon;
        if (d->mContentWidget->mIconItem) {
			d->mContentWidget->mIconItem->show();
			d->mContentWidget->setProperty("hasIcon",true);
            HbStyleOptionProgressDialog progressDialogOption;
            initStyleOption(&progressDialogOption);
            style()->updatePrimitive(d->mContentWidget->mIconItem, HbStyle::P_ProgressDialog_icon, &progressDialogOption);
			d->mContentWidget->repolishContent();
        }

		
	}
}


/*!
    @beta
    Returns icon of the note.
    \sa setIcon()
*/
HbIcon HbProgressDialog::icon() const
{
    Q_D(const HbProgressDialog);
    return d->mIcon;
}


/*!
    @deprecated
    Sets the text alignment.
    \param align Qt defined alignment options can used.

    The default value is Qt::AlignLeft|Qt::AlignVCenter

    \sa mTextAlignment()
*/
void HbProgressDialog::setTextAlignment( Qt::Alignment align )
{
	
    Q_D(HbProgressDialog);
	if (align != d->mAlign ) {
		d->mAlign = align;
	    HbStyleOptionProgressDialog progressDialogOption;
        initStyleOption(&progressDialogOption);
		style()->updatePrimitive(d->mContentWidget->mText, HbStyle::P_ProgressDialog_text, &progressDialogOption);
       
   }
}

/*!
    @deprecated
    Returns the text alignment.

    \sa setTextAlignment()
*/
Qt::Alignment HbProgressDialog::textAlignment() const
{
	Q_D(const HbProgressDialog);
	return d->mAlign;
}


/*!
    @deprecated
    Sets the icon alignment.

    \param align Qt defined alignment options can used.

    The default value is Qt::AlignCenter.

    \sa mIconAlignment()
*/
void HbProgressDialog::setIconAlignment( Qt::Alignment align )
{
    Q_D(HbProgressDialog);
    if (align != d->mIconAlignment){
        d->mIconAlignment = align;
        if (d->mContentWidget->mIconItem) {
            HbStyleOptionProgressDialog progressDialogOption;
            initStyleOption(&progressDialogOption);
            style()->updatePrimitive(d->mContentWidget->mIconItem, HbStyle::P_ProgressDialog_icon, &progressDialogOption);
        }
    }
}


/*!
    @deprecated
    Returns the icon alignment.

    \sa setIconAlignment()
*/
Qt::Alignment HbProgressDialog::iconAlignment() const
{
    Q_D(const HbProgressDialog);
    return d->mIconAlignment;
}

#include "moc_hbprogressdialog.cpp"
#include "hbprogressdialog.moc"
