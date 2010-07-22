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

#include <QApplication>
#include <QFileInfo>
#include <QLocale>
#include <QTranslator> 
#include <QHash>

#include <hbfindfile.h>
#include <hbtranslator.h>
#include <hbtranslator_p.h>

#if defined(Q_OS_SYMBIAN)
    #define PLATFORM_WITH_DRIVES
#elif defined(Q_OS_WIN32)
    #define PLATFORM_WITH_DRIVES
#else
    #undef PLATFORM_WITH_DRIVES
#endif

#ifdef Q_OS_SYMBIAN
const char* defaultPath   = "/resource/qt/translations/";
const char* defaultDrive  = "Z:";
const char* defaultCommon = "common_";
#else
const char* defaultPath = "";
const char* defaultDrive  = "";
const char* defaultCommon = "common_";

#endif

#ifdef Q_OS_SYMBIAN
#include <f32file.h>
#include <eikenv.h>
#endif

#ifdef Q_OS_SYMBIAN
/*!
    Convert path to Symbian version
*/
static void toSymbianPath(QString &path) {    
    int len=path.length();
    for (int i=0; i<len; i++) {
        QCharRef ref=path[i];
        if (ref == '/') {
           ref= '\\';
        }
    }
}
#endif

/*!
    Load data from translator
*/
bool loadTranslatorData(QTranslator &translator, QString &qmFile, uchar* &buffer, bool doFallback=true) {
#ifndef Q_OS_SYMBIAN    
    Q_UNUSED(doFallback);
    buffer = 0;
    return translator.load(qmFile);
#else    
    RFile fl;
    RFs& fs = CCoeEnv::Static()->FsSession();
    QString qmFile2;        
    QString delims="_.";
    TInt err;
    for (;;) {
        qmFile2=qmFile;
        qmFile2 += QString(".qm");        

        TPtrC ptrFName(reinterpret_cast<const TText*>(qmFile2.constData()));                   
        err= fl.Open(fs, ptrFName, EFileShareReadersOrWriters | EFileRead | EFileStream );
        if (err == KErrNotFound) { 
           if (!doFallback) { // no fallback, then return
               return false;
           }        
           // else continue
        } else {
            if (err != KErrNone ) { // if some other error return error, don't look anymore
                return false;
            } else {
                break; // file was found
            }
        }
        // do fallback
        qmFile2 = qmFile;
        ptrFName.Set((ushort*)(qmFile2.constData()), qmFile2.length());
        err= fl.Open(fs, ptrFName, EFileShareReadersOrWriters | EFileRead | EFileStream );
        if (err == KErrNone) {
            break;
        } else {
            if (err != KErrNotFound) {
                return false;
            }
        }
        // check fallback names
        int rightmost = 0;
        for (int i=0; i<(int)delims.length(); i++) {
            int k=qmFile.lastIndexOf(delims[i]);
            if (k>rightmost) {
                rightmost=k;
            }    
        }

        if (rightmost==0) {
            return false;
        }
        qmFile.truncate(rightmost);                
    }
    
    TInt sz;
    err = fl.Size(sz);
    if (err != KErrNone) {
        fl.Close();
        return false;
    }   
    uchar *buf = new uchar[sz];
    TPtr8 bufPtr(buf,0,sz); 
    err = fl.Read(bufPtr, sz);
    if (err != KErrNone) {
        fl.Close();
        return false;
    }        
    fl.Close();
    if (!translator.load(bufPtr.Ptr(),sz)) {
        delete buf;
        return false;
    }
    buffer = buf;
    return true;
    
#endif    
}

/*!
    @stable
    @hbcore
    \class HbTranslator
    \brief HbTranslator installs QTranslator(s) automatically needed in localisation
    and loads translation files into QTranslator.

    Note: file name should be given without language and .qm postfix.
    Eg. if English qm file name is testapp_en.qm
    \code
    HbTranslator trans("testapp");
    \endcode
    which loads correct testapp_xx.qm file from default location, where xx is postfix for current system locale, eg "en".

    \sa hbTrId
*/

