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

#ifndef HBICONANIMATIONPARSER_H
#define HBICONANIMATIONPARSER_H

#include "hbiconanimationdatatypes_p.h"
#include <hbglobal.h>
#include <qglobal.h>
#include <QXmlStreamReader>
#include <QHash>
#include <QString>

class HB_AUTOTEST_EXPORT HbIconAnimationParser : public QXmlStreamReader
{
public:
    HbIconAnimationParser();
    ~HbIconAnimationParser();

    bool parseDefinitionFileShared(const QString &fileName, AnimHash &animations, const QString &realFileName);
    bool parseDefinitionFile(const QString &fileName, AnimHash &animations, const QString &realFileName);

private:
    bool doParseFile(const QString &fileName, AnimHash &animations);
    bool readAnimations(const QString &fileName, AnimHash &animations);
    QString readIconData(HbIconAnimationData &data);
    void readUnknownElement();

private:
    Q_DISABLE_COPY(HbIconAnimationParser)
};

#endif // HBICONANIMATIONPARSER_H
