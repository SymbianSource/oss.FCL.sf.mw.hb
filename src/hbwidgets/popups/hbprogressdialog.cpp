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

#include <hbprogressdialog.h>
#include <hbprogressdialog_p.h>
#include <hbstyleoptionprogressdialog_p.h>

#include <hbnamespace_p.h>
#include <hbaction.h>
#include <hbpopup.h>
#include <hbtextitem.h>
#include <QGraphicsItem>

/*
    \internal
    HbProgressDialogContentWidget class

    HbProgressDialogContentWidget is internal to HbProgressDialog and is the content widget to progressDialog

    HbProgressDialogContentWidget holds the text, icon and progressbar.

*/
class HbProgressDialogContentWidget : public HbWidget
{
    Q_OBJECT
    
    public:
    
    QGraphicsItem *mIconItem;
    HbProgressBar *mProgressBar;
    QGraphicsItem *mTextItem;
    HbProgressDialogPrivate *d;
    
    enum { Type = HbPrivate::ItemType_ProgressDialogContentWidget };
    int type() const { return Type; }
    
    HbProgressDialogContentWidget(HbProgressDialogPrivate *priv,QGraphicsItem* parent =0 ):HbWidget(parent),
        mIconItem(0),mProgressBar(0),mTextItem(0),d(priv)
    {
        setProperty("text",QVariant(true));
        setProperty("icon",QVariant(false));
    }
  
    void repolishContent()
    {
        repolish();
    }
};

/*!
    \internal

    HbProgressDialogPrivate class constructor
*/
HbProgressDialogPrivate::HbProgressDialogPrivate() :mTimer(0),mIcon(0),mContentWidget(0),
                                                    mMinDuration(0),mDelayTime(0),mTextString(QString()),
                                                    mAlign(Qt::AlignTop|Qt::AlignLeft)
{
}

/*!
    \internal

    HbProgressDialogPrivate class destructor
*/
HbProgressDialogPrivate::~HbProgressDialogPrivate()
{  
}

/*!
    \internal

    Initializes HbProgressDialogPrivate class 
*/
void HbProgressDialogPrivate::init(HbProgressDialog::ProgressDialogType type)
{
    Q_Q(HbProgressDialog);
    
    mContentWidget = new HbProgressDialogContentWidget(this);
    q->setContentWidget(mContentWidget);
    mNoteType = type ;
        
    createPrimitives();

    if(mNoteType == HbProgressDialog::WaitDialog){
        mContentWidget->mProgressBar->setRange(0,0);
    }
    else{
        mContentWidget->mProgressBar->setRange(0,100);
    }
    
    HbAction *action = new HbAction(q->tr("Cancel"), q);
    QObject::connect(action, SIGNAL(triggered()), q, SLOT(_q_userCancel()));
    q->addAction(action);
    
    mMinDuration = 1500;
    mDelayTime = 1000;

    mTimer = new QTimeLine(mDelayTime, q);
    mTimer->setFrameRange(0, 100);  
    QObject::connect(mTimer, SIGNAL(finished()), q, SLOT(_q_finished()));

    q->setTimeout(HbPopup::NoTimeout);
    q->setDismissPolicy(HbPopup::NoDismiss);
    q->hide();
}

/*
    \internal

    createPrimitives.
 */
void HbProgressDialogPrivate::createPrimitives()
{
    Q_Q(HbProgressDialog);

    if( !mContentWidget->mProgressBar ){
            mContentWidget->mProgressBar = new HbProgressBar(mContentWidget);
            HbStyle::setItemName(mContentWidget->mProgressBar, "pbar");
    }

    if ( !mContentWidget->mTextItem ) {
        mContentWidget->mTextItem = q->style()->createPrimitive(HbStyle::P_ProgressDialog_text,mContentWidget);
        HbStyle::setItemName(mContentWidget->mTextItem, "text");
    }

    if ( !mIcon.isNull() ) {
        if ( !mContentWidget->mIconItem ) {
            mContentWidget->mIconItem = q->style()->createPrimitive(HbStyle::P_ProgressDialog_icon, mContentWidget); 
            HbStyle::setItemName(mContentWidget->mIconItem, "icon");
        }
    } else {
        if( mContentWidget->mIconItem ){
            delete mContentWidget->mIconItem;
        }
        mContentWidget->mIconItem = 0;
    }
}

