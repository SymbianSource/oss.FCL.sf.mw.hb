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

#ifndef HBINPUTDIALOG_P_H
#define HBINPUTDIALOG_P_H

#include "hbdialog_p.h"
#include <hbinputdialog.h>

#ifdef HB_EFFECTS
#define HB_INPUTDIALOG_ITEM_TYPE "HB_INPUTDIALOG"
#endif

class HbInputDialogContentWidget;
class HbInputCustomButtonBank;
class HbInputCustomButton;
class HbValidator;
class HbLineEdit;

class HbInputDialogPrivate : public HbDialogPrivate
{
    Q_DECLARE_PUBLIC(HbInputDialog)

public:
    HbInputDialogPrivate();
    ~HbInputDialogPrivate();

    void init();

    void setInputMode(HbLineEdit *pEdit, HbInputDialog::InputMode mode);
    void setInputMode(HbInputDialog::InputMode mode,int row);
    void setPromptText(const QString& text,int row);
    QString promptText(int row) const;
    void setText(const QString& text,int row);
    QString text(int row)const;
    void setAdditionalRowVisible(bool bVisible);
    bool isAdditionalRowVisible()const;
    void _q_notesOrientationChanged(Qt::Orientation);//private slot

public:
    HbInputDialogContentWidget *mContentWidget;
    HbInputCustomButtonBank *mCustomButtonBank;
    HbInputCustomButton *mDotButton;
    HbInputCustomButton *mDashButton;
    QPointer<HbValidator> mValid;
    
    HbInputDialog::InputMode mPrimaryMode;
    HbInputDialog::InputMode mSecondaryMode;
    HbLineEdit::EchoMode mEchoMode;//echo mode of second row
    QString mPromptText;
    QString mPromptAdditionalText;
    QString mText;
    HbAction *action1;
public slots:
    void textChange(const QString);
};

#endif //HBINPUTDIALOGPRIVATE_H

