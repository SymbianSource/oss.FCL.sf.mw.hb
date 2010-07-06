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
#include <QStringList>

#if defined(Q_OS_SYMBIAN)
#include <e32lang.h>
#include <e32property.h>
#include <centralrepository.h> 
#include <hal.h>
#include <syslangutil.h>
#include <CommonEngineDomainCRKeys.h> //Ui language
#endif // Q_OS_SYMBIAN

#include <hbglobal.h>
#include <hblocaleutil.h>

#if defined(Q_OS_SYMBIAN)
#define LANGUAGE_LIST_FILE "/resource/hbi18n/translations/language_list.txt"
#define LANGUAGE_MAPPINGS_FILE "/resource/hbi18n/translations/locale_mappings.txt"
#define LANGUAGE_ID_PREFIX "language"
#define LANGUAGE_TRANSLATOR_PATH "/resource/hbi18n/translations/languages"

#define REGION_LIST_FILE "z:/resource/bootdata/regions.txt"
#define REGION_ID_PREFIX "region"
#define REGION_TRANSLATOR_PATH "/resource/hbi18n/translations/regions"
#define REGION_DLL_PREFIX "elocl_reg."

#define COLLATION_LIST_FILE "z:/resource/bootdata/collations.txt"
#define COLLATION_ID_PREFIX "collation"
#define COLLATION_TRANSLATOR_PATH "/resource/hbi18n/translations/collations"
#define COLLATION_DLL_PREFIX "elocl_col."
#define COLLATION_DLL_PREFIX_POSITION 3
#endif // Q_OS_SYMBIAN

/*!
    @beta
    @hbcore
    \class HbLocaleUtil
    \brief HbLocaleUtil provides functions for quering supported languages, regions and collations and activing them.
     
    Language and collation identifiers typically corresponds with two-letter ISO 639 language code, but for certain languages and collations combination of ISO 639 language code and  ISO 3166 country code id used.
    Region identifiers always corresponds with two-letter ISO 3166 country code.
    
    HbLocaleUtil also provides functions for converting language, region and collation identifiers to their localised names. 
*/

#if defined(Q_OS_SYMBIAN)

struct HbLocaleMapping
{
    int symLangValue;
    QString languageDllId;
    QString collationDllId;
    QString regionDllId;
    QString langName;
    QString regName;
    QString collName;
};

QList<HbLocaleMapping> mappingList;
QList<int> regions;
QStringList availRegions;
QHash<QString, QString> locRegionNames;
QList<int> collations;
QStringList availCollations;
QHash<QString, QString> locCollationNames;

/*!
    \brief Reads langauge, region and collation mappings.
*/
void readMappings()
{
    QString path = "c:";
    path += QString(LANGUAGE_MAPPINGS_FILE);
    QFile* file = new QFile(path);
    if (!file->exists() ) {
        path = "z:";
        path += QString(LANGUAGE_MAPPINGS_FILE);
        delete file;
        file = new QFile(path);
    }
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        delete file;
        return;
    }
    QTextStream in(file);
    while (!in.atEnd()) {
        QString line = in.readLine(256);
        if (!line.isEmpty()) {
            QStringList list = line.split(',', QString::SkipEmptyParts);
            if (list.count() < 7) {
                continue;
            }
            QString strCode = list[0];
            QString strLang = list[1];
            QString strRegion = list[2];
            QString strCollation = list[3];
            QString lanName = list[4]; //en_GB
            QString region = list[5]; //en_GB
            QString collation = list[6]; //en_GB
            
            bool ok;
            int code = strCode.toUInt(&ok);
            if (!ok) {
                continue;
            }
            
            HbLocaleMapping map;
            map.symLangValue = code;
            map.languageDllId = strLang;
            map.collationDllId = strCollation;
            map.regionDllId = strRegion;
            map.langName = lanName;
            map.regName = region;
            map.collName = collation;
            mappingList.append(map);
        }
    }
    delete file;
    return;
}
#endif // Q_OS_SYMBIAN

#if defined(Q_OS_SYMBIAN)

