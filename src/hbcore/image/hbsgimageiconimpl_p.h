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

#ifndef HBSGIMAGEICONIMPL_P_H
#define HBSGIMAGEICONIMPL_P_H

#include "hbiconimpl_p.h"
#include <sgresource/sgimage.h>
#include <VG/openvg.h>

#include <QBitmap>

class HbEglStates;

class HB_AUTOTEST_EXPORT HbSgimageIconImpl : public HbIconImpl
{
public :
    HbSgimageIconImpl(const HbSharedIconInfo &iconData,
                      const QString& name,
                      const QSizeF& keySize,
                      Qt::AspectRatioMode aspectRatioMode,
                      QIcon::Mode mode,
                      bool mirrored);

    ~HbSgimageIconImpl();
    QPixmap pixmap();
    void paint(QPainter* painter,
               const QRectF &rect,
               Qt::Alignment alignment,
               HbMaskableIconImpl * maskIconData = 0);

    QSize defaultSize() const;
    QSize size();
    void destroyMaskedData(IconMaskedData data);

private :
    void retrieveSgImageData();
    VGImage getVgImageFromSgImage();
    QPointF setAlignment(const QRectF& rect,
                         QSizeF& renderSize,
                         Qt::Alignment alignment);
    void updatePainterTransformation(QPainter * painter, const QPointF & pos);
    void applySpecialCases(QPainter * painter,
                           const QPointF & topLeft,
                           HbMaskableIconImpl * maskIconData);

private:
    TSgDrawableId sgImageId;
    VGImage vgImage;
    QPixmap currentPixmap;
    bool readyToRender;
    bool specialCaseApplied;
    QSize contentSize;
    VGPaint opacityPaint;
    qreal   lastOpacity;
    HbEglStates *eglStates;
};

#endif // HBSGIMAGEICONIMPL_P_H