/*!
    \internal

    Private slot for, when user clicks on cancel button. This stops the timer and 
    dismisses the dialog.
*/
void HbProgressDialogPrivate::_q_userCancel()
{
    Q_Q(HbProgressDialog);
    
    flags &= ~HbProgressDialogPrivate::Closetimer;
    mTimer->stop();
    q->cancel();
}

/*!
    \internal

    Private slot which stops the timer and 
    dismisses the dialog
*/
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

/*!
    \internal

    Private slot, for when progress value changes. When maximum value, stop the timer and close the dialog.
*/
void HbProgressDialogPrivate::_q_progressValueChanged(int value)
{
    Q_Q(HbProgressDialog);
    
    if(value >= mContentWidget->mProgressBar->maximum() && q->autoClose()){
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
    \brief HbProgressDialog provides feedback on the progress of a slow operation.

    \image html hbprogressdialog.png  A progress dialog.

    ProgressDialog widget displays that a process is active and also the completion level of the process to the user.

    A progress dialog is used to give the user an indication of how long an operation is going to take, and 
    to demonstrate that the application has not frozen. It can also give the user an opportunity to abort the operation.

    Progress dialog provides several types of notifications. The supported  types are:
    
    \code
    enum ProgressDialogType { ProgressDialog, WaitDialog };
    \endcode

    Progress dialog has the following mandatory elements:
    \li Progress bar (Determinate type) 
    \li Description
    \li Cancel button (for canceling the process) 

    Optionally, progress dialog may also includes:
    \li  Icon
    \li  Heading
    \li  Close button (same as ‘Cancel’ button during active process, closes the dialog after successful process)

    Progress dialog is modal and requires user intervention to dismiss it. 
    \sa HbDialog

    The first example is a dialog  displayed when the application is connecting
    to network. As the delay is unknown the waiting progressbar will be shown indefinitely. 
    The dialog disappears if the user clicks Cancel button or
    after the connection is established in which case the application closes the progress dialog.

    Here is an example of using the infinite progress dialog(wait dialog):
    
    \code
    HbProgressDialog *pDialog = new HbProgressDialog(HbProgressDialog::WaitDialog);
    pDialog->show();
    \endcode

    The progress note is closed explicitly when the user clicks Cancel button or the application 
    calls cancel().

    Another use case is an application downloading a file. 
    \code
    HbProgressDialog *pDialog = new HbProgressDialog(HbProgressDialog::ProgressDialog);
    pDialog->setMinimum(0);
    pDialog->setMaximum(1000);
    pDialog->show();
    for (int i=0;i<1000;i+=100) 
    {
      pDialog->setProgressValue(i);
      pDialog->setText(QString("Downloaded %1/1000 KB").arg(i));
    }

    \endcode
*/

/*!
    \fn void HbProgressDialog::cancelled()

    This signal is emitted when the user clicks the cancel action.
 */

/*!
    \enum HbProgressDialog::ProgressDialogType

    This enum defines available progress dialog type values.

 */

/*!
    \var HbProgressDialog::ProgressDialog

    This enum describes the type value as ProgressDialog.
 */

/*!
    \var HbProgressDialog::WaitDialog

    TThis enum describes the type value as WaitDialog.
 */


/*!
    @beta
    Constructs a new HbProgressDialog with \a type and \a parent.

    \param type ProgressDialogType.
    \param parent  progress dialog's parent.
    
    Default settings:
    \li The text is empty.
    \li The icon is null.
    \li The cancel button text is (translated) "Cancel".
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
    Constructs a new HbProgressDialog with \a parent.
    
    Default settings:
    \li The text is empty.
    \li The icon is null.
    \li The cancel button text is (translated) "Cancel".
    \li minimum is 0;
    \li maximum is 100
    
    The parent argument is dialog's parent widget.
*/
HbProgressDialog::HbProgressDialog(QGraphicsItem *parent) :
    HbDialog(*new HbProgressDialogPrivate, parent)
{
    Q_D(HbProgressDialog);

    d->init(ProgressDialog);
    setAutoClose(true);
}

/*!
    Destructs the progressDialog.
*/
HbProgressDialog::~HbProgressDialog()
{
}

/*!
    @beta
    Returns the maximum value of the progressbar within the progressdialog.

    The default value is \c 100.

    \sa setMaximum()
*/
int HbProgressDialog::maximum() const
{
    Q_D(const HbProgressDialog);

    return d->mContentWidget->mProgressBar->maximum();
}

/*!
    @beta
    Sets the maximum value of the progressbar within the progressdialog.

    \sa maximum()
*/
void HbProgressDialog::setMaximum(int max)
{
    Q_D(HbProgressDialog);

    d->mContentWidget->mProgressBar->setMaximum(max);
}

/*!
    @beta
    Returns the minimum value of the progressbar within the progressdialog.

    The default value is \c 0.

    \sa setMinimum()
*/
int HbProgressDialog::minimum() const
{
    Q_D(const HbProgressDialog);

    return d->mContentWidget->mProgressBar->minimum();
}

/*!
    @beta
    Sets the minimum value of the progressbar within the progressdialog.

    \param min minimum value of the progressbar.

    \sa minimum()
*/
void HbProgressDialog::setMinimum(int min)
{
    Q_D(HbProgressDialog);

    d->mContentWidget->mProgressBar->setMinimum(min);
}

/*!
    @beta
    Sets the progress dialog's minimum and maximum values to minimum and maximum, respectively.

    If maximum is smaller than minimum, minimum becomes the only legal value.

    \param min minimum value of the progressbar within the progressdialog
    \param max maximum value of the progressbar within the progressdialog

    \sa minimum()
    \sa maximum()
*/
void HbProgressDialog::setRange(int min,int max)
{
    Q_D(HbProgressDialog);
    
    d->mContentWidget->mProgressBar->setRange(min,max);
}

/*!
    @beta
    Returns the value of the progressbar within the dialog.

    This value is constrained as follows:
    \b minimum <= \c value <= \b maximum.

    \sa setValue()
 */
int HbProgressDialog::progressValue() const
{
    Q_D(const HbProgressDialog);

    return d->mContentWidget->mProgressBar->progressValue();
}

/*!
    @beta
    Sets the value for the progressbar within the progress dialog.
    After the timeline has been started the value is updated automatically.
    Use this function only if the timer is not used.

    This value is constrained as follows:
    \b minimum <= \c value <= \b maximum.

    \param value progressbar value

    \sa value()
*/
void HbProgressDialog::setProgressValue(int value)
{
    Q_D(HbProgressDialog);

    d->mContentWidget->mProgressBar->setProgressValue(value);
    d->_q_progressValueChanged(value);
}

/*!
    @beta
    Closes the dialog while emitting the cancelled() signal. This function is called when 
    user presses the Cancel button or when the timer expires.
 */
void HbProgressDialog::cancel() 
{
    emit cancelled();
    close();
}

/*!

    \deprecated HbProgressDialog::primitive(HbStyle::Primitive)
        is deprecated.

    Returns the pointer for \a primitive passed.
    Will return NULL if \a primitive passed is invalid
*/
QGraphicsItem* HbProgressDialog::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbProgressDialog);

    switch (primitive) {
        case HbStyle::P_ProgressDialog_icon:
            return d->mContentWidget->mIconItem;
        case HbStyle::P_ProgressDialog_text:
            return d->mContentWidget->mTextItem;
        default:
            return 0;
        }
}

