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

#include <hbinputdialog.h>
#include "hbinputdialog_p.h"
#include "hbglobal_p.h"
#include <hblineedit.h>
#include <hbaction.h>
#include "hbinputdialogcontent_p.h"
#include <hbstyleoptioninputdialog_p.h>
#include <hbvalidator.h>

#include <QGraphicsScene>

#ifdef HBINPUTDIALOG_DEBUG
#include <QtDebug>
#endif


/*!
    @beta
    @hbwidgets
    \class HbInputDialog
    \brief HbInputDialog is a convenient class for getting user inputs. 
    Based on the \InputMode user can enter text, int, double or an IP address. InputDialog can have one or two line edit input fields.

    \image html inputdialog.png  "An Input dialog with prompt text and a user input field."

    HbInputDialog by default will have a label to display a descriptive text which gives information on the user input field, 
    an edit field for entering text and,two action buttons.

    example code example:
    \code
    HbInputDialog *object = new HbInputDialog();
    \endcode

    The HbDialog::open( QObject* receiver, const char* member ) method  from the HbDialog is used to show the HbInputDialog. 

    The open method can be attached with a SLOT of the format finished(HbAction*). When the open is used with a slot then the slot 
    will be invoked once the user does any action such as accept or reject (The Ok or Cancel press).

    
    below is the declaration of the slot.
    \code
    public slots : 
    void dialogClosed(HbAction *action);
    \endcode

    below code shows how the open method can be called using this slot.

    \code
    HbInputDialog *object = new HbInputDialog(parent);
    object->open(this,SLOT(dialogClosed(HbAction*)));
    \endcode

    A sample slot definition is shown below
    
    \code
    void dialogClosed(HbAction *action)
    {
          HbInputDialog *dlg=static_cast<HbInputDialog*>(sender());
          if(dlg->actions().first() == action) { // user is clicked OK
              //Get the string enter by user
               QString myString = dlg->value().toString();
              //Do the action here
           }
           else if(dlg->actions().at(1) == action) {
              // user is clicked CANCEL
           }    
    }
    \endcode


    In HbInputDialog four static convenience API's are provided: getText(), getInteger(), getDouble(), and getIp()
    static API's can be used to quickly get an input from user.

    \enum HbInputDialog::InputMode

    \value \b TextInput When this value is set as Input mode, Input Dialog accepts text input in its 
        correspoinding line edit field.

    \value \b IntInput When this value is set as Input mode, Input Dialog accepts Integer input in its 
        correspoinding line edit field.

    \value \b RealInput When this value is set as Input mode, Input Dialog accepts double or float input in its 
        correspoinding line edit field.

    \value \b IpInput When this value is set as Input mode, Input Dialog accepts Ip address as input in its 
        correspoinding line edit field.

 */


/*!
    Constructor of HbInputDialog with a \a parent.
    \param parent. Parent widget
*/
HbInputDialog::HbInputDialog(QGraphicsItem* parent) :
                                HbDialog(*new HbInputDialogPrivate, parent)
{
    Q_D(HbInputDialog);
    d->init();
}

/*!
    \internal
*/
HbInputDialog::HbInputDialog(HbDialogPrivate &dd, QGraphicsItem *parent) :
    HbDialog(dd, parent)
{
    Q_D(HbInputDialog);
    d->q_ptr = this;
    d->init();
}

/*!
    Destructs the HbInputDialog.
 */
HbInputDialog::~HbInputDialog()
{
}

/*!
    Sets the input mode of the user field. The default InputMode is TextInput.
    The other available modes are IntInput,RealInput and IpInput.
    
    \param mode. InputMode can be TextMode, IntMode, RealMode and Ip address mode.
        each mode will affect how the line edit filters its input.
    
    \param row. This parameter indicates which row of the field.0 (by default) means the 
    the first user field and 1 means second user field.

    \sa inputMode() 
*/    
void HbInputDialog::setInputMode(InputMode mode ,int row)
{
    Q_D(HbInputDialog);
    d->setInputMode(mode,row);
}

/*!
    Returns input mode of the user field.The default InputMode is TextInput.

    \param row This parameter indicates which row of the field.0 means the
    the first user field and 1 means second user field.

    \sa setInputMode()
*/
HbInputDialog::InputMode HbInputDialog::inputMode(int row) const
{
    Q_D(const HbInputDialog);
    if(row == 0) {
        return d->mPrimaryMode;
    } else {
        if (row ==1) {
            return d->mSecondaryMode;
        } else {
            return (HbInputDialog::InputMode)0;
        }
    }
}

