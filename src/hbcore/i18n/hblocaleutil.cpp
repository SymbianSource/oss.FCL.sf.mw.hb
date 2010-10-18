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
#define LANGUAGE_LIST_FILE "/resource/hb/translations/language_list.txt"
#define LANGUAGE_MAPPINGS_FILE "/resource/hb/translations/locale_mappings.txt"
#define LANGUAGE_TRANSLATOR_PATH "/resource/hb/translations/languages"
#define REGION_TRANSLATOR_PATH "/resource/hb/translations/regions"
#define COLLATION_TRANSLATOR_PATH "/resource/hb/translations/collations"

#define LANGUAGE_ID_PREFIX "txt_language"
#define REGION_ID_PREFIX "txt_region"
#define COLLATION_ID_PREFIX "txt_collation"

#define COLLATION_DLL_PREFIX_POSITION 3
#endif // Q_OS_SYMBIAN

/*!
    @beta
    @hbcore
    \class HbLocaleUtil
    \brief The HbLocaleUtil class supports querying the supported languages, regions and collations, and activating them.
     
    With HbLocaleUtil, you can query the supported languages, regions and 
    collations from the device, and the ones currently set in the device. You 
    can then use this data to activate the language, region and collation in the 
    device; either individually, or as a set determined by the specified 
    language.
    
    HbLocaleUtil returns the data from the device as identifiers. The language 
    and collation identifiers typically correspond to two-letter ISO 639 
    language codes. However, some languages and collations use a combination of 
    an ISO 639 language code and an ISO 3166 country code, separated by an 
    underscore character. Region identifiers always correspond to a two-letter 
    ISO 3166 country code.
    
    HbLocaleUtil also provides functions for converting language, region and 
    collation identifiers to their localized names. 
    
    Example:
    \snippet{unittest_hblocaleutil/unittest_hblocaleutil.cpp,1}
    
    
    \sa HbStringUtil, HbExtendedLocale
    
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
QStringList availRegions;
QHash<QString, QString> locRegionNames;
QStringList availCollations;
QHash<QString, QString> locCollationNames;

/*!
    \relates HbLocaleUtil
    
    Reads language, region and collation mappings.
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

/*!

    Returns the identifier of the current UI language. Typically the language 
    identifier is an ISO 639 language code, but in some cases, it is a 
    combination of an ISO 639 language code and an ISO 3166 country code. For 
    example, for UK English, this returns "en", and for US English, this returns 
    "en_US".
    
    \attention Only fully implemented on the Symbian platform. For other 
    platforms, always returns an empty QString.
    
    \sa supportedLanguages(), localisedLanguageName(), changeLanguage(), languageRegionMappings()

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
    
    Returns the identifiers of the languages supported on the device. Typically 
    the language identifier is an ISO 639 language code, but in some cases, it 
    is a combination of an ISO 639 language code and an ISO 3166 country code. 
    For example, for UK English, this returns "en", and for US English, this 
    returns "en_US".
    
    \attention Only fully implemented on the Symbian platform. For other 
    platforms, always returns an empty QStringList.
    
    \sa localisedLanguageName(), currentLanguage(), changeLanguage(), changeLocale()
    
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
    
    Returns the localised language name for the specified language identifier. 
    If the translation fails, returns an empty QString.
    
    \param language The two- or five-letter language identifier that corresponds to one of the following:
    - An identifier returned by supportedLanguages() or currentLanguage()
    - An HbLanguageRegionMapping::languageId string
    
    \attention Only fully implemented on the Symbian platform. For other 
    platforms, always returns an empty QString.
    
    \sa changeLanguage(), currentLanguage()
 
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

    Changes the system language to the one specified in \a language.
  
    \param language The language identifier. This must correspond to an 
    identifier returned by supportedLanguages(), or an 
    HbLanguageRegionMapping::languageId string.
    
    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. For other 
    platforms, always returns \c false.
    
    \sa localisedLanguageName(), currentLanguage()
    
*/ 
bool HbLocaleUtil::changeLanguage( const QString &language )
{
#if defined(Q_OS_SYMBIAN)
    if(mappingList.isEmpty()) {
        readMappings();
    }
    int lanCode = -1;
    for (int i = 0;  i < mappingList.count(); ++i) {
        HbLocaleMapping map = mappingList.at(i);
        if (map.langName == language) {
            int retVal = -1;
            lanCode = map.symLangValue;
            retVal = SysLangUtil::ChangeLanguage(lanCode);
            if (!retVal) {
                // Never set Language code 0 to HAL
                if ( language != 0 ) {
                    HAL::Set( HAL::ELanguageIndex, lanCode );
                }
                return true;
            }
            else {
                return false;
            }
        }
    }        
    return false;
#else
    Q_UNUSED(language);
    return false;
#endif
}