/*!
    @beta
    Sets the progressdialog type. 

    The type of the progress dialog can be specified with one of the values:
    \li HbProgressDialog::ProgressDialog
    \li HbProgressDialog::WaitDialog

    \param type type for the progress dialog.

    \sa progressDialogType()
 */
void HbProgressDialog::setProgressDialogType(HbProgressDialog::ProgressDialogType type)
{
    Q_D(HbProgressDialog);
    
    if(d->mNoteType != type) {
        d->mNoteType = type;
        if(type == WaitDialog){
            d->mContentWidget->mProgressBar->setRange(0,0);
        }
        else {
            d->mContentWidget->mProgressBar->setRange(0,100);
        }
    }
}

/*!
    @beta
    Returns progressDialog type.
    \sa setProgressDialogType()
 */
HbProgressDialog::ProgressDialogType HbProgressDialog::progressDialogType() const
{
    Q_D(const HbProgressDialog);
    
    return d->mNoteType;
}

/*!
    @beta
    Shows the progress dialog after a delay, say 1sec. This is a convenient slot for the user.
    if user cancels progress dialog before delay getting expired, then progress dialog wont be shown at all.
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
    \return autoclose flag.
    \sa setAutoClose()
 */
bool HbProgressDialog::autoClose () const 
{
    Q_D(const HbProgressDialog);
    
    return d->flags.testFlag(HbProgressDialogPrivate::Autoclose);
}

