/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbServers module of the UI Extensions for Mobile.
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

#ifndef HBICONPROCESSOR_P_H
#define HBICONPROCESSOR_P_H

#include <QSizeF>
#include <QHash>
#include "hbiconloader_p.h"
#include "hbthemecommon_p.h"
#include "hbmemorymanager_p.h"

struct HbIconKey
{
    HbIconKey()
    {
    }

    HbIconKey(const QString &file,
              const QSizeF &s,
              Qt::AspectRatioMode aspectRatioM,
              QIcon::Mode m,
              bool mir,
              const QColor &clr):
            filename(file),
            size(s),
            aspectRatioMode(aspectRatioM),
            mode(m),
            mirrored(mir),
            color(clr)
    {
    }

    bool operator == (const HbIconKey &other) const
    {
        // compare filename in the last when everything else is matching
        // to avoid unnecessary string comparisons
        return qFuzzyCompare(size.width(), other.size.width())
               && qFuzzyCompare(size.height(), other.size.height())
               && (aspectRatioMode == other.aspectRatioMode)
               && (mode == other.mode)
               && (mirrored == other.mirrored)
               && (filename == other.filename)
               && (color == other.color);
    }

    HbIconKey& operator = (const HbIconKey &other)
    {
        if (this != &other) {
            size = other.size;
            aspectRatioMode = other.aspectRatioMode;
            mode = other.mode;
            mirrored = other.mirrored;
            filename = other.filename;
            color = other.color;
        }
        return *this;
    }

    QString filename;
    QSizeF size;
    Qt::AspectRatioMode aspectRatioMode;
    QIcon::Mode mode;
    bool mirrored;
    QColor color;
};

inline uint qHash(const HbIconKey &key)
{
    // not using all the members of key to avoid big computation
    return qHash(key.filename) ^(int)key.size.height();
}


class HbIconProcessor
{
public :

    HbIconProcessor()
    {
    }

    virtual ~HbIconProcessor()
    {
    }
    virtual bool createIconData(const QString& iconPath) = 0;

    virtual HbSharedIconInfo sharedIconData() const = 0;

    virtual int sharedIconDataCost() const = 0;


    inline  void applyResolutionCorrection(QSizeF &size)
    {
        // ToDo: Read these params (resolutionfrom somewhere.
        // HbIconLoader has setResolution() and setSourceResolution() functions
        // Know who calls these and who will set the same on server

        qreal resolution = 144.0;
        qreal sourceResolution = 144.0;
        qreal zoom = 1.0;
        size = size * resolution / sourceResolution * zoom;
    }

    virtual bool createMultiPieceIconData(const QVector<HbSharedIconInfo> &multiPieceIconInfo,
                                          HbMultiIconParams &multiPieceIconParams)
    {
        Q_UNUSED(multiPieceIconInfo);
        Q_UNUSED(multiPieceIconParams);
        return false;
    }

protected:

    HbIconKey iconKey;
    HbIconLoader::IconLoaderOptions iconOptions;
    QString iconType;
    QSize defaultSize;
    QSize size;
};

#endif // end of HBICONPROCESSOR_P_H
