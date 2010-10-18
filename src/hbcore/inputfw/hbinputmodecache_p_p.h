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

#include <QString>
#include <QDir>

#include <hbinputmethoddescriptor.h>
#include <hbinputmethod.h>

class QInputContextPlugin;
class QSharedMemory;

const char HbInputMethodListKey[] = "HbInputMethodList";
const char HbInputMethodListModTimeKey[] = "HbInputMethodListModTime";

class HB_CORE_PRIVATE_EXPORT HbInputMethodListItem
{
public:
    HbInputMethodListItem() : cached(0), toBeRemoved(false) {}
    bool operator==(const HbInputMethodListItem &item) const {
        return (descriptor.pluginNameAndPath() == item.descriptor.pluginNameAndPath() &&
                descriptor.key() == item.descriptor.key());
    }

    void setValues(QInputContextPlugin *plugin, const QString &key);

public:
    HbInputMethodDescriptor descriptor;
    QStringList languages;
    HbInputMethod *cached;
    bool toBeRemoved;
};

HB_CORE_PRIVATE_EXPORT QDataStream &operator<<(QDataStream &stream, const HbInputMethodListItem &item);
HB_CORE_PRIVATE_EXPORT QDataStream &operator>>(QDataStream &stream, HbInputMethodListItem &item);

class HB_CORE_PRIVATE_EXPORT HbInputModeCachePrivate
{
public:
    HbInputModeCachePrivate();
    ~HbInputModeCachePrivate();
    void refresh();
    static void readInputMethodDataFromDisk(QList<HbInputMethodListItem> *methodList, const QDir &readPath = QDir());
    bool readInputMethodDataFromSharedMemory();
    void pruneRemovedMethods();
    static QInputContextPlugin *pluginInstance(const QString &pluginFileName);
    HbInputMethod *methodInstance(const QString &pluginFileName, const QString &key) const;
    HbInputMethod *cachedMethod(HbInputMethodListItem &item);
    bool isMappedLanguage(const HbInputLanguage &language) const;

public:
    QList<HbInputMethodListItem> mMethods;
    QSharedMemory *mSharedMethodList;
    QSharedMemory *mMethodListModTime;
    uint mMethodListLastUpdate;
    bool mMethodsFetchedFromDisk;
    bool mShuttingDown;
};
