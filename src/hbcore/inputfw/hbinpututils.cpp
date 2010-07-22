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
#include "hbinpututils.h"

#include <QObject>
#include <QLocale>
#include <QDir>
#include <QPluginLoader>
#include <QInputContextPlugin>
#include <QGraphicsScene>
#include <QWidget>
#include <QGraphicsWidget>
#include <QGraphicsView>
#include <QGridLayout>
#include <QGraphicsProxyWidget>

#include "hbinputkeymap.h"
#include "hbinputkeymapfactory.h"
#include "hbinputsettingproxy.h"
#include "hbinputlanguagedatabase.h"
#include "hbinputmodecache_p.h"
#include "hbinputlanguage.h"

#define HB_DIGIT_ARABIC_INDIC_START_VALUE   0x0660
#define HB_DIGIT_EASTERN_ARABIC_START_VALUE 0x06F0


/*!
\class HbInputUtils
\brief A collection input related utility methods.

This class contains a collection of static input related utility methods that do not
naturally belong to any other scope. There are convenience methods for testing
attributes of keyboard and input mode types, instantiating plugins etc.

\sa HbInputMethod
\sa QInputContextPlugin
*/

/*!
Finds the fist number character bound to key using given mapping data.
*/
QChar HbInputUtils::findFirstNumberCharacterBoundToKey(const HbMappedKey *key,
        const HbInputLanguage language,
        const HbInputDigitType digitType)
{
    Q_UNUSED(language);

    if (key) {
        QString chars = key->characters(HbModifierNone);
        if (digitType == HbDigitTypeLatin) {
            for (int i = 0; i < chars.length(); i++) {
                if (chars.at(i) >= '0' && chars.at(i) <= '9') {
                    return chars.at(i);
                }
            }
        } else if (digitType == HbDigitTypeDevanagari) {
            for (int i = 0; i < chars.length(); i++) {
                if (chars.at(i) >= 0x0966 && chars.at(i) <= 0x096F) {
                    return chars.at(i);
                }
            }
        } else if (digitType == HbDigitTypeArabicIndic) {
            for (int i = 0; i < chars.length(); i++) {
                if (chars.at(i) >= '0' && chars.at(i) <= '9') {
                    return HB_DIGIT_ARABIC_INDIC_START_VALUE +
                           (chars.at(i).unicode() - '0');
                }
            }
        } else if (digitType == HbDigitTypeEasternArabic) {
            for (int i = 0; i < chars.length(); i++) {
                if (chars.at(i) >= '0' && chars.at(i) <= '9') {
                    return HB_DIGIT_EASTERN_ARABIC_START_VALUE +
                           (chars.at(i).unicode() - '0');
                }
            }
        }
    }

    return 0;
}

/*!
Returns true if the concept of "text case" can be applied to given input mode.
For example Chinese and Japanese modes do not have text case.
*/
bool HbInputUtils::isCaseSensitiveMode(HbInputModeType inputMode)
{
    if (isChineseInputMode(inputMode)) {
        return false;
    }

    return true;
}

/*!
Lists all Qt library files in language database directory. May return
files that are not language database plugins. languageDatabasePluginInstance()
checks the validity of plugin files provided by this function.

\sa languageDatabasePluginInstance
*/
void HbInputUtils::listAvailableLanguageDatabasePlugins(QStringList &result, const QString &subfolder)
{
    QString path(HbInputSettingProxy::languageDatabasePath());
    path += QDir::separator();
    path += subfolder;

    QDir dir(path);

    for (unsigned int i = 0; i < dir.count(); i++) {
        if (QLibrary::isLibrary(dir[i])) {
            result.append(dir[i]);
        }
    }
}

/*!
Creates an instance of given language database plugin, if valid.
*/
HbLanguageDatabaseInterface *HbInputUtils::languageDatabasePluginInstance(const QString &pluginFileName, const QString &subfolder)
{
    if (!QLibrary::isLibrary(pluginFileName)) {
        qDebug("HbInputUtils::languageDatabasePluginInstance: Not a library!");
        return NULL;
    }

    HbLanguageDatabaseInterface *res = NULL;

    QString fullName(HbInputSettingProxy::languageDatabasePath());
    fullName += QDir::separator();
    if (subfolder.length() > 0) {
        fullName += subfolder;
        fullName += QDir::separator();
    }
    fullName += pluginFileName;

    QPluginLoader loader(fullName);
    QObject *plugin = loader.instance();

    if (plugin) {
        res = qobject_cast<HbLanguageDatabaseInterface *>(plugin);
    } else {
        qDebug("HbInputUtils::languageDatabasePluginInstance: Unable to instantiate plugin");
    }

    return res;
}

/*!
Lists all the input languages that the set of available input methods supports. Excludes custom input methods.
*/
void HbInputUtils::listSupportedInputLanguages(QList<HbInputLanguage>& results)
{
    results = HbInputModeCache::instance()->listInputLanguages();
}

/*!
This method creates an instance of QWidget and wraps given graphics widget inside it.
It creates QGraphicsScene, adds given widget there and creates a view to the scene
inside returned QWidget. This is utility method is mainly for internal use.
*/
QWidget *HbInputUtils::createWrapperWidget(QGraphicsWidget *graphicsWidget)
{
    QWidget *ret = 0;

    if (graphicsWidget) {
        ret = new QWidget;
        ret->setContentsMargins(0, 0, 0, 0);
        QGraphicsScene *scene = new QGraphicsScene(ret);
        scene->addItem(graphicsWidget);
        QGraphicsView *view = new QGraphicsView(scene);
        view->setContentsMargins(0, 0, 0, 0);
        QGridLayout *layout = new QGridLayout;
        layout->setMargin(0);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(view);
        ret->setLayout(layout);
    }

    return ret;
}

/*!
A convenience method that wraps given widget inside QGraphicsProxyWidget
and returns it. This is utility method is mainly for internal use.
*/
QGraphicsWidget *HbInputUtils::createGraphicsProxyWidget(QWidget *widget)
{

    QGraphicsProxyWidget *proxy = 0;

    if (widget) {
        proxy = new QGraphicsProxyWidget;
        proxy->setWidget(widget);
    }

    return proxy;
}

/*!
returns the input digit type for the given input language
*/

HbInputDigitType HbInputUtils::inputDigitType(HbInputLanguage language)
{
    HbInputDigitType digitType = HbDigitTypeNone;

    switch (language.language()) {
    case QLocale::Arabic:
        digitType = HbDigitTypeArabicIndic;
        break;
    case QLocale::Persian:
    case QLocale::Urdu:
        digitType = HbDigitTypeEasternArabic;
        break;
    case QLocale::Hindi:
        digitType = HbDigitTypeDevanagari;
        break;
    default:
        digitType = HbDigitTypeLatin;
        break;
    }
    return digitType;
}


/*!
Returns the proxy widget of the embedded widget in a graphics view ;
if widget does not have the proxy widget then it returns the proxy widget of its window.
 otherwise returns 0.
*/
QGraphicsProxyWidget *HbInputUtils::graphicsProxyWidget(const QWidget *w)
{
    QGraphicsProxyWidget *pw = w ? w->graphicsProxyWidget() : 0;
    if (w && !pw) {
        pw = w->window() ? w->window()->graphicsProxyWidget() : w->graphicsProxyWidget();
    }
    return pw;
}


// End of file