/*!
    
    Returns the identifiers of regions supported on the device. The region 
    identifier is a two-letter ISO 3166 code, for example, "GB" for United 
    Kingdom.
    
    \attention Only fully implemented on the Symbian platform. For other 
    platforms, always returns an empty QStringList.
    
    \sa localisedRegionName(), currentRegion(), changeRegion()

*/
QStringList HbLocaleUtil::supportedRegions()
{
#if defined(Q_OS_SYMBIAN)
    if(!availRegions.isEmpty())
    {
        return availRegions;
    }

    QStringList regions; 
    CArrayFixFlat<TInt>* systemEpocRegionCodes = 0;
    TInt error = SysLangUtil::GetInstalledRegions( systemEpocRegionCodes );
    if ( error != KErrNone ) {
        delete systemEpocRegionCodes;
        return regions;
    }
    
    if(mappingList.isEmpty()) {
        readMappings();
    }
    
    for (int i = 0; i < systemEpocRegionCodes->Count(); ++i) {
        int code = systemEpocRegionCodes->At(i);
        for (int j = 0; j < mappingList.count(); ++j) {
            HbLocaleMapping map = mappingList.at(j);
            QString dllid = map.regionDllId;
            bool ok;
            int symRegionValue = dllid.toInt(&ok,10);
            if (symRegionValue == code) {
                availRegions.append(map.regName);
                break;
            }
        }
    }
    
    delete systemEpocRegionCodes;
    return availRegions;
#else 
    return QStringList();
#endif
}

