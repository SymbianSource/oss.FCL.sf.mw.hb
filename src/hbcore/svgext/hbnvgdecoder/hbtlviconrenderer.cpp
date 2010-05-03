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

#include "hbtlviconrenderer_p.h"
#include "hbnvgimagebinder_p.h"
#include "hbnvgutil_p.h"
#include "hbnvgicondata_p.h"
#include "hbnvgtlvicon_p.h"

#include <VG/openvg.h>

HbTlvIconRenderer::HbTlvIconRenderer(const QByteArray &buffer, qint32 width, qint32 height)
        : HbTlvRenderer(buffer, width, height)
{
}

HbTlvIconRenderer::~HbTlvIconRenderer()
{
}

void HbTlvIconRenderer::execute(qint32 index)
{
    if (index == HbNvgTlvIcon::TlvPath) {
        drawPath();
    } else {
        HbTlvRenderer::execute(index);;
    }
}

void HbTlvIconRenderer::drawPath()
{
    VGPath path = (VGPath)mNvgIconData->readInt32();
    VGPaintMode paintMode = (VGPaintMode)mNvgIconData->readInt32();
    vgDrawPath(path, paintMode);
}
