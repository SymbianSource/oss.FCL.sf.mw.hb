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

#ifndef HBINDICATORCLIENTINFO_H
#define HBINDICATORCLIENTINFO_H

//  INCLUDES
#include <hbindicatorinterface.h>
#include <QDataStream>

enum IndicatorState { IndicatorActivated = 0, IndicatorUpdated, IndicatorDeactivated};

struct IndicatorClientInfo {
    inline IndicatorClientInfo() :
        category(HbIndicatorInterface::SettingCategory) {}

    inline IndicatorClientInfo(const QString &type,
        const QString &iconPath,
        HbIndicatorInterface::Category category) :
            type(type),
            iconPath(iconPath),
            category(category) {}

    inline IndicatorClientInfo(const IndicatorClientInfo &copySource) :
        type(copySource.type),
        iconPath(copySource.iconPath),
        category(copySource.category) {}

    inline bool isValid() const {
        return !iconPath.isEmpty();
    }

    QString type;
    QString iconPath;
    HbIndicatorInterface::Category category;
};

inline QDataStream& operator << (QDataStream &outStream,
                                 const IndicatorClientInfo &obj)
{
    outStream << obj.type;
    outStream << obj.iconPath;
    outStream << obj.category;
    return outStream;
}

inline QDataStream& operator >> (QDataStream &inStream,
                                 IndicatorClientInfo &obj)
{
    inStream >> obj.type;
    inStream >> obj.iconPath;
    int category;
    inStream >> category;
    obj.category= static_cast<HbIndicatorInterface::Category>(category);
    return inStream;
}

#endif // HBINDICATORCLIENTINFO_H

// End of File
