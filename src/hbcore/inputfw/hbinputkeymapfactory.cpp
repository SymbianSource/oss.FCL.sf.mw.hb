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

#include <QPluginLoader>
#include <QDir>
#include <QLibrary>
#include <QTextStream>
#include <QVector>

#include "hbinputkeymapfactory.h"
#include "hbinputkeymap.h"
#include "hbinputsettingproxy.h"

/// @cond

class HbKeymapFactoryPrivate
{
public:
    HbKeymapFactoryPrivate();
    ~HbKeymapFactoryPrivate();
    void parseFiles(QStringList& files, QList<HbInputLanguage>& languages);

public:
    QList<HbKeymap*> mKeymaps;
    QList<HbInputLanguage> mRomLanguages;
};


HbKeymapFactoryPrivate::HbKeymapFactoryPrivate()
{
}

HbKeymapFactoryPrivate::~HbKeymapFactoryPrivate()
{
    foreach (HbKeymap* keymap, mKeymaps) {
        delete keymap;
    }
    mKeymaps.clear();
    mRomLanguages.clear();
}

void HbKeymapFactoryPrivate::parseFiles(QStringList& files, QList<HbInputLanguage>& languages)
{
    bool ok = false;
    QLocale::Language language = QLocale::C;
    QLocale::Country country = QLocale::AnyCountry;
    foreach(QString file, files) {
        int underscorePos = file.indexOf('_');
        int periodPos = file.indexOf('.');
        if (underscorePos > 0 && underscorePos < periodPos) {
            language = static_cast<QLocale::Language>(file.left(underscorePos).toUInt(&ok));
            if (!ok) {
                continue;
            }
            country = static_cast<QLocale::Country>(file.mid(underscorePos+1, periodPos-underscorePos-1).toUInt(&ok));
            if (!ok) {
                continue;
            }
            HbInputLanguage toAdd(language, country);
            if (!languages.contains(toAdd)) {
                languages.append(toAdd);
            }
        } else if (periodPos > 0) {
            language = static_cast<QLocale::Language>(file.left(periodPos).toUInt(&ok));
            if (!ok) {
                continue;
            }
            HbInputLanguage toAdd(language);
            if (!languages.contains(toAdd)) {
                languages.append(toAdd);
            }
        }
    }
}

/// @endcond

/*!
@alpha
@hbcore
\class HbKeymapFactory
\brief A factory class for accessing keymap data.

Keymap factory constructs HbKeymap objects for given languages based on keymap resource files.
It can also provide a list of available keymap files in the system.

\sa HbKeymap
*/

/*!
Returns reference to singleton instance.
*/
HbKeymapFactory* HbKeymapFactory::instance()
{
    static HbKeymapFactory myInstance;
    return &myInstance;
}

/*!
Constructs the object.
*/
HbKeymapFactory::HbKeymapFactory()
{
    mPrivate = new HbKeymapFactoryPrivate();
}

/*!
Destructs the object.
*/
HbKeymapFactory::~HbKeymapFactory()
{
    delete mPrivate;
}

/*!
Returns a HbKeymap object initialized using keymap resource data in the system. Ownership of the 
HbKeymap object remains with HbKeymapFactory.

If no data is found for the given language, 0 is returned.

\param language Language for the keymap
\param country Country for the keymap. If empty or AnyCountry, non-country specific version or a country-specific version will be returned.

\sa HbKeymap
*/
const HbKeymap* HbKeymapFactory::keymap(const QLocale::Language language, const QLocale::Country country)
{
    return keymap(HbInputLanguage(language, country));
}

