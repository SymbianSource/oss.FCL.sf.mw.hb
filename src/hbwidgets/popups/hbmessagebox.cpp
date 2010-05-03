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

#include "hbnamespace_p.h"
#include <hbmessagebox.h>
#include "hbmessagebox_p.h"
#include <hbstyleoptionmessagebox.h>
#include <hbmainwindow.h>
#include <hbaction.h>
#include <hblineedit.h>
#include <hbscrollbar.h>
#include <hbscrollarea.h>
#include "hbglobal_p.h"
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>
#include <QTextOption>

class HbStyle;

class HbMessageBoxEditor : public HbLineEdit
{
public:
	HbMessageBoxEditor(QGraphicsItem* parent =0) : HbLineEdit(parent)
	{
        setReadOnly(true);
        setCursorVisibility(Hb::TextCursorHidden);
		HbScrollArea *scroll = scrollArea();
		scroll->setVerticalScrollBarPolicy(HbScrollArea::ScrollBarAsNeeded);
		clearContextMenuFlag(Hb::ShowTextContextMenuOnLongPress);
		clearContextMenuFlag(Hb::ShowTextContextMenuOnSelectionClicked);
		primitive(HbStyle::P_LineEdit_frame_normal)->setVisible(false);		
		primitive(HbStyle::P_LineEdit_frame_highlight)->setVisible(false);		
	}

	HbScrollBar * getScrollBar() const
	{
		HbScrollArea *scroll = scrollArea();
		return scroll->verticalScrollBar();
	}

	void focusInEvent(QFocusEvent * event)
	{
		Q_UNUSED(event);		 
	}
	void focusOutEvent(QFocusEvent * event)
	{
		Q_UNUSED(event);
	}

};

class HbMessageBoxContentWidget : public HbWidget
{
    Q_OBJECT
public:
    HbMessageBoxPrivate *d;
    HbMessageBoxEditor *mTextEdit;
    QGraphicsItem *mIconItem;
    HbMessageBoxContentWidget(HbMessageBoxPrivate *priv,
        QGraphicsItem* parent =0) : HbWidget(parent),d(priv),mTextEdit(0),mIconItem(0)
    {

        mTextEdit = new HbMessageBoxEditor(this);
        mIconItem = style()->createPrimitive(HbStyle::P_MessageBox_icon, this);
        setProperty("hasIcon",true);
        HbStyle::setItemName(mTextEdit, "text");
        HbStyle::setItemName(mIconItem, "icon");
    }
    enum { Type = HbPrivate::ItemType_MessageNoteContentWidget };
    int type() const { return Type; }
};

/*
    constructor

*/

HbMessageBoxPrivate::HbMessageBoxPrivate() :
    HbDialogPrivate(),
    mIcon(),
    mIconAlignment(Qt::AlignCenter),
	mMessageBoxContentWidget(0),
	mMessageBoxType(HbMessageBox::MessageTypeInformation),
	mIconVisible(true)
{
}

void HbMessageBoxPrivate::_q_closeOnGesture()
{
}

/*
    destructor
*/
HbMessageBoxPrivate::~HbMessageBoxPrivate()
{
}

/*
    init()
*/
void HbMessageBoxPrivate::init()
{
    Q_Q(HbMessageBox);

    switch(mMessageBoxType) {
    case HbMessageBox::MessageTypeInformation:
	case HbMessageBox::MessageTypeWarning:
		mMessageBoxContentWidget = new HbMessageBoxContentWidget( this );
        q->setContentWidget( mMessageBoxContentWidget );
		q->setDismissPolicy(HbPopup::NoDismiss);
		q->setTimeout(HbPopup::NoTimeout);      
		q->setPrimaryAction(new HbAction(q->tr("OK"), q));
        break;

    case HbMessageBox::MessageTypeQuestion:
		mMessageBoxContentWidget = new HbMessageBoxContentWidget( this );
        q->setContentWidget( mMessageBoxContentWidget );
		q->setDismissPolicy(HbPopup::NoDismiss);
		q->setTimeout(HbPopup::NoTimeout);       
		q->setPrimaryAction(new HbAction(q->tr("Yes"), q));
        q->setSecondaryAction(new HbAction(q->tr("No"), q));
        break;
    }

}