/*!
    Sets the prompt text for the user field. This prompt text text can be very descriptive like username,password etc.

    \param text The des
    \param row This parameter indicates which row of the user field.0 (by default) means the 
    the first user field and 1 means second user field.

    \sa promtText()
*/
void HbInputDialog::setPromptText(const QString &text, int row)
{
    Q_D(HbInputDialog);
    d->setPromptText(text, row);
}

/*!
    Returns descriptive prompt text.

    \param row This parameter indicates which row of the field. 0 means the 
    the user field in the first row and 1 means user field in the second row.

    \sa setPromptText()
*/
QString HbInputDialog::promptText(int row) const
{
    Q_D(const HbInputDialog);
    return d->promptText(row);
}

/*!
    Sets the value for the user input field.The value should in sync HbInputDialog::InputMode of the field.
    
    \param value The value that should be presented to the user.
    
    \param row This parameter indicates which row of the user field.0 (by default) means the 
    the first user field and 1 means second user field.

    \sa value()
*/
void HbInputDialog::setValue(const QVariant &value,int row)
{
    Q_D(HbInputDialog);
    d->setText(value.toString(),row);
}

/*!
    This returns the value of the user field row. The return type is QVariant which can be converted to
    corresponding type based on HbInputDialog::InputMode of the field.
   
   \param row This parameter indicates which row of the user field.0 (by default) means the 
    the first user field and 1 means second user field

    \sa setValue()
*/
QVariant HbInputDialog::value(int row) const
{
    Q_D(const HbInputDialog);
    return QVariant(d->text(row));
}

/*!
    HbInputDialog is capable of showing one or two user input fields. This can be set using this function.
    By default this is false and only first row is visible.

    \param visible true or false. If true then two user fields will be visible otherwise one.

    \sa isAdditionalRowVisible()
*/
void HbInputDialog::setAdditionalRowVisible(bool visible)
{
    Q_D(HbInputDialog);
    d->setAdditionalRowVisible(visible);
}

/*!
    Returns the visibility of second row user input field.The default is false.

    \sa setAdditionalRowVisible()
*/
bool HbInputDialog::isAdditionalRowVisible()
{
    Q_D(HbInputDialog);
    return d->isAdditionalRowVisible();
}

/*!
   
    This API allows the user to set any validator to the user input field.

    \param validator Validator uses undo stack to back out invalid changes. Therefore undo
    is enabled when validator is set.

	\param row This parameter indicates which row of the user field.0 means the 
    the first user field and 1 means second user field

    \sa HbAbstractEdit::validator
*/
void HbInputDialog::setValidator(HbValidator *validator,int row)
{
    Q_D(HbInputDialog);
    if( (row == 0) && (d->mContentWidget->mEdit1) ) {
        d->mContentWidget->mEdit1->setValidator(validator);
    } else if( (row == 1) && (d->mContentWidget->mEdit2) ) {
        d->mContentWidget->mEdit2->setValidator(validator);
    }
}

/*!
    This API returns the validator of the corresponding user input row.

    \param row This parameter indicates which row of the user field.0 (by default) means the 
    the first user field and 1 means second user field

    \sa setValidator()
*/
HbValidator * HbInputDialog::validator(int row) const
{
    Q_D(const HbInputDialog);
    if( (row == 0) && (d->mContentWidget->mEdit1) ) {
        return d->mContentWidget->mEdit1->validator();
    } else if( (row == 1) && (d->mContentWidget->mEdit2) ) {
        return d->mContentWidget->mEdit2->validator();
    }
    return NULL;
}

/*!
    This returns the editor instance used in HbInputDialog.
    \param row This parameter indicates which row of the user field.0 (by default) means the 
    the first user field and 1 means second user field
*/
HbLineEdit* HbInputDialog::lineEdit(int row) const
{
    Q_D(const HbInputDialog);
    if( (row == 0) && (d->mContentWidget->mEdit1) ) {
        return d->mContentWidget->mEdit1;
    } else if( (row == 1) && (d->mContentWidget->mEdit2) ) {
        return d->mContentWidget->mEdit2;
    }
    return NULL;
}

