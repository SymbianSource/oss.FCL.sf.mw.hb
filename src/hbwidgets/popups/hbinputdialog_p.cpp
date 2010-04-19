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

#include "hbinputdialog_p.h"
#include "hbinputdialogcontent_p.h"
#include "hbstyleoptioninputdialog.h"

#include <hblineedit.h>
#include <hbaction.h>
#include <hbvalidator.h>
#include <hbstyle.h>
#include <hbinputeditorinterface.h>
#include <hbinputsettingproxy.h>
#include <hbmainwindow.h>

//#define HBINPUTDIALOG_DEBUG
#ifdef HBINPUTDIALOG_DEBUG
#include <QtDebug>
#endif

HbInputDialogContentWidget::HbInputDialogContentWidget(HbInputDialogPrivate* priv,QGraphicsItem* parent) : 
    HbWidget(parent),
    d(priv),
    mLabel1(0),
    mLabel2(0),
    mEdit2(0),
    mAdditionalRowVisible(false)
{
    mLabel1 = style()->createPrimitive(HbStyle::P_InputDialog_text,this);
    mEdit1 = new HbLineEdit(this);
    HbStyle::setItemName(mEdit1, "text-1");

    this->setProperty("additionalRowVisible",QVariant(false));
}


void HbInputDialogContentWidget::setAdditionalRowVisible(bool visible)
{
    mAdditionalRowVisible = visible;

    if(!mLabel2 && visible) {
        mLabel2 = style()->createPrimitive(HbStyle::P_InputDialog_additional_text,this);
    }

    if(!mEdit2 && visible) {
        //Retrieve the cached data here and assign//
        mEdit2 = new HbLineEdit(this);
        mEdit2->setText(d->mText);
        mEdit2->setEchoMode(d->mEchoMode);
        //Retrieve the cached  data here//
        d->setInputMode(mEdit2, d->mSecondaryMode);
        HbStyle::setItemName(mEdit2, "text-2");
        this->setProperty("additionalRowVisible",QVariant(true));
    }
    if(!visible) {
        if(mEdit2) {
            delete mEdit2; 
            mEdit2 = 0;
        }
        this->setProperty("additionalRowVisible",QVariant(false));
    }
    repolish();
}


HbInputDialogPrivate::HbInputDialogPrivate() :
    mCustomButtonBank(0),
    mDotButton(0),
    mDashButton(0),
    mValid(0),
    mSecondaryMode(HbInputDialog::TextInput),
    mEchoMode(HbLineEdit::Normal),
    mPromptText(),
    mPromptAdditionalText(),
    mText()
{
}


HbInputDialogPrivate::~HbInputDialogPrivate()
{
    if(primaryAction){
        delete primaryAction;
    }
    if(secondaryAction){
        delete secondaryAction;
    }
}


void HbInputDialogPrivate::init()
{
#ifdef HBINPUTDIALOG_DEBUG
    qDebug()<<" Entering init()";
#endif
    Q_Q(HbInputDialog);

    mPrimaryMode = HbInputDialog::TextInput; //Default input mode is text input

    //Populate the widget
    mContentWidget = new HbInputDialogContentWidget(this);

    q->setContentWidget(mContentWidget);
    primaryAction = new HbAction(QString(q->tr("Ok")));
    q->setPrimaryAction(primaryAction);

    secondaryAction = new HbAction(QString(q->tr("Cancel")));
    q->setSecondaryAction(secondaryAction);

    q->setTimeout(HbPopup::NoTimeout); 
    q->setModal(true); // Dialog is modal  
    q->setDismissPolicy(HbPopup::NoDismiss);

    QObject::connect( q->mainWindow(), 
                    SIGNAL( orientationChanged(Qt::Orientation )), 
                    q, 
                    SLOT( _q_notesOrientationChanged(Qt::Orientation) ) );
}


