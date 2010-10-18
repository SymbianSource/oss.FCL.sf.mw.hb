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
#include "hbinputmethoddescriptor.h"

/*!
@stable
@hbcore
\class HbInputMethodDescriptor
\brief Describes an input method

This class is for input method developers, it should never be needed in application code.
*/

const QString HbInputDefaultMethodString("<DEFAULT>");

/*!
Constructs the object->
*/
HbInputMethodDescriptor::HbInputMethodDescriptor()
{
}

/*!
Copy constructor.
*/
HbInputMethodDescriptor::HbInputMethodDescriptor(const HbInputMethodDescriptor &desc)
{
    mPluginNameAndPath = desc.mPluginNameAndPath;
    mKey = desc.mKey;
    mDisplayName = desc.mDisplayName;
    mDisplayNames = desc.mDisplayNames;
    mIcon = desc.mIcon;
    mIcons = desc.mIcons;
}

/*!
Returns name and path of the plugin file.
*/
QString HbInputMethodDescriptor::pluginNameAndPath() const
{
    return mPluginNameAndPath;
}

/*!
Sets plugin name and path.
*/
void HbInputMethodDescriptor::setPluginNameAndPath(const QString &nameAndPath)
{
    mPluginNameAndPath = nameAndPath;
}

/*!
Returns the key of the input method. See QInputContext for more
information.
*/
QString HbInputMethodDescriptor::key() const
{
    return mKey;
}

/*!
Sets the input method key.
*/
void HbInputMethodDescriptor::setKey(const QString &newKey)
{
    mKey = newKey;
}

/*!
Returns the display name of the input method.
*/
QString HbInputMethodDescriptor::displayName() const
{
    return mDisplayName;
}

/*!
Sets the display name of the input method.
*/
void HbInputMethodDescriptor::setDisplayName(const QString &name)
{
    mDisplayName = name;
}

/*!
Returns the display names of the input method.
*/
QStringList HbInputMethodDescriptor::displayNames() const
{
    return mDisplayNames;
}

/*!
Sets the display names of the input method.
*/
void HbInputMethodDescriptor::setDisplayNames(const QStringList &names)
{
    mDisplayNames = names;
}

/*!
Returns the icon of the input method.
*/
HbIcon HbInputMethodDescriptor::icon() const
{
    return mIcon;
}

/*!
Sets the icon of the input method.
*/
void HbInputMethodDescriptor::setIcon(const HbIcon &icon)
{
    mIcon = icon;
}

/*!
Returns the icons of the input method.
*/
QList<HbIcon> HbInputMethodDescriptor::icons() const
{
    return mIcons;
}

/*!
Sets the icons of the input method.
*/
void HbInputMethodDescriptor::setIcons(const QList<HbIcon> &icons)
{
    mIcons = icons;
}

/*!
Returns true if the descriptor is empty and doesn't point to an input plugin.
*/
bool HbInputMethodDescriptor::isEmpty() const
{
    return mPluginNameAndPath.isEmpty();
}

/*!
Assign operator.
*/
void HbInputMethodDescriptor::operator=(const HbInputMethodDescriptor &descriptor)
{
    mPluginNameAndPath = descriptor.mPluginNameAndPath;
    mKey = descriptor.mKey;
    mDisplayName = descriptor.mDisplayName;
    mDisplayNames = descriptor.mDisplayNames;
    mIcon = descriptor.mIcon;
    mIcons = descriptor.mIcons;
}

/*!
Sets up the descriptor so that it points to system default input method. What
that is must be resolved elsewhere by the framework. Once the descriptor is set to point
to default input method, the information inside it is meaningful only inside the framework
but the actual plugin can't be loaded directly with the descriptor. This method is not
usually needed in application side code but input method implementor may need it when displaying
input method selection dialogs.
*/
void HbInputMethodDescriptor::setDefault()
{
    mPluginNameAndPath = HbInputDefaultMethodString;
    mKey.clear();
    mDisplayName = QString("Default");
}

/*!
Returns true if this descriptor points to system default input.
*/
bool HbInputMethodDescriptor::isDefault() const
{
    return (mPluginNameAndPath == HbInputDefaultMethodString);
}

/*!
Serializes the input method \a descriptor to stream \a out.
*/
QDataStream &operator<<(QDataStream &out, const HbInputMethodDescriptor &descriptor)
{
    out << descriptor.pluginNameAndPath();
    out << descriptor.key();
    out << descriptor.displayName();
    out << descriptor.displayNames();
    out << descriptor.icon().iconName();
    out << descriptor.icons().count();
    for (int i = 0; i < descriptor.icons().count(); ++i) {
        out << descriptor.icons().at(i).iconName();
    }
    return out;
}

/*!
Deserializes input method \a descriptor from stream \a in.
*/
QDataStream &operator>>(QDataStream &in, HbInputMethodDescriptor &descriptor)
{
    QString pluginNameAndPath;
    in >> pluginNameAndPath;
    descriptor.setPluginNameAndPath(pluginNameAndPath);
    QString key;
    in >> key;
    descriptor.setKey(key);
    QString displayName;
    in >> displayName;
    descriptor.setDisplayName(displayName);
    QStringList displayNames;
    in >> displayNames;
    descriptor.setDisplayNames(displayNames);
    QString iconName;
    in >> iconName;
    descriptor.setIcon(HbIcon(iconName));
    int iconCount;
    in >> iconCount;
    QList<HbIcon> icons;
    for (int i = 0; i < iconCount; ++i) {
        in >> iconName;
        icons.append(HbIcon(iconName));
    }
    descriptor.setIcons(icons);
    return in;
}

// End of file
