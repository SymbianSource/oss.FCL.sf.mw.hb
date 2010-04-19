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

#ifndef HbAutoComplete_H 
#define HbAutoComplete_H 

#include <QVector>
#include <QStringList>

#include "hbinputpredictionengine.h"

class HbAutoCompletePrivate;
class HbExtraUserDictionary;

const QString HbAutoCompleteVendorIdString("Nokia HbAutoComplete");

class HbAutoComplete : public HbPredictionEngine
{
public:
    HbAutoComplete();
    ~HbAutoComplete();

public: // From HbPredictionBase
    QList<HbInputLanguage> languages() const;
    void deleteKeyPress(HbPredictionCallback* callback = 0);
    void commit(const QString &word = QString());
    void clear();
    void addUsedWord(const QString& word);
    void appendKeyPress(const int keycode, const Qt::KeyboardModifiers modifiers, const HbTextCase textCase = HbTextCaseNone, HbPredictionCallback* callback = 0);
    void setWord(const QString& word, HbPredictionCallback* callback = 0);
    void updateCandidates(int& bestGuessLocation, bool& noMoreCandidates); 
    HbInputPredictionFeature features() const;
    QString vendorIdString() const;
    QString engineVersion() const;
    bool supportsKeyboardType(const HbInputLanguage &language, HbKeyboardType keyboard) const;
    void setExtraUserDictionary(int aId);
    void setExtraUserDictionaries(const QList<int>& idList);

public: // From HbPredictionEngine
    int inputLength();
    void setCandidateList(QStringList* candidateList);
    QStringList candidateList();
    void setKeyboard(HbKeyboardType aKeyboardType);
    void appendCharacter(const QChar aChar, const HbTextCase textCase = HbTextCaseNone, HbPredictionCallback* callback = 0);
 
protected:
    HbAutoCompletePrivate *const d_ptr;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbAutoComplete)
    Q_DISABLE_COPY(HbAutoComplete)
};

#endif // HbAutoComplete_H

// End of file 

