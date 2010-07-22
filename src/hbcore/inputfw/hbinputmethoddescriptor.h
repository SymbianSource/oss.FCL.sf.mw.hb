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
#ifndef HB_INPUT_METHOD_DESCRIPTOR_H
#define HB_INPUT_METHOD_DESCRIPTOR_H

#include <QString>
#include <hbinputdef.h>
#include <hbicon.h>

class HB_CORE_EXPORT HbInputMethodDescriptor
{
public:
    HbInputMethodDescriptor();
    HbInputMethodDescriptor(const HbInputMethodDescriptor &desc);

    QString pluginNameAndPath() const;
    void setPluginNameAndPath(const QString &nameAndPath);
    QString key() const;
    void setKey(const QString &newKey);
    QString displayName() const;
    void setDisplayName(const QString &name);
    QStringList displayNames() const;
    void setDisplayNames(const QStringList &names);
    HbIcon icon() const;
    void setIcon(const HbIcon &icon);
    QList<HbIcon> icons() const;
    void setIcons(const QList<HbIcon> &icons);
    bool isEmpty() const;
    void operator=(const HbInputMethodDescriptor &descriptor);
    void setDefault();
    bool isDefault() const;

private:
    QString mPluginNameAndPath;
    QString mKey;
    QString mDisplayName;
    QStringList mDisplayNames;
    HbIcon mIcon;
    QList<HbIcon> mIcons;
};

#endif // HB_INPUT_METHOD_DESCRIPTOR_H

// End of file
