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
#include "hbinputextradictionaryfactory.h"

#include <QDir>

#include "hbinputsettingproxy.h"
#include "hbinputextrauserdictionary.h"

/*!
@proto
@hbcore
\class HbExtraDictionaryFactory
\brief A factory class for creating and accessing instances of HbExtraDictionary database.

This is a singleton factory class for creating instances of HbExtraDictionary class by id number.
It knows where the databases are stored in the system, loads and maintains them in the memory.

\sa HbExtraUserDictionary
*/

/// @cond

class HbExtraDictionaryFactoryPrivate
{
public:
    void shutdown();

public:
    HbExtraDictionaryFactory *q_ptr;
    QList<HbExtraUserDictionary *> instances;
};


void HbExtraDictionaryFactoryPrivate::shutdown()
{
    Q_FOREACH(const HbExtraUserDictionary *instance, instances) {
        delete instance;
    }

    instances.clear();
}

/// @endcond

/*!
Returns pointer to the singleton instance.
*/
HbExtraDictionaryFactory *HbExtraDictionaryFactory::instance()
{
    static HbExtraDictionaryFactory singletonInstance;
    return &singletonInstance;
}


/*!
Constructs the object.
*/
HbExtraDictionaryFactory::HbExtraDictionaryFactory() : d_ptr(new HbExtraDictionaryFactoryPrivate)
{
    Q_D(HbExtraDictionaryFactory);
    d->q_ptr = this;
}


/*!
Destructs the object.
*/
HbExtraDictionaryFactory::~HbExtraDictionaryFactory()
{
    Q_D(HbExtraDictionaryFactory);
    delete d;
}


/*!
Returns pointer to dictionary by given id if it exists. Otherwise returns zero.
First goes through the dictionary instances in memory, then tries to load
from disk.
*/
HbExtraUserDictionary *HbExtraDictionaryFactory::existingDictionary(int dictionaryId)
{
    Q_D(HbExtraDictionaryFactory);

    foreach(HbExtraUserDictionary *dictPtr, d->instances) {
        if (dictPtr->id() == dictionaryId) {
            return dictPtr;
        }
    }

    // At this point make sure that target directory exists.
    QDir targetDir;
    targetDir.mkpath(HbInputSettingProxy::extraDictionaryPath());

    HbExtraUserDictionary *newDb = new HbExtraUserDictionary;
    newDb->setId(dictionaryId);

    if (newDb->isAlreadyInMemory()) {
        newDb->attach();
        d->instances.append(newDb);
        return newDb;
    }

    if (newDb->attach()) {
        // Not available in memory, try to load from permanent memory.
        if (newDb->load(newDb->fileName())) {
            d->instances.append(newDb);
            return newDb;
        }
    }

    delete newDb;
    return 0;
}

/*!
Creates dictionary if it doesn't already exists. If initialContent is given, new dictionary will
be filled with it. If a dictionary for given id already exists, it is returned and initialContent
is discarded.
*/
HbExtraUserDictionary *HbExtraDictionaryFactory::createDictionary(int dictionaryId, const QStringList &initialContent)
{
    Q_D(HbExtraDictionaryFactory);

    HbExtraUserDictionary *newDb = existingDictionary(dictionaryId);
    if (newDb == 0) {
        newDb = new HbExtraUserDictionary;
        newDb->setId(dictionaryId);
        if (!newDb->attach()) {
            delete newDb;
            return 0;
        }
        d->instances.append(newDb);
    }

    newDb->addWords(initialContent);
    return newDb;
}

/*!
Shuts down the dictionary and closes shared memory interface. Saves shared data contents to disc if this was
last instance in memory. This is done in separate method (instead of destructor) in order to
do singleton shutdown safely (the slot is connected to QApplication::aboutToQuit by the framework).
*/
void HbExtraDictionaryFactory::shutdown()
{
    Q_D(HbExtraDictionaryFactory);
    d->shutdown();
}

// End of file

