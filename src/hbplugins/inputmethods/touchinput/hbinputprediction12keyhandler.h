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
#ifndef HB_INPUT_PREDICTION_12KEY_HANDLER
#define HB_INPUT_PREDICTION_12KEY_HANDLER

#include "hbinputmodehandler.h"

class HbInputPrediction12KeyHandlerPrivate;
class HbInputPrediction12KeyHandler: public HbInputPredictionHandler
{
    Q_OBJECT
public:
    HbInputPrediction12KeyHandler(HbInputAbstractMethod *inputMethod);
    ~HbInputPrediction12KeyHandler();

    bool actionHandler(HbInputModeAction action);
    bool filterEvent(const QKeyEvent * event);
    bool isActive() const; 
    void processCustomWord(QString customWord);
	void mouseHandler(int cursorPosition, QMouseEvent* mouseEvent);
protected:
	HbInputPrediction12KeyHandler(HbInputPrediction12KeyHandlerPrivate &dd, HbInputAbstractMethod* inputMethod);
private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputPrediction12KeyHandler)
    Q_DISABLE_COPY(HbInputPrediction12KeyHandler)
};

#endif //HB_INPUT_PREDICTION_12KEY_HANDLER
