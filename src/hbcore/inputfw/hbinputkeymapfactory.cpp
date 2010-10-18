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
#include "hbinputkeymapfactory.h"
#include "hbinputkeymapfactory_p.h"

#include <QPluginLoader>
#include <QDir>
#include <QLibrary>
#include <QTextStream>
#include <QVector>
#include <QDebug>

#include "hbinputkeymap.h"
#include "hbinputsettingproxy.h"

/// @cond

void removeNonExistingPaths(QStringList &list)
{
    for (int i = list.count()-1; i >= 0; --i) {
        if (!QFile::exists(list.at(i))) {
            list.removeAt(i);
        }
    }
}

HbKeymapFactoryPrivate::HbKeymapFactoryPrivate()
{
}

HbKeymapFactoryPrivate::~HbKeymapFactoryPrivate()
{
    foreach(HbKeymap *keymap, mKeymaps) {
        delete keymap;
    }
    mKeymaps.clear();
    mRomLanguages.clear();
}

void HbKeymapFactoryPrivate::parseFileNames(const QStringList &files, QList<HbInputLanguage> &languages) const
{
    bool ok = false;
    QLocale::Language language = QLocale::C;
    QLocale::Country country = QLocale::AnyCountry;
    foreach(const QString &file, files) {
        int underscorePos = file.indexOf('_');
        int periodPos = file.indexOf('.');
        if (underscorePos > 0 && underscorePos < periodPos) {
            language = static_cast<QLocale::Language>(file.left(underscorePos).toUInt(&ok));
            if (!ok) {
                continue;
            }
            country = static_cast<QLocale::Country>(file.mid(underscorePos + 1, periodPos - underscorePos - 1).toUInt(&ok));
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

int HbKeymapFactoryPrivate::keymapVersion(QFile &file) const
{
    int version = -1;
    if (file.fileName().length() > 0) {
        QChar driveLetter = file.fileName().at(0).toLower();
        // The version on the 'softest' drive is given precedence
        // Paths starting with z, :, or any other character are taken after these
        if (driveLetter >= 'a' && driveLetter <= 'y') {
            version = driveLetter.toAscii() - 'a' + 1;
        } else {
            version = 0;
        }
    }
    return version;
}

bool HbKeymapFactoryPrivate::findKeymapFile(const HbInputLanguage &language, const QStringList &searchPaths, QFile &file) const
{
    QString filename, latestVersionFilename;
    qreal maxVersionNumber = -1;

    foreach(const QString &path, searchPaths) {
        if (language.variant() == QLocale::AnyCountry) {
            filename = path + '/' + QString::number(language.language()) + ".txt";
        } else {
            filename = path + '/' + QString::number(language.language()) + '_'
                       + QString::number(language.variant()) + ".txt";
        }

        if (QFile::exists(filename)) {
            file.setFileName(filename);
            int fileVersion = keymapVersion(file);
            if (fileVersion > maxVersionNumber) {
                latestVersionFilename = filename;
                maxVersionNumber = fileVersion;
            }
        }
    }

    if (latestVersionFilename.isEmpty()) {
        if (language.variant() == QLocale::AnyCountry) {
            // File not found when trying to open with AnyCountry (no location specified), check whether
            // the language is available as a country-specific version

            foreach(const HbInputLanguage &availableLanguage, HbKeymapFactory::availableLanguages()) {
                if (availableLanguage.language() == language.language()) {
                    return findKeymapFile(availableLanguage, searchPaths, file);
                }
            }
        }
        return false;
    } else {
        file.setFileName(latestVersionFilename);
        return true;
    }
}

HbKeymap *HbKeymapFactoryPrivate::parseKeymapDefinition(const HbInputLanguage &language, QTextStream &stream) const
{
    HbKeymap *keymap = 0;
    HbKeyboardMap *keyboard = 0;
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
            QStringList splitResult = line.split('\t');
            if (splitResult.count() == 0) {
                continue;
            }
            HbMappedKey *mappedKey = new HbMappedKey();
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
                case 5:
                    mappedKey->chars.append(splitResult.at(5));
                    break;
                default:
                    break;
                }
            }
            keyboard->keys.append(mappedKey);
        }
    }
    if (keyboard) {
        // The last keyboard definition was not terminated properly, so it needs to be freed at this point
        delete keyboard;
        keyboard = 0;
        qDebug() << "HbInputKeymapFactory: unterminated keyboard definition detected";
    }
    if (!isValid(keymap)) {
        delete keymap;
        keymap = 0;
        qDebug() << "HbInputKeymapFactory: invalid keymap definition detected";
    }
    return keymap;
}