/*!
    @beta
    Sets the auto close flag.
    \param close based on which autoclose flag is set or reset.
    \sa autoClose()
 */
void HbProgressDialog::setAutoClose ( bool close )
{
    Q_D(HbProgressDialog);
    
    close?d->flags 
        |= HbProgressDialogPrivate::Autoclose : d->flags &= ~HbProgressDialogPrivate::Autoclose;
}

/*!
    \reimp
 */
void HbProgressDialog::initStyleOption(HbStyleOptionProgressDialog *option) const
{
    Q_D(const HbProgressDialog);

    HbDialog::initStyleOption(option);

    option->progressBarSize = d->mContentWidget->mProgressBar->minimumSize();
    option->icon = d->mIcon;
    option->iconAlignment = d->mIconAlignment;
    option->text = d->mTextString;
    option->textAlignment = d->mAlign;
}

/*!
    \reimp
 */
void HbProgressDialog::showEvent(QShowEvent *event)
{
    Q_D(HbProgressDialog);
    
    d->mContentWidget->mProgressBar->show();
    HbDialog::showEvent(event);   
}

/*!
    \reimp
 */
void HbProgressDialog::closeEvent ( QCloseEvent * event )
{
    Q_D(HbProgressDialog);    

    if(d->flags.testFlag(HbProgressDialogPrivate::Closetimer)){
        d->flags |= HbProgressDialogPrivate::Closepending;
        event->setAccepted(false);
        return;
    }
    if(d->flags.testFlag(HbProgressDialogPrivate::Showtimer)){
        d->mTimer->stop();
        d->flags &= ~HbProgressDialogPrivate::Showtimer;
    }
    d->mContentWidget->mProgressBar->close();
    HbDialog::closeEvent(event);    
}

/*!
    @beta
    Sets the text to be shown on progress dialog.
    
    \param text user defined text.

    \sa text()
*/
void HbProgressDialog::setText(const QString &text)
{
    Q_D(HbProgressDialog);
    
    if ( text != d->mTextString) {
        d->mTextString = text;
        d->createPrimitives();

        if(d->mContentWidget->mTextItem)
        {
            HbStyleOptionProgressDialog progressDialogOption;
            initStyleOption(&progressDialogOption);
            style()->updatePrimitive(d->mContentWidget->mTextItem, HbStyle::P_ProgressDialog_text, &progressDialogOption);
        }
    }
}

/*!
    @beta
    Returns the  text shown on progress dialog.

    The default text is an empty string.
    \sa setText()
*/
QString HbProgressDialog::text() const
{
    Q_D(const HbProgressDialog);

    return d->mTextString;
}

/*!
    @beta
    Sets  the icon to be shown on progress dialog.

    \param icon user defined icon.
    \sa icon()
*/
void HbProgressDialog::setIcon(const HbIcon &icon)
{
    Q_D(HbProgressDialog);
    
    if (icon != d->mIcon){
        d->mIcon = icon;
        d->createPrimitives();

        if (d->mContentWidget->mIconItem) {
            d->mContentWidget->setProperty("icon",true);

            HbStyleOptionProgressDialog progressDialogOption;
            initStyleOption(&progressDialogOption);
            style()->updatePrimitive(d->mContentWidget->mIconItem, HbStyle::P_ProgressDialog_icon, &progressDialogOption);
        }
        else
        {
            d->mContentWidget->setProperty("icon",false);
        }
        d->mContentWidget->repolishContent();
    }
}

/*!
    @beta
    Returns the  icon shown on progress dialog.

    The default icon is a null icon.
    \sa setIcon()
*/
HbIcon HbProgressDialog::icon() const
{
    Q_D(const HbProgressDialog);

    return d->mIcon;
}


#include "moc_hbprogressdialog.cpp"
#include "hbprogressdialog.moc"
