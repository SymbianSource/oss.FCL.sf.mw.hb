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

#include "hbmessagebox_p.h"
#include "hbnamespace_p.h"
#include <hbmessagebox.h>
#include <hbstyleoptionmessagebox_p.h>
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
#ifdef Q_OS_SYMBIAN
#include <systemtoneservice.h>
#endif

class HbStyle;

class HbMessageBoxEditor : public HbLineEdit
{
	Q_OBJECT
public:
    HbMessageBoxEditor(QGraphicsItem* parent =0) : HbLineEdit(parent),mText()
    {
        setReadOnly(true);
        setCursorVisibility(Hb::TextCursorHidden);
        HbScrollArea *scroll = scrollArea();
        scroll->setVerticalScrollBarPolicy(HbScrollArea::ScrollBarAsNeeded);
        clearContextMenuFlag(Hb::ShowTextContextMenuOnLongPress);
        clearContextMenuFlag(Hb::ShowTextContextMenuOnSelectionClicked);
        setBackgroundItem(0,0);     
    }

    void focusInEvent(QFocusEvent * event)
    {
        Q_UNUSED(event);         
    }
    void focusOutEvent(QFocusEvent * event)
    {
        Q_UNUSED(event);
    }

    void setHtmlText(const QString &text)
    {
        mText = text;
        if(Qt::mightBeRichText(mText)){
            setHtml(text);
        }
        else {
             QString htmlString = Qt::convertFromPlainText(mText);
             setHtml(htmlString);
        }
     }

