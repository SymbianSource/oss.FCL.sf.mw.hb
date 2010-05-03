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

#ifndef HB_NVGTLVICON_P_H
#define HB_NVGTLVICON_P_H

#include <VG/openvg.h>
#include "hbnvgicon_p.h"

#include <QSize>
#include <QByteArray>

class HbNvgIconData;
class HbVgImageBinder;
class HbOpenVgHandleStore;

class HbNvgTlvIcon : public HbNvgIcon
{
private:

public:
    enum HbNvgTlvIconCommands {
        TlvPath = 0x50,
        TlvNone
    };

public:
    HbNvgTlvIcon();

    virtual ~HbNvgTlvIcon();

    void setPreserveAspectRatio(HbNvgEngine::HbNvgAlignType preserveAspectSetting,
                                HbNvgEngine::HbNvgMeetType smilFitSetting);

    void rotate(float angle, float x, float y) ;

    void enableMirroring(bool mirroringMode);

    virtual HbNvgEngine::HbNvgErrorType draw(const QSize &size);


    void directDraw(const QByteArray &buffer, const QSize &targetSize);

    void create(const QByteArray &buffer, const QSize& targetSize);

    void setVgImageBinder(HbVgImageBinder *imageBinder) {
        mVgImageBinder = imageBinder;
    }

    void addDrawPathCommand(VGPath path, VGPaintMode paintMode);

    void addCommand(const quint8 * commandBuffer, qint32 commandBufferLength);

    void addCommand(qint8 commandType, const quint8 * commandBuffer, qint32 commandBufferLength);


private:

    void doDraw(const QSize &size);

    void addPathHandle(VGPath path);

    void updateClientMatrices();

    void restoreClientMatrices();

    VGint             mMatrixMode;
    VGfloat             mImageMatrix[9];
    VGfloat             mPathMatrix[9];

    HbNvgIconData *      mNvgIconData;
    HbVgImageBinder *    mVgImageBinder;
    HbOpenVgHandleStore* mOpenVgHandles;
    bool                 mMirrored;
};

#endif