/*!
Returns a HbKeymap object initialized using keymap resource data in the system. Ownership of the 
HbKeymap object remains with HbKeymapFactory.

If no data is found for the given input language, 0 is returned. If the variant of the input language
is AnyCountry, the function can return either a keymap with no variant specified or a keymap with any variant.

\param language HbInputLanguage defining the language-country combination.

\sa HbKeymap
\sa HbInputLanguage
*/
const HbKeymap* HbKeymapFactory::keymap(const HbInputLanguage language)
{
    foreach (HbKeymap* keymap, mPrivate->mKeymaps) {
        if (keymap->language() == language) {
            return keymap;
        }
    }

    QString filename;

    foreach (QString path, HbInputSettingProxy::keymapPluginPaths()) {
        if (language.variant() == QLocale::AnyCountry) {
            filename = path + '/' + QString::number(language.language()) + ".txt";
        } else {
            filename = path + '/' + QString::number(language.language()) + "_"
                + QString::number(language.variant()) + ".txt";
        }
        if (QFile::exists(filename)) {
            break;
        }
    }
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (language.variant() == QLocale::AnyCountry) {
            // File not found when trying to open with AnyCountry (no location specified), check whether
            // the language is available as a country-specific version
            foreach(HbInputLanguage availableLanguage, availableLanguages()) {
                if (availableLanguage.language() == language.language()) {
                    return keymap(availableLanguage);
                }
            }
        }
        return 0;
    }
    QTextStream stream(&file);

    HbKeymap* keymap = 0;
    HbKeyboardMap* keyboard = 0;

    while (!stream.atEnd()) {
        QString line = stream.readLine();
        // When an empty line is encountered, an ongoing keyboard definition ends
        if (line.isEmpty()) {
            if (keyboard) {
                if (!keymap) {
                    keymap = new HbKeymap(language);
                }
                keymap->addKeyboard(keyboard);
                keyboard = 0;
            }
            continue;
        }
        // Line starting with "//" is a comment
        if (line.length() >= 2 && line.at(0) == '/' && line.at(1) == '/') {
            continue;
        }
        // Non-empty line without ongoing keyboard definition is the start of a definition,
        // containing the keyboard type as hex
        if (!keyboard) {
            bool ok = false;
            int keyType = line.toInt(&ok, 16);
            if (ok) {
                keyboard = new HbKeyboardMap();
                keyboard->type = static_cast<HbKeyboardType>(keyType);
            }
        // Non-empty line with ongoing keyboard definition contains a key definition
        // Format: <keycode(char)><tab><keys_nomod><tab><keys_shiftmod><tab><keys_fnmod><tab><keys_fn+shiftmod>
        // Keycode and keys_nomod should always be present, but the rest are optional
        } else {
            QStringList splitResult = line.split("\t");
            if (splitResult.count() == 0) {
                continue;
            }
            HbMappedKey* mappedKey = new HbMappedKey();
            mappedKey->keycode = splitResult.at(0).at(0);
            for (int i = 1; i < splitResult.count(); ++i) {
                switch (i) {
                case 1:
                    mappedKey->chars.append(splitResult.at(1));
                    break;
                case 2:
                    mappedKey->chars.append(splitResult.at(2));
                    break;
                case 3:
                    mappedKey->chars.append(splitResult.at(3));
                    break;
                case 4:
                    mappedKey->chars.append(splitResult.at(4));
                    break;
                default:
                    break;
                }
            }
            keyboard->keys.append(mappedKey);
        }
    }
    if (keymap) {
        mPrivate->mKeymaps.append(keymap);
    }
    return keymap;
}

/*!
Returns a list of available languages (or keymap data files) in the system.

\sa HbInputLanguage
*/
QList<HbInputLanguage> HbKeymapFactory::availableLanguages()
{
    HbKeymapFactory* instance = HbKeymapFactory::instance();
    bool romLanguagesCached = !instance->mPrivate->mRomLanguages.isEmpty();
    QList<HbInputLanguage> languages;
    QStringList files;
    QStringList romFiles;
    foreach (QString path, HbInputSettingProxy::keymapPluginPaths()) {
        if (path.left(2) == ":/" || path.left(2) == "z:") {
            if (romLanguagesCached) {
                continue;
            }
            QDir languagedir(path);
            romFiles += languagedir.entryList(QStringList(QString("*.txt")), QDir::Files);
        } else {
            QDir languagedir(path);
            files += languagedir.entryList(QStringList(QString("*.txt")), QDir::Files);
        }
    }
    if (romLanguagesCached) {
        languages = instance->mPrivate->mRomLanguages;
    } else {
        instance->mPrivate->parseFiles(romFiles, languages);
    }
    instance->mPrivate->parseFiles(files, languages);

    if (instance->mPrivate->mRomLanguages.isEmpty()) {
        instance->mPrivate->mRomLanguages = languages;
    }
    return languages;
}

// End of file