/*!
    \brief Changes the system UI language.
      
    \param dllExtension extension of the locale dll
    \return true if operation was successful
*/
bool setLocale( const QString &dllExtension )
{
    TExtendedLocale dummy;
    dummy.LoadSystemSettings();
    QString name = QString( "elocl_lan." ).append( dllExtension );
    TPtrC nameptr(name.utf16());
    
    TInt err = dummy.LoadLocaleAspect( nameptr );
    if( err != KErrNone )
        return false;
    dummy.SaveSystemSettings();
    // cause localeprivate update on next qlocale object( glp->m_language_id = 0 )
    QSystemLocale dummy2;
    return true;
}
#endif //Q_OS_SYMBIAN

/*!
    \brief Return identifier of the current UI language.
    
    \attention Symbian specific API

    \return Identifier of the language code for Symbian and empty QString for other platforms.
*/ 
QString HbLocaleUtil::currentLanguage()
{
#if defined(Q_OS_SYMBIAN)
    TLanguage l = User::Language();
    
    if(mappingList.isEmpty()) {
        readMappings();
    }
    
    for (int i = 0; i < mappingList.count(); ++i) {
        HbLocaleMapping mapping = mappingList.at(i);
        if (mapping.symLangValue == l) {
            return mapping.langName;
        }
    }
#endif
   return QString();
}

/*!
    \brief Returns identifiers of languages supported in a device. 
    Language identifier may be two-letter ISO 639 language code or combination of ISO 639 language code and ISO 3166 country code 
    Ex: Great Britain english it returns "en".
    Ex: For U.S. english it returns "en_US"  
    
    \attention Symbian specific API
    
    \return identifiers of supported languages for Symbian and empty QStringList for other platforms 
*/
QStringList HbLocaleUtil::supportedLanguages()
{
#if defined(Q_OS_SYMBIAN)   
    QStringList languages; 
    CArrayFixFlat<TInt>* systemEpocLanguageCodes = 0;
    TInt error = SysLangUtil::GetInstalledLanguages( systemEpocLanguageCodes );
    if ( error != KErrNone ) {
        delete systemEpocLanguageCodes;
        return languages;
    }
    
    if(mappingList.isEmpty()) {
        readMappings();
    }
    
    for (int i = 0; i < systemEpocLanguageCodes->Count(); ++i) {
        int code = systemEpocLanguageCodes->At(i);
        for (int j = 0; j < mappingList.count(); ++j) {
            HbLocaleMapping map = mappingList.at(j);
            if (map.symLangValue == code) {
                languages.append(map.langName);
                break;
            }
        }
    }
    
    delete systemEpocLanguageCodes;
    return languages;
#else 
    return QStringList();
#endif
}

/*!
    \brief Converts two or five letter language identifier code to localised language name. 
    
    \attention Symbian specific API
    
    \param language identifier 
    
    \return Symbian - localised name of the language, an empty String if translation fails
    \return other platforms - empty QString    
*/ 
QString HbLocaleUtil::localisedLanguageName( const QString &language )
{
#if defined(Q_OS_SYMBIAN)   
    QTranslator translator;
    QString path = "c:";
    path += QString(LANGUAGE_TRANSLATOR_PATH);
    if (!translator.load(path)) {
        path = "z:";
        path += QString(LANGUAGE_TRANSLATOR_PATH);
        if (!translator.load(path)) {
            return QString("");
        } 
    } 
    
    QCoreApplication::installTranslator(&translator);
    QString languageName = QString(LANGUAGE_ID_PREFIX);
    languageName += '_';
    languageName += language;
    QString translated = hbTrId(languageName.toAscii().constData());  
    if (translated == languageName) {
        return QString("");
    }
    return translated;
#else 
    Q_UNUSED(language); 
    return QString();
#endif
}

