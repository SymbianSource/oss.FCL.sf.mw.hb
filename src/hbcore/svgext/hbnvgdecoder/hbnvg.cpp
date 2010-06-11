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

#include "hbnvg_p_p.h"
#include "hbnvg_p.h"
#include "hbnvgutil_p.h"
#include "hbnvgcsicon_p.h"
#include "hbnvgiconfactory_p.h"
#include "hbnvgexception_p.h"
#include "hbdereferencer_p.h"

#include <QByteArray>

/*!
  Constants for nvg file header offsets
 */
const quint8 NVG_SIGNATURE[]      = "nvg";
const qint32 NVG_SIGNATURE_LENGTH    = sizeof(NVG_SIGNATURE) - sizeof('\0');
const qint32 NVG_VIEWBOX_WIDTH_OFS   = 44;
const qint32 NVG_VIEWBOX_HEIGHT_OFS  = 48;
const qint32 NvgOffsetReserved1       = 6;

HbNvgEnginePrivate::HbNvgEnginePrivate():
        mCurrentBufferSize(1, 1),
        mRotateAngle(0.0),
        mCentreX(0.0),
        mCentreY(0.0),
        mPreserveAspectSetting(HbNvgEngine::NvgPreserveAspectRatioXmidYmid),
        mSmilFitSetting(HbNvgEngine::NvgMeet),
        mCurrentNvgIcon(0),
        mLastError(HbNvgEngine::NvgErrNone),
        mMirrored(false)
{
    vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_BETTER);
    vgSeti(VG_FILL_RULE, VG_NON_ZERO);
}

HbNvgEnginePrivate::~HbNvgEnginePrivate()
{
    delete mCurrentNvgIcon;
}

void HbNvgEnginePrivate::rotate(float angle, float centreX, float centreY)
{
    mCentreX        = centreX;
    mCentreY        = centreY;
    mRotateAngle    = angle;
}

void HbNvgEnginePrivate::setPreserveAspectRatio(HbNvgEngine::HbNvgAlignType preserveAspectSetting,
        HbNvgEngine::HbNvgMeetType smilFitSetting)
{
    mPreserveAspectSetting = preserveAspectSetting;
    mSmilFitSetting = smilFitSetting;
}

QSize HbNvgEnginePrivate::contentDimensions(const QByteArray &buffer) const
{
    QSize ret(0, 0);
    if (buffer.length() < NVG_VIEWBOX_HEIGHT_OFS + sizeof(float)) {
        ret = QSize(0, 0);
    }

    const quint8* buf = (quint8*) buffer.data();
    if ((buffer.length() > NVG_VIEWBOX_WIDTH_OFS) && (buffer.length() > NVG_VIEWBOX_HEIGHT_OFS)) {
        float viewboxWidth = * (float*)(buf + NVG_VIEWBOX_WIDTH_OFS);
        float viewboxHeight = * (float*)(buf + NVG_VIEWBOX_HEIGHT_OFS);

        if (viewboxWidth > 0 && viewboxHeight > 0) {
            ret = QSize(viewboxWidth, viewboxHeight);
        } 
    }
    return ret;
}

void HbNvgEnginePrivate::updateClientMatrices()
{
    mMatrixMode = vgGeti(VG_MATRIX_MODE);
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgGetMatrix(mPathMatrix);
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
    vgGetMatrix(mImageMatrix);
    vgSeti(VG_MATRIX_MODE, mMatrixMode);
}

void HbNvgEnginePrivate::restoreClientMatrices()
{
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgLoadMatrix(mPathMatrix);
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
    vgLoadMatrix(mImageMatrix);
    vgSeti(VG_MATRIX_MODE, mMatrixMode);
}

HbNvgEngine::HbNvgErrorType HbNvgEnginePrivate::drawNvg(const QByteArray &buffer, const QSize &size)
{
    mLastError = HbNvgEngine::NvgErrNone;

    updateClientMatrices();

    try {
        doDrawNvg(buffer, size);
    } catch (const std::bad_alloc & e) {
        mLastError = HbNvgEngine::NvgErrNoMemory;
    } catch (const HbNvgException & e) {
        mLastError = (HbNvgEngine::HbNvgErrorType) e.errorID();
    }

    restoreClientMatrices();

    return mLastError;
}

