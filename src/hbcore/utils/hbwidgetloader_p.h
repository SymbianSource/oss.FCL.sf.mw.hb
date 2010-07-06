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

#ifndef HBWIDGETLOADER_P_H
#define HBWIDGETLOADER_P_H

#include <hbglobal.h>
#include <hbnamespace.h>
#include <hbanchor.h>
#include <QString>
#include "hbstring_p.h"
#include "hbvector_p.h"
#include "hbmemorymanager_p.h"
#include "hbxmlloaderabstractsyntax_p.h"

QT_BEGIN_NAMESPACE
class QIODevice;
QT_END_NAMESPACE
class HbWidget;
class HbWidgetLoaderPrivate;

class HB_CORE_PRIVATE_EXPORT HbWidgetLoader
{
public:

    struct AnchorItem
    {
        HbString srcId;
        HbString dstId;
        Hb::Edge srcEdge;
        Hb::Edge dstEdge;
        
        HbXmlLengthValue::Type minType;
        float minVal;
        HbString minText;

        HbXmlLengthValue::Type prefType;
        float prefVal;
        HbString prefText;

        HbXmlLengthValue::Type maxType;
        float maxVal;
        HbString maxText;

        int sizepolicy;
        int direction;

        HbString anchorId;

        AnchorItem(HbMemoryManager::MemoryType type = HbMemoryManager::HeapMemory)
            : srcId(type), dstId(type), minText(type), prefText(type), maxText(type), anchorId(type) {}
    };

    struct LayoutDefinition {
        HbMemoryManager::MemoryType type;
        HbVector<AnchorItem> anchorItems;
        LayoutDefinition(HbMemoryManager::MemoryType memtype)
            : type(memtype), anchorItems(memtype)
        {
        }
    };

    HbWidgetLoader();
    virtual ~HbWidgetLoader();

    bool load(
        HbWidget* widget,
        const QString &fileName,
        const QString &name,
        const QString &section = QString(),
        HbMemoryManager::MemoryType storage = HbMemoryManager::HeapMemory);

    bool load(
        HbWidget* widget,
        QIODevice *device,
        const QString &name,
        const QString &section = QString());
    
    bool loadLayoutDefinition(
        LayoutDefinition *targetLayoutDef,
        QIODevice *device,
        const QString &name,
        const QString &section );
    
    static QString version();

private:
    HbWidgetLoaderPrivate * const d_ptr;

    Q_DISABLE_COPY(HbWidgetLoader)
    Q_DECLARE_PRIVATE_D(d_ptr, HbWidgetLoader)

};

#endif // HBWIDGETLOADER_P_H
