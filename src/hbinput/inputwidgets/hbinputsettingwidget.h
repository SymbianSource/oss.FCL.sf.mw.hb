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

#ifndef HB_INPUT_SETTING_WIDGET_H
#define HB_INPUT_SETTING_WIDGET_H

#include <QObject>
#include <hbinputdef.h>

class HbInputSettingWidgetPrivate;
class QGraphicsWidget;
class QModelIndex;
class HbDataForm;
class HbDataFormModelItem;
class HbInputLanguage;

class HB_INPUT_EXPORT HbInputSettingWidget : public QObject
{
    Q_OBJECT

public:
    explicit HbInputSettingWidget(HbDataForm *dataForm, QGraphicsWidget *parent = 0);
    virtual ~HbInputSettingWidget();

    void initializeWidget();
    
    void resetWidget();

public slots:
    void updateGlobalInputLanguage(const HbInputLanguage &newLanguage);
    void updateGlobalSecondaryInputLanguage(const HbInputLanguage &newLanguage);
    void updatePredictiveInputState(HbKeyboardSettingFlags keyboardType, bool newState);
    void updateCharacterPreviewStateForQwerty(bool newState);
    void updateKeypressTimeout(int newTimeout);
    void updateAutocompletionState(HbKeyboardSettingFlags keyboardType, bool newState);
    void updateTypingCorrectionLevel(HbTypingCorrectionLevel newLevel);
    void updatePrimaryCandidateMode(HbPrimaryCandidateMode newMode);

    void setPrimaryLanguage(int index);
    void setSecondaryLanguage(int index);
    void setKeypressTimeoutValue(int value);
    void setCharacterPreviewState();
    void setPredictionState(const QModelIndex &index);
    void setAutocompletionState(const QModelIndex &index);
    void setCorrectionLevel(int index);
    void setPrimaryCandidateMode();
    void dataChange(const QModelIndex &startIn, const QModelIndex &endIn);

protected:
    HbInputSettingWidgetPrivate *const d_ptr;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputSettingWidget)
    Q_DISABLE_COPY(HbInputSettingWidget)
};

#endif // HB_INPUT_SETTING_WIDGET_H

// End of file
