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

#include "hbcssthemeinterface_p.h"  //class header
#include <QGraphicsWidget>

#define FILE_VERSION(major, minor) (int)((major<<16)|minor)

/*!
 \class HbCssThemeInterface
 \brief HbCssThemeInterface is an internal class.
 *
 */
 
 
 /*!
 * \fn HbCssThemeInterface::initialise(const QStringList& list,bool loadAllFiles,bool enableBinarySupport =false)
 * Function to populate the interface with css files
 * \a list   list of css files, priority wise
 * \a loadAllFiles ,On application launch all files are loaded and on theme change unchanged files are not loaded.
 * \a enableBinarySupport optional flag for using the binary functionality
 */
void HbCssThemeInterface::initialise(const QMap<int,QString> & list,bool loadAllFiles,
                                     bool enableBinarySupport)
{
    int handle;
    HbLayeredStyleLoader *loader = HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Colors);
    
    //first unload the layers, for which the contents are different after theme change
    if (handles.size() != 0)  {
        QMap<int,HbLayeredStyleLoader::LayerPriority>::const_iterator itr;
        for (itr = handles.constBegin(); itr != handles.constEnd(); ++itr){
            loader->unload(itr.key(),itr.value());
        }
        handles.clear();
    }

    QMap<int,QString>::const_iterator i;
    for (i = list.constBegin(); i != list.constEnd(); ++i){
        if(loadAllFiles) {
            handle =loader->load(i.value(),(HbLayeredStyleLoader::LayerPriority) i.key(),enableBinarySupport);
            if (((HbLayeredStyleLoader::LayerPriority) i.key() != HbLayeredStyleLoader::Priority_Core) &&  ((HbLayeredStyleLoader::LayerPriority) i.key() != HbLayeredStyleLoader::Priority_Operator )) {
                handles.insertMulti(handle, (HbLayeredStyleLoader::LayerPriority) i.key());
            }

        }
        else{
            if (((HbLayeredStyleLoader::LayerPriority) i.key() != HbLayeredStyleLoader::Priority_Core) &&  ((HbLayeredStyleLoader::LayerPriority) i.key() != HbLayeredStyleLoader::Priority_Operator )) {
                handle = loader->load(i.value(),(HbLayeredStyleLoader::LayerPriority) i.key(),enableBinarySupport);
                handles.insertMulti(handle, (HbLayeredStyleLoader::LayerPriority) i.key());
            }

        }

    }
}

/*!
 * \fn HbCssThemeInterface::flush()
 *
 */
void HbCssThemeInterface::flush()
{
    HbLayeredStyleLoader *loader = HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Colors);
    loader->clear();
}

/*!
 * \fn HbCssThemeInterface::findAttribute(const QGraphicsWidget* w, QString attribute)
 * Function for finding an attribute
 * \a w   widget pointer of QGraphicsWidget which will act as a selector
 *              For Universal Selector (*) pass NULL.
 * \a attribute name of attribute
 * \return list of values.
 */
HbCss::Value HbCssThemeInterface::findAttribute(
        const QGraphicsWidget *w, const QString& attribute) const
{
    StyleSelector::NodePtr n;
    n.ptr = (void *)w;
    HbCss::Value value;

    HbLayeredStyleLoader *loader = HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Colors);
	HbDeviceProfile profile(HbDeviceProfile::profile(w));
	HbCss::ValueExtractor valueExtractor(loader->declarationsForNode(n, profile.orientation()), true);
    valueExtractor.extractValue (attribute, value);
    

    if ( value.type == Value::Variable) {
        value = findVariable ( value.variant.toString ());
    }

    return value;
}

/*!
 * \fn HbCssThemeInterface::findVariable(const QString& variableName) const
 * Function for finding an attribute
 * \a variableName name of color group
 * \return list of values.
 */
HbCss::Value HbCssThemeInterface::findVariable(
        const QString& variableName) const
{
    HbLayeredStyleLoader *loader = HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Colors);
    HbCss::Value val;
    HbCss::Value value;

    HbCss::ValueExtractor valueExtractor(loader->variableRuleSets(),true);
    valueExtractor.extractValue (variableName, value);

    //for varibale cascading support
    if ( value.type == Value::Variable){
        value = findVariable ( value.variant.toString ());
    }

    return value;
}

