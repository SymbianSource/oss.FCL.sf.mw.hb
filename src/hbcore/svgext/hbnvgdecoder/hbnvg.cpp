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
#include "hbnvgtlvicon_p.h"
#include "hbnvgiconfactory_p.h"
#include "hbnvgicondata_p.h"
#include "hbnvgexception_p.h"

#include <QByteArray>

/*!
  Constants for nvg file header offsets
 */ 
const quint8 NVG_SIGNATURE[]      = "nvg";
const int NVG_SIGNATURE_LENGTH    = sizeof(NVG_SIGNATURE) - sizeof('\0');
const int NVG_VIEWBOX_WIDTH_OFS   = 44;
const int NVG_VIEWBOX_HEIGHT_OFS  = 48;
const int NvgOffsetReserved1       = 6;

void NVGIconList::addNVGIcon(HbNvgIconFactory::HbNvgIconType type, HbNvgIcon * nvgICon)
{
    if (type <= HbNvgIconFactory::NVGTLV) {
        if (icons[type]) {
            delete icons[type];
        }
        icons[type] = nvgICon;
    }
}

HbNvgIcon * NVGIconList::getIcon(HbNvgIconFactory::HbNvgIconType type)
{
    if (type <= HbNvgIconFactory::NVGTLV) {
        return icons[type]; 
    }
    return 0;
}

NVGIconList::~NVGIconList()
{
    delete icons[HbNvgIconFactory::NVGCS];
    delete icons[HbNvgIconFactory::NVGTLV];
}

HbNvgEnginePrivate::HbNvgEnginePrivate():
        mCurrentBufferSize(1, 1),
        mRotateAngle(0.0),
		mCentreX(0.0),
        mCentreY(0.0),
        mPreserveAspectSetting(HbNvgEngine::NvgPreserveAspectRatioXmidYmid),
        mSmilFitSetting(HbNvgEngine::NvgMeet),
        mVgImageBinder(0),
        mCreatingNvgIcon(false),
		mCurrentNvgIcon(NULL),
		mLastError(HbNvgEngine::NvgErrNone),
		mMirroringMode(false)
{
    vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_BETTER);
    vgSeti(VG_FILL_RULE, VG_NON_ZERO);
}

HbNvgEnginePrivate::~HbNvgEnginePrivate()
{
#ifdef    OPENVG_OBJECT_CACHING
    delete  mCurrentNvgIcon;
#endif
}

void HbNvgEnginePrivate::rotate(float angle, float centreX, float centreY)
{
    mCentreX        = centreX;
    mCentreY        = centreY;
    mRotateAngle    = angle;
}

void HbNvgEnginePrivate::setPreserveAspectRatio(HbNvgEngine::NvgAlignStatusType preserveAspectSetting,
        HbNvgEngine::NvgMeetOrSliceType smilFitSetting)
    {
        mPreserveAspectSetting = preserveAspectSetting;
        mSmilFitSetting = smilFitSetting;
    }

