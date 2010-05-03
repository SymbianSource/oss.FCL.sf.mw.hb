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
#include <hbstyleoptioninputdialog.h>
#include <hbvalidator.h>

#include <QGraphicsScene>

#ifdef HBINPUTDIALOG_DEBUG
#include <QtDebug>
#endif


/*!
    @beta
    @hbwidgets

    \class HbInputDialog
    \brief A class used to get information from the user.

    the input value can be a text, int, double or ip address.

    example code example:
    \code
    HbInputDialog *object = new HbInputDialog(parent);
    object->show();
    \endcode
    
    Four static convenience API's are provided: getText(), getInteger(), getDouble(), and getIp()
    static API's can be used to quickly get an input from user.
 */


/*!
    Constructor of HbInputDialog
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
    @beta
    Sets the input mode of the primary(Top/default)line edit in the query widget.
    
    \param mode. InputMode can be TextMode, IntMode, RealMode and Ip address mode.
    each mode will affect how the line edit filters its input.
    
    \param row. value 0 or 1
    
    \sa inputMode() 
*/	
void HbInputDialog::setInputMode(InputMode mode ,int row)
{
    Q_D(HbInputDialog);
    d->setInputMode(mode,row);
}


/*!
    @beta
    Returns input mode for top/default line edit.
    The default InputMode is TextInput
    \param row. value 0 or 1

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
    @beta
    Sets the prompt \a text for top/default line edit.

    \param text. user defined text
    \param row. value 0 or 1

    \sa promtText()
*/
void HbInputDialog::setPromptText(const QString &text, int row)
{
    Q_D(HbInputDialog);
    d->setPromptText(text, row);
}


/*!
    @beta
    Returns prompt text for top/default line edit.
    the default is null string.
    \param row. value 0 or 1

    \sa setPromptText()
*/
QString HbInputDialog::promptText(int row) const
{
    Q_D(const HbInputDialog);
    return d->promptText(row);
}

/*!
    @beta
    Sets the top/default line edit value in \a text format.

    \param value. user defined value
    \param row. value 0 or 1

    \sa value()
*/
void HbInputDialog::setValue(const QVariant &value,int row)
{
    Q_D(HbInputDialog);
    d->setText(value.toString(),row);
}

/*!
    @beta
    Returns top/default line edit value as QVariant object.

    \param row. value 0 or 1

    \sa setValue()
*/
QVariant HbInputDialog::value(int row) const
{
    Q_D(const HbInputDialog);
    return QVariant(d->text(row));
}

/*!
    @beta
    Sets the visibility of bottom line edit and prompt text.

    \param visible true or false.

    \sa isAdditionalRowVisible()
*/
void HbInputDialog::setAdditionalRowVisible(bool visible)
{
    Q_D(HbInputDialog);
    d->setAdditionalRowVisible(visible);
}

/*!
    @beta
    Returns the visibility of secondary row(bottom line edit and prompt text).
    the default is false
    \sa setAdditionalRowVisible()
*/
bool HbInputDialog::isAdditionalRowVisible()
{
    Q_D(HbInputDialog);
    return d->isAdditionalRowVisible();
}


/*!
    @beta
    Validator is used to validate the content and cursor movements.

    \param validator. Validator uses undo stack to back out invalid changes. Therefore undo
    is enabled when validator is set.

    \sa HbAbstractEdit::setValidator
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
    @beta
    returns the validator of the inputDialog's line edit.

    \param row. A value 0 or 1

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
    @beta
    returns the lineEdit pointer. will return NULL if row is greater than 2.

    \param row. A value 0 or 1
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
    @beta
    sets the echo mode for the given row.

    \param echoMode
    \param row. A value 0 or 1

    \sa HbLineEdit::setEchoMode
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
    @beta
    \reimp
    Initializes \a option with the values from this HbInputDialog.
*/
void HbInputDialog::initStyleOption(HbStyleOptionInputDialog *option) const
{
    Q_D(const HbInputDialog);
    HbDialog::initStyleOption(option);
    option->text = d->mPromptText;
    option->additionalText = d->mPromptAdditionalText;
}