/*!
    @beta
    
    \class HbMessageBox
    \brief HbMessageBox is a graphics widget which shows text message and an icon.

    HbMessageBox is derived from HbDialog that provides most of functionality such as
    modality, and timeouts.

	HbMessageBox can used to show dialogs which contains a question ,information and warning.
	By default HbMessageBox launches a information dialog.

	example code to show a information messagebox:
    \code
    HbMessageBox *box = new HbMessageBox("This is a information note");
	box->setAttribute(Qt::WA_DeleteOnClose);
	box->open();
    \endcode

	example code to show a information messagebox with two action keys:
    \code
    HbMessageBox *box = new HbMessageBox("This will delete XX . Continue ? ");
	box->setSecondaryAction(new HbAction("Cancel"));
	box->setAttribute(Qt::WA_DeleteOnClose);
	box->open();
    \endcode

	example code to show a qustion messagebox with a return value based action
    \code
    HbMessageBox *box = new HbMessageBox(" Delete file IC0002 ? ",HbMessageBox::MessageTypeQuestion);
	box->setAttribute(Qt::WA_DeleteOnClose);
	box->open(this,SLOT(dialogClosed(HbAction*)));
    \endcode

	The declaration of SLOT is as shown below

	\code
	void dialogClosed(HbAction *action)
    {
        HbMessageBox *dlg = static_cast<HbMessageBox*>(sender());
        if(action == dlg->primaryAction())
        {
            // Delete file 
        }
        else
        {
           // Cancellation is done.Dont delete the file
        }
     }
	 \endcode

    \enum HbMessageBox::MessageBoxType

    \value \b MessageTypeInformation creates a dialog which will have some information for the user.
		MessageBox will have Ok button and will not be auto dissmissed. The user need to press
		Ok to dismiss.

    \value \b MessageTypeWarning is a simple message dialog with warning icon. The user needs to press Ok
        to dismiss. 

    \value \b MessageTypeQuestion Shows a message dialog with question icon. The user can either confirm or
		reject the dialog. 

*/



/*!
    @beta
    Constructor.
	\param type An optional parameter.The type of messagebox.information/warning/question
    \param parent An optional parameter.

*/
HbMessageBox::HbMessageBox(MessageBoxType type,QGraphicsItem *parent) : 
    HbDialog(*new HbMessageBoxPrivate, parent)
{
    Q_D(HbMessageBox);
    d->mMessageBoxType = type;
    d->q_ptr = this;
    d->init();
}


/*!
    @beta
    Constructs a new HbMessageBox with \a text and \a parent.
	\param text The text for the MessageBox.
	\param type An optional parameter.The type of messagebox.information/warning/question
	\param parent An optional parameter.
 */
HbMessageBox::HbMessageBox(const QString &text,MessageBoxType type, QGraphicsItem *parent)
    : HbDialog(*new HbMessageBoxPrivate, parent)
{
    Q_D(HbMessageBox);
    d->mMessageBoxType = type;
    d->q_ptr = this;
	d->init();
	d->mMessageBoxContentWidget->mTextEdit->setText(text);
}


/*!
    @beta
    destructor

*/
HbMessageBox::~HbMessageBox()
{

}


/*!
    \internal
 */
HbMessageBox::HbMessageBox(HbMessageBoxPrivate &dd, QGraphicsItem *parent) : 
    HbDialog(dd, parent)
{
    Q_D(HbMessageBox);
    d->q_ptr = this;
    d->init();
}


/*!

    \deprecated HbMessageBox::primitive(HbStyle::Primitive)
        is deprecated.

    Provides access to primitives of HbMessageBox. 
    \param primitive is the type of the requested primitive. The available 
    primitives are P_MessageBox_icon.

*/
QGraphicsItem *HbMessageBox::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbMessageBox);
    switch (primitive) {
        case HbStyle::P_MessageBox_icon:
            return d->mMessageBoxContentWidget->mIconItem;
        default:
            return 0;
    }
}