void HbInputDialogPrivate::setInputMode(HbLineEdit *pEdit, HbInputDialog::InputMode mode)
{
    Q_Q(HbInputDialog);
#ifdef HBINPUTDIALOG_DEBUG
    qDebug()<<"Entering setInputMode";
#endif
    HbEditorInterface eInt(pEdit);
    switch(mode) {
    case HbInputDialog::TextInput:
#ifdef HBINPUTDIALOG_DEBUG
        qDebug()<<"TextInputMode";
#endif
        eInt.setConstraints(HbEditorConstraintNone);
        break;
    case HbInputDialog::IntInput: 
    {
#ifdef HBINPUTDIALOG_DEBUG
        qDebug()<<"IntInputMode";
#endif
        //set the validator
        if(mValid) {
            // NOTE:This validation is for readability. mValid is being deleted 
            // when setValidator is called on editor.
            mValid = 0;
        }
        mValid = new HbValidator();
        QValidator *intValidator = new QIntValidator(q);
        mValid->addField(intValidator, "0");
        pEdit->setValidator(mValid);

        eInt.setConstraints(HbEditorConstraintFixedInputMode);
        eInt.setInputMode(HbInputModeNumeric);
        //eInt.setFilter(HbConverterNumberFilter::instance());
/*          Old custom button API has been deprecated for some time now. Commented out to prevent a build break.
             customButtonBank = HbInputCustomButtonStore::instance()->newBank();
        1, intValidator->locale().negativeSign(), intValidator->locale().negativeSign());
        mCustomButtonBank->addButton(mDashButton);
            dashButton = new HbInputCustomButton(HbInputCustomButton::HbCustomButtonShortcut,
                1, intValidator->locale().negativeSign(), intValidator->locale().negativeSign());
            customButtonBank->addButton(dashButton);
            eInt.setCustomButtonBank(customButtonBank->id());  */

        break;
    }
    case HbInputDialog::RealInput:
    {
#ifdef HBINPUTDIALOG_DEBUG
        qDebug()<<"RealInputMode";
#endif
        //set the validator
        if(mValid) {
            mValid = 0;
        }
            
        mValid = new HbValidator();
        QValidator *doubleValidator = new QDoubleValidator(q);
        mValid->addField(doubleValidator, "0");
        pEdit->setValidator(mValid);
        
        eInt.setConstraints(HbEditorConstraintFixedInputMode);
        eInt.setInputMode(HbInputModeNumeric);
        //eInt.setFilter(HbConverterNumberFilter::instance());
/*          Old custom button API has been deprecated for some time now. Commented out to prevent a build break.
            customButtonBank = HbInputCustomButtonStore::instance()->newBank();
            dotButton = new HbInputCustomButton(HbInputCustomButton::HbCustomButtonShortcut,
                0, doubleValidator->locale().decimalPoint(), doubleValidator->locale().decimalPoint());
            customButtonBank->addButton(dotButton);
            dashButton = new HbInputCustomButton(HbInputCustomButton::HbCustomButtonShortcut,
                1, doubleValidator->locale().negativeSign(), doubleValidator->locale().negativeSign());
            customButtonBank->addButton(dashButton);
            eInt.setCustomButtonBank(customButtonBank->id()); */
            
        break;
    }                     
    case HbInputDialog::IpInput:
    {
        mValid = new HbValidator;
        QRegExp r("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}");
        mValid->setMasterValidator(new QRegExpValidator(r,0));
        mValid->addField(new QIntValidator(0,255,0),"127");
        mValid->addSeparator(QString("."));
        mValid->addField(new QIntValidator(0,255,0),"0");
        mValid->addSeparator(QString("."));
        mValid->addField(new QIntValidator(0,255,0),"0");
        mValid->addSeparator(QString("."));
        mValid->addField(new QIntValidator(0,255,0),"1");

        pEdit->setValidator(mValid);
        eInt.setInputMode(HbInputModeNumeric);

        break;
    }  
    default:
        break;
    }
}


void HbInputDialogPrivate::setInputMode(HbInputDialog::InputMode mode,int row)
{
    if(row > 1) {
        return;
    }
    if(row == 0) {
        mPrimaryMode = mode;
	    setInputMode(mContentWidget->mEdit1, mode);
    } else {
        mSecondaryMode = mode;
        if(mContentWidget->mAdditionalRowVisible) {
            setInputMode(mContentWidget->mEdit2,mode);
        }
    }
}


void HbInputDialogPrivate::setPromptText(const QString& text,int row)
{
    Q_Q(HbInputDialog);
    if(row > 1) {
	    return;
    }
    HbStyleOptionInputDialog option;
    if(row == 0) {
        mPromptText = text;
        q->initStyleOption(&option);
        q->style()->updatePrimitive(mContentWidget->mLabel1,HbStyle::P_InputDialog_text,&option);
    } else {
        mPromptAdditionalText = text;
        q->initStyleOption(&option);
        if(mContentWidget->mAdditionalRowVisible) {
           q->style()->updatePrimitive(mContentWidget->mLabel2,HbStyle::P_InputDialog_additional_text,&option);
        }
    }
}


QString HbInputDialogPrivate::promptText(int row) const
{
    if( row > 1 ) {
        return QString();
    }
    if(row == 0) {
	    return mPromptText;
    } else {
        return mPromptAdditionalText;
    }
}


void HbInputDialogPrivate::setText(const QString& text,int row)
{
    if(row > 1) {
        return;
    }
    if(row == 0) {
	    mContentWidget->mEdit1->setText(text);
    } else {
        mText = text;
        if(mContentWidget->mAdditionalRowVisible) {
            mContentWidget->mEdit2->setText(mText);
        }
    }
}


QString HbInputDialogPrivate::text(int row) const
{
    if(row > 1) {
        return QString();
    }
    if(row == 0) {
        return mContentWidget->mEdit1->text();
    } else {
        if(mContentWidget->mEdit2) {
            return mContentWidget->mEdit2->text();
        } else {
            return mText;
        }
    }
}


void HbInputDialogPrivate::setAdditionalRowVisible(bool visible)
{
    mContentWidget->setAdditionalRowVisible(visible);
}


bool HbInputDialogPrivate::isAdditionalRowVisible()const
{
    return mContentWidget->mAdditionalRowVisible;
}

void HbInputDialogPrivate::_q_notesOrientationChanged(Qt::Orientation)
{
    Q_Q(HbInputDialog);
    q->repolish();
}
