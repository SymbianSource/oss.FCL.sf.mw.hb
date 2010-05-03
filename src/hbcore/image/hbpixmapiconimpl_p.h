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

#ifndef HBPIXMAPICONIMPL_P_H
#define HBPIXMAPICONIMPL_P_H

#include "hbiconimpl_p.h"
#include <QBitmap>

class HbPixmapIconRenderer;

class HB_AUTOTEST_EXPORT HbPixmapIconImpl : public HbIconImpl
{
public :
    HbPixmapIconImpl(HbSharedIconInfo iconData, QString& name,
                     const QSizeF& keySize,
                     Qt::AspectRatioMode aspectRatioMode,
                     QIcon::Mode mode,
                     bool mirrored);
    HbPixmapIconImpl(const QPixmap& pixmap, const QString& name = QString());
    ~HbPixmapIconImpl();

    QPixmap pixmap();
    void paint(QPainter* painter,
               const QRectF &rect,
               Qt::Alignment alignment,
               const QPainterPath &clipPath = QPainterPath(),
               HbMaskableIconImpl * maskIconData = 0);
    QSize defaultSize() const;
    QSize size();
    void destroyMaskedData(HbIconMaskedData *data);

private :
    void retrievePixmapData();

private:
    QPixmap pixmapData;
    HbPixmapIconRenderer *pixmapIconRenderer;
};

#endif

