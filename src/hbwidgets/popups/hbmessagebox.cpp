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

class HbStyle;

class HbMessageBoxEditor : public HbLineEdit
{
public:
    HbMessageBoxEditor(QGraphicsItem* parent =0) : HbLineEdit(parent),mText()
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

    void setHtmlText(const QString &text)
    {
        mText = text;
        setHtml(text);
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
        q->addAction(new HbAction(q->tr("OK"),q));
        break;

    case HbMessageBox::MessageTypeQuestion:
        mMessageBoxContentWidget = new HbMessageBoxContentWidget( this );
        q->setContentWidget( mMessageBoxContentWidget );
        q->setDismissPolicy(HbPopup::NoDismiss);
        q->setTimeout(HbPopup::NoTimeout);       
        q->addAction(new HbAction(q->tr("Yes"),q));
        q->addAction(new HbAction(q->tr("No"),q));
        break;
    }

}

/*!
    @beta
    
    \class HbMessageBox
    \brief The HbMessageBox class provides a modal dialog for informing the user or for asking the user a question and receiving an answer.

    \image html information.PNG  "An information MessageBox"
    \image html question.PNG  "A question MessageBox"
    \image html warning.PNG  "A warning MessageBox"

    Using HbMessageBox, the following dialogs can be created:

    <b>Information:</b> a statement to the user to which they may respond by acknowledging the information ('OK').<br>
    <b>Question:</b> a query to the user requiring a response. User needs to select between two alternatives, the positive or negative (For example: 'Delete Mailbox?' 'Yes'/'No').<br>
    <b>Warning:</b> a statement to the user to which they may respond by acknowledging the warning ('OK').<br>
    
    By default, Message box launches an information dialog which contains a description text and user actions visualized as command buttons.

    Default properties for the MessageBox (warning, information and question dialogs) are:

    Description text: Text shown to the user as information. The amount of text rows is not limited, but after five rows the text starts scrolling.
    Icon: Default icons are available for each dialog type using the MessageBox template. Changing the default icons is not recommended.
    Action buttons (one or two): one button for information and warning MessageBox, two buttons for question MessageBox.
    
    All the three dialogs(information, warning, question) supported by MessageBox are by default modal in nature, with
    a dismiss policy of NoDismiss, timeout policy of NoTimeout, and with a BackgroundFade property on.
    The user must click the OK/Yes/No buttons to dismiss the Message Box.

    Example code for launching MessageBox using static convenience functions:

    \code
    //Information MessageBox
    HbMessageBox::information(informationText, this, SLOT(onDialogClose(HbAction*)), headWidget, scene, parent);

    //Warning MessageBox
    HbMessageBox::warning(warningText, this, SLOT(onDialogClose(HbAction*)), headWidget, scene, parent);

    //Question MessageBox
    HbMessageBox::question(questionText, this, SLOT(onDialogClose(HbAction*)), primaryButtonText, secondaryButtonText, headWidget, scene, parent);
    \endcode

    Example code to show a question messagebox with a return value based action
    \code
    HbMessageBox *box = new HbMessageBox(" Delete file IC0002 ? ",HbMessageBox::MessageTypeQuestion);
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->open(this,SLOT(dialogClosed(HbAction*)));

    //Slot implementation
    void dialogClosed(HbAction *action)
    {
        HbMessageBox *dlg = static_cast<HbMessageBox*>(sender());
        if(action == dlg->actions().at(0)) 
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

    \value \b MessageTypeInformation creates a modal information dialog, which by default will have one OK button 
        for the user to dismiss the dialog.

    \value \b MessageTypeWarning creates a simple modal dialog with a warning icon and a description text. 
        Dialog by default will have one OK button, for the user to dismiss the dialog. 

    \value \b MessageTypeQuestion Shows a modal dialog with question icon and a description text. The user can either confirm or
        reject the dialog. By default dialog supports two buttons, using which user can dismiss the dialog. 

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
    Sets the descriptive text for the messagebox.
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
    This is a convenience function for showing a question dialog with \a questionText and buttons with specified \a primaryButtonText and
    \a secondaryButtonText. 
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
#include "moc_hbmessagebox.cpp"
#include "hbmessagebox.moc"

