/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbInput module of the UI Extensions for Mobile.
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

#ifndef HB_INPUT_SETTING_DIALOG_H
#define HB_INPUT_SETTING_DIALOG_H

#include <hbdialog.h>

class HbInputSettingDialogPrivate;

class HB_INPUT_EXPORT HbInputSettingDialog : public HbDialog
{
    Q_OBJECT

public:
    enum HbSettingItem
    {
        HbSettingItemNone          = 0x00000000,  //to create none
        HbSettingItemWritingLang   = 0x00000001,
        HbSettingItemSecondaryLang = 0x00000002,
        HbSettingItemPrediction    = 0x00000004,
        HbSettingItemAll           = 0xFFFFFFFF   //to create all
    };
    Q_DECLARE_FLAGS(HbSettingItems, HbSettingItem)

public:
    HbInputSettingDialog(HbSettingItems items = HbSettingItemAll, QGraphicsWidget* parent = 0);
    virtual ~HbInputSettingDialog();

public slots:
    void selected();
    void settingItemDisplayed(const QModelIndex &index); //deprecated
    void primaryLanguageChanged(int index);
    void secondaryLanguageChanged(int index);
    void predictionStatusChanged();

protected:
    void showEvent(QShowEvent *event);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputSettingDialog)
    Q_DISABLE_COPY(HbInputSettingDialog)
};

#endif // HB_INPUT_SETTING_DIALOG_H

// End of file
