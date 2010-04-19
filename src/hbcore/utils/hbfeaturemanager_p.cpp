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

#include "hbfeaturemanager_p.h"

#include <QSettings>

HbFeatureManager::HbFeatureManager()
{
#if defined(Q_WS_S60)
    TRAPD( err, mRepo = CRepository::NewL( HBFM_CREPO_ID ) );
    if( err ) {
        qWarning( "HbFeatureManager construction fails, error code = %d", err );
    }
    // Default values defined in cenrep file.
#else
    mSettings = new QSettings( "Nokia", "Hb feature manager" );
    // Set default values:
    if( !mSettings->contains( toString( TextMeasurement ) ) ) {
        mSettings->setValue( toString( TextMeasurement ), 0 );
    }
    if( !mSettings->contains( toString( TheTestUtility ) ) ) {
        mSettings->setValue( toString( TheTestUtility ), 0 );
    }
#endif // Q_WS_S60
}

HbFeatureManager *HbFeatureManager::instance()
{
    static HbFeatureManager theManager;
    return &theManager;
}


HbFeatureManager::~HbFeatureManager()
{
#if defined(Q_WS_S60)
    delete mRepo;
#else
    delete mSettings;
#endif // Q_WS_S60
}

int HbFeatureManager::featureStatus( HbFeature feature )
{
#if defined(Q_WS_S60)
    if (!mRepo) {
        return 0;
    }
    TUint32 aKey = (TUint32)feature;
    TInt aValue = 0;
    TInt error = mRepo->Get( aKey, aValue );
    if( error != KErrNone ) {
        qWarning( "HbFeatureManager getting the feature fails, error code = %d", error );
    } else {
        return (int)aValue;
    }

#else
    if( mSettings->contains( toString( feature ) ) ) {
        return mSettings->value( toString( feature ) ).toInt();
    }
#endif
    return 0;
}


void HbFeatureManager::setFeatureStatus( HbFeature feature, int status )
{
#if defined(Q_WS_S60)
    if (!mRepo) {
        return;
    }
    TUint32 aKey = (TUint32)feature;
    TInt aValue = (TInt)status;
    TInt error = mRepo->Set( aKey, aValue );
    if( error != KErrNone ) {
        qWarning( "HbFeatureManager setting the feature fails, error code = %d", error );
    }

#else
    mSettings->setValue( toString( feature ), status );
#endif
}

QString HbFeatureManager::toString( HbFeature feature )
{
    return QString( "HbFeature_" ) + QString::number( ( int )feature );
}
