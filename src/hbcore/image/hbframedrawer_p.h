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

#ifndef HBFRAMEDRAWER_P_H
#define HBFRAMEDRAWER_P_H

#include "hbframedrawer.h"
#include "hbiconloader_p.h"
#include "hbthemecommon_p.h"

#include <QByteArray>
#include <QBitmap>
#include <QPainterPath>

class HbMaskableIconImpl;

class HbFrameDrawerPrivate : public QSharedData
{
public:
    HbFrameDrawerPrivate();
    HbFrameDrawerPrivate(const QString &frameGraphicsName, HbFrameDrawer::FrameType type);
    HbFrameDrawerPrivate(const HbFrameDrawerPrivate &other);

    ~HbFrameDrawerPrivate();
    HbIconFormatType iconFormatType() const;
    void checkFrameParts();
    QStringList fileNameSuffixList() const;
    QSize divideSpace(HbMultiPartSizeData &data);
    void createFrameIcon();
    void prepareFrameIcon();

    bool testBorderApiProtectionFlag() const;
    void setBorderApiProtectionFlag(bool on);


    void paint(QPainter *painter);

    void reset( bool resetFrameCount = true );
    void resetMaskableIcon();    
    bool fillWholeRect() const;
    QString multiPartIconId() const;
    HbIconLoader::IconLoaderOptions iconLoaderOptions();
    void unLoadIcon();

private:
    void calculateShrinkedNinePieceCorners(
            HbMultiPartSizeData &data,
            const QSize& tlSize, const QSize& brSize,
            const QSize& fullSize, const QPoint& splitPoint);
    QSizeF defaultSize(const QString &framePartSuffix);
    bool isMirrored();
    bool hasBorderWidths() const;
    // disabled
    HbFrameDrawerPrivate& operator=(const HbFrameDrawerPrivate &other);
    HbIconFormatType iconType;   
public:
    QString frameGraphicsName;
    HbFrameDrawer::FrameType type;
    QRectF rect;
    qreal borderWidths[4];
    QStringList suffixList;
    QBitmap mask;

    // - 0 if frame part files not checked
    // - 1 if not all frame parts exist and fall back to 1 piece frame
    // - frame part count otherwise
    int frameParts; 

    HbIcon::MirroringMode mirroring;

    enum DefaultMirroring
    {
        Unknown = 0,
        Enabled = 1,
        Disabled = 2
    };
    DefaultMirroring defaultMirroring;

    enum internalFlags
    {
        LayoutDirectionSet  = 0x01,
        FillWholeRect       = 0x02,
	    BorderWidthSetByApi = 0x04,
	    DoNotCache           = 0x08,
        ResolutionCorrected  = 0x10,
        NoAutoStartAnimation = 0x20
    };
    int flags; 

    Qt::LayoutDirection layoutDirection;

    QGraphicsItem *graphicsItem;
    QColor color;
    HbMaskableIconImpl *icon;
    bool maskChanged;
    QVector<HbMaskableIconImpl *> fallbackMaskableIconList;
    HbMultiPartSizeData multiPartSizeData;
    QRect prevRect;
    QPainterPath clipPath;
};

#endif // HBFRAMEDRAWER_P_H