/*!
    @beta
    updatePrimitives.
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
    @beta
    returns the echoMode of line edit. returns -1 if row is more than 2.

    \param row. A value 0 or 1

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
    @beta
    Static convenience function to get a string from the user. \a
    label is the text which is shown to the user (it should
    say what should be entered). \a text is the default text which is
    placed in the line edit. If \a ok is non-null \e *\a ok will be 
    set to true if the user pressed \gui OK and to false if the user pressed
    \gui Cancel. The dialog's parent is \a parent. The dialog will be
    modal.

    This function return data has to be queried in the finished(HbAction*) slot.

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
    \deprecated HbInputDialog::getText(const QString&,const QString&,bool*,QGraphicsScene*,QGraphicsItem*)
        is deprecated. Please use the other available getText API.

    Static convenience function to get a string from the user. \a
    label is the text which is shown to the user (it should
    say what should be entered). \a text is the default text which is
    placed in the line edit. If \a ok is non-null \e *\a ok will be 
    set to true if the user pressed \gui OK and to false if the user pressed
    \gui Cancel. The dialog's parent is \a parent. The dialog will be
    modal.

    This function returns the text which has been entered in the line
    edit. It will not return an empty string.

    \sa getInteger(), getDouble(), getIp()
*/
QString HbInputDialog::getText(const QString &label,const QString &text,
                                bool *ok, QGraphicsScene *scene, QGraphicsItem *parent)
{
    HB_DEPRECATED("HbInputDialog::getText is deprecated. Please use HbInputDialog::getText((const QString&, QObject*, const char*, const QString&, QGraphicsScene*, QGraphicsItem*) instead.");
    Q_UNUSED(parent);//this is a bug in popup framework, cannot set parent will affect fading

    HbInputDialog *dlg = new HbInputDialog();
    if (scene) {
        scene->addItem(dlg);
    }
    QString result;
    dlg->setPromptText(label);
    dlg->setInputMode(TextInput);
    dlg->setValue(text);	
    HbAction* action = dlg->exec();
    if(action == dlg->secondaryAction()) { //Cancel was pressed
        if(ok) {
			*ok = false;
        }
    } else { //OK was pressed
        if(ok) {
			*ok = true;
        }
        result = dlg->value().toString();
    }
    delete dlg;
    return result;	
}


/*!
    @beta
    Static convenience function to get an integer input from the
    user.\a label is the text which is shown to the user
    (it should say what should be entered). \a value is the default
    integer which the spinbox will be set to.  
    If \a ok is non-null *\a ok will be set to true if the user
    pressed \gui OK and to false if the user pressed \gui Cancel. The
    dialog's parent is \a parent. The dialog will be modal.

    This function return data has to be queried in the finished(HbAction*) slot.

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
     \deprecated HbInputDialog::getInteger(const QString&,int,bool*,QGraphicsScene*,QGraphicsItem*)
        is deprecated. Please use the other available HbInputDialog::getInteger API.

    Static convenience function to get an integer input from the
    user.\a label is the text which is shown to the user
    (it should say what should be entered). \a value is the default
    integer which the spinbox will be set to.  
    If \a ok is non-null *\a ok will be set to true if the user
    pressed \gui OK and to false if the user pressed \gui Cancel. The
    dialog's parent is \a parent. The dialog will be modal.

    This function returns the integer which has been entered by the user.

    \sa getText(), getDouble(), getIp()
*/
int HbInputDialog::getInteger(const QString &label, int value,
                            bool *ok,QGraphicsScene *scene,QGraphicsItem *parent)
{
    HB_DEPRECATED("HbInputDialog::getInteger is deprecated. Please use HbInputDialog::getInteger(const QString&, QObject*, const char*,int,QGraphicsScene*,QGraphicsItem *) instead.");

    Q_UNUSED(parent)

    HbInputDialog *dlg = new HbInputDialog();
    QString result;
    dlg->setPromptText(label);
    dlg->setInputMode(IntInput);
    dlg->setValue(QString::number(value));
    if(scene) {
        scene->addItem(dlg);
    }
    HbAction* action = dlg->exec();
    if(action == dlg->secondaryAction()) { //Cancel was pressed
        if(ok) {
            *ok = false;
        }
    } else { //OK was pressed
        if(ok) {
			*ok = true;
        }
	    result = dlg->value().toString();
    }
    delete dlg;
#ifdef HBINPUTDIALOG_DEBUG
    qDebug()<<"Value:"<<result;
#endif
    return result.toInt();
}