bool HbKeymapFactoryPrivate::isValid(const HbKeymap *keymap) const
{
    if (!keymap) {
        return false;
    }
    // Very basic sanity checking
    // Check that basic keyboards are present and the number of keys make sense
    const HbKeyboardMap* keyboard = keymap->keyboard(HbKeyboardTouchPortrait);
    if (!keyboard || keyboard->keys.count() != 10) {
        return false;
    }
    keyboard = keymap->keyboard(HbKeyboardTouchLandscape);
    if (!keyboard || keyboard->keys.count() < 30 || keyboard->keys.count() > 45) {
        return false;
    }
    keyboard = keymap->keyboard(HbKeyboardSctPortrait);
    if (!keyboard || keyboard->keys.count() < 16) {
        return false;
    }
    keyboard = keymap->keyboard(HbKeyboardSctLandscape);
    if (!keyboard || keyboard->keys.count() < 30) {
        return false;
    }
    return true;
}

HbKeymap *HbKeymapFactoryPrivate::keymap(const HbInputLanguage &language) const
{
    QFile file;
    HbKeymap *keymap = 0;
    QStringList paths = HbInputSettingProxy::keymapPluginPaths();
    removeNonExistingPaths(paths);
    // First try to load the highest priority version of the keymap
    if (findKeymapFile(language, paths, file)) {
        QTextStream stream;
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            stream.setDevice(&file);
            keymap = parseKeymapDefinition(language, stream);
            file.close();
        }
        // If reading the keymap fails (and it was not in system resources to begin with),
        // try to load a version from system resources
        if (!keymap && file.fileName().left(2) != ":/") {
            if (findKeymapFile(language, paths.filter(":/"), file)) {
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    stream.setDevice(&file);
                    keymap = parseKeymapDefinition(language, stream);
                    file.close();
                }
            }
        }
    }
    return keymap;
}

/// @endcond

/*!
@stable
@hbcore
\class HbKeymapFactory
\brief A factory class for accessing keymap data.

Keymap factory constructs HbKeymap objects for given languages based on keymap resource files.
It can also provide a list of available keymap files in the system.

\sa HbKeymap
*/

/*!
\enum HbKeymapFactory::Flag

Flags for controlling keymap loading by the factory.
*/

/*!
\var HbKeymapFactory::Flag HbKeymapFactory::Default

Default functionality, loaded keymaps are cached inside the factory and factory retains ownership of keymap objects.
*/

/*!
\var HbKeymapFactory::Flag HbKeymapFactory::NoCaching

Disabled cache for this function call. Keymaps are read directly from disk and are not stored in cache.
Ownership of the returned keymaps is transferred to the caller.
*/

/*!
Returns reference to singleton instance.
*/
HbKeymapFactory *HbKeymapFactory::instance()
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
const HbKeymap *HbKeymapFactory::keymap(const QLocale::Language language, const QLocale::Country country)
{
    return keymap(HbInputLanguage(language, country), Default);
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
const HbKeymap *HbKeymapFactory::keymap(const HbInputLanguage language)
{
    return keymap(language, Default);
}

/*!
Returns a HbKeymap object initialized using keymap resource data in the system. Ownership of the
HbKeymap object remains with HbKeymapFactory unless otherwise specified with flags.

If no data is found for the given input language, 0 is returned. If the variant of the input language
is AnyCountry, the function can return either a keymap with no variant specified or a keymap with any variant.

\param language HbInputLanguage defining the language-country combination.
\param flags HbKeymapFactory flags for controlling this keymap loading.

\sa HbKeymap
\sa HbInputLanguage
\sa HbKeymapFactory::Flags
*/
const HbKeymap *HbKeymapFactory::keymap(const HbInputLanguage &language, HbKeymapFactory::Flags flags)
{
    if (!flags.testFlag(NoCaching)) {
        foreach(HbKeymap *keymap, mPrivate->mKeymaps) {
            if (keymap->language() == language) {
                return keymap;
            }
        }
    }

    HbKeymap *keymap = mPrivate->keymap(language);

    if (keymap && !flags.testFlag(NoCaching)) {
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
    HbKeymapFactory *instance = HbKeymapFactory::instance();
    bool romLanguagesCached = !instance->mPrivate->mRomLanguages.isEmpty();
    QList<HbInputLanguage> languages;
    QStringList* files = new QStringList();
    QStringList* romFiles = new QStringList();
    QStringList paths = HbInputSettingProxy::keymapPluginPaths();
    removeNonExistingPaths(paths);
    foreach(const QString &path, paths) {
        if (path.left(2) == ":/" || path.left(2) == "z:") {
            if (romLanguagesCached) {
                continue;
            }
            QDir languagedir(path);
            *romFiles += languagedir.entryList(QStringList(QString("*.txt")), QDir::Files);
        } else {
            QDir languagedir(path);
            *files += languagedir.entryList(QStringList(QString("*.txt")), QDir::Files);
        }
    }
    if (romLanguagesCached) {
        languages = instance->mPrivate->mRomLanguages;
    } else {
        instance->mPrivate->parseFileNames(*romFiles, languages);
        instance->mPrivate->mRomLanguages = languages;
    }
    instance->mPrivate->parseFileNames(*files, languages);

    delete files;
    delete romFiles;
    return languages;
}

// End of file
