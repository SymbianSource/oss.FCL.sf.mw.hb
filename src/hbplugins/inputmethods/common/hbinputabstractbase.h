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

#ifndef HB_INPUT_ABSTRACT_BASE
#define HB_INPUT_ABSTRACT_BASE

#include <hbinputmethod.h>

class HbCandidateList;
class HbInputVkbWidget;

class HbInputAbstractMethod : public HbInputMethod
{
Q_OBJECT

public: //constructor
    HbInputAbstractMethod();
    virtual ~HbInputAbstractMethod();

public:
    virtual void launchCandidatePopup(const QStringList &candidates);
    virtual void closeKeypad();
    virtual void launchCharacterPreviewPane(int key);
    virtual void switchSpecialCharacterTable();
    virtual void selectSpecialCharacterTableMode();
    virtual void closeExactWordPopup();
    virtual void launchExactWordPopup(QString exactWord); 
    virtual void launchAutoCompletionPopup(const QStringList& candidates);
    virtual void closeAutoCompletionPopup();
    virtual void switchMode(int keyCode);
    virtual void starKeySelected();
    virtual void getCandidatePositionAndSize(HbCandidateList *candidatePopup,
                    HbInputVkbWidget *currentKeypad, QPointF & pos,QSizeF & size);
private:
    Q_DISABLE_COPY(HbInputAbstractMethod)
};

#endif //HB_INPUT_ABSTRACT_BASE