/*!
    Static convenience function to get a floating point number from
    the user.\a label is the text which is shown to the user
    (it should say what should be entered). \a value is the default
    floating point number that the line edit will be set to.

    If \a ok is non-null, *\a ok will be set to true if the user
    pressed \gui OK and to false if the user pressed \gui Cancel. The
    dialog's parent is \a parent. The dialog will be modal.

    This function return data has to be queried in the finished(HbAction*) slot.

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
    \deprecated HbInputDialog::getDouble(const QString &,double,bool*,QGraphicsScene*,QGraphicsItem*)
        is deprecated. Please use the other available HbInputDialog::getDouble(...) API.

    Static convenience function to get a floating point number from
    the user.\a label is the text which is shown to the user
    (it should say what should be entered). \a value is the default
    floating point number that the line edit will be set to.

    If \a ok is non-null, *\a ok will be set to true if the user
    pressed \gui OK and to false if the user pressed \gui Cancel. The
    dialog's parent is \a parent. The dialog will be modal.

    This function returns the floating point number which has been
    entered by the user.

    \sa getText(), getInteger(), getIp()
*/
double HbInputDialog::getDouble(const QString &label, double value, 
                                bool *ok, QGraphicsScene *scene, QGraphicsItem *parent)
{
    HB_DEPRECATED("HbInputDialog::getDouble is deprecated.Please use HbInputDialog::getDouble(const QString&, QObject*,const char*,double, QGraphicsItem*) instead.");

    Q_UNUSED(parent)

    HbInputDialog *dlg = new HbInputDialog();
    QString result;
    dlg->setPromptText(label);
    dlg->setInputMode(RealInput);
    dlg->setValue(QString::number(value));
    if(scene) {
        scene->addItem(dlg);	
    }
    HbAction* action = dlg->exec();
    if(action == dlg->secondaryAction()){ //Cancel was pressed
        if(ok) {
			*ok = false;
        }
    } else { //OK was pressed
        if(ok) {
            *ok = true;
        }
        result = dlg->value().toString();
    }
    delete dlg;
#ifdef HBINPUTDIALOG_DEBUG
    qDebug()<<"Value:"<<result;
#endif
    return result.toDouble();
}

/*!
    @beta
    Static convenience function to get a ip address from
    the user.\a label is the text which is shown to the user
    (it should say what should be entered). \a address is the default
    QHostAddress that the line edit will be set to.

    If \a ok is non-null, *\a ok will be set to true if the user
    pressed \gui OK and to false if the user pressed \gui Cancel. The
    dialog's parent is \a parent. The dialog will be modal.

    This function return data has to be queried in the finished(HbAction*) slot.

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
    dlg->setInputMode(IpInput);
    dlg->setValue(ipaddress);
    dlg->open(receiver,member);
}


/*!
    \deprecated HbInputDialog::getIp(const QString &,const QString&, bool *,QGraphicsScene*,QGraphicsItem*)
        is deprecated. Please use the other available HbInputDialog::getIp API.

    Static convenience function to get a ip address from
    the user.\a label is the text which is shown to the user
    (it should say what should be entered). \a address is the default
    QHostAddress that the line edit will be set to.

    If \a ok is non-null, *\a ok will be set to true if the user
    pressed \gui OK and to false if the user pressed \gui Cancel. The
    dialog's parent is \a parent. The dialog will be modal.

    This function returns the ip address number which has been
    entered by the user.

    \sa getText(), getInteger(), getDouble()
*/
QString HbInputDialog::getIp(const QString &label, const QString &ipaddress, bool *ok, QGraphicsScene *scene, 
                                    QGraphicsItem *parent)
{
    HB_DEPRECATED("HbInputDialog::getIp is deprecated. Please use HbInputDialog::getIp(const QString &,QObject *,const char *,const QString &, QGraphicsScene *, QGraphicsItem *)");

    Q_UNUSED(parent)

    HbInputDialog *dlg = new HbInputDialog();
    QString result;
    dlg->setPromptText(label);
    dlg->setInputMode(IpInput);
    dlg->setValue(ipaddress);
    if(scene) {
        scene->addItem(dlg);	
    }
    HbAction* action = dlg->exec();
    if(action == dlg->secondaryAction()){ //Cancel was pressed
        if(ok) {
            *ok = false;
        }
    } else { //OK was pressed
        if(ok) {
            *ok = true;
        }
        result = dlg->value().toString();
    }
    delete dlg;
#ifdef HBINPUTDIALOG_DEBUG
    qDebug()<<"Value:"<<result;
#endif
    return result;
}
#include "moc_hbinputdialog.cpp"

