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
#ifndef HBACTIONMANAGERXMLPARSER_H
#define HBACTIONMANAGERXMLPARSER_H

#include "hbglobal.h"
#include <QXmlStreamReader>

QT_BEGIN_NAMESPACE
class QIODevice;
QT_END_NAMESPACE

class HbActionManager;
class HbViewActionManager;

class HB_AUTOTEST_EXPORT HbActionManagerXmlParser : public QXmlStreamReader
{
public:
    HbActionManagerXmlParser(HbActionManager *actionManager);
    HbActionManagerXmlParser(HbViewActionManager *viewActionManager);
    ~HbActionManagerXmlParser();

    bool read(QIODevice *device);

private:
    void readTemplate();
    void readGuide();
    void readDistributionItem();
    HbAction::CommandRole enumValue(const QMetaObject &metaObject, 
        const char* propertyName, const QString &enumKeyString) const;

private:
    HbActionManager *actionManager; // Not owned
    HbViewActionManager *viewActionManager; // Not owned
};

#endif // HBACTIONMANAGERXMLPARSER_H
