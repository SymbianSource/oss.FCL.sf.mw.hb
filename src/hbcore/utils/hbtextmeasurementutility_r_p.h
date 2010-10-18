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

#ifndef HBTEXTMEASUREMENTUTILITY_R_P_H
#define HBTEXTMEASUREMENTUTILITY_R_P_H

#include <QObject>

class HbMainWindow;
class HbDeviceProfile;
class QTextStream;
class HbTextRecord;

namespace HbTextMeasurementUtilityNameSpace {
    const char textIdPropertyName[] = "__Hb__Text__Measurement__Utility_Text__Id__";
    const char textMaxLines[] = "__Hb__Text__Measurement__Utility_Max__Lines__";
}

class HbTextMeasurementUtilityPrivate : public QObject
{
    Q_OBJECT

public:
    void readEntries(QTextStream &csvReader);
    void writeHeaders(QTextStream &csvWriter);
    void writeEntry(QTextStream &csvWriter, const HbTextRecord *record);
    bool validateRecords(HbDeviceProfile &profile);
    QString reportFilePath(HbDeviceProfile &profile, const QString &domainName) const;

public slots:
    void doMeasureItems();

public:
    QList<HbTextRecord*> records;
    HbMainWindow *mWindow;
    int mLocTestMode_cached;
};

#endif // HBTEXTMEASUREMENTUTILITY_R_P_H
