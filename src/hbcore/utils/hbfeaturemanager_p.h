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


#ifndef HBFEATUREMANAGER_P_H
#define HBFEATUREMANAGER_P_H

#include <hbglobal.h>

#if defined(Q_WS_S60)

#include <centralrepository.h>

const TUid HBFM_CREPO_ID  = {0x2002C304};

#else

class QSettings;

#endif // Q_WS_S60


class HB_CORE_PRIVATE_EXPORT HbFeatureManager
{

public:

    typedef enum {
        TextMeasurement = 0x1,
        TheTestUtility  = 0x2,
        LanguageSwitch  = 0x4
    } HbFeature;


    static HbFeatureManager *instance();
    ~HbFeatureManager();

    int featureStatus( HbFeature feature );
    void setFeatureStatus( HbFeature feature, int status );

private:
    HbFeatureManager();

    QString toString( HbFeature feature );

#if defined(Q_WS_S60)
    CRepository *mRepo;
#else
    QSettings *mSettings;
#endif // Q_WS_S60

};

#endif // HBFEATUREMANAGER_P_H