/*!
    @beta
    \reimp
    Initializes \a option with the values from this HbMessageBox.
*/
void HbMessageBox::initStyleOption(HbStyleOptionMessageBox *option) const
{
    Q_D(const HbMessageBox);
    HbDialog::initStyleOption(option);
    option->icon = d->mIcon;
    option->iconAlignment = d->mIconAlignment;
    option->messageBoxType = d->mMessageBoxType;
}



/*!
    @beta
    updatePrimitives.

*/
void HbMessageBox::updatePrimitives()
{
    Q_D(HbMessageBox); 
        HbDialog::updatePrimitives();
    HbStyleOptionMessageBox option;
    initStyleOption(&option);
    if (d->mMessageBoxContentWidget->mIconItem) {
         style()->updatePrimitive(d->mMessageBoxContentWidget->mIconItem, HbStyle::P_MessageBox_icon, &option);
    }
   
}


/*!
    @beta
    Sets text of the messagebox.
    \param text This is the text for the MessageBox
	\sa text()
*/
void HbMessageBox::setText(const QString &text)
{
    Q_D(HbMessageBox);
	if ( text !=  d->mMessageBoxContentWidget->mTextEdit->text() ) {
		d->mMessageBoxContentWidget->mTextEdit->setText(text);
    }
}

/*!
    @beta
    Returns text of the messagebox.
    \sa setText()
*/

QString HbMessageBox::text() const
{
    Q_D(const HbMessageBox);
	return d->mMessageBoxContentWidget->mTextEdit->text();
}


/*!
    @beta
    Sets a custon Icon for the MessageBox. Not recommended to change the icon unless there is a real use case.
    \icon An icon instance
	\sa icon()
*/
void HbMessageBox::setIcon(const HbIcon &icon)
{
    Q_D(HbMessageBox);
    if (icon != d->mIcon){
        d->mIcon = icon;
        if (d->mMessageBoxContentWidget->mIconItem) {
            HbStyleOptionMessageBox option;
            initStyleOption(&option);
            style()->updatePrimitive(d->mMessageBoxContentWidget->mIconItem, HbStyle::P_MessageBox_icon, &option);
        }
	}
}


/*!
    @beta
    Returns icon of the messagebox.
    \sa setIcon()
*/
HbIcon HbMessageBox::icon() const
{
    Q_D(const HbMessageBox);
    return d->mIcon;
}
/*!

    \deprecated HbMessageBox::setIconAlignment(Qt::Alignment)
        is deprecated.   

    There is no icon alignment support
*/
void HbMessageBox::setIconAlignment( Qt::Alignment align )
{
    Q_D(HbMessageBox);
	HB_DEPRECATED("HbMessageBox::setIconAlignment is deprecated.");
    if (align != d->mIconAlignment){
        d->mIconAlignment = align;
        if (d->mMessageBoxContentWidget->mIconItem) {
            HbStyleOptionMessageBox option;
            initStyleOption(&option);
            style()->updatePrimitive(d->mMessageBoxContentWidget->mIconItem, HbStyle::P_MessageBox_icon, &option);
        }
    }
}
/*!
    @beta
    Sets wheather icon is visible or not.By default the icon is visible.

    \param visible the visibility flag
	\sa iconVisible()

*/
void HbMessageBox::setIconVisible(bool visible)
{
	Q_D(HbMessageBox);
	if(visible != d->mIconVisible) {
		if(visible) {
			d->mMessageBoxContentWidget->mIconItem->show();
			d->mMessageBoxContentWidget->setProperty("hasIcon",true);
		}
		else {
			d->mMessageBoxContentWidget->mIconItem->hide();
			d->mMessageBoxContentWidget->setProperty("hasIcon",false);
		}

		d->mIconVisible = visible;
		repolish();
	}
}
/*!
    @beta
    Returns the icon visibilty flag
	\sa setIconVisible()
*/   

bool  HbMessageBox::iconVisible() const
{
	Q_D(const HbMessageBox);
	return d->mIconVisible;

}

/*!

    \deprecated HbMessageBox::iconAlignment() const
        is deprecated.   

	There is no icon alignment support
*/

