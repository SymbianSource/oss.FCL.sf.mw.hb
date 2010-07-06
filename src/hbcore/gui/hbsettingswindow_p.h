/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbCore module of the UI Extensions for Mobile.
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

#ifndef HBSETTINGSWINDOW_P_H
#define HBSETTINGSWINDOW_P_H

#include <QWidget>
#include "hbmainwindoworientation_p.h"

QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QPushButton)

class HbSettingsWindow : public QWidget
{
    Q_OBJECT

public:
    static HbSettingsWindow *instance();
    ~HbSettingsWindow();

public slots:
    void refresh();

private slots:
    void changeWindow(int index);
    void changeResolution(int index);
    void changeDirection(int index);
    void changeTouchArea(int index);
    void changeTextBoxes(int index);
    void changeIconBoxes(int index);
    void changeFpsCounter(int index);
    void changeDragToResize(int index);
    void changeSensorValue(int index);
    void changeGSettingsForSensors(int index);
    void unsetOrientation();
    void toggleLights();
    void toggleAnimation();
    void resizeViewPort();
    void toggleWindowObscured();

private:
    void initStartUpValues();

private:
    explicit HbSettingsWindow(QWidget *parent = 0);
    QComboBox *windowComboBox;
    QComboBox *resolutionComboBox;
    QComboBox *orientationComboBox;
    QComboBox *directionComboBox;
    QComboBox *touchAreaComboBox;
    QComboBox *textBoxesComboBox;
    QComboBox *iconBoxesComboBox;
    QComboBox *fpsCounterComboBox;
    QComboBox *dragToResizeComboBox;
    QComboBox *mSensorComboBox;
    QComboBox *mGeneralSettingsForSensorsComboBox;
    QPushButton *mUnsetOrientationButton;
    QPushButton *mLightsButton;
    QPushButton *mAnimationButton;
    QPushButton *mViewPortSizeButton;
    QPushButton *mWindowObscuredButton;
    bool mLights;
    bool mAnimation;
    bool mCustomViewPortSize;
    bool mWindowObscured;
    QSizeF mViewPortOriginalSize;

#ifdef HB_CSS_INSPECTOR
    QPushButton *cssWindowButton;
#endif
};

#endif // HBSETTINGSWINDOW_P_H
