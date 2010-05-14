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

#ifndef HBICONIMPL_P_H
#define HBICONIMPL_P_H

#include <QIcon>
#include <hbglobal.h>

#include "hbthemecommon_p.h"

typedef void HbIconMaskedData ;

class HbMaskableIconImpl;

class HbIconImpl
{
public:

    HbIconImpl()
      : iconRefCount(1)

    {
        createdOnServer = false;
    }

    HbIconImpl(const HbSharedIconInfo& iconData,
               const QString& name,
               const QSizeF& keySize,
               Qt::AspectRatioMode aspectRatioMode,
               QIcon::Mode mode,
               bool mirrored,
               HbRenderingMode renderMode):
                    sharedIconData(iconData),
                    fileName(name),
                    cacheKeySize(keySize),
                    aspectRatioMode(aspectRatioMode),
                    mode(mode),
                    mirrored(mirrored),
                    defaultIconSize(QSize(0,0)),
                    createdOnServer(true),
                    iconRefCount(1),
                    multiPieceIcon(false),
                    renderMode(renderMode)
                    
    {
    }

    virtual QPixmap pixmap() = 0;
    virtual void paint(QPainter* painter,
                        const QRectF &childRect,
                        Qt::Alignment alignment,
                        const QPainterPath &clipPath = QPainterPath(),
						HbMaskableIconImpl * maskIconData = 0) = 0;
    virtual QSize defaultSize() const = 0;
    virtual QSize size() = 0;
    void setColor(const QColor &color)
    {
         this->iconColor = color;
    }

    QColor color()
    {
        return iconColor;
    }

    void setMultiPieceIcon(bool value = true)
    {
        multiPieceIcon = value;
    }

    bool isMultiPieceIcon()
    {
        return multiPieceIcon;
    }

    HbSharedIconInfo iconData() const
    {
        return sharedIconData;
    }

    QString iconFileName() const
    {
        return fileName;
    }

    QSizeF keySize() const
    {
        return cacheKeySize;
    }

    Qt::AspectRatioMode iconAspectRatioMode() const
    {
        return aspectRatioMode;
    }

    QIcon::Mode iconMode() const
    {
        return mode;
    }

    bool isMirrored() const
    {
        return mirrored;
    }

    bool isCreatedOnServer() const
    {
        return createdOnServer;
    }

    void incrementRefCount()
    {
        this->iconRefCount++;
    }

    uint refCount()
    {
        return this->iconRefCount;
    }

    void decrementRefCount()
    {
        this->iconRefCount--;
    }

    void dispose()
    {
        if (iconRefCount == 0) {
            delete this;
        }
    }

    virtual void destroyMaskedData(HbIconMaskedData *data)
    {
        Q_UNUSED(data);
    }
    
    HbRenderingMode iconRenderingMode() const
    {
        return renderMode;
    }

protected:
    virtual ~HbIconImpl()
    {
    }

    HbSharedIconInfo sharedIconData;
    QString fileName;
    QSizeF cacheKeySize;
    Qt::AspectRatioMode aspectRatioMode;
    QIcon::Mode mode;
    bool mirrored;
    QSize defaultIconSize;
    bool createdOnServer;
    uint iconRefCount;
    QColor iconColor;
    bool multiPieceIcon;
    HbRenderingMode renderMode;
};

#endif // HBICONIMPL_P_H

