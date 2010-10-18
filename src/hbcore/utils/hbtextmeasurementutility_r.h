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

#ifndef HBTEXTMEASUREMENTUTILITY_R_H
#define HBTEXTMEASUREMENTUTILITY_R_H

#include <hbglobal.h>
#include <hbdeviceprofile.h>

class HbTextMeasurementUtilityPrivate;
class HbMainWindow;

class HB_CORE_RESTRICTED_EXPORT HbTextMeasurementUtility
{
public:
    virtual ~HbTextMeasurementUtility();
    static HbTextMeasurementUtility *instance();

    enum LocTestMode {
        Disabled = 0,
        Manual = 1,
        Automatic = 2
    };

private:
    HbTextMeasurementUtility();

public:
    void setLocTestMode( int mode );
    int locTestMode() const;
    void measureItems( int after = 0, HbMainWindow *window = 0 );
    bool readReport( HbDeviceProfile &profile, const QString &domainName );
    bool writeReport( HbDeviceProfile &profile, const QString &domainName );
    bool writeReport( HbDeviceProfile &profile, QIODevice *device );
    void reset();

private:
    HbTextMeasurementUtilityPrivate *d;
};

#endif // HBTEXTMEASUREMENTUTILITY_R_H