/*!
    Default case: searches translation file from default location (/resource/qt/translations/) with default name, which is EXECUTABLENAME_xx.qm
    
    \attention Cross-Platform API
*/
HbTranslator::HbTranslator(): d(new HbTranslatorPrivate())
{
    QFileInfo info(qApp->applicationFilePath());
    QString defaultName = info.baseName();  // defaultname = <executablename>
    d->translatorPath=defaultPath;
    d->translatorFile=defaultName;
    d->installTranslator(defaultPath, defaultName);
}

/*!
    Searches translation file \a file from default location.
    
    \attention Cross-Platform API
*/
HbTranslator::HbTranslator(const QString &file): d(new HbTranslatorPrivate())
{
    d->translatorPath=defaultPath;
    d->translatorFile=file;    
    d->installTranslator(defaultPath, file);
}

/*!
    Searches translation file \a file from path \a path.
    
    \attention Cross-Platform API

    \code
    HbTranslator trans("/resource/qt/custom/", "customfile");
    \endcode
*/
HbTranslator::HbTranslator(const QString &path, const QString &file): d(new HbTranslatorPrivate())
{
    d->translatorPath=defaultPath;
    d->translatorFile=file;    
    d->installTranslator(path, file);
}

/*!
    Destructor  
*/ 
HbTranslator::~HbTranslator()
{
    delete d;
}

static bool commonTr = false;
#include <QMutex>
#include <QMutexLocker>
Q_GLOBAL_STATIC_WITH_ARGS(QMutex, gs_CommonTrMutex, (QMutex::Recursive) )

/*!
    Loads common.ts translations from default location.
    
    \attention Cross-Platform API
*/
void HbTranslator::loadCommon()
{   
    QMutexLocker ml(gs_CommonTrMutex());
    if (!commonTr) {
        QString lang = QLocale::system().name();
        d->languageDowngrade(lang);    
        QString commonts;
        commonts += QString(defaultDrive);
        commonts += QString(defaultPath);
        commonts += QString(defaultCommon);
        commonts += lang;
#ifdef Q_OS_SYMBIAN        
        toSymbianPath(commonts);
#endif    
        bool loaded;
        uchar *commonData=0;
        loaded = loadTranslatorData(d->common, commonts, d->commonData);
        if (loaded) {
            d->commonData = commonData;
            d->commonTr=true;
            qApp->installTranslator(&d->common);    
            commonTr=true;
        }
    }    
}

/*!
    Destructor  
*/ 
HbTranslatorPrivate::~HbTranslatorPrivate()
{
    qApp->removeTranslator(&translator);    
    if (translatorData) {
        delete [] translatorData;
    }
    qApp->removeTranslator(&common);    
    if (commonData) {        
        delete [] commonData;
    }
    if (commonTr) {
       ::commonTr=false;
    }    
}

/*!
    Internal function for common operations of HbTranslator.  
     
    \param pth path for translation file
    \param name translation file
*/ 
void HbTranslatorPrivate::installTranslator(const QString &pth, const QString &name)
{
    QString lang = QLocale::system().name();
    QString lang2 = lang;
    languageDowngrade(lang);
    QString path(pth);
    if (path.isNull() || path.isEmpty()) {
        return;
    }
    if (path.at(0) == ':') {
        QString tsfile = path + name + QString("_") + lang;
        if ( translator.load(tsfile) ) {
            qApp->installTranslator(&translator);
        }
        return;
    }
    
#ifdef PLATFORM_WITH_DRIVES
#ifdef Q_OS_SYMBIAN        
    toSymbianPath(path);
#endif    
    QString filepath = qApp->applicationFilePath();
    QChar drive;
    if (filepath.length()>=2 && filepath.at(1) == ':') {
        drive = filepath.at(0);
    }    
            
    QString tsfile = path + name + QString("_") + lang;
    QString tsfileQM = tsfile + QString(".qm"); 

    bool loaded = false;    
    if (HbFindFile::hbFindFile(tsfileQM, drive)) {
        tsfileQM.chop(3);        
        loaded = loadTranslatorData(translator,tsfileQM,translatorData);        
    }
    else {
        tsfile = path + name + QString("_") + lang2;
        tsfileQM = tsfile + QString(".qm");
        if (HbFindFile::hbFindFile(tsfileQM, drive)) {
            tsfileQM.chop(3);
            loaded = loadTranslatorData(translator,tsfileQM,translatorData);        
        }
        else {
            QList<QString> fallBack;
            fallBack.append("en");
            fallBack.append("en_US");
            int len = fallBack.length();
            for (int i=0; i<len; i++) {
                QString lang;
                tsfile = path + name + QString("_") + fallBack.at(i);
                tsfileQM = tsfile + QString(".qm");
                if (HbFindFile::hbFindFile(tsfileQM, drive)) {
                    tsfileQM.chop(3);
                    loaded = loadTranslatorData(translator,tsfileQM,translatorData);        
                }    
            }        
        }
    }

    if (loaded) {
        qApp->installTranslator(&translator);
    }
#else    
    QString tsfile2 = path + name + QString("_") + lang;
    if ( translator.load(tsfile2) ) {
        qApp->installTranslator(&translator);
    }
#endif    
}

