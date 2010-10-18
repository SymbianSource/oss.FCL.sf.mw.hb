/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbApps module of the UI Extensions for Mobile.
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

#ifndef SETTINGSVIEW_H
#define SETTINGSVIEW_H

#include <hbview.h>

class HbMainWindow;
class HbDocumentLoader;
class HbComboBox;
class HbLabel;
class HbScrollArea;

class SettingsView : public HbView
{
    Q_OBJECT

public:
    SettingsView(HbDocumentLoader* loader, HbMainWindow *window, HbView *infoView);

private:
    void setupMenu();
    void setupLayout();
    void connectSignals();
    void disconnectSignals();
    
    void sendEvent(const int eventID, const int data=0);

private slots:
    void changeOrientation();
    void changeResolution(int index);
    void changeOrientation(int index);
    void changeLayoutDirection(int index);
    void changeTouchAreaVisibility(int index);
    void changeTextItemVisibility(int index);
    void changeIconItemVisibility(int index);
    void changeLocMode(int index);
    void changeTestUtilMode(int index);
    void changeFpsCounterVisibility(int index);

    void updateLayout();
    void updateSelections();
    
    void goBack();

private:
    HbMainWindow *mWindow;
    HbDocumentLoader* mLoader;
    HbComboBox* mResolutionComboBox;
    HbComboBox* mOrientationComboBox;
    HbComboBox* mLayoutDirectionComboBox;
    HbComboBox* mTouchAreaComboBox;
    HbComboBox* mTextItemComboBox;
    HbComboBox* mIconItemComboBox;
    HbComboBox* mLocalizationComboBox;
    HbComboBox* mTestUtilityComboBox;
    HbComboBox* mFpsCounterComboBox;
    HbView* mInfoView;
};


#endif // SETTINGSVIEW_H

