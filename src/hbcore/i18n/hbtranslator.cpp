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

#ifdef Q_OS_SYMBIAN
const char* defaultPath   = "/resource/qt/translations/";
const char* defaultDrive  = "Z:";
const char* defaultCommon = "common_";
#else
const QString defaultPath = "";
const char* defaultDrive  = "";
const char* defaultCommon = "common_";

#endif

#ifdef Q_OS_SYMBIAN
#include <f32file.h>
#include <eikenv.h>
#endif

#ifdef Q_OS_SYMBIAN
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

static uchar* loadTranslatorData(QTranslator &translator, QString &qmFile,bool doFallback=true) {
#ifndef Q_OS_SYMBIAN
    translator.load(qmFile);
    Q_UNUSED(doFallback);
    return 0;
#else    
    RFile fl;
    RFs& fs = CCoeEnv::Static()->FsSession();
    // TPtrC ptrFName;
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
               return 0;
           }        
           // else continue
        } 
        else {
            if (err != KErrNone ) { // if some other error return error, don't look anymore
                return 0;
            }
            else {
                break; // file was found
            }
        }
        // do fallback
        qmFile2 = qmFile;
        ptrFName.Set((ushort*)(qmFile2.constData()), qmFile2.length());
        err= fl.Open(fs, ptrFName, EFileShareReadersOrWriters | EFileRead | EFileStream );
        if (err == KErrNone) {
            break;
        }
        else {
            if (err != KErrNotFound) {
                return 0;
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

        // no truncations? fail
        if (rightmost==0) {
            return 0;
        }
        qmFile.truncate(rightmost);                
    }
    
    TInt sz;
    err = fl.Size(sz);
    if (err != KErrNone) {
        fl.Close();
        return 0;
    }   
    uchar *buf = new uchar[sz];
    TPtr8 bufPtr(buf,0,sz); 
    err = fl.Read(bufPtr, sz);
    if (err != KErrNone) {
        fl.Close();
        return 0;
    }        
    fl.Close();
    if (!translator.load(bufPtr.Ptr(),sz)) {
        delete buf;
        return 0;
    }
    return buf;
#endif    
}

/*!
    @beta
    @hbcore
    \class HbTranslator
    \brief HbTranslator installs QTranslator(s) automatically needed in localisation
    and loads translation files into QTranslator.
*/

/*!
    Default case: searches translation file from default location (/resource/qt/translations/) with default name, which is <executablename>.qm
    
    \attention Cross-Platform API
*/
HbTranslator::HbTranslator(): d(new HbTranslatorPrivate())
{
    QFileInfo info(qApp->applicationFilePath());
    QString defaultName = info.baseName();  // defaultname = <executablename>
    d->installTranslator(defaultPath, defaultName);
}

/*!
    Searches translation file \a file from default location.
    
    \attention Cross-Platform API
*/
HbTranslator::HbTranslator(const QString &file): d(new HbTranslatorPrivate())
{
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
    d->installTranslator(path, file);
}

HbTranslator::~HbTranslator()
{
    delete d;
}

/*!
    Loads common.ts translations from default location.
    
    \attention Cross-Platform API
*/
void HbTranslator::loadCommon()
{    
    QString lang = QLocale::system().name();
    QString commonts = QString(defaultDrive) + QString(defaultPath) + QString(defaultCommon) + lang;
    bool loaded;
    loaded = (d->commonData=loadTranslatorData(d->common, commonts));
    if (loaded) {
        qApp->installTranslator(&d->common);    
    }
}

// internal function for common operations of HbTranslator
void HbTranslatorPrivate::installTranslator(const QString &pth, const QString &name)
{
    QString path(pth);
#ifdef Q_OS_SYMBIAN        
    toSymbianPath(path);
#endif    
    QString filepath = qApp->applicationFilePath();
    QChar drive;
    if (filepath.length()>=2 && filepath.at(1) == ':') {
        drive = filepath.at(0);
    }    
            
    QString lang = QLocale::system().name();
    QString lang2 = lang;
    languageDowngrade(lang);
    QString tsfile = path + name + QString("_") + lang;
    QString tsfileQM = tsfile + QString(".qm"); 

    bool loaded = false;    
    if (HbFindFile::hbFindFile(tsfileQM, drive)) {
        tsfileQM.chop(3);        
        loaded = (translatorData=loadTranslatorData(translator,tsfileQM));        
    }
    else {
        tsfile = path + name + QString("_") + lang2;
        tsfileQM = tsfile + QString(".qm");
        if (HbFindFile::hbFindFile(tsfileQM, drive)) {
            tsfileQM.chop(3);
            loaded = (translatorData=loadTranslatorData(translator,tsfileQM));        
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
                    loaded = (translatorData=loadTranslatorData(translator,tsfileQM));        
                }    
            }        
        }
    }

    if (loaded) {
        qApp->installTranslator(&translator);
    }
}

class LanguageHash : public QHash<QString,QString>
{
public:
    LanguageHash();    
};

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
    (*this)["es_MX"] = "es_MX"; //!!
    (*this)["uk_UA"] = "uk";
    (*this)["ur_PK"] = "ur";
    (*this)["vi_VN"] = "vi";
    (*this)["eu_ES"] = "eu";
    (*this)["gl_ES"] = "gl";
    
}
                         
Q_GLOBAL_STATIC(LanguageHash, gs_LanguageHash)

// internal function for solving conflict between QLocale::system().name() and actual ts file naming convention.
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
