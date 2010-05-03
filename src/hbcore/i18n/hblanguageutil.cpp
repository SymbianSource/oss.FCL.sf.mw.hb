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

#include <QFile>
#include <QLocale>
#include <QTimer>
#include <QHash>
#include <QHashIterator>
#include <QTextStream>
#include <QTranslator>
#include <QTextCodec>
#include <QCoreApplication>

#if defined(Q_OS_SYMBIAN)
#include <e32lang.h>
#include <e32property.h>
#include <centralrepository.h> 
#include <hal.h>
#include <syslangutil.h>
#include <CommonEngineDomainCRKeys.h> //Ui language
#endif // Q_OS_SYMBIAN

#include "hblanguageutil.h"
#include "hbfeaturemanager_p.h"

#if defined(Q_OS_SYMBIAN)
#define LANGUAGE_LIST_FILE "/resource/hbi18n/translations/language_list.txt"
#define LANGUAGE_ID_PREFIX "language_"
#define TRANSLATOR_PATH "/resource/hbi18n/translations/languages"
#endif // Q_OS_SYMBIAN

/*!
    @beta
    @hbcore
    \class HbLanguageUtil
    \brief HbLanguageUtil provides functions for quering supported languages and switching the system language.
*/

#if defined(Q_OS_SYMBIAN)
/*!
    \brief Returns identifiers and localized names of all known languages.
      
    \return Localized names and integer identifiers of languages supported in a device  
*/
QHash<int, QString> readLanguageList()
{
    QHash<int, QString> hashLanguages;
    QString path = "c:";
    path += QString(LANGUAGE_LIST_FILE);
    QFile* file = new QFile(path);
    if (!file->exists() ) {
        path = "z:";
        path += QString(LANGUAGE_LIST_FILE);
        delete file;
        file = new QFile(path);
    }
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        delete file;
        return hashLanguages;
    }
    QTextStream in(file);
    while (!in.atEnd()) {
        QString line = in.readLine(256);
        if (!line.isEmpty()) {
            int sep = line.indexOf(',');
            QString strCode = line.left(sep);
            QString name = line.mid(sep+1);

            bool ok;
            int code = strCode.toUInt(&ok);
            if (!ok) {
                continue;
            }
            hashLanguages.insert(code, name);
        }
    }
    delete file;
    return hashLanguages;
}
#endif // Q_OS_SYMBIAN

#if defined(Q_OS_SYMBIAN)

/*!
    \brief Changes the system UI language.
      
    \param language identifier of the language  
*/
bool setLocale( int language )
{
    TExtendedLocale dummy;
    QString no;
    no = QString( "%1" ).arg( language, 2, 10, QLatin1Char( '0' ) );
    QString name = QString( "elocl." ).append( no );
    TPtrC nameptr(name.utf16());
    
    TInt err = dummy.LoadLocale( nameptr );
    if( err != KErrNone )
        return false;
    dummy.SaveSystemSettings();
    // cause localeprivate update on next qlocale object( glp->m_language_id = 0 )
    QSystemLocale dummy2;
    return true;
}
#endif // Q_OS_SYMBIAN


/*!
    \brief Returns names and identifiers of supported languages in a phone.
 
    Language names are localized according the language's native presentation.
    Language ID's returned by this functions may be used as language parameter for changeLanguage(int language) function.
    Language IDs and names are OS specific and may vary across the platforms and releases.
     
    \return Localized names and integer identifiers of languages supported in a device  
*/
QHash<int, QString> HbLanguageUtil::supportedLanguages()
{
#if defined(Q_OS_SYMBIAN)   
    QHash<int, QString> languages; 
    
    QTranslator translator;
    if (!translator.load(TRANSLATOR_PATH)) {
        return languages;
    } 
    QCoreApplication::installTranslator(&translator);
    QHash<int, QString> hashLanguageNames = readLanguageList();
 
    CArrayFixFlat<TInt>* systemEpocLanguageCodes = 0;
    TInt error = SysLangUtil::GetInstalledLanguages( systemEpocLanguageCodes );
    if ( error != KErrNone ) {
        delete systemEpocLanguageCodes;
        return languages;
    }
    
    for (int i = 0; i < systemEpocLanguageCodes->Count(); ++i) {
        int code = systemEpocLanguageCodes->At(i);
        QString id = QString(LANGUAGE_ID_PREFIX);
        id += QString::number(code);
        QString locName = hbTrId(id.toAscii().constData());
        if (locName.isEmpty() || locName == id) {
            locName = hashLanguageNames.value(code);
        }
        languages.insert(code, locName);
    }
    
    delete systemEpocLanguageCodes;
    return languages;
#else 
    QHash<int, QString> dummy;
    return dummy;
#endif
}

/*!
    \brief Returns names and identifiers of all known languages.
 
    Language names are localized according the language's native presentation.
    Language ID's returned by this functions may be used as language parameter for changeLanguage(int language) function.
    Language IDs and names are OS specific and may vary across the platforms and releases.
     
    \return Localized names and integer identifiers of known languages 
*/
QHash<int, QString> HbLanguageUtil::allLanguages()
{
#if defined(Q_OS_SYMBIAN)  
    QHash<int, QString> langs; 
    
    QTranslator translator;
    if (!translator.load(TRANSLATOR_PATH)) {
        return langs;
    } 
    QCoreApplication::installTranslator(&translator);
    
    QHash<int, QString> languageNameList = readLanguageList();
    QHashIterator<int, QString> i(languageNameList);
    while (i.hasNext()) {
        i.next();
        int code = i.key();
        QString id = QString(LANGUAGE_ID_PREFIX);
        id += QString::number(code);
        QString locName = hbTrId(id.toAscii().constData());
        if (locName.isEmpty()) {
            locName = i.value();
        }
        langs.insert(code, locName);
    }
    return langs;
#else 
    QHash<int, QString> dummy;
    return dummy;
#endif
}

/*!
    \brief Changes the device system language.  
  
    \param identifier of language to set active
    \return true if language change was successful
*/ 
bool HbLanguageUtil::changeLanguage( int language )
{
#if defined(Q_OS_SYMBIAN)
    if ( !HbFeatureManager::instance()->featureStatus(HbFeatureManager::LanguageSwitch) ) {
        return false;
    }
    
    CRepository* commonEngineRepository = 0;
    TRAPD( err1, commonEngineRepository = CRepository::NewL( KCRUidCommonEngineKeys ) );    
    if ( err1 != KErrNone ) { 
        return false;
    }
    
    if (!setLocale(language)) {
    		delete commonEngineRepository;
        return false;
    }
        
    // Never set Language code 0 to HAL
    if ( language !=0 ) {
        if ( HAL::Set( HAL::ELanguageIndex, language ) != KErrNone ) {
            delete commonEngineRepository;
            return false;
        }
    }
    if ( commonEngineRepository->Set( KGSDisplayTxtLang, language ) != KErrNone ) {
        delete commonEngineRepository;
        return false;
    }
    delete commonEngineRepository;
    return true;

#else
    Q_UNUSED(language);
    return false;
#endif
}

/*!
    \brief Returns ID of current language. Localized name 
  
    \return identifier of current system language
*/ 
int HbLanguageUtil::currentLanguage()
{
    #if defined(Q_OS_SYMBIAN)
        TLanguage l = User::Language();
        return l;
    #else 
        return 0;
    #endif
}