/*!
    sets the echo mode for the user input fiels. The echo mode is defined in HbLineEdit.
    Normal, NoEcho, Password, PasswordEchoOnEdit are the different echo modes supportted in HbLineEdit.

    \param echoMode which can be HbLineEdit::Normal, HbLineEdit::NoEcho, HbLineEdit::Password or HbLineEdit::PasswordEchoOnEdit.
    
    \param row This parameter indicates which row of the user field.0 (by default) means the 
    the first user input field and 1 means second user input field

    \sa echoMode() , HbLineEdit::setEchoMode()
*/
void HbInputDialog::setEchoMode(HbLineEdit::EchoMode echoMode,int row)
{
    Q_D(HbInputDialog);
    if( (row == 0) && (d->mContentWidget->mEdit1) ) {
        d->mContentWidget->mEdit1->setEchoMode(echoMode);
    } else if( (row == 1) ) {
        if(d->mContentWidget->mEdit2) {
            d->mContentWidget->mEdit2->setEchoMode(echoMode);
        }
        d->mEchoMode = echoMode;
    }
};

/*!
    \deprecated HbInputDialog::primitive(HbStyle::Primitive)
        is deprecated.
    
    Provides access to primitives of HbInputDialog. 
    \param primitive is the type of the requested primitive. The available 
    primitives are P_InputDialog_text, and P_InputDialog_additionaltext.

*/
QGraphicsItem* HbInputDialog::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbInputDialog);
    switch (primitive) {
    case HbStyle::P_InputDialog_text:
        return d->mContentWidget->mLabel1;
    case HbStyle::P_InputDialog_additionaltext:
        return d->mContentWidget->mLabel2;
    default:
        return 0;
    }
}

/*!
    \reimp
*/
void HbInputDialog::initStyleOption(HbStyleOptionInputDialog *option) const
{
    Q_D(const HbInputDialog);
    HbDialog::initStyleOption(option);
    option->text = d->mPromptText;
    option->additionalText = d->mPromptAdditionalText;
}

/*!
    \reimp
*/
void HbInputDialog::updatePrimitives()
{
    Q_D(HbInputDialog); 
    HbDialog::updatePrimitives();
    HbStyleOptionInputDialog option;
    initStyleOption(&option);
    if (d->mContentWidget->mLabel1) {
        style()->updatePrimitive(d->mContentWidget->mLabel1, HbStyle::P_InputDialog_text, &option);
    }

    if (d->mContentWidget->mLabel2 && d->mContentWidget->mAdditionalRowVisible) {
        style()->updatePrimitive(d->mContentWidget->mLabel2, HbStyle::P_InputDialog_additionaltext, &option);
    }
}

/*!
    Returns the echoMode of the user input field.It returns HbLineEdit::EchoMode.

    \param row This parameter indicates which row of the user field.0  means the 
    the first user input field and 1 means second user input field

    \sa setEchoMode()
  */
HbLineEdit::EchoMode HbInputDialog::echoMode(int row) const
{
    Q_D(const HbInputDialog);
    if( (row == 0) && (d->mContentWidget->mEdit1) ) {
        return d->mContentWidget->mEdit1->echoMode();
    } else if( (row == 1)  ) {
        return d->mEchoMode;
    }
    return HbLineEdit::EchoMode(-1);//
}

/*!
    This is a static convenience function for creating an Input Dialog and to get a string data from the the user. The Application can use this 
    function in order to get any text data from user like username,search data etc. This API allows a slot to be passed as a param. This slot will 
    be invoked when the user does the action like OK press or CANCEL press.  
    
    HbInputDialog::getText(iTitle,this,SLOT(dialogClosed(HbAction*)),iText);  iTitle is the prompt text.dialogClosed will be invoked when the user does the action.
    Please refer the class documentation to know how to handle the slot.
   
    \param label The prompt text which gives information on user input field.
    \param receiver The instance where the slot is declared.
    \param member The slot which has dialogClosed(HbAction*) signature. 
    \param text The default text that should be presented to the user.
    \param scene The scene parameter.
    \param parent The parent item for the dialog.
   
    \sa getInteger(), getDouble(), getIp()
*/
void HbInputDialog::getText(const QString &label,
                                QObject *receiver,
                                const char* member,
                                const QString &text,
                                QGraphicsScene *scene, 
                                QGraphicsItem *parent)
{
    HbInputDialog *dlg = new HbInputDialog(parent);
    if (scene && !parent) {
        scene->addItem(dlg);
    }
    dlg->setPromptText(label);
    dlg->setInputMode(TextInput);
    dlg->setValue(text);    
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->open(receiver,member);
}


