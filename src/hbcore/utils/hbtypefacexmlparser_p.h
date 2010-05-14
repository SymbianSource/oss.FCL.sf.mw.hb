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

#ifndef HBTYPEFACEXMLPARSER_P_H
#define HBTYPEFACEXMLPARSER_P_H

#include "hbglobal.h"

#include <QXmlStreamReader>
#include <QFile>
#include <QDir>
#include <hbextendedlocale.h>

QT_BEGIN_NAMESPACE
class QIODevice;
QT_END_NAMESPACE

#ifdef Q_OS_SYMBIAN	
	#define TYPEFACE_RESOURCE_FOLDER "z:\\resource\\fonts"
#else
	#define TYPEFACE_RESOURCE_FOLDER HB_RESOURCES_DIR
#endif

class HB_AUTOTEST_EXPORT HbTypefaceXmlParser : public QXmlStreamReader
{
public:
    enum StartElement
    {
        Undefined,
        NoStartElement,
        TypefaceSet,
        TypefaceMetrics
    };

public:
    HbTypefaceXmlParser();
    ~HbTypefaceXmlParser();

public:
    void setFilePath(const QString &path);
    bool init();
    StartElement readToStartElement();
    bool readMapping(QString &role, QString &family, bool &isBold);
    bool readMetric(int &textHeight, int &size, int &baseline);
    const QString metricsTypefaceFamily() const;
	bool readAndPositionTypefaceSet();
    void close();

private:
    bool readMapItem(QString &role, QString &family, bool &isBold);
    bool readMetricItem(int &textHeight, int &size, int &baseline);
	bool matchLanguageAndCountry() const;

private:
	QString mFilePath;
	QFile *mFile;
	QString mCurrentFamily;
};

#endif // HBTYPEFACEXMLPARSER_P_H