/*!
    Internal helper class.  
*/ 
class LanguageHash : public QHash<QString,QString>
{
public:
    LanguageHash();    
};

/*!
    Table for downgrade.  
*/ 
LanguageHash::LanguageHash(){
    (*this)["en_GB"] = "en";
    (*this)["fr_FR"] = "fr";
    (*this)["de_DE"] = "de";
    (*this)["es_ES"] = "es";
    (*this)["it_IT"] = "it";
    (*this)["sv_SE"] = "sv";
    (*this)["da_DK"] = "da";
    (*this)["no_NO"] = "no";
    (*this)["fi_FI"] = "fi";
    (*this)["en_US"] = "en_US";
    (*this)["pt_PT"] = "pt";
    (*this)["tr_TR"] = "tr";
    (*this)["is_IS"] = "is";
    (*this)["ru_RU"] = "ru";
    (*this)["hu_HU"] = "hu";
    (*this)["nl_NL"] = "nl";
    (*this)["cs_CZ"] = "cs";
    (*this)["sk_SK"] = "sk";
    (*this)["pl_PL"] = "pl";
    (*this)["sl_SI"] = "sl";
    (*this)["zh_TW"] = "zh_TW";
    (*this)["zh_HK"] = "zh_HK";
    (*this)["zh_CN"] = "zh_CN";
    (*this)["ja_JP"] = "ja";
    (*this)["th_TH"] = "th";
    (*this)["ar_AE"] = "ar";
    (*this)["tl_PH"] = "tl";
    (*this)["bg_BG"] = "bg";
    (*this)["ca_ES"] = "ca";
    (*this)["hr_HR"] = "hr";
    (*this)["et_EE"] = "et";
    (*this)["fa_IR"] = "fa";
    (*this)["fr_CA"] = "fr_CA";
    (*this)["el_GR"] = "el";
    (*this)["he_IL"] = "he";
    (*this)["hi_IN"] = "hi";
    (*this)["id_ID"] = "id";
    (*this)["ko_KR"] = "ko";
    (*this)["lv_LV"] = "lv";
    (*this)["lt_LT"] = "lt";
    (*this)["ms_MY"] = "ms";
    (*this)["pt_BR"] = "pt_BR";
    (*this)["ro_RO"] = "ro";
    (*this)["sr_YU"] = "sr";
    (*this)["es_MX"] = "es_MX";
    (*this)["uk_UA"] = "uk";
    (*this)["ur_PK"] = "ur";
    (*this)["vi_VN"] = "vi";
    (*this)["eu_ES"] = "eu";
    (*this)["gl_ES"] = "gl";
    
}
                         
Q_GLOBAL_STATIC(LanguageHash, gs_LanguageHash)

/*!
    Internal function for solving conflict between QLocale::system().name() and actual ts file naming convention.  
     
    \param lang language for downgrading
    \return true if successful
*/ 
bool HbTranslatorPrivate::languageDowngrade(QString &lang)
{
    QHash<QString,QString> *languageHash = gs_LanguageHash();
    if (languageHash) {
        if (languageHash->contains(lang)) {
           lang  = languageHash->value(lang);
           return true;
        }                
    }
    return false;
}