Qt::Alignment HbMessageBox::iconAlignment() const
{
    Q_D(const HbMessageBox);
	HB_DEPRECATED("HbMessageBox::iconAlignment is deprecated.");
    return d->mIconAlignment;
}
/*!
    \deprecated HbMessageBox::mousePressEvent(QGraphicsSceneMouseEvent *)
        is deprecated.

*/
void HbMessageBox::mousePressEvent(QGraphicsSceneMouseEvent *event )
{
    HB_DEPRECATED("HbMessageBox::mousePressEvent is deprecated.");
    HbDialog::mousePressEvent(event);
    event->accept();
}


/*!
    \deprecated HbMessageBox::question(const QString &,const QString &,const QString &,QGraphicsWidget *,QGraphicsScene *,QGraphicsItem*)
        is deprecated. Deprecating due to migration from exec() to open call. Please use the other available question API.

    This is a convenient function to show a default message box with the question and buttons specified.
    this will return true when first button is clicked. false on the second.
    a heading widget can be set ex:
    \code
        HbMessageBox::question("are you sure?","yes","no",new HbLabel("Delete Confirm"));
    \endcode
*/
bool HbMessageBox::question(const QString &questionText,
                                            const QString &primaryButtonText,
                                            const QString &secondaryButtonText,
                                            QGraphicsWidget *headWidget,
                                            QGraphicsScene *scene,
                                            QGraphicsItem *parent)
{	
    HB_DEPRECATED("HbMessageBox::question is deprecated. Please use HbMessageBox::question(const QString&,const QString&,QGraphicsWidget*,QGraphicsScene*,QGraphicsItem*) instead");
    
    HbMessageBox *messageBox = new HbMessageBox(HbMessageBox::MessageTypeQuestion, parent);
    if (scene) {
        scene->addItem(messageBox);
    }
    messageBox->setText(questionText);
	
    HbAction *primaryAction = new HbAction(primaryButtonText);
    messageBox->setPrimaryAction(primaryAction);

    HbAction *secondaryAction = new HbAction(secondaryButtonText);
    messageBox->setSecondaryAction(secondaryAction);
    if(headWidget) {
        messageBox->setHeadingWidget(headWidget);
    }
    HbAction *action = messageBox->exec(); 

    if (action == messageBox->primaryAction() ){
       return true;
    }
    else {
       return false;
    }
}
     
/*!
    \deprecated HbMessageBox::information(const QString &,QGraphicsWidget*,QGraphicsScene*,QGraphicsItem*)
        is deprecated. Please use the other available HbMessageBox::information(...) API.

    This is a convenient function to show a default message box with the with informationText.
    optionally a heading widget can be set ex:
    \code
        HbMessageBox::information("new message received",new HbLabel("incoming message"));
    \endcode
*/       
void HbMessageBox::information(const QString &informationText,
                                               QGraphicsWidget *headWidget,
                                               QGraphicsScene *scene,
                                               QGraphicsItem *parent)
{
    HB_DEPRECATED("HbMessageBox::information is deprecated. Please use HbMessageBox::information(const QString&, QObject*, const char*, QGraphicsWidget*,QGraphicsScene*, QGraphicsItem*) instead.");

    HbMessageBox *messageBox = new HbMessageBox(HbMessageBox::MessageTypeInformation, parent);
    if (scene) {
        scene->addItem(messageBox);
    }
    messageBox->setText(informationText);
    if(headWidget) {
        messageBox->setHeadingWidget(headWidget);
    }
	messageBox->exec();
}
                                                                                              