/*!
    \brief Changes the system language.  
    The language parameter should correspond with one of the identifiers returned by supportedLanguages(). 
    
    \attention Symbian specific API
  
    \param language identifier of language to set active
    
    \return true if language change was successful and false for other platforms
*/ 
bool HbLocaleUtil::changeLanguage( const QString &language )
{
#if defined(Q_OS_SYMBIAN)
    if(mappingList.isEmpty()) {
        readMappings();
    }
	
    int lanCode = -1;
    QString dllExt = "";
    for (int i = 0;  i < mappingList.count(); ++i) {
        HbLocaleMapping map = mappingList.at(i);
        if (map.langName == language) {
            lanCode = map.symLangValue;
            dllExt = map.languageDllId;
            break;
        }
    }
    
    if (lanCode == -1) {
        return false;
    }
    
    CRepository* commonEngineRepository = 0;
    TRAPD( err1, commonEngineRepository = CRepository::NewL( KCRUidCommonEngineKeys ) );    
    if ( err1 != KErrNone ) { 
        return false;
    }
    
    if (!setLocale(dllExt)) {
        delete commonEngineRepository;
        return false;
    }
        
    // Never set Language code 0 to HAL
    if ( language !=0 ) {
        if ( HAL::Set( HAL::ELanguageIndex, lanCode ) != KErrNone ) {
            delete commonEngineRepository;
            return false;
        }
    }
    if ( commonEngineRepository->Set( KGSDisplayTxtLang, lanCode ) != KErrNone ) {
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

#if defined(Q_OS_SYMBIAN)  
/*!
    \brief reads the regions.txt file and reads the list of symbian region codes 
*/
void readRegions()
{
    QFile* file = new QFile(REGION_LIST_FILE);
    if (!file->exists() ) 
    {
        delete file;
        return;
    }
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        delete file;
        return;
    } 
    QTextStream in(file);
    while (!in.atEnd())
    {
        QString line = in.readLine(256);
        if (!line.isEmpty())
        {
            int regCode = line.toUInt();
            regions.append(regCode);
        }
    }
    return;
}
#endif

/*!
    \brief Returns names supported regions in a phone. 
    Region names are identified by 2 letter code(ISO 3166 standard).
    Ex: For United Kingdom it returns GB 
    
    \attention Symbian specific API

    \return list of supported regions in a device for Symbian and empty QStringList for other platforms 
*/
QStringList HbLocaleUtil::supportedRegions()
{
#if defined(Q_OS_SYMBIAN)
    if(!availRegions.isEmpty())
    {
        return availRegions;
    }

    if(regions.isEmpty())
    {
        readRegions();
    }
    
    if(mappingList.isEmpty())
    {
        readMappings();
    }
    int regCount = regions.count();
    for(int i = 0; i < regCount; i++)
    {
        int region = regions.at(i);
        int count = mappingList.count();
        for (int j = 0; j < count; j++)
        {
            HbLocaleMapping mapping = mappingList.at(j);
            QString regCode = mapping.regionDllId;
            if(region == regCode.toUInt())
            {
                availRegions.append(mapping.regName);
                break;
            }
        }
    }
    return availRegions;
#else
    return QStringList();
#endif
}

/*!
    \brief Converts two letter region identifier to localised region name. 
    
    \attention Symbian specific API
    
    \param region region identifier 
    
    \return Symbian - localised name of the region, an empty String if translation fails
    \return other platforms - empty QString    
*/ 
QString HbLocaleUtil::localisedRegionName( const QString &region ) 
{
#if defined(Q_OS_SYMBIAN)       
    if(locRegionNames.isEmpty())
    {
        QTranslator translator;
        QString path = "c:";
        path += QString(REGION_TRANSLATOR_PATH);
        if (!translator.load(path)) {
            path = "z:";
            path += QString(REGION_TRANSLATOR_PATH);
            if (!translator.load(path)) {
                return QString("");
            } 
        } 

        QCoreApplication::installTranslator(&translator);
            
        if(mappingList.isEmpty()) 
        {
            readMappings();
        }
        int cnt = mappingList.count();
        for(int i = 0 ; i < cnt; i++ )
        {
            HbLocaleMapping map = mappingList.at(i);
            QString regionName = QString(REGION_ID_PREFIX);
            regionName += '_';
            regionName += map.regName;
            QString locRegName = hbTrId(regionName.toAscii().constData());
            if(locRegName != regionName)
                locRegionNames.insert(map.regName, locRegName);
        }
    }
    
    return locRegionNames.value(region);
#else
    Q_UNUSED(region);
    return QString();
#endif    
}

/*!
    \brief Changes the system region.  
    The region parameter should correspond with one of the identifiers returned by supportedRegions(). 
  
    \attention Symbian specific API

    \param region identifier of region to set active
    
    \return true if region change was successful for Symbian and false for other platforms
*/ 
bool HbLocaleUtil::changeRegion( const QString &region )
{
#if defined(Q_OS_SYMBIAN)   
    TExtendedLocale dummy;
    QString regDllName = QString( "elocl_reg." );
    
    if(mappingList.isEmpty())
    {
        readMappings();
    }
    int count = mappingList.count();
    for (int j = 0; j < count; j++)
    {
        HbLocaleMapping mapping = mappingList.at(j);
        QString name = mapping.regName;
        if(name == region)
        {
            dummy.LoadSystemSettings();
            regDllName += mapping.regionDllId;
            TPtrC nameptr(regDllName.utf16());
            TInt err = dummy.LoadLocaleAspect( nameptr );
            if( err != KErrNone )
                return false;
            dummy.SaveSystemSettings();
            // cause localeprivate update on next qlocale object
            QSystemLocale dummy2;
            return true; 
        }
    }
    return false;
#else
    Q_UNUSED(region);
    return false;
#endif    
}

/*!
    \brief Return identifier of the current region.

    \attention Symbian specific API

    \return identifier of the region for Symbian and empty QString for other platforms
*/ 
QString HbLocaleUtil::currentRegion()
{
#if defined(Q_OS_SYMBIAN)      
    if(mappingList.isEmpty())
    {
        readMappings();
    }
    TRegionCode reg = User::RegionCode();
    int cnt = mappingList.count();
    for(int i = 0; i < cnt; i++)
    {
        HbLocaleMapping mapping = mappingList.at(i);
        int dllid = mapping.regionDllId.toInt();
        if(dllid == reg)
        {
            return mapping.regName;
        }
    }
#endif    
    return QString();
}

#if defined(Q_OS_SYMBIAN)      
/*!
    \brief reads the collations.txt file and reads the list of symbian collation codes 
*/
void readCollations()
{
    QFile* file = new QFile(COLLATION_LIST_FILE);
    if (!file->exists() ) 
    {
        delete file;
        return ;
    }
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        delete file;
        return ;
    } 
    QTextStream in(file);
    while (!in.atEnd())
    {
        QString line = in.readLine(256);
        if (!line.isEmpty())
        {
            int colCode = line.toUInt();
            collations.append(colCode);
        }
    }
    return ;
}
#endif