void HbNvgEnginePrivate::doDrawNvg(const QByteArray & buffer, const QSize &size)
{
    // if size is null then return without doing anything
    if (size.isNull()) {
        return;
    }

    mCurrentBufferSize = size;

    HbDereferencer nvgIconData(buffer);

    quint8 * signature = nvgIconData.derefInt8Array(NVG_SIGNATURE_LENGTH);

    // checking the 'nvg' signature
    if (memcmp(signature, NVG_SIGNATURE, NVG_SIGNATURE_LENGTH) != 0) {
        NVG_DEBUGP1("Not an NVG icon");
        throw HbNvgException(HbNvgEngine::NvgErrNotSupported);
    }

    quint16 reserved1 = nvgIconData.derefInt16(NvgOffsetReserved1) & 0x03;
    HbNvgIconFactory::HbNvgIconType iconType = (HbNvgIconFactory::HbNvgIconType)(reserved1 & 0x03);

    if (!mCurrentNvgIcon) {
        mCurrentNvgIcon = HbNvgIconFactory::create(iconType);   
    } 
    
    mCurrentNvgIcon->setPreserveAspectRatio(mPreserveAspectSetting, mSmilFitSetting);
    mCurrentNvgIcon->rotate(mRotateAngle, mCentreX, mCentreY);
    mCurrentNvgIcon->enableMirroring(mMirrored);
    mCurrentNvgIcon->directDraw(buffer, size);
}

/*!
  Constructs a HbNvgEngine
 */
HbNvgEngine::HbNvgEngine(): d_ptr(new HbNvgEnginePrivate())
{
    Q_CHECK_PTR(d_ptr);
}

/*!
 Destructor
 */
HbNvgEngine::~HbNvgEngine()
{
    delete  d_ptr;
}

/*!
  Set the \a angle for rotation of the NVG graphic at the
  coordiantes  \a xval and \a yval.
 */
void  HbNvgEngine::rotate(float angle, float xval, float yval)
{
    d_ptr->rotate(angle, xval, yval);
}

/*!
  Sets the aspectratio \a preserveAspectSetting and \a smilFitSetting
  to be applied on the nvg graphic.
 */
void HbNvgEngine::setPreserveAspectRatio(HbNvgAlignType preserveAspectSetting,
        HbNvgMeetType smilFitSetting)
{
    d_ptr->setPreserveAspectRatio(preserveAspectSetting, smilFitSetting);
}

/*!
  Returns viewbox dimensions of the nvg graphic as QSize .
 */
QSize HbNvgEngine::contentDimensions(const QByteArray &buffer)const
{
    return d_ptr->contentDimensions(buffer);
}

/*!
  Draw the  nvg graphic with the content \a buffer of size \a size.
 */
HbNvgEngine::HbNvgErrorType HbNvgEngine::drawNvg(const QByteArray &buffer, const QSize &size)
{
    return d_ptr->drawNvg(buffer, size);
}

/*!
  Returns HbNvgEngine::HbNvgErrorType as draw status of the HbNvgEngine.
 */
HbNvgEngine::HbNvgErrorType HbNvgEngine::error()const
{
    return d_ptr->error();
}

void HbNvgEngine::enableMirroring(bool mirroringMode)
{
    d_ptr->enableMirroring(mirroringMode);
}


HbNvgIcon * HbNvgEngine::createNvgIcon(const QByteArray &buffer, const QSize &size)
{
    // TODO @ Deprecated
    Q_UNUSED(buffer);
    Q_UNUSED(size);
    return 0;
}

void HbNvgEngine::setVgImageBinder(HbVgImageBinder *imageBinder)
{
    // TODO @ Deprecated
    Q_UNUSED(imageBinder);
}

void HbNvgEngine::setBackgroundColor(const QColor &rgba8888Color)
{
    // TODO @ Deprecated
    Q_UNUSED(rgba8888Color);
}

void HbNvgEngine::clearBackground()
{
    // TODO @ Deprecated
}


HbNvgEngine::HbNvgErrorType openVgErrorToHbNvgError(qint32 error)
{
    HbNvgEngine::HbNvgErrorType symError = HbNvgEngine::NvgErrNone;
    switch (error) {

    case VGU_OUT_OF_MEMORY_ERROR:
    case VG_OUT_OF_MEMORY_ERROR: {
        symError = HbNvgEngine::NvgErrNoMemory;
        break;
    }

    case VG_ILLEGAL_ARGUMENT_ERROR:
    case VGU_ILLEGAL_ARGUMENT_ERROR: {
        symError = HbNvgEngine::NvgErrArgument;
        break;
    }

    case VG_UNSUPPORTED_PATH_FORMAT_ERROR:
    case VG_UNSUPPORTED_IMAGE_FORMAT_ERROR: {
        symError = HbNvgEngine::NvgErrNotSupported;
        break;
    }

    case VG_IMAGE_IN_USE_ERROR: {
        symError = HbNvgEngine::NvgErrInUse;
        break;
    }

    case VG_BAD_HANDLE_ERROR:
    case VG_PATH_CAPABILITY_ERROR:
    case VGU_PATH_CAPABILITY_ERROR:
    case VGU_BAD_WARP_ERROR:
    case VGU_BAD_HANDLE_ERROR: {
        symError = HbNvgEngine::NvgErrUnknown;
        break;
    }

    default: {
        symError = HbNvgEngine::NvgErrUnknown;
    }
    }

    return symError;
}
