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
#ifndef HB_INPUT_PREDICTION_12KEY_HANDLER_PRIVATE
#define HB_INPUT_PREDICTION_12KEY_HANDLER_PRIVATE

#include <hbinputdialog.h>
#include <hbinputstate.h>

#include "hbinputpredictionhandler_p.h"
#include "hbinputprediction12keyhandler.h"

class HbAction;
class HbInputSpellQuery;

class HbInputPrediction12KeyHandlerPrivate: public HbInputPredictionHandlerPrivate
{
    Q_DECLARE_PUBLIC(HbInputPrediction12KeyHandler)

public:
    HbInputPrediction12KeyHandlerPrivate();
    ~HbInputPrediction12KeyHandlerPrivate();

    bool buttonReleased(const QKeyEvent *keyEvent);
    bool buttonPressed(const QKeyEvent *keyEvent);
    void cancelButtonPress();
    void chopQMarkAndUpdateEditor();
public:
    int mLastKey;
    bool mButtonDown;
    QChar mCurrentChar;
    bool mLongPressHappened;
    bool mShiftKeyDoubleTap;
    HbInputSpellQuery *mInputSpellQuery;
};

class HbInputSpellQuery : public HbInputDialog
{
Q_OBJECT
public:
    HbInputSpellQuery(HbInputPrediction12KeyHandlerPrivate *owner);
	void getPositionAndSize(QPointF & pos,QSizeF & size, QRectF &geom);
	void launch(QString editorText);
public slots:
    void dialogClosed(HbAction* action);    
private:
    HbInputState mSavedState;
    bool mDidHandleFinish;
    QPointer<QObject> mSavedFocusObject;
    HbInputPrediction12KeyHandlerPrivate* mOwner;
    QString mSavedEditorText;
};
#endif //HB_INPUT_PREDICTION_12KEY_HANDLER_PRIVATE
