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

#ifndef HBSGIMAGEICONPROCESSOR_P_H
#define HBSGIMAGEICONPROCESSOR_P_H

#include <QByteArray>
#include "hbiconprocessor_p.h"
#include "hbnvg_p.h"
#include "hbpixmapiconimpl_p.h"
#include "hbnvgiconimpl_p.h"
#include "hbsgimageiconimpl_p.h"
#include "hbsgimagerenderer_p.h"

class HbSgimageIconProcessor : public HbIconProcessor
{
public :
    HbSgimageIconProcessor(const HbIconKey &key,
                           const HbIconLoader::IconLoaderOptions &options,
                           const QString &type);
    ~HbSgimageIconProcessor();

    HbSharedIconInfo sharedIconData()const;
    int sharedIconDataCost() const;
    bool createIconData(const QString &iconPath);
    bool createMultiPieceIconData(const QVector<HbSharedIconInfo> &multiPieceIconInfo,
                                  HbMultiIconParams &multiPieceIconParams);


private:
    bool createSgimage(const QString &iconPath);
    NvgAspectRatioSettings mapKeyAspectRatioToNvgAspectRatio(Qt::AspectRatioMode aspectRatio);
    int nextPowerOf2(int value);
    bool renderNvg(const QByteArray& byteArray, const QRect& rect, Qt::AspectRatioMode aspectRatioMode, bool mirrored);

private:
    QByteArray byteArray;
    HbSharedIconInfo data;
    static  HbSgImageRenderer *sgImageRenderer;
};
#endif // end of HBSGIMAGEICONPROCESSOR_P_H
