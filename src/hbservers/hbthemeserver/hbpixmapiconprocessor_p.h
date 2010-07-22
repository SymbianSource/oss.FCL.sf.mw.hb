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

#ifndef HBPIXMAPICONPROCESSOR_P_H
#define HBPIXMAPICONPROCESSOR_P_H

#include "hbiconprocessor_p.h"
#include <QPixmap>

#if defined (HB_NVG_CS_ICON) 
#include <nvg.h>

// Note: Cases of the following two directory names intentionally differ to
//       match the cases of the corresponding directories in Symbian 4.
#include <VG/openvg.h>
#include <vg/vgcontext_symbian.h>
#endif //HB_NVG_CS_ICON

QT_BEGIN_NAMESPACE
class QSvgRenderer;
class QPicture;
class QPixmap;
QT_END_NAMESPACE

class HbThemeServerSymbian;

#if defined (HB_NVG_CS_ICON)
struct HbNvgAspectRatioSettings
{
    TNvgAlignStatusType nvgAlignStatusAndAspectRatio;
    TNvgMeetOrSliceType type;
};
#endif //HB_NVG_CS_ICON

class HbPixmapIconProcessor : public HbIconProcessor
{
public :
    HbPixmapIconProcessor(
        const HbIconKey &key,
        const HbIconLoader::IconLoaderOptions &options,
        const QString &type);

    ~HbPixmapIconProcessor();

    HbSharedIconInfo sharedIconData()const;
    int sharedIconDataCost() const;
    bool createIconData(const QString& iconPath);
    void setPixmap(const QPixmap & pixmap);
    bool createMultiPieceIconData(const QVector<HbSharedIconInfo> &multiPieceIconInfo,
                                  HbMultiIconParams &multiPieceIconParams);

private :
    bool renderSvgToPixmap(const QString& iconPath);
    bool renderPicToPixmap(const QString& iconPath);
    bool renderOtherFormatsToPixmap(const QString& iconPath);

#if defined (HB_NVG_CS_ICON)
    VGIColorBufferFormat mapToVgiDisplayFormat(QImage::Format imageFormat) const;
    bool renderNvgToPixmap(const QString& iconPath);
    HbNvgAspectRatioSettings mapKeyAspectRatioToNvgAspectRatio(
	                                                Qt::AspectRatioMode aspectRatio) const;
#endif //HB_NVG_CS_ICON
private:
    QPixmap pixmap;
};

#endif // end of HBPIXMAPICONPROCESSOR_P_H
