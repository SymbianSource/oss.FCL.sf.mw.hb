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

#include "hbinputbasichandler_p.h"

class HbInputBasic12KeyHandlerPrivate: public HbInputBasicHandlerPrivate
{
    Q_DECLARE_PUBLIC(HbInputBasic12KeyHandler)

public:
    HbInputBasic12KeyHandlerPrivate();
    ~HbInputBasic12KeyHandlerPrivate();

    bool handleAlphaEvent(int buttonId, HbKeyboardType type);
    bool buttonPressed(const QKeyEvent *keyEvent);
    bool buttonReleased(const QKeyEvent *keyEvent);
    bool actionHandler(HbInputModeHandler::HbInputModeAction action);

    void _q_timeout();

public:
    int mLastKey;
    QChar mCurrentChar;
    int mNumChr;
    int mDownKey;
    HbInputFocusObject *mCurrentlyFocused;
    bool mLongPressHappened;
	bool mShiftKeyDoubleTapped;
};