/*!

    Returns the localised region name for the given two-letter region 
    identifier. If the translation fails, returns an empty QString.
    
    \param region The region identifier that corresponds to one of the following:
    - An identifier returned by supportedRegions() or currentRegion()
    - An HbLanguageRegionMapping::regionId string
    
    \attention Only fully implemented on the Symbian platform. For other 
    platforms, always returns an empty QString.
    
    \sa currentRegion(), changeRegion()
  
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
    Changes the system region to the one specified in \a region.

    \param region The region identifier. This must correspond to an identifier 
    returned by supportedRegions(), or an HbLanguageRegionMapping::regionId 
    string.
    
    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. For other 
    platforms, always returns \c false.
    
    \sa localisedRegionName(), currentRegion()
    
*/ 
bool HbLocaleUtil::changeRegion( const QString &region )
{
#if defined(Q_OS_SYMBIAN)   
   
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
            int retVal = -1;
            QString dllid = mapping.regionDllId;
            bool ok;
            int regioncode = dllid.toInt(&ok,10);
            if(!regioncode)
                return false;
            retVal = SysLangUtil::ChangeRegion(regioncode);
            if(retVal != 0)
                return false;
            else
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
    
    Returns the identifier of the current region. The region identifier is a 
    two-letter ISO 3166 code, for example, "GB" for United Kingdom.

    \attention Only fully implemented on the Symbian platform. For other 
    platforms, always returns an empty QString.
    
    \sa supportedRegions(), localisedRegionName(), changeRegion(), languageRegionMappings()

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

/*!
    
    Returns the identifiers of the collations supported on a device. Typically 
    the collation identifier is an ISO 639 language code, but in some cases, it 
    is a combination of an ISO 639 language code and an ISO 3166 country code. 
    For example, for UK English, this returns "en", and for US English, this 
    returns "en_US".
    
    \attention Only fully implemented on the Symbian platform. For other 
    platforms, always returns an empty QStringList.
    
    \sa localisedCollationName(), currentCollation(), changeCollation()
 
*/
QStringList HbLocaleUtil::supportedCollations()
{
#if defined(Q_OS_SYMBIAN)
    if(!availCollations.isEmpty())
    {
        return availCollations;
    }

    QStringList collations; 
    CArrayFixFlat<TInt>* systemEpocCollationCodes = 0;
    TInt error = SysLangUtil::GetInstalledCollations( systemEpocCollationCodes );
    if ( error != KErrNone ) {
        delete systemEpocCollationCodes;
        return collations;
    }
    
    if(mappingList.isEmpty()) {
        readMappings();
    }
    
    for (int i = 0; i < systemEpocCollationCodes->Count(); ++i) {
        int code = systemEpocCollationCodes->At(i);
        for (int j = 0; j < mappingList.count(); ++j) {
            HbLocaleMapping map = mappingList.at(j);
            bool ok;
            int symCollatValue = map.collationDllId.toInt(&ok,10);
            if (symCollatValue == code) {
                availCollations.append(map.collName);
                break;
            }
        }
    }
    
    delete systemEpocCollationCodes;
    return availCollations;
#else 
    return QStringList();
#endif
}

/*!
    
    Returns the localised collation name for the given collation identifier. If 
    the translation fails, returns an empty QString.
    
    \param collation The region collation identifier that corresponds to one of the following:
    - An identifier returned by supportedCollations() or currentCollation()
    - An HbLanguageRegionMapping::collationId string
    
    \attention Only fully implemented on the Symbian platform. For other 
    platforms, always returns an empty QString.
    
    \sa currentCollation(), changeCollation()
  
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
    
    Changes the system collation to the one specified in \a collation.
    
    \param collation The collation identifier. This must correspond to an 
    identifier returned by supportedCollations(), or an 
    HbLanguageRegionMapping::collationId string.
    
    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. For other 
    platforms, always returns \c false.
    
    \sa localisedCollationName(), currentCollation()
*/ 
bool HbLocaleUtil::changeCollation( const QString &collation )
{
#if defined(Q_OS_SYMBIAN) 
    
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
            int retVal = -1;
            QString dllid = mapping.collationDllId;
            bool ok;
            int collCode = dllid.toInt(&ok,10);
            if(!collCode)
                return false;
            retVal = SysLangUtil::ChangeCollation(collCode);
            if(retVal != 0)
                return false;
            else
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
    
    Returns the identifier of the current collation. Typically the collation 
    identifier is an ISO 639 language code, but in some cases, it is a 
    combination of an ISO 639 language code and an ISO 3166 country code. For 
    example, for UK English, this returns "en", and for US English, this returns 
    "en_US".

    \attention Only fully implemented on the Symbian platform. For other 
    platforms, always returns an empty QString.
    
    \sa supportedCollations(), localisedCollationName(), changeCollation(), languageRegionMappings()
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

    Changes the system language, region and collation as specified by \a 
    language. The region and collation are selected automatically based on the 
    specified language.

    \param language The language identifier. This must correspond to an 
    identifier returned by supportedLanguages(), or an 
    HbLanguageRegionMapping::languageId string.
  
    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. For other 
    platforms, always returns \c false.
    
    \sa supportedRegions(), supportedCollations(), languageRegionMappings()
 
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

/*!
    
    \struct HbLanguageRegionMapping hblocaleutil.h
    
    Defines the data structure for language, region and collation identifier 
    mappings.
    
    \sa HbLocaleUtil::languageRegionMappings()
  
*/

/*!
    
    \var HbLanguageRegionMapping::languageId
    
    A language identifier. Typically this is an ISO 639 language code, but in 
    some cases, it is a combination of an ISO 639 language code and an ISO 3166 
    country code. For example, for UK English, this returns "en", and for US 
    English, this returns "en_US".
  
*/ 

/*!
    
    \var HbLanguageRegionMapping::regionId
    
    A region identifier. This is a two-letter ISO 3166 code, for example, "GB" 
    for United Kingdom.
  
*/ 

/*!
    
    \var HbLanguageRegionMapping::collationId
    
    A collation identifier. Typically this is an ISO 639 language code, but in 
    some cases, it is a combination of an ISO 639 language code and an ISO 3166 
    country code. For example, for UK English, this returns "en", and for US 
    English, this returns "en_US".
  
*/ 

/*!
    
    Returns a list of language, region and collation identifier mappings. For 
    example, for UK English, the mapping has "en" for language, "GB" for region 
    and "en" for collation. For US English, the corresponding values are 
    "en_US", "US" and "en_US". You can use the optional \a onlySupported 
    parameter to limit the search only to the languages, regions and collations 
    supported on the device.
        
    If there is a mapping with a language that does not have a corresponding 
    region, the region is returned as an empty QString. Similarly, if there is a 
    mapping with a region that does not have a corresponding language and 
    collation, the language and collation are returned as empty QStrings.
    
    \param onlySupported Specify \c true to only return the mappings for 
    languages, regions and collations that are supported on the device. To 
    return mappings for all known languages, regions and collations, specify \c 
    false.
    
    For example:
    \snippet{unittest_hblocaleutil/unittest_hblocaleutil.cpp,2}

    \attention Only fully implemented on the Symbian platform. For other 
    platforms, always returns an empty list.
    
    \sa currentLanguage(), currentRegion(), currentCollation()
  
*/ 
QList<HbLanguageRegionMapping> HbLocaleUtil::languageRegionMappings( bool onlySupported )
{
    QList<HbLanguageRegionMapping> mps;
#if defined(Q_OS_SYMBIAN) 
    if (mappingList.isEmpty()) {
        readMappings();
    }
   
    QStringList supportedLanguages;
    QStringList supportedRegions;
    if (onlySupported) {
        supportedLanguages = HbLocaleUtil::supportedLanguages();
        supportedRegions = HbLocaleUtil::supportedRegions();
    }
    
    for (int i = 0;  i < mappingList.count(); ++i) {

        HbLocaleMapping map = mappingList.at(i);

        HbLanguageRegionMapping mapping;
        mapping.languageId = map.langName;
        mapping.regionId = map.regName;
        mapping.collationId = map.collName;

        if (onlySupported) {
            bool langSupported = supportedLanguages.contains(map.langName);
            bool regSupported = supportedRegions.contains(map.regName);
            if (!langSupported && !regSupported) {
                continue;
            }
            if (!langSupported) {
                mapping.languageId = "";
            }
            if (!regSupported) {
                mapping.regionId = "";
            }
        }

        mps.append(mapping);

    }
#else
    Q_UNUSED(onlySupported);
#endif // Q_OS_SYMBIAN
    return mps;
}