/*!
    \deprecated HbMessageBox::warning(const QString &,QGraphicsWidget *,QGraphicsScene*,QGraphicsItem*)
        is deprecated. Deprecated as part of moving exec to open call. Please use the other warning API.

    This is a convenient function to show a warning message box.
    optionally a heading widget can be set ex:
    \code
        HbMessageBox::warning("charge the phone",new HbLabel("battery low!"));
    \endcode
*/                                                                                            
void HbMessageBox::warning(const QString &warningText,
                                           QGraphicsWidget *headWidget,
                                           QGraphicsScene *scene,
                                           QGraphicsItem *parent)
{
    HB_DEPRECATED("HbMessageBox::warning is deprecated. Use HbMessageBox::warning(const QString&, QObject*, const char*,QGraphicsWidget*, QGraphicsScene*, QGraphicsItem *) ");

    HbMessageBox *messageBox = new HbMessageBox(HbMessageBox::MessageTypeWarning, parent);
    if (scene) {
        scene->addItem(messageBox);
    }
    messageBox->setText(warningText);
    if(headWidget) {
        messageBox->setHeadingWidget(headWidget);
    }
	messageBox->exec();
}
/*!
    @beta
    This is a convenient function to show a default message box with the question and buttons specified.
    this will return true when first button is clicked. false on the second.
    \param questionText the text for the messagebox
	\param receiver the reciever of the slot
	\param member the slot
	\param primaryButtonText string for the primary button text
	\param secondaryButtonText string for the secondary button text
	\param headWidget the heading widget
	\param scene the scene 
	\param parent the parent widget
*/

void HbMessageBox::question(const QString &questionText,
                                            QObject *receiver,
                                            const char *member,
                                            const QString &primaryButtonText,
                                            const QString &secondaryButtonText,
                                            QGraphicsWidget *headWidget,
                                            QGraphicsScene *scene,
                                            QGraphicsItem *parent)
{	
    HbMessageBox *messageBox = new HbMessageBox(HbMessageBox::MessageTypeQuestion, parent);
    if (scene && !parent) {
        scene->addItem(messageBox);
    }
    messageBox->setText(questionText);
	
    HbAction *primaryAction = new HbAction(primaryButtonText,messageBox);
    messageBox->setPrimaryAction(primaryAction);

    HbAction *secondaryAction = new HbAction(secondaryButtonText,messageBox);
    messageBox->setSecondaryAction(secondaryAction);
    if(headWidget) {
        messageBox->setHeadingWidget(headWidget);
    }
    messageBox->setAttribute(Qt::WA_DeleteOnClose);
    messageBox->open(receiver,member);
}
     
/*!
    @beta
    This is a convenient function to show a default message box with the information and buttons specified.
    this will return true when first button is clicked. false on the second.
    \param informationText the text for the messagebox
	\param receiver the reciever of the slot
	\param member the slot
	\param headWidget the heading widget
	\param scene the scene 
	\param parent the parent widget
*/
     
void HbMessageBox::information(const QString &informationText,
                                               QObject *receiver,
                                               const char *member,
                                               QGraphicsWidget *headWidget,
                                               QGraphicsScene *scene,
                                               QGraphicsItem *parent)
{
    HbMessageBox *messageBox = new HbMessageBox(HbMessageBox::MessageTypeInformation, parent);
    if (scene && !parent) {
        scene->addItem(messageBox);
    }
    messageBox->setText(informationText);
    if(headWidget) {
        messageBox->setHeadingWidget(headWidget);
    }
    messageBox->setAttribute(Qt::WA_DeleteOnClose);
    messageBox->open(receiver,member);
}
                                                                                              
/*!
    @beta
    This is a convenient function to show a default message box with the information and buttons specified.
    this will return true when first button is clicked. false on the second.
    \param warningText the text for the messagebox
	\param receiver the reciever of the slot
	\param member the slot
	\param headWidget the heading widget
	\param scene the scene 
	\param parent the parent widget
*/
                                                                                          
void HbMessageBox::warning(const QString &warningText,
                                           QObject *receiver,
                                           const char *member,
                                           QGraphicsWidget *headWidget,
                                           QGraphicsScene *scene,
                                           QGraphicsItem *parent)
{
    HbMessageBox *messageBox = new HbMessageBox(HbMessageBox::MessageTypeWarning, parent);
    if (scene && !parent) {
        scene->addItem(messageBox);
    }
    messageBox->setText(warningText);
    if(headWidget) {
        messageBox->setHeadingWidget(headWidget);
    }
    messageBox->setAttribute(Qt::WA_DeleteOnClose);
    messageBox->open(receiver,member);
}
#include "moc_hbmessagebox.cpp"
#include "hbmessagebox.moc"