/*!
    \brief Returns identifiers of collations supported in a device. 
    Collation identifier may be two-letter ISO 639 language code or combination of ISO 639 language code and ISO 3166 country code 
    Ex: Great Britain english it returns "en".
    Ex: For U.S. english it returns "en_US"  
    
    \attention Symbian specific API

    \return identifiers of supported collations for Symbian and empty QStringList for other platforms 
*/
QStringList HbLocaleUtil::supportedCollations()
{
#if defined(Q_OS_SYMBIAN)
    if(!availCollations.isEmpty())
    {
        return availCollations;
    }

    if(collations.isEmpty())
    {
        readCollations();
    }
    
    if(mappingList.isEmpty())
    {
        readMappings();
    }
    int colCount = collations.count();
    for(int i = 0; i < colCount; i++)
    {
        int collation = collations.at(i);
        int count = mappingList.count();
        for (int j = 0; j < count; j++)
        {
            HbLocaleMapping mapping = mappingList.at(j);
            QString colCode = mapping.collationDllId;
            if(collation == colCode.toUInt())
            {
                availCollations.append(mapping.collName);
                break;
            }
        }
    }
    return availCollations;
#else
    return QStringList();
#endif
}

/*!
    \brief Converts collation identifier to localised collation name. 
    
    \attention Symbian specific API
    
    \param collation region collation identifier 
    
    \return Symbian - localised name of the collation, an empty String if translation fails
    \return other platforms - empty QString    
*/ 
QString HbLocaleUtil::localisedCollationName( const QString &collation ) 
{
#if defined(Q_OS_SYMBIAN)       
    if(locCollationNames.isEmpty())
    {
        QTranslator translator;
        QString path = "c:";
        path += QString(COLLATION_TRANSLATOR_PATH);
        if (!translator.load(path)) {
            path = "z:";
            path += QString(COLLATION_TRANSLATOR_PATH);
            if (!translator.load(path)) {
                return QString("");
            } 
        } 

        QCoreApplication::installTranslator(&translator);
            
        if(mappingList.isEmpty()) 
        {
            readMappings();
        }
        int cnt = mappingList.count();
        for(int i = 0 ; i < cnt; i++ )
        {
            HbLocaleMapping map = mappingList.at(i);
            QString collationName = QString(COLLATION_ID_PREFIX);
            collationName += '_';
            collationName += map.collName;
            QString locColName = hbTrId(collationName.toAscii().constData());
            if(locColName != collationName)
                locCollationNames.insert(map.collName, locColName);
        }
    }
    
    return locCollationNames.value(collation);
#else
    Q_UNUSED(collation);
    return QString();
#endif    
}

