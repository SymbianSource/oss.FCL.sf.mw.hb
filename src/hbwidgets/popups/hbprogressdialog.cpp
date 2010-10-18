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
#include <hbstyleprimitivedata.h>
#include <hbstyleiconprimitivedata.h>
#include <hbstyletextprimitivedata.h>
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
    
    QGraphicsObject  *mIconItem;
    HbProgressBar *mProgressBar;
    QGraphicsObject  *mTextItem;
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
    
    mAction = new HbAction(hbTrId("txt_common_button_cancel"), q);
    QObject::connect(mAction, SIGNAL(triggered()), q, SLOT(_q_userCancel()));
    q->addAction(mAction);
    
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
    bool repolish = false;

    if( !mContentWidget->mProgressBar ){ 
        mContentWidget->mProgressBar = new HbProgressBar(mContentWidget); 
        HbStyle::setItemName(mContentWidget->mProgressBar, "pbar"); 
        repolish = true; 
    }
    
    if ( !mContentWidget->mTextItem ) { 
        mContentWidget->mTextItem = q->style()->createPrimitive(HbStyle::PT_TextItem, "text", mContentWidget); 
        repolish = true;
    }

    if ( !mIcon.isNull() ) {
        if ( !mContentWidget->mIconItem ) { 
            mContentWidget->mIconItem = q->style()->createPrimitive(HbStyle::PT_IconItem, "icon", mContentWidget); 
            repolish = true; 
        }
    } else {
        if( mContentWidget->mIconItem ){
            delete mContentWidget->mIconItem; 
            repolish = true; 
        }
        mContentWidget->mIconItem = 0;
    }

    if (repolish)
        mContentWidget->repolishContent();
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
             flags &= ~HbProgressDialogPrivate::Closepending;
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
    if(value >= mContentWidget->mProgressBar->maximum() && !(q->autoClose()) && (mAction->text() == hbTrId("txt_common_button_cancel")) ){
        if(mAction) {
            if(mAction->text() == hbTrId("txt_common_button_cancel")) {
                mAction->setText(hbTrId("txt_common_button_close"));
            }
        }
    }
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

    \image html progressdialog1.png  "A normal Progress Dialog with icon and text"
    \image html progressdialog2.png  "A wait Progress Dialog with icon and text"

    ProgressDialog widget displays that an application is active and also the completion level of the process to the user.

    A progress dialog is used to give the user an indication of how long an operation is going to take, and 
    to demonstrate that the application has not frozen. It can also give the user an opportunity to abort the operation.

    Progress dialog provides 2 types of notifications. They are:
    
    \code
    enum ProgressDialogType { ProgressDialog, WaitDialog };
    \endcode

    Progress dialog has the following default elements:
    \li Progress bar (Determinate type) 
    \li Description
    \li Cancel button (for canceling the process) 

    Optionally, progress dialog may also includes:
    \li  Icon
    \li  Heading

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
    calls cancel() API.

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

    Below sample code shows how the \a cancel() signal can be connected so that application  can terminate the task. 

    \code
    HbProgressDialog *pDialog = new HbProgressDialog(HbProgressDialog::ProgressDialog);
    connect(pDialog,SIGNAL(cancelled()),this,SLOT(dialogCancelled()));
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

    This enum describes the type value as WaitDialog.
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

void HbProgressDialog::initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive)
{
    HbWidgetBase::initPrimitiveData(primitiveData, primitive);
    QString itemName = HbStyle::itemName(primitive);
    if (itemName == QLatin1String("text")) {
        HbStyleTextPrimitiveData *data = hbstyleprimitivedata_cast<HbStyleTextPrimitiveData*>(primitiveData);
        data->text = text();
        data->textWrapping = Hb::TextWordWrap;
    }

    if (itemName == QLatin1String("icon")) {
        HbStyleIconPrimitiveData *data = hbstyleprimitivedata_cast<HbStyleIconPrimitiveData*>(primitiveData);
        data->icon = icon();        
    }
}

/*!
    \reimp
 */
void HbProgressDialog::showEvent(QShowEvent *event)
{
    Q_D(HbProgressDialog);
    d->mContentWidget->mProgressBar->show();
    if(d->mAction) {
        if(d->mAction->text() == hbTrId("txt_common_button_close")) {
            d->mAction->setText(hbTrId("txt_common_button_cancel"));
        }
    }
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
            HbStyleTextPrimitiveData data;
            initPrimitiveData(&data, d->mContentWidget->mTextItem);
            style()->updatePrimitive(d->mContentWidget->mTextItem, &data, this);
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
            HbStyleIconPrimitiveData data;
            initPrimitiveData(&data, d->mContentWidget->mIconItem);
            style()->updatePrimitive(d->mContentWidget->mIconItem, &data, this);

        }
        else
        {
            d->mContentWidget->setProperty("icon",false);
        }
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
/*!
    updation of pri
 */
void HbProgressDialog::updatePrimitives()
{
    Q_D(HbProgressDialog); 
    HbDialog::updatePrimitives();
    if (d->mContentWidget->mIconItem) {
        HbStyleIconPrimitiveData data;
        initPrimitiveData(&data, d->mContentWidget->mIconItem);
        style()->updatePrimitive(d->mContentWidget->mIconItem, &data, this);
    }    
    
    if (d->mContentWidget->mTextItem) {
        HbStyleTextPrimitiveData data;
        initPrimitiveData(&data, d->mContentWidget->mTextItem);
        style()->updatePrimitive(d->mContentWidget->mTextItem, &data, this);
    }   
}
/*!
    Recreaction of all the primitives.
 */
void HbProgressDialog::recreatePrimitives()
{
    Q_D(HbProgressDialog);
    if (d->mContentWidget->mIconItem) {
        delete d->mContentWidget->mIconItem;
        d->mContentWidget->mIconItem = 0;
        d->mContentWidget->mIconItem = style()->createPrimitive(HbStyle::PT_IconItem, "icon",d->mContentWidget);
    }
    if (d->mContentWidget->mTextItem) {
        delete d->mContentWidget->mTextItem;
        d->mContentWidget->mTextItem = 0;
        d->mContentWidget->mTextItem = style()->createPrimitive(HbStyle::PT_TextItem, "text",d->mContentWidget);
    }
}

QGraphicsItem *HbProgressDialog::primitive(const QString &itemName) const
{
    Q_D(const HbProgressDialog);

    if(!itemName.compare(QString("icon"))){
        return d->mContentWidget->mIconItem;
    }

    if(!itemName.compare(QString("text"))){
        return d->mContentWidget->mTextItem;
    }

    return HbDialog::primitive(itemName);
}

#include "moc_hbprogressdialog.cpp"
#include "hbprogressdialog.moc"
