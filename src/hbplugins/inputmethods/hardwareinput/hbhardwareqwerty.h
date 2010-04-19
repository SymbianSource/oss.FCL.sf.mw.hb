/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbPlugins module of the UI Extensions for Mobile.
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
#ifndef _HB_HARDWARE_QWERTY_H 
#define _HB_HARDWARE_QWERTY_H 

#include <QPointer>

#include <hbinputmethod.h>
#include <hbinputdef.h>
#include <hbinputhwtoolcluster.h>
#include <hbinputvkbhost.h>


class HbKeyMapFactory;
class HbKeyMapData;
class HbExactWordPopup;
class HbCandidateList;
class HbCharPreviewPane;

class HbInputModeHandler;
class HbHardwareInputBasicQwertyHandler;
class HbHardwareInputPredictionQwertyHandler;
class HbHardwareInputNumericQwertyHandler;

class HbHardwareQwerty : public HbInputMethod
{
Q_OBJECT

public: //construction
    HbHardwareQwerty();
    ~HbHardwareQwerty();

public: // From QInputContext
    QString identifierName();
    bool isComposing() const;
    QString language();
    void reset();
    void mouseHandler(int x, QMouseEvent* aEvent);
    bool filterEvent(const QEvent *event);

public: // From HbInputMethod
    void listInputModes(QVector<HbInputModeProperties>& results);
    void focusReceived();
    void focusLost(bool focusSwitch);
    void InputLanguageChanged(int newLanguage); 
    void inputStateActivated(const HbInputState& newState);
    void candidatePopupClosed(int closingKey);
    bool usePrediction() const;
    
public: //defined locally
    void initializeModeHandlers();
    int displaySpecialCharacterTable(QObject* receiver);
    void launchCandidatePopup(QStringList *candidates);
    void closeKeypad();
    void launchCharacterPreviewPane(int key);
    void switchSpecialCharacterTable();
    void closeExactWordPopup();
    void launchExactWordPopup(QString exactWord);
signals:
    void flipStatusChange();
protected:
    void predictiveInputStatusChanged(int newStatus);	
    void openKeypad(bool inMinimizedMode = false);   
    
public slots:
    void sctCharacterSelected(QChar character);
    QPointF getCursorCoordinatePosition();
    void keypadClosed();
    void keypadOpened();
    void keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod vkbCloseMethod);
    void orientationAboutToChange();
        
private:        
    HbHwToolCluster* constructKeypad();

private:
    HbKeyMapData *mKeyData;
    int mKeyCode;
    HbExactWordPopup *mExactWordPopup;
    HbCandidateList *mCandidatePopup;
    HbCharPreviewPane *mPreviewPane;
    HbInputModeType mMode;

    // mode handlers
    HbInputModeHandler *mActiveModeHandler;
    HbHardwareInputBasicQwertyHandler *mBasicModeHandler;
    HbHardwareInputPredictionQwertyHandler *mPredictionModeHandler;
    HbHardwareInputNumericQwertyHandler *mNumericModeHandler;
    QPointer<HbHwToolCluster> mKeypad;
    HbVkbHost *mVkbHost;
    };

#endif // _HbHardwareQwerty_H

// End of file 

