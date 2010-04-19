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
#ifndef HBEFFECTCONTROLLER_P_H
#define HBEFFECTCONTROLLER_P_H

#include <hbglobal.h>
#include <hbeffectxmlparser_p.h>
#include <hbeffectfxmldata_p.h>

QT_BEGIN_NAMESPACE
class QGraphicsItem;
QT_END_NAMESPACE

class HB_CORE_PRIVATE_EXPORT HbEffectController
{
public:
    HbEffectController();
    ~HbEffectController();

    HbEffectFxmlData fetchFxmlData(
        const QString &componentType,
        QGraphicsItem *component,
        const QString &effectEvent = QString(),
        bool shared = false) const;

    bool addFXML(const QString &componentType, const QString &filePath, const QString &effectEvent = QString(), bool shared = false);
    bool addFXML(QGraphicsItem *item, const QString &filePath, const QString &effectEvent = QString(), bool shared = false);
    void removeFXML(const QString &componentType, const QString &filePath, const QString &effectEvent = QString());
    void removeFXML(QGraphicsItem *item, const QString &filePath, const QString &effectEvent = QString());
    void removeFXML(QGraphicsItem *item);
    QList<HbEffectInfo> effectEntries() const;
    // Explicitly control sharing
    void setSharing(bool enabled);
    void unsetSharing();

private:
    bool addEffectDefinitionFile(
        const QString &componentType,
        QGraphicsItem *item,
        const QString &filePath,
        const QString &effectEvent = QString(),
        bool shared = false);

    QString expandFileName(const QString &fn, bool *fromTheme = 0, bool *shared = 0);

private:
    QList<HbEffectInfo> mEffectEntries;
    HbEffectXmlParser* mParser;
    bool mShared;
    bool mSharingSet;
};

#endif // HBEFFECTCONTROLLER_P_H
