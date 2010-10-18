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

#include "hbfeaturemanager_r.h"

#include <QString>

#if defined(Q_OS_SYMBIAN)
#include <centralrepository.h>
#else
#include <QSettings>
#endif // Q_OS_SYMBIAN

/*!
    @alpha
    @hbcore
    \class HbFeatureManager
    \brief HbFeatureManager is used to control hb internal features.
*/

/*!
    \enum HbFeatureManager::Feature

    This enum defines the hb internal features.
*/

/*!
    \var HbFeatureManager::TextMeasurement

    Runtime variation flag for text measurement feature.

    This needs to be enabled if you want to get the localization layout
    metrics from your application.

    Zero means disabled, non-zero means enabled. Value 2 means that
    text measurement is done automatically when Hb applications are run.
*/

/*!
    \var HbFeatureManager::TheTestUtility

    Runtime variation flag for "the test utility"

    This needs to be enabled if you want to utilize "the test utility"
    (the four floating buttons) in your application.

    Zero means disabled, non-zero means enabled.
*/

/*!
    \var HbFeatureManager::LanguageSwitch
*/


#if defined(Q_OS_SYMBIAN)
const TUid HBFM_CREPO_ID  = {0x2002C304};
#endif


class HbFeatureManagerPrivate
{
public:
    HbFeatureManagerPrivate();
    virtual ~HbFeatureManagerPrivate();

    QString toString( HbFeatureManager::Feature feature );

#if defined(Q_OS_SYMBIAN)
    CRepository *mRepo;
#else
    QSettings *mSettings;
#endif // Q_OS_SYMBIAN
};

/*!
\internal
*/
HbFeatureManagerPrivate::HbFeatureManagerPrivate()
{
#if defined(Q_OS_SYMBIAN)
    TRAPD( err, mRepo = CRepository::NewL( HBFM_CREPO_ID ) );
    if( err ) {
        qWarning( "HbFeatureManager construction fails, error code = %d", err );
    }
    // Default values defined in cenrep file.
#else
    mSettings = new QSettings( "Nokia", "Hb" );
    mSettings->beginGroup( "FeatureManager" );
    // Set default values:
    if( !mSettings->contains( toString( HbFeatureManager::TextMeasurement ) ) ) {
        mSettings->setValue( toString( HbFeatureManager::TextMeasurement ), 0 );
    }
    if( !mSettings->contains( toString( HbFeatureManager::TheTestUtility ) ) ) {
        mSettings->setValue( toString( HbFeatureManager::TheTestUtility ), 0 );
    }
#endif // Q_OS_SYMBIAN
}

/*!
\internal
*/
HbFeatureManagerPrivate::~HbFeatureManagerPrivate()
{
#if defined(Q_OS_SYMBIAN)
    delete mRepo;
#else
    delete mSettings;
#endif // Q_OS_SYMBIAN
}

/*!
\internal
*/
QString HbFeatureManagerPrivate::toString( HbFeatureManager::Feature feature )
{
    return QString( "HbFeature_" ) + QString::number( ( int )feature );
}

/*!
    Default constructor.
*/
HbFeatureManager::HbFeatureManager() :
    d(new HbFeatureManagerPrivate)
{
}

/*!
    Returns singleton instance
*/
HbFeatureManager *HbFeatureManager::instance()
{
    static HbFeatureManager theManager;
    return &theManager;
}

/*!
    Destructor
*/
HbFeatureManager::~HbFeatureManager()
{
    delete d;
}

/*!
    Returns the status of requested feature.
*/
int HbFeatureManager::featureStatus( Feature feature )
{
#if defined(Q_OS_SYMBIAN)
    if (!d->mRepo) {
        return 0;
    }
    TUint32 key = (TUint32)feature;
    TInt value = 0;
    TInt error = d->mRepo->Get( key, value );
    if( error != KErrNone ) {
        qWarning( "HbFeatureManager getting the feature fails, error code = %d", error );
    } else {
        return (int)value;
    }

#else
    if( d->mSettings->contains( d->toString( feature ) ) ) {
        return d->mSettings->value( d->toString( feature ) ).toInt();
    }
#endif
    return 0;
}


/*!
    Sets the status of requested feature to given value.
*/
void HbFeatureManager::setFeatureStatus( Feature feature, int status )
{
#if defined(Q_OS_SYMBIAN)
    if (!d->mRepo) {
        return;
    }
    TUint32 key = (TUint32)feature;
    TInt value = (TInt)status;
    TInt error = d->mRepo->Set( key, value );
    if( error != KErrNone ) {
        qWarning( "HbFeatureManager setting the feature fails, error code = %d", error );
    }

#else
    d->mSettings->setValue( d->toString( feature ), status );
#endif
}