/*!
    This is a static convenience function for creating an Input Dialog and to get an integer data from the the user. The Application can use this 
    function in order to get any integer data from user like year , any number etc. This API allows a slot to be passed as a param. This slot will 
    be invoked when the user does the action like OK press or CANCEL press.  
    
    HbInputDialog::getInt(iTitle,this,SLOT(dialogClosed(HbAction*)),iText);  iTitle is the prompt text.dialogClosed will be invoked when the user does the action.
    Please refer the class documentation to know how to handle the slot.
   
    \param label The prompt text which gives information on user input field.
    \param receiver The instance where the slot is declared.
    \param member The slot which has dialogClosed(HbAction*) signature. 
    \param value The default value that should be presented to the user.
    \param scene The scene parameter.
    \param parent The parent widget for the dialog.
   
    \sa getText(), getDouble(), getIp()
*/
void HbInputDialog::getInteger(const QString &label, 
                                QObject *receiver,
                                const char *member,
                                int value,
                                QGraphicsScene *scene,
                                QGraphicsItem *parent)
{
    HbInputDialog *dlg = new HbInputDialog(parent);
    if(scene && !parent) {
        scene->addItem(dlg);
    }
    dlg->setPromptText(label);
    dlg->setInputMode(IntInput);
    dlg->setValue(QString::number(value));
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->open(receiver,member);
}
/*!
    This is a static convenience function for creating an Input Dialog and to get a double data from the the user. The Application can use this 
    function in order to get any double data from user. This API allows a slot to be passed as a param. This slot will 
    be invoked when the user does the action like OK press or CANCEL press.  
    
    HbInputDialog::getDouble(iTitle,this,SLOT(dialogClosed(HbAction*)),iText);  iTitle is the prompt text.dialogClosed will be invoked when the user does the action.
    Please refer the class documentation to know how to handle the slot.
   
    \param label The prompt text which gives information on user input field.
    \param receiver the instance where the slot is declared.
    \param member the slot which has dialogClosed(HbAction*) signature. 
    \param value the default value that should be presented to the user.
    \param scene the scene parameter.
    \param parent the parent widget for the dialog.
   
    \sa getText(), getInteger(), getIp()
*/
void HbInputDialog::getDouble(const QString &label, 
                                QObject *receiver,
                                const char *member,
                                double value, 
                                QGraphicsScene *scene, 
                                QGraphicsItem *parent)
{
    HbInputDialog *dlg = new HbInputDialog(parent);
    if(scene && !parent) {
        scene->addItem(dlg);    
    }
    dlg->setPromptText(label);
    dlg->setInputMode(RealInput);
    dlg->setValue(QString::number(value));
    dlg->open(receiver,member);
}


/*!
    This is a static convenience function for creating an Input Dialog and to get an IP information from the the user. This API allows a slot to be passed as a param. This slot will 
    be invoked when the user does the action like OK press or CANCEL press.  
    
    HbInputDialog::getIp(iTitle,this,SLOT(dialogClosed(HbAction*)),iText);  iTitle is the prompt text.dialogClosed will be invoked when the user does the action.
    Please refer the class documentation to know how to handle the slot.
   
    \param label The prompt text which gives information on user input field.
    \param receiver the instance where the slot is declared.
    \param member the slot which has dialogClosed(HbAction*) signature. 
    \param ipaddress the default value that should be presented to the user.
    \param scene the scene parameter.
    \param parent the parent widget for the dialog.
   
    \sa getText(), getInteger(), getDouble()
*/

void HbInputDialog::getIp(const QString &label, 
                            QObject *receiver,
                            const char *member,
                            const QString &ipaddress, 
                            QGraphicsScene *scene, 
                            QGraphicsItem *parent)
{
    HbInputDialog *dlg = new HbInputDialog(parent);
    if(scene && !parent) {
        scene->addItem(dlg);    
    }
    dlg->setPromptText(label);
    dlg->setValue(ipaddress);
    dlg->setInputMode(IpInput);    
    dlg->open(receiver,member);
}
#include "moc_hbinputdialog.cpp"

