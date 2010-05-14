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


const QString defaultpath = "/resource/qt/translations/";

/*!
    @beta
    @hbcore
    \class HbTranslator
    \brief HbTranslator installs QTranslator(s) automatically needed in localisation
    and loads translation files into QTranslator.
*/

/*!
    Default case: searches translation file from default location (/resource/qt/translations/) with default name, which is <executablename>.qm
*/
HbTranslator::HbTranslator(): d(new HbTranslatorPrivate())
{
    QFileInfo info(qApp->applicationFilePath());
    QString defaultname = info.baseName();  // defaultname = <executablename>
    d->installTranslator(defaultpath, defaultname);
}

/*!
    Searches translation file \a file from default location.
*/

HbTranslator::HbTranslator(const QString &file): d(new HbTranslatorPrivate())
{
    d->installTranslator(defaultpath, file);
}

/*!
    Searches translation file \a file from path \a path.
    \code
    HbTranslator trans = new HbTranslator("/resource/qt/custom/", "customfile");
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
*/
void HbTranslator::loadCommon()
{
    QString lang = QLocale::system().name();
    QString commonts = QString("Z:") + defaultpath + QString("common_") + lang;
    d->common.load(commonts);
    qApp->installTranslator(&d->common);
}

// internal function for common operations of HbTranslator
void HbTranslatorPrivate::installTranslator(const QString &path, const QString &name)
{
    QString filepath = qApp->applicationFilePath();
    QChar drive = filepath.at(0);
    QString lang = QLocale::system().name();
    QString lang2 = lang;
    languageDowngrade(lang);
    QString tsfile = path + name + QString("_") + lang + QString(".qm");
    if (!HbFindFile::hbFindFile(tsfile, drive)) {
    	tsfile = path + name + QString("_") + lang2 + QString(".qm");
        HbFindFile::hbFindFile(tsfile);
    }
    translator.load(tsfile);
    qApp->installTranslator(&translator);
}

// internal function for solving conflict between QLocale::system().name() and actual ts file naming convention.
bool HbTranslatorPrivate::languageDowngrade(QString &lang)
{
    static QHash<QString, QString> languages;
    languages["en_GB"] = "en";
    languages["fr_FR"] = "fr";
    languages["de_DE"] = "de";
    languages["es_ES"] = "es";
    languages["it_IT"] = "it";
    languages["sv_SE"] = "sv";
    languages["da_DK"] = "da";
    languages["no_NO"] = "no";
    languages["fi_FI"] = "fi";
    languages["en_US"] = "en_US";
    languages["pt_PT"] = "pt";
    languages["tr_TR"] = "tr";
    languages["is_IS"] = "is";
    languages["ru_RU"] = "ru";
    languages["hu_HU"] = "hu";
    languages["nl_NL"] = "nl";
    languages["cs_CZ"] = "cs";
    languages["sk_SK"] = "sk";
    languages["pl_PL"] = "pl";
    languages["sl_SI"] = "sl";
    languages["zh_TW"] = "zh_TW";
    languages["zh_HK"] = "zh_HK";
    languages["zh_CN"] = "zh_CN";
    languages["ja_JP"] = "ja";
    languages["th_TH"] = "th";
    languages["ar_AE"] = "ar";
    languages["tl_PH"] = "tl";
    languages["bg_BG"] = "bg";
    languages["ca_ES"] = "ca";
    languages["hr_HR"] = "hr";
    languages["et_EE"] = "et";
    languages["fa_IR"] = "fa";
    languages["fr_CA"] = "fr_CA";
    languages["el_GR"] = "el";
    languages["he_IL"] = "he";
    languages["hi_IN"] = "hi";
    languages["id_ID"] = "id";
    languages["ko_KR"] = "ko";
    languages["lv_LV"] = "lv";
    languages["lt_LT"] = "lt";
    languages["ms_MY"] = "ms";
    languages["pt_BR"] = "pt_BR";
    languages["ro_RO"] = "ro";
    languages["sr_YU"] = "sr";
    languages["es_MX"] = "es_MX"; //!!
    languages["uk_UA"] = "uk";
    languages["ur_PK"] = "ur";
    languages["vi_VN"] = "vi";
    languages["eu_ES"] = "eu";
    languages["gl_ES"] = "gl";
    
    if (languages.contains(lang)) {
    	lang  = languages.value(lang);
    	return true;
    }
    return false;
}
