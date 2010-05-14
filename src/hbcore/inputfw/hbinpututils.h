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

#ifndef HB_INPUT_UTILS_H
#define HB_INPUT_UTILS_H

#include <QStringList>

#include <hbinputdef.h>
#include <hbinputlanguage.h>

class HbMappedKey;
class HbLanguageDatabaseInterface;
class QInputContextPlugin;
class QWidget;
class QGraphicsWidget;

struct HbInputMethodList
{
    QStringList dllName;
    QStringList key;
};

class HB_CORE_EXPORT HbInputUtils
{
public:
    static inline bool isChineseInputMode(HbInputModeType inputMode);
    static inline bool isQwertyKeyboard(HbKeyboardType keyboardType);
    static inline bool isTouchKeyboard(HbKeyboardType keyboardType);
    static bool isCaseSensitiveMode(HbInputModeType inputMode);
    static QChar findFirstNumberCharacterBoundToKey(const HbMappedKey* key,
                                                    const HbInputLanguage language,
                                                    const HbInputDigitType digitType = HbDigitTypeLatin);

    static void listAvailableLanguageDatabasePlugins(QStringList& result, const QString& subfolder);
    static HbLanguageDatabaseInterface* languageDatabasePluginInstance(const QString& pluginFileName, const QString& subfolder);

    static void listSupportedInputLanguages(QList<HbInputLanguage>& results);

    static QWidget* createWrapperWidget(QGraphicsWidget* graphicsWidget);
    static QGraphicsWidget* createGraphicsProxyWidget(QWidget* widget);
    static HbInputDigitType inputDigitType(HbInputLanguage language);	
};

/*!
Returns true if given input mode is Chinese input mode.
*/
inline bool HbInputUtils::isChineseInputMode(HbInputModeType inputMode)
{
    return (inputMode & HbChineseModeMask) != 0;
}

/*!
Returns true if given keyboard type is qwerty keyboard.
*/
inline bool HbInputUtils::isQwertyKeyboard(HbKeyboardType keyboardType)
{
    return (keyboardType & HbQwertyKeyboardMask) != 0;
}

/*!
Returns true if given keyboard type is virtual keyboard.
*/
inline bool HbInputUtils::isTouchKeyboard(HbKeyboardType keyboardType)
{
   return (keyboardType & HbTouchInputMask) != 0;
}

#endif // HB_INPUT_UTILS_H

// End of file
