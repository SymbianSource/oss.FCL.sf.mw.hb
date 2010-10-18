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
#include "hbstyleoptioninputdialog_p.h"
#include <hbstyletextprimitivedata.h>
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
    mLabel1 = style()->createPrimitive(HbStyle::PT_TextItem, "label-1", this);
    mEdit1 = new HbLineEdit(this);
    HbStyle::setItemName(mEdit1, "text-1");

    connect(mEdit1,SIGNAL(textChanged(const QString)),this,SLOT(emitTextChange(const QString)));
    this->setProperty("additionalRowVisible",QVariant(false));
}

void HbInputDialogContentWidget::emitTextChange(const QString & data)
{
    emit textChanged(data);
}
void HbInputDialogContentWidget::setAdditionalRowVisible(bool visible)
{
    mAdditionalRowVisible = visible;

    if(!mLabel2 && visible) {
        mLabel2 = style()->createPrimitive(HbStyle::PT_TextItem, "label-2", this);
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

    q->connect(mContentWidget,SIGNAL(textChanged(const QString)),q,SLOT(textChange(const QString)));
    q->setContentWidget(mContentWidget);

    action1=new HbAction(hbTrId("txt_common_button_ok"),q);
    action1->setEnabled(false);
    q->addAction(action1);
    q->connect(action1,SIGNAL(triggered()),q,SLOT(accept()));

    HbAction *action2=new HbAction(hbTrId("txt_common_button_cancel"),q);
    q->addAction(action2);
    q->connect(action2,SIGNAL(triggered()),q,SLOT(reject()));

    q->setTimeout(HbPopup::NoTimeout); 
    q->setModal(true); // Dialog is modal  
    q->setDismissPolicy(HbPopup::NoDismiss);

    QObject::connect( q->mainWindow(), 
                    SIGNAL( orientationChanged(Qt::Orientation )), 
                    q, 
                    SLOT( _q_notesOrientationChanged(Qt::Orientation) ) );
}

void HbInputDialogPrivate::textChange(const QString data)
{
    Q_UNUSED(data);
    Q_Q(HbInputDialog);

    if(q->actions().count() == 0) {
        return;
    }

    if(mContentWidget->mEdit1->hasAcceptableInput()) {
        q->actions().at(0)->setEnabled(true);
    } else {
        q->actions().at(0)->setEnabled(false);
    }

}

void HbInputDialogPrivate::setInputMode(HbLineEdit *pEdit, HbInputDialog::InputMode mode)
{
    Q_Q(HbInputDialog);

    switch(mode) {
    case HbInputDialog::TextInput:
        break;

    case HbInputDialog::IntInput: 
        {
            //set the validator
            if(mValid) {
                delete mValid;
            }
            mValid = new HbValidator(q);
            QValidator *intValidator = new QIntValidator;
            mValid->addField(intValidator, "0");
            pEdit->setValidator(mValid);

            pEdit->setInputMethodHints(Qt::ImhDigitsOnly);

            break;
        }
    case HbInputDialog::RealInput:
        {
            //set the validator
            if(mValid) {
                delete mValid;
            }

            mValid = new HbValidator(q);
            QValidator *doubleValidator = new QDoubleValidator(mValid);
            mValid->addField(doubleValidator, "0");
            pEdit->setValidator(mValid);

            pEdit->setInputMethodHints(Qt::ImhFormattedNumbersOnly);

            break;
        }                     
    case HbInputDialog::IpInput:
        {
            QString text = pEdit->text();
            if(mValid){
                delete mValid;
            }
            mValid = new HbValidator(q);
            mValid->setDefaultSeparator(".");
            QStringList list = text.split('.');
            if (list.count() != 4 ) {
                    mValid->setDefaultSeparator(".");
                    mValid->addField(new QIntValidator(0, 255, 0), "127");
                    mValid->addField(new QIntValidator(0, 255, 0), "0");
                    mValid->addField(new QIntValidator(0, 255, 0), "0");
                    mValid->addField(new QIntValidator(0, 255, 0), "1");

            }
            else {

                    mValid->addField(new QIntValidator(0, 255, 0),list[0]);
                    mValid->addField(new QIntValidator(0, 255, 0),list[1]);
                    mValid->addField(new QIntValidator(0, 255, 0),list[2]);
                    mValid->addField(new QIntValidator(0, 255, 0),list[3]);
            }
            pEdit->setValidator(mValid);
            pEdit->setInputMethodHints(Qt::ImhDigitsOnly);

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
        HbStyleTextPrimitiveData data;
        q->initPrimitiveData(&data, mContentWidget->mLabel1);
        q->style()->updatePrimitive(mContentWidget->mLabel1, &data, mContentWidget);
    } 
    else {
        mPromptAdditionalText = text;
        HbStyleTextPrimitiveData data;
        q->initPrimitiveData(&data, mContentWidget->mLabel2);
        q->style()->updatePrimitive(mContentWidget->mLabel2, &data, mContentWidget);
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