    QString htmlText() const
    {
        return mText;
    }
private:
    QString mText;

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
    mMessageBoxContentWidget(0),
    mMessageBoxType(HbMessageBox::MessageTypeNone),
    mIconVisible(true)
{
}

void HbMessageBoxPrivate::_q_buttonClicked()
{
    Q_Q(HbMessageBox);    
    HbAction *action = static_cast<HbAction*>(q->sender());
    q->done(mActionList.value(action));
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
       
    mMessageBoxContentWidget = new HbMessageBoxContentWidget( this );
    q->setContentWidget( mMessageBoxContentWidget );
    q->setDismissPolicy(HbPopup::NoDismiss);
    q->setTimeout(HbPopup::NoTimeout);      
    q->setStandardButtons(HbMessageBox::Ok);
    
    switch(mMessageBoxType) {
    case HbMessageBox::MessageTypeNone:
        mMessageBoxContentWidget->mIconItem->hide();
        mMessageBoxContentWidget->setProperty("hasIcon",false);
        break;
    case HbMessageBox::MessageTypeInformation:
    case HbMessageBox::MessageTypeWarning:
    case HbMessageBox::MessageTypeQuestion:
        break;
        
    }

}
void HbMessageBoxPrivate::_q_appearEffectEnded(HbEffect::EffectStatus status)
{
#ifdef Q_OS_SYMBIAN 
	
    	if ( (status.reason == Hb::EffectFinished) ||  ( (status.reason == Hb::EffectCancelled) && (!mStartEffect) ))  {
	        CSystemToneService *pSystemTone = systemToneService();
			if(!pSystemTone) {
				return ;
			}
			switch(mMessageBoxType) {
    		case HbMessageBox::MessageTypeInformation:
    			pSystemTone->PlayTone(CSystemToneService::EInformationBeep); 
    			break;
    		case HbMessageBox::MessageTypeWarning:
    			pSystemTone->PlayTone(CSystemToneService::EWarningBeep); 
    			break;
    		case HbMessageBox::MessageTypeQuestion:
    			pSystemTone->PlayTone(CSystemToneService::EConfirmationBeep); 
        	break;
      	default:
      		break;        
    	}

		}
#else
	Q_UNUSED(status);
#endif // Q_OS_SYMBIAN
}

/*!
    @beta
    
    \class HbMessageBox
    \brief HbMessageBox is a convenience modal dialog class. HbMessageBox can be used to launch a information,question,warning or any other 
    general messages.

	\image html information.PNG  "An information MessageBox"
	\image html question.PNG  "A question MessageBox"
	\image html warning.PNG  "A warning MessageBox"

    Using HbMessageBox, the following dialogs can be created:

    <b>Information:</b> a statement to the user to which they may respond by acknowledging the information ('Ok').<br>
    <b>Question:</b> a query to the user requiring a response. User needs to select between two alternatives, the positive or negative (For example: 'Delete Mailbox?' 'Yes'/'No').<br>
    <b>Warning:</b> a statement to the user to which they may respond by acknowledging the warning ('OK').<br>
    <b>None:</b> Any general messages to which user responds. 
    
    By default, Message box launches a None dialog which can contain a text, an icon and action buttons

    Default properties for the MessageBox (warning, information and question dialogs) are:

    Description text: Text shown to the user as information. The amount of text rows is not limited, but after five rows the text starts scrolling.
    Icon: Default icons are available for each dialog type using the MessageBox template. Changing the default icons is not recommended.
    Action button : One action button ("Ok") for all the types.
    
    All the four dialogs(information, warning, question and none) supported by MessageBox are by default modal in nature, with
    a dismiss policy of NoDismiss, timeout policy of NoTimeout, and with a BackgroundFade property on.

    Example code for launching MessageBox using static convenience functions:

    \code
    //Information MessageBox
    HbMessageBox::information("The file is deleted",0,0,HbMessageBox::Ok);

    //Warning MessageBox
    HbMessageBox::warning("This will change the name",0,0,HbMessageBox::Ok);

    //Question MessageBox
     HbMessageBox::question("Do you want to delete the file ?", this, SLOT(onDialogClose(int)),HbMessageBox::Yes |HbMessageBox::No );
    \endcode

    Example code to show an information messagebox:
    \code
    HbMessageBox *box = new HbMessageBox("This is a general note.");
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->open();
    \endcode

    Example code to show a question messagebox with a return value based action
    \code
    HbMessageBox *box = new HbMessageBox(HbMessageBox::MessageTypeQuestion);
    box->setText("Delete file IC0002 ? ");
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setStandardButtons(HbMessageBox::Yes | HbMessageBox::No);
    box->open(this,SLOT(onDialogClosed(int)));
    
    //Slot implementation
    void MessageBoxView::onDialogClosed(int action)
    {
         if (action == HbMessageBox::Yes) {
          //User Clicked Yes//
          //Delete the file//
         }
         else if (action == HbMessageBox::No) {
          //User Clicked No//
          //do not delete the file//
         }
        
    }
    \endcode     

    Example code to show a question messagebox using static API
     \code
     HbMessageBox::question("Delete file IC0002 ? ",this,SLOT(onDialogClosed(int)),HbMessageBox::Yes | HbMessageBox::No);  
     // Here the SLOT implementation is same as above
     \endcode     


    \enum HbMessageBox::MessageBoxType

    \value \b MessageTypeNone creates a modal dialog, which by default will have one OK button 
        for the user to dismiss the dialog.

    \value \b MessageTypeWarning creates a simple modal dialog with a warning icon and a description text. 
        This Dialog by default will have one Ok button, for the user to dismiss the dialog. 

    \value \b MessageTypeWarning creates a simple modal dialog with a information icon and a description text. 
        This Dialog by default will have one Ok button, for the user to dismiss the dialog. 

     \value \b MessageTypeQuestion creates a simple modal dialog with a question icon and a description text. 
        This Dialog by default will have one Ok button, for the user to dismiss the dialog.  
*/

/*!
    Constructs a MessageBox with \a type and a \a parent.
    \param type User can create information/warning/question dialogs by passing appropriate MessageBoxType.
    \param parent parent item to MessageBox

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
    Constructs a new MessageBox with \a text and \a parent.
    \param text The descriptive text for the MessageBox.
    \param type Messagebox type can be MessageTypeInformation, MessageTypeQuestion, MessageTypeWarning
    \param parent parent item to MessageBox.
 */
HbMessageBox::HbMessageBox(const QString &text,MessageBoxType type, QGraphicsItem *parent)
    : HbDialog(*new HbMessageBoxPrivate, parent)
{
    Q_D(HbMessageBox);
    d->mMessageBoxType = type;
    d->q_ptr = this;
    d->init();
    d->mMessageBoxContentWidget->mTextEdit->setHtmlText(text);
}

/*!
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
    \reimp
*/
void HbMessageBox::initStyleOption(HbStyleOptionMessageBox *option) const
{
    Q_D(const HbMessageBox);
    HbDialog::initStyleOption(option);
    option->icon = d->mIcon;
    option->messageBoxType = d->mMessageBoxType;
}

/*!
   \reimp
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
    Sets the descriptive text for the messagebox. It can be in plain text format or html format.
    \param text Descriptive text for the MessageBox
    \sa text()
*/
void HbMessageBox::setText(const QString &text)
{
    Q_D(HbMessageBox);
    if ( text !=  d->mMessageBoxContentWidget->mTextEdit->htmlText() ) {
        d->mMessageBoxContentWidget->mTextEdit->setHtmlText(text);
    }
}

/*!
    Returns descriptive text from the messagebox.
    \sa setText()
*/
QString HbMessageBox::text() const
{
    Q_D(const HbMessageBox);
    return d->mMessageBoxContentWidget->mTextEdit->htmlText();
}

/*!
    Sets a custom Icon for the MessageBox. Not recommended to change the icon unless there is a real use case.
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
        if(iconVisible() ) {
            d->mMessageBoxContentWidget->mIconItem->show();
            d->mMessageBoxContentWidget->setProperty("hasIcon",true);
        }
    }
}

/*!
    Returns icon of the messagebox.
    \sa setIcon()
*/
HbIcon HbMessageBox::icon() const
{
    Q_D(const HbMessageBox);
    return d->mIcon;
}


/*!
    sets the icon \a visible property to true or false.

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
    Returns the icon visibilty flag. Icon visibility is true by default.
    \sa setIconVisible()
*/   

bool  HbMessageBox::iconVisible() const
{
    Q_D(const HbMessageBox);
    return d->mIconVisible;

}
/*!
    sets the buttons for the dialog. If the Application wants "Yes" and "No" buttons 
    can set it like setStandardButtons(HbMessageBox::Yes | HbMessageBox::No).
    If only yes Button then setStandardButtons(HbMessageBox::Yes).
   
   \param buttons the button set
    \sa standardButtons()

*/
void HbMessageBox::setStandardButtons(HbMessageBox::StandardButtons buttons)
{
    Q_D(HbMessageBox); 
    if(d->mStandardButtons == buttons ) {
        return;
    }
    clearActions();
    d->mStandardButtons = buttons;
    uint i = HbMessageBox::Ok;        
    int count =0;
    while (i <= HbMessageBox::Reset) {
         HbAction *action=0;
         if (i & buttons) {
             StandardButton button = HbMessageBox::StandardButton(i);
             switch(button){
                    case Ok:
                          action = new HbAction(hbTrId("txt_common_button_ok"),this); 
                          d->mActionList.insert(action,Ok);
                          break;
                    case Save:
                          action = new HbAction(hbTrId("txt_common_button_save"),this); 
                          d->mActionList.insert(action,Save);
                          break;
                    case Open:
                          action = new HbAction(hbTrId("txt_common_button_open"),this); 
                          d->mActionList.insert(action,Open);
                          break;
                     case Yes:
                          action = new HbAction(hbTrId("txt_common_button_yes"),this); 
                          d->mActionList.insert(action,Yes);
                          break;
                      case No:
                          action = new HbAction(hbTrId("txt_common_button_no"),this);                         
                          d->mActionList.insert(action,No);
                          break;
                      case Retry:
                          action = new HbAction(hbTrId("txt_common_button_retry"),this);  
                          d->mActionList.insert(action,Retry);
                          break;                      
                      case Continue:
                          action = new HbAction(hbTrId("txt_common_button_continue"),this);  
                          d->mActionList.insert(action,Continue);
                          break;
                      case Close:
                          action = new HbAction(hbTrId("txt_common_button_close"),this);
                          d->mActionList.insert(action,Close);
                          break;
                       case Cancel:
                          action = new HbAction(hbTrId("txt_common_button_cancel"),this); 
                          d->mActionList.insert(action,Cancel);
                          break;
                       case Help:
                          action = new HbAction(hbTrId("txt_common_button_help"),this); 
                          d->mActionList.insert(action,Help);
                          break;
                       case Reset:
                          action = new HbAction(hbTrId("txt_common_button_reset"),this);    
                          d->mActionList.insert(action,Reset);
                          break;
                       case Delete:
                          action = new HbAction(hbTrId("txt_common_button_delete"),this);
                          d->mActionList.insert(action,Delete);
                          break;
                       default :
                           break;

                };
            
            }
            i = i << 1;

            if(action) {
                connect(action,SIGNAL(triggered()),this,SLOT(_q_buttonClicked()));
                addAction(action);                
                count++;
            }
            
        }
}
/*!
   Returns the standared button list
   
   \sa setStandardButtons()

*/
HbMessageBox::StandardButtons HbMessageBox::standardButtons() const
{
    Q_D(const HbMessageBox);
    return d->mStandardButtons;

}

/*!

    \deprecated HbMessageBox::question(const QString&,QObject*,const char*,const QString&,const QString&,QGraphicsWidget*,QGraphicsScene*,QGraphicsItem*)
        is deprecated.

    Please use 
    
    question(const QString &questionText,
                                            QObject *receiver,
                                            const char *member,
                                            HbMessageBox::StandardButtons buttons,
                                            QGraphicsWidget *headWidget,
                                            QGraphicsScene *scene,
                                            QGraphicsItem *parent)

    This is a convenience function for showing a question dialog with \a questionText with provided buttons.
    \param questionText descriptive text for the messagebox
    \param receiver Object which has the slot, which acts as a handler once the dialog closes.
    \param member the slot, where the control will come, once the dialog is closed.
    \param primaryButtonText text for the primary button.
    \param secondaryButtonText text for the secondary button.
    \param headWidget the heading widget, where the user can set a title, Null by default.
    \param scene the scene for the MessageBox. Null by default.
    \param parent the parent widget. Null by default.
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
    
    if((messageBox->actions().count() > 0) && messageBox->actions().at(0)){
        messageBox->actions().at(0)->setText(primaryButtonText);
    }

    if((messageBox->actions().count() > 1) && messageBox->actions().at(1)){
        messageBox->actions().at(1)->setText(secondaryButtonText);
    }

    if(headWidget) {
        messageBox->setHeadingWidget(headWidget);
    }
    messageBox->setAttribute(Qt::WA_DeleteOnClose);
    messageBox->open(receiver,member);
}
     
/*!
    \deprecated HbMessageBox::information(const QString&,QObject*,const char*,QGraphicsWidget*,QGraphicsScene*,QGraphicsItem*)
        is deprecated.

    Please use 
    
    information(const QString &questionText,
                                            QObject *receiver,
                                            const char *member,
                                            HbMessageBox::StandardButtons buttons,
                                            QGraphicsWidget *headWidget,
                                            QGraphicsScene *scene,
                                            QGraphicsItem *parent)
    This is a convenience function for showing an information dialog with a descriptive text and a default OK button.
    \param informationText Descriptive text for the information dialog.
    \param receiver Which has the slot, which acts as a handler once the dialog closes.
    \param member the slot, where the control will come, once the dialog is closed.
    \param headWidget This can used by the user to set a title widget. Null by default.
    \param scene the scene for the MessageBox, Null by default.
    \param parent the parent widget. Null by default
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
    \deprecated HbMessageBox::warning(const QString&,QObject*,const char*,QGraphicsWidget*,QGraphicsScene*,QGraphicsItem*)
        is deprecated.

    Please use 
    
    warning(const QString &questionText,
                                            QObject *receiver,
                                            const char *member,
                                            HbMessageBox::StandardButtons buttons,
                                            QGraphicsWidget *headWidget,
                                            QGraphicsScene *scene,
                                            QGraphicsItem *parent)

    This is a convenience function for showing a warning dialog with a descriptive text and an OK button.
    \param warningText Descriptive text for the warning dialog.
    \param receiver Which has the slot, which acts as a handler once the dialog closes.
    \param member the slot, where the control will come, once the dialog is closed.
    \param headWidget This can used by the user to set a title widget, Null by default.
    \param scene the scene for the messagebox, Null by default.
    \param parent the parent widget, Null by default.
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

/*!
    This is a convenience function for showing a question dialog with \a questionText and a default OK button.
    \param questionText descriptive text for the messagebox
    \param receiver Object which has the slot, which acts as a handler once the dialog closes.
    \param member the slot, where the control will come, once the dialog is closed.The signature of member is void dialogClosed(int val);
    \param buttons The action buttons of the dialog.
    \param headWidget the heading widget, where the user can set a title, Null by default.
    \param scene the scene for the MessageBox. Null by default.
    \param parent the parent widget. Null by default.
*/
void HbMessageBox::question(const QString &questionText,
                                            QObject *receiver,
                                            const char *member,
                                            HbMessageBox::StandardButtons buttons,
                                            QGraphicsWidget *headingWidget,
                                            QGraphicsScene *scene,
                                            QGraphicsItem *parent)
{    
    HbMessageBox *messageBox = new HbMessageBox(HbMessageBox::MessageTypeQuestion, parent);
    if (scene && !parent) {
        scene->addItem(messageBox);
    }
    messageBox->setText(questionText);

    messageBox->setStandardButtons(buttons);   
    
    if(headingWidget) {
        messageBox->setHeadingWidget(headingWidget);
    }
    messageBox->setAttribute(Qt::WA_DeleteOnClose);
    messageBox->open(receiver,member);
}
     
/*!
    This is a convenience function for showing an information dialog with a descriptive text and a default OK button.
    \param informationText Descriptive text for the information dialog.
    \param receiver Which has the slot, which acts as a handler once the dialog closes.
    \param member the slot, where the control will come, once the dialog is closed.The signature of member is void dialogClosed(int val);
    \param buttons The action buttons of the dialog.
    \param headWidget This can used by the user to set a title widget. Null by default.
    \param scene the scene for the MessageBox, Null by default.
    \param parent the parent widget. Null by default
*/
void HbMessageBox::information(const QString &informationText,
                                               QObject *receiver,
                                               const char *member,
                                               HbMessageBox::StandardButtons buttons,
                                               QGraphicsWidget *headingWidget,
                                               QGraphicsScene *scene,
                                               QGraphicsItem *parent)
{
    HbMessageBox *messageBox = new HbMessageBox(HbMessageBox::MessageTypeInformation, parent);
    if (scene && !parent) {
        scene->addItem(messageBox);
    }
    messageBox->setText(informationText);

    messageBox->setStandardButtons(buttons);   

    if(headingWidget) {
        messageBox->setHeadingWidget(headingWidget);
    }
    messageBox->setAttribute(Qt::WA_DeleteOnClose);
    messageBox->open(receiver,member);
}
                                                                                              
/*!
    This is a convenience function for showing a warning dialog with a descriptive text and an OK button.
    \param warningText Descriptive text for the warning dialog.
    \param receiver Which has the slot, which acts as a handler once the dialog closes.
    \param member the slot, where the control will come, once the dialog is closed.The signature of member is void dialogClosed(int val);
    \param buttons The action buttons of the dialog.
    \param headWidget This can used by the user to set a title widget, Null by default.
    \param scene the scene for the messagebox, Null by default.
    \param parent the parent widget, Null by default.
*/
void HbMessageBox::warning(const QString &warningText,
                                           QObject *receiver,
                                           const char *member,
                                           HbMessageBox::StandardButtons buttons,
                                           QGraphicsWidget *headingWidget,
                                           QGraphicsScene *scene,
                                           QGraphicsItem *parent)
{
    HbMessageBox *messageBox = new HbMessageBox(HbMessageBox::MessageTypeWarning, parent);
    if (scene && !parent) {
        scene->addItem(messageBox);
    }
    messageBox->setText(warningText);

    messageBox->setStandardButtons(buttons);   

    if(headingWidget) {
        messageBox->setHeadingWidget(headingWidget);
    }
    messageBox->setAttribute(Qt::WA_DeleteOnClose);
    messageBox->open(receiver,member);
}
#include "moc_hbmessagebox.cpp"
#include "hbmessagebox.moc"

