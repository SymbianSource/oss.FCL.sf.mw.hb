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

#ifndef HBOFFSETMAPBUILDER_P_H
#define HBOFFSETMAPBUILDER_P_H

#include <hbsharedcache_p.h>

#include <QString>
#include <QMap>
#include <QList>

class QFileInfo;

enum CSSFileType {
  CSSFile = 0,
  ColorCSSFile,
  CSSFileTypeEnd
};

struct CSSLayoutInfo
{
    QString layoutname;
    QString section;
};

struct LayoutItem
{
    LayoutItem(const CSSLayoutInfo* layout) : layout(layout), offset(-1)
    {
        Q_ASSERT(layout);
    }

    const CSSLayoutInfo* layout;
    int offset;
};

struct HbBinMakerOffsetItem : public HbOffsetItem
{
    QString name;
    QList<HbLayoutIndexItem> layoutIndexItemList;
    bool isNull() const
    {
        return offsetCSS == -1
               && offsetColorCSS == -1
               && offsetLayoutIndexTable == -1;
    }
};

class HbOffsetMapBuilder
{
public:
    HbOffsetMapBuilder() {}

    bool addWidgetOffsets(const QString &className,
             const QFileInfo *fileInfo,
             int offsets[]);
    bool addWidgetMLOffsets(const QString &fileName,
                            quint32 classNameHash,
                            const QList<LayoutItem> &layoutInfoList);

    QByteArray result();
    int size() const { return _mapItems.count(); }

    QList<HbBinMakerOffsetItem> items() const
    {
        return _mapItems.values();
    }

private:
    QMap<quint32, HbBinMakerOffsetItem> _mapItems;
};

#endif // HBOFFSETMAPBUILDER_P_H
