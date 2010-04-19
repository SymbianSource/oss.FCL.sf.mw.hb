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

#include "hbsmileythemeparser_p.h"
#include "hbsmileytheme.h"

#include <QDebug>
#include <QFile>

#undef HB_SMILEY_THEME_TRACES


// Private class
HbSmileyThemeParser::HbSmileyThemeParser()
{
}

HbSmileyThemeParser::~HbSmileyThemeParser()
{
}

bool HbSmileyThemeParser::parseDefinitionFile(const QString& fileName, HbSmileyTheme &theme)
{
    QFile file(fileName);

    if (!file.exists()) {
#ifdef HB_SMILEY_THEME_TRACES
        qDebug() << "HbSmileyThemeParser: File does not exist:" << fileName;
#endif
        return false;
    }

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
#ifdef HB_SMILEY_THEME_TRACES
        qDebug() << "HbSmileyThemeParser: File opening failed:" << fileName;
#endif
        return false;
    }

    QByteArray line;

    while(!file.atEnd()) {
        line=file.readLine().simplified();
         if (line.isEmpty() || line.at(0) == '#') {
            continue;
        }

        QString strLine(line);
        QStringList strList = strLine.split(QRegExp("\\s+"));
        QString smiley = strList.first();
        strList.removeFirst();

        if (!strList.isEmpty()) {
            theme.insert(smiley,strList);
        } else {
#ifdef HB_SMILEY_THEME_TRACES
            qDebug() << "HbSmileyThemeParser: Parsing error: "
                     <<  fileName << "Smiley: " << smiley << "doesn't contain any patterns!";
#endif
            return false;
        }
    }

    return !theme.isNull();
}

// End of File