/*!
    \brief Changes the system collation.  
    The collation parameter should correspond with one of the identifiers returned by supportedCollations(). 
  
    \attention Symbian specific API
    
    \param collation identifier of collation to set active
    \return true if collation change was successful for Symbian and false for other platforms
*/ 
bool HbLocaleUtil::changeCollation( const QString &collation )
{
#if defined(Q_OS_SYMBIAN) 
    TExtendedLocale dummy;
    QString colDllName = QString( "elocl_col." );
    
    if(mappingList.isEmpty())
    {
        readMappings();
    }
    int count = mappingList.count();
    for (int j = 0; j < count; j++)
    {
        HbLocaleMapping mapping = mappingList.at(j);
        QString name = mapping.collName;
        if(name == collation)
        {
            dummy.LoadSystemSettings();
            colDllName += mapping.collationDllId;
            TPtrC nameptr(colDllName.utf16());
            TInt err = dummy.LoadLocaleAspect( nameptr );
            if( err != KErrNone )
                return false;
            dummy.SaveSystemSettings();
            // cause localeprivate update on next qlocale object
            QSystemLocale dummy2;
            return true; 
        }
    }
    return false;
#else
    Q_UNUSED(collation);
    return false;
#endif    
}

/*!
    \brief Return identifier of the current collation.

    \attention Symbian specific API
    
    \return identifier of the collation for Symbian and empty QString for other platforms
*/ 
QString HbLocaleUtil::currentCollation()
{
#if defined(Q_OS_SYMBIAN)      
    if(mappingList.isEmpty())
    {
        readMappings();
    }
    TExtendedLocale dummy;
    dummy.LoadSystemSettings();
    TBuf<256> name;
    dummy.GetLocaleDllName(ELocaleCollateSetting,name);

    QString dllname;
#ifdef QT_NO_UNICODE
    dllname = QString::fromLocal8Bit(name.Ptr(), name.Length());
#else
    dllname = QString::fromUtf16(name.Ptr(), name.Length());
#endif    
    
    dllname = dllname.right(COLLATION_DLL_PREFIX_POSITION);
    int cnt = mappingList.count();
    for(int i = 0; i < cnt; i++)
    {
        HbLocaleMapping mapping = mappingList.at(i);
        QString dllid = mapping.collationDllId;
        if(dllid == dllname)
        {
            return mapping.collName;
        }
    }
#endif    
    return QString();    
}

/*!
    \brief Changes the system language, region and collation.  
    The language parameter should correspond with one of the identifiers returned by supportedLanguages(). 
    Proper region and collation is selected automatically according the language.  
  
    \attention Symbian specific API
    
    \param language identifier of language which language, region and collation settings should set active  
    
    \return Symbian - true if language, region and collation change was successful
    \return other platforms - false    
*/ 
bool HbLocaleUtil::changeLocale( const QString &language )
{
#if defined(Q_OS_SYMBIAN) 
    if(mappingList.isEmpty()) {
        readMappings();
    }

    for (int i = 0;  i < mappingList.count(); ++i) {
        HbLocaleMapping map = mappingList.at(i);
        if (map.langName == language) {
            if (!changeLanguage(map.langName)) {
                return false;
            } 
            if (!changeRegion(map.regName)) {
                return false;
            } 
            if (!changeCollation(map.collName)) {
                return false;
            } 
            return true;
        }
    }
#else
    Q_UNUSED(language);
#endif // Q_OS_SYMBIAN
    return false;   
}