QSize HbNvgEnginePrivate::contentDimensions(const QByteArray &buffer) const
{
    QSize ret(0,0);
     if (buffer.length() < NVG_VIEWBOX_HEIGHT_OFS + sizeof (float)){
        ret = QSize(0, 0);
        } 
     
    const quint8* lBuf = (quint8*) buffer.data();
    if((buffer.length() > NVG_VIEWBOX_WIDTH_OFS) && (buffer.length() > NVG_VIEWBOX_HEIGHT_OFS)) {	
	    float lViewboxWidth = * (float*)(lBuf + NVG_VIEWBOX_WIDTH_OFS);
	    float lViewboxHeight = * (float*)(lBuf + NVG_VIEWBOX_HEIGHT_OFS);

	    if (lViewboxWidth > 0 && lViewboxHeight > 0) {
	        ret = QSize(lViewboxWidth, lViewboxHeight);
	    } else {
	        ret = QSize(0, 0);
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

HbNvgEngine::NvgErrorType HbNvgEnginePrivate::drawNvg(const QByteArray &buffer, const QSize &size)
{
    mLastError = HbNvgEngine::NvgErrNone;
    
    updateClientMatrices();

    try
    {
        doDrawNvg(buffer, size);        
        
    } catch (const std::bad_alloc & e)
    {
        mLastError = HbNvgEngine::NvgErrNoMemory;
    } catch (const HbNvgException & e)
    {
        mLastError = (HbNvgEngine::NvgErrorType) e.errorID();
    }
    
	restoreClientMatrices();
	
    return mLastError;
}

HbNvgIcon * HbNvgEnginePrivate::createNvgIcon(const QByteArray &buffer, const QSize& size)
{
    NVG_DEBUGP1("Creating NVGCSIcon");

    mCurrentNvgIcon     = NULL;
    
#ifdef OPENVG_OBJECT_CACHING
    mCreatingNvgIcon    = true;

    mLastError = drawNvg(buffer, size); 
    if ( mLastError != NvgErrNone) {
        delete mCurrentNvgIcon;
        mCurrentNvgIcon = NULL;
        NVG_DEBUGP2("Creating NVGCSIcon failed %d\n", mLastError);
    }

    mCreatingNvgIcon    = false;  
#else
    QByteArray tempArray = buffer;
    QSize tempSize = size;
    Q_UNUSED(tempArray);
    Q_UNUSED(tempSize);
#endif 
    return mCurrentNvgIcon;
}

void HbNvgEnginePrivate::doDrawNvg(const QByteArray & buffer, const QSize &size)
{    
    // if size is null then return without doing anything
    if (size.isNull()) {
        return;
    }
	
    if (mCurrentBufferSize != size) {
        mCurrentBufferSize = size;
    }

    HbDereferencer nvgIconData(buffer);

    quint8 * signature = nvgIconData.derefInt8Array(NVG_SIGNATURE_LENGTH);
    
    // checking the 'nvg' signature
    if (memcmp(signature, NVG_SIGNATURE, NVG_SIGNATURE_LENGTH) != 0){
        NVG_DEBUGP1("Not an NVG icon");
        throw HbNvgException(HbNvgEngine::NvgErrNotSupported);
    }

    quint16 reserved1 = nvgIconData.derefInt16(NvgOffsetReserved1) & 0x03;
    HbNvgIconFactory::HbNvgIconType iconType = (HbNvgIconFactory::HbNvgIconType)(reserved1 & 0x03);
    
#ifdef OPENVG_OBJECT_CACHING
    mCurrentNvgIcon = HbNvgIconFactory::create(iconType);
    QScopedPointer <HbNvgIcon> nvgIcon(mCurrentNvgIcon);
#else
    HbNvgIcon * nvGIcon = mIconList.getIcon(iconType);
    if (!nvGIcon) {
        mCurrentNvgIcon = HbNvgIconFactory::create(iconType);
        mIconList.addNVGIcon(iconType, mCurrentNvgIcon);
    } else {
        mCurrentNvgIcon = nvGIcon;
    }
#endif
    
    mCurrentNvgIcon->setPreserveAspectRatio(mPreserveAspectSetting, mSmilFitSetting);
    mCurrentNvgIcon->rotate(mRotateAngle, mCentreX, mCentreY);
    mCurrentNvgIcon->setVgImageBinder(mVgImageBinder);
    mCurrentNvgIcon->setMirroringMode(mMirroringMode);

#ifdef OPENVG_OBJECT_CACHING
    if (mCreatingNvgIcon) {
        mCurrentNvgIcon->create(buffer, size);
        nvgIcon.take();
    } else {
        mCurrentNvgIcon->directDraw(buffer, size);
    }
#else
    mCurrentNvgIcon->directDraw(buffer, size);
#endif
}

void HbNvgEnginePrivate::setBackgroundColor(const QColor &rgba8888Color)
{
    mBackgroundColor = rgba8888Color;
}

void HbNvgEnginePrivate::clearBackground()
{
    quint32 rgba = (mBackgroundColor.rgba () << 8) | (mBackgroundColor.rgba () >> 24);
    qint32 r, g, b, a;
    r = (int)((rgba & 0xFF000000) >> 24);
    g = (int)((rgba & 0x00FF0000) >> 16);
    b = (int)((rgba & 0x0000FF00) >> 8);
    a = (int)(rgba & 0x000000FF);

    r += r >> 7; g += g >> 7; b += b >> 7; a += a >> 7;

    const VGfloat Inverse255 =  1.0f / 256.0f;
    const VGfloat clearColor[4] = {(Inverse255 * VGfloat(r)),
                                   (Inverse255 * VGfloat(g)),
                                   (Inverse255 * VGfloat(b)),
                                   (Inverse255 * VGfloat(a))
                                  };

    vgSeti(VG_SCISSORING, VG_FALSE);
    vgSetfv(VG_CLEAR_COLOR, 4, clearColor);
    vgClear(0, 0, mCurrentBufferSize.width(), mCurrentBufferSize.height());
    vgSeti(VG_SCISSORING, VG_TRUE);
}

/*!
  Constructs a HbNvgEngine 
 */
 HbNvgEngine::HbNvgEngine():d_ptr( new HbNvgEnginePrivate())      
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
 void HbNvgEngine::setPreserveAspectRatio(NvgAlignStatusType preserveAspectSetting,
             NvgMeetOrSliceType smilFitSetting)
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
   Creates the nvgicon with the content \a buffer of size \a size and 
   return pointer to the HbNvgIcon.
  */
 HbNvgIcon * HbNvgEngine::createNvgIcon(const QByteArray &buffer, const QSize &size)
 {
     return d_ptr->createNvgIcon(buffer, size);
 }
 
 /*!
   Draw the  nvg graphic with the content \a buffer of size \a size.
  */
 HbNvgEngine::NvgErrorType HbNvgEngine::drawNvg(const QByteArray &buffer, const QSize &size)
 {
     return d_ptr->drawNvg(buffer,size);
 }
 
 /*!
   Set the HbVgImageBinder \a imageBinder  to the HbNvgEngine 
  */
 void HbNvgEngine::setVgImageBinder(HbVgImageBinder *imageBinder) 
 {
     d_ptr->setVgImageBinder(imageBinder);
 }

 /*!
   Returns HbNvgEngine::NvgErrorType as draw status of the HbNvgEngine. 
  */
 HbNvgEngine::NvgErrorType HbNvgEngine::error()const
 {
     return d_ptr->error();
 }
 
 /*!
   Sets the \a rgba8888Color as background color of the nvg graphics. 
  */
 void HbNvgEngine::setBackgroundColor(const QColor &rgba8888Color)
 {
     d_ptr->setBackgroundColor(rgba8888Color);
 }
 
 void HbNvgEngine::setMirroringMode(bool mirroringMode)
{
    d_ptr->setMirroringMode(mirroringMode); 
}
 
 /*!
   Clears the background color of the nvg graphic.
  */
 void HbNvgEngine::clearBackground()
 {
     d_ptr->clearBackground();
 }
     
HbNvgEngine::NvgErrorType openVgErrorToHbNvgError(int error)
{
     HbNvgEngine::NvgErrorType symError = HbNvgEngine::NvgErrNone;
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
