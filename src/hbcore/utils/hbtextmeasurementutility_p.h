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

#ifndef HBTEXTMEASUREMENTUTILITY_H
#define HBTEXTMEASUREMENTUTILITY_H

#include <QObject>

#include <hbglobal.h>
#include <hbdeviceprofile.h>

class HbTextMeasurementUtilityPrivate;
class HbDeviceProfile;

namespace HbTextMeasurementUtilityNameSpace {
    const char textIdPropertyName[] = "__Hb__Text__Measurement__Utility_Text__Id__";
    const char textMaxLines[] = "__Hb__Text__Measurement__Utility_Max__Lines__";
}


class HB_CORE_PRIVATE_EXPORT HbTextMeasurementUtility : public QObject
{
    Q_OBJECT

public:
    ~HbTextMeasurementUtility();
    static HbTextMeasurementUtility *instance();


private:
    HbTextMeasurementUtility();

public:
    void setLocTestMode( bool enabled );
    bool locTestMode() const;
    void measureItems(int after=0);
    bool writeReport(HbDeviceProfile &profile, const QString &domainName);
    bool writeReport(HbDeviceProfile &profile, QIODevice *device );
    void reset();

private slots:
    void doMeasureItems();

private:
    HbTextMeasurementUtilityPrivate *d;
};

#endif // HBTEXTMEASUREMENTUTILITY_H
