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

#include "hbnvgtlvicon_p.h"
#include "hbnvgicondata_p.h"
#include "hbnvgutil_p.h"
#include "hbtlvrenderer_p.h"
#include "hbtlviconcreator_p.h"
#include "hbtlviconrenderer_p.h"
#include "hbopenvghandlestore_p.h"

HbNvgTlvIcon::HbNvgTlvIcon()
        : mNvgIconData(0),
        mVgImageBinder(0),
        mMirrored(false)
{
    mNvgIconData = new HbNvgIconData();
    Q_CHECK_PTR(mNvgIconData);
    mOpenVgHandles = new HbOpenVgHandleStore();
    Q_CHECK_PTR(mNvgIconData);
}

HbNvgTlvIcon::~HbNvgTlvIcon()
{
    delete mNvgIconData;
    delete mOpenVgHandles;
}

/*!
    set the aspectRatio \a preserveAspectSetting and \a smilFitSetting
    to be applied on the nvgicon.
*/

void HbNvgTlvIcon::setPreserveAspectRatio(HbNvgEngine::HbNvgAlignType /*preserveAspectSetting*/,
        HbNvgEngine::HbNvgMeetType /*smilFitSetting*/)
{
}

/*!
    Set the \a angle for rotation of the nvgicon at the
    coordiantes  \a x and \a y.
*/

void HbNvgTlvIcon::rotate(float /*angle*/, float /*x*/, float /*y*/)
{
}

void HbNvgTlvIcon::enableMirroring(bool mirroringMode)
{
    mMirrored = mirroringMode;
}

void HbNvgTlvIcon::directDraw(const QByteArray &buffer, const QSize &targetSize)
{
    // Try to set user's matrix to path matrix
    VGfloat origMatrix[9];
    vgGetMatrix(origMatrix);

    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgLoadMatrix(origMatrix);

    if (mMirrored) {
        vgTranslate((VGfloat)(targetSize.width()), 0);
        vgScale(-1.0f, 1.0f);
    }


#ifndef __MIRROR_
    vgScale(1.0f, -1.0f);
    vgTranslate(0, (VGfloat)(-targetSize.height()));
#endif
    HbTlvRenderer *tlvRenderer = new HbTlvRenderer(buffer, targetSize.width(), targetSize.height());
    Q_CHECK_PTR(tlvRenderer);
    QScopedPointer<HbTlvRenderer> iconRenderer(tlvRenderer);

    iconRenderer->initialize();

    iconRenderer->setVgImageBinder(mVgImageBinder);

    iconRenderer->execute();
}

void HbNvgTlvIcon::create(const QByteArray &buffer, const QSize &targetSize)
{
    HbTlvIconCreator *tlvIconCreator = new HbTlvIconCreator(buffer, targetSize.width(), targetSize.height(), this);
    Q_CHECK_PTR(tlvIconCreator);
    QScopedPointer<HbTlvIconCreator> iconCreater(tlvIconCreator);

    iconCreater->initialize();

    iconCreater->execute();
}

/*!
    Draw the nvgicon the nvgicon created of size \a size.
*/
HbNvgEngine::HbNvgErrorType HbNvgTlvIcon::draw(const QSize &size)
{
    HbNvgEngine::HbNvgErrorType error = HbNvgEngine::NvgErrNone;

    updateClientMatrices();

    try {
        doDraw(size);
    } catch (const std::bad_alloc & e) {
        error = HbNvgEngine::NvgErrNoMemory;
    } catch (const HbNvgException & e) {
        error = (HbNvgEngine::HbNvgErrorType) e.errorID();
    }

    // restore everything as we may have changed matrix mode
    restoreClientMatrices();

    vgSeti(VG_SCISSORING, VG_FALSE);

    return error;
}


void HbNvgTlvIcon::doDraw(const QSize &size)
{
    const char* bufPtr = mNvgIconData->buffer();

    HbTlvIconRenderer *tlvIconRenderer = new HbTlvIconRenderer(bufPtr, size.width(), size.height());
    Q_CHECK_PTR(tlvIconRenderer);
    QScopedPointer<HbTlvRenderer> iconRenderer(tlvIconRenderer);

    iconRenderer->setVgImageBinder(mVgImageBinder);

#ifndef __MIRROR_
    vgScale(1.0f, -1.0f);
    vgTranslate(0, (VGfloat)(-size.height()));
#endif

    iconRenderer->execute();
}

void HbNvgTlvIcon::addPathHandle(VGPath path)
{
    if (path) {
        mOpenVgHandles->addPath(path);
    }
}

void HbNvgTlvIcon::addDrawPathCommand(VGPath path, VGPaintMode paintMode)
{
    mOpenVgHandles->addPath(path);
    mNvgIconData->encodeUint8(TlvPath);
    mNvgIconData->encodeUint32(path);
    mNvgIconData->encodeUint32(paintMode);
}

void HbNvgTlvIcon::addCommand(const quint8 * commandBuffer, qint32 commandBufferLength)
{
    mNvgIconData->encodeData(commandBuffer, commandBufferLength);
}

void HbNvgTlvIcon::addCommand(qint8 commandType, const quint8 * commandBuffer, qint32 commandBufferLength)
{
    mNvgIconData->encodeUint8(commandType);
    mNvgIconData->encodeData(commandBuffer, commandBufferLength);
}

void HbNvgTlvIcon::updateClientMatrices()
{
    mMatrixMode = vgGeti(VG_MATRIX_MODE);
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgGetMatrix(mPathMatrix);
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
    vgGetMatrix(mImageMatrix);
    vgSeti(VG_MATRIX_MODE, mMatrixMode);
}

void HbNvgTlvIcon::restoreClientMatrices()
{
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgLoadMatrix(mPathMatrix);
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
    vgLoadMatrix(mImageMatrix);
    vgSeti(VG_MATRIX_MODE, mMatrixMode);
}

