/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbTools module of the UI Extensions for Mobile.
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

#ifndef HBCSSCONVERTERUTILS_P_H
#define HBCSSCONVERTERUTILS_P_H

#include <QList>
#include <QMultiHash>
#include <hbstring_p.h>

class HbCssConverterUtils
{
public:
    // Shared chunk offset management
    static void registerOffsetHolder(int *offset);
    static void unregisterOffsetHolder(int *offset);
    static QMultiHash<int, int *> registeredOffsetHolders();
    static void unregisterAll();

    // Shared chunk allocation management
    static void cellAllocated(int offset, int size);
    static void cellFreed(int offset);
    static void cellMoved(int offset, int newOffset);
    static int defragmentChunk();

    //shared string management
    static void addSharedStringData(const QString &str, const HbString &hbstr);
    static HbString* sharedStringData(const QString &str);

private:

    HbCssConverterUtils();
    ~HbCssConverterUtils();
};

#endif // HBCSSCONVERTERUTILS_P_H
