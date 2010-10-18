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
#ifndef HB_INPUT_PREDICTION_HANDLER_PRIVATE
#define HB_INPUT_PREDICTION_HANDLER_PRIVATE

#include <QPointer>

#include "hbinputmodehandler_p.h"

class HbPredictionEngine;
class HbInputSpellQuery;
QT_BEGIN_NAMESPACE
class QStringList;
class QInputMethodEvent;
QT_END_NAMESPACE

class HbInputPredictionHandlerPrivate :public HbInputModeHandlerPrivate
{
    Q_DECLARE_PUBLIC(HbInputPredictionHandler)
public:
    HbInputPredictionHandlerPrivate();
    virtual ~HbInputPredictionHandlerPrivate();

    // data members
    void mouseHandler(int cursorPosition, QMouseEvent* mouseEvent);
    void init();
    bool filterEvent(const QKeyEvent * event);
    void keyHandlerUnicode(const QChar& character);
    void reset();
    void commit();
    void commit(const QString& string,bool addToUsedWordDict = false, bool isAsync = false);
    void commit(QInputMethodEvent & event,bool addToUsedWordDict = false);
    void candidatePopupClosed(int closingKey, const QString& activatedText);
    void commitAndAppendCharacter(QChar character);
    virtual void deleteOneCharacter();
    void commitExactWord();
    void handleEmptyCandidateList();
    virtual void showExactWordPopupIfNeeded();
    QList<HbKeyPressProbability> probableKeypresses();
    void setPreEditTextToEditor(QString string, bool showAutocompletionPart);
    virtual QString getCommitString();
public:
    HbPredictionEngine  *mEngine;
    QStringList         *mCandidates;
    int                 mBestGuessLocation;
    bool                mShowTail;
    bool                mAutoAddedSpace;
    bool                mCanContinuePrediction;
    bool                mShowTooltip;
    Qt::KeyboardModifiers mModifiers;
    QPointer<HbInputSpellQuery> mSpellQueryDialog;
    bool mAutocompletionEnabled;
    HbInputLanguage mSecLanguage;
public:
    virtual void updateEditor();
};

#endif //HB_INPUT_PREDICTION_HANDLER_PRIVATE
