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
#ifndef HB_INPUT_EXTRA_USER_DICTIONARY_PRIVATE_H
#define HB_INPUT_EXTRA_USER_DICTIONARY_PRIVATE_H

#include <QFile>
#include <QDir>
#include <QSharedMemory>

#include "hbinputextrauserdictionary.h"
#include "hbinputsettingproxy.h"

const QString KExtraUserDictKeyBase("ExtraUD_");
const QString KExtraFileExt(".dat");
const int KExtraUDBlockSize = 8192;

/// @cond

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Hb Inputs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

class HbExtraUDHeader
{
public:
    HbExtraUDHeader() : numUsers(0), numWords(0), modified(false), dataSize(0)
      {}

public:
    int numUsers;
    int numWords;
    bool modified;
    int dataSize;   // Size character data area in QChar's.
};


class HbExtraUserDictionaryPrivate
{
public:
    HbExtraUserDictionaryPrivate() : id(0), hostEngine(0)
        {}

     bool createSharedBlock(int aSize);
     QString name() const;
     QString fileName() const;

     void removeEntry(int index);
     void addEntry(int index, const QString& newWord);

     HbExtraUDDirectoryEntry *directory() const {
         return (HbExtraUDDirectoryEntry*)((char*)sharedMemory.data() + sizeof(HbExtraUDHeader));
     }

     QChar *dataArea() const {
         return (QChar*)((char*)sharedMemory.data() + sizeof(HbExtraUDHeader) + (dataHeader()->numWords * sizeof(HbExtraUDDirectoryEntry)));
     }

     int findWord(int start, int end, const QString& newWord, Qt::CaseSensitivity caseSensitivity= Qt::CaseSensitive) const;
     int findIndexForNewWord(int start, int end, const QString& newWord) const;
     int findFirstMatch(int start, int end, const QString& searchString, int knownMatch = -1, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive) const;
     int compareWords(int index, const QString& otherWord, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive) const;
     bool hasEnoughSpaceForNewWord(const QString& newWord) const;

     bool save(QString aFileName);

     HbExtraUDHeader* dataHeader() const {
         return (HbExtraUDHeader*)sharedMemory.data();
     }

     int dataAreaSize() const {
         return sharedMemory.size() - sizeof(HbExtraUDHeader) - (dataHeader()->numWords * sizeof(HbExtraUDDirectoryEntry));
     }

     void lock() { sharedMemory.lock(); }
     void unlock() { sharedMemory.unlock(); }

public:
     int id;
     HbPredictionBase* hostEngine;
     QSharedMemory sharedMemory;
};

/// @endcond

#endif // HB_INPUT_EXTRA_USER_DICTIONARY_PRIVATE_H

// End of file
