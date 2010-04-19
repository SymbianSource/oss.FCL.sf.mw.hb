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

#ifndef HB_INPUT_USER_DICTIONARY_H
#define HB_INPUT_USER_DICTIONARY_H

#include <QStringList>

#include <hbinputdef.h>

class HbPredictionBase;
class HbPredictionCallback;

/*!
\class HbUserDictionary
\brief Abstract base class for user dictionary access.

\sa HbPredictionBase
*/
class HB_CORE_EXPORT HbUserDictionary
{
public:
    virtual ~HbUserDictionary() {} 

    virtual int id() const = 0;
    virtual bool addWord(const QString& aWord, HbPredictionCallback* aCallback = 0) = 0;
    virtual bool addWords(const QStringList& aWordList) = 0;
    virtual bool removeWord(const QString& aWord) = 0;
    virtual int numberOfWords() const = 0;
    virtual QStringList listWords() = 0;
    virtual void clear() = 0;
    virtual HbPredictionBase* hostEngine() const = 0;
};

#endif // HB_INPUT_USER_DICTIONARY_H

// End of file
