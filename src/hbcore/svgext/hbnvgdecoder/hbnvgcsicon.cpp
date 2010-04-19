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

#include "hbnvgcsicon_p.h"
#include "hbnvgicondata_p.h"
#include "hbnvgfittoviewbox_p.h"
#include "hbnvgutil_p.h"
#include "hbopenvghandlestore_p.h"
#include "hbnvgexception_p.h"

#include <QScopedPointer>
#include <QScopedArrayPointer>

/*!
  NVG-CS version
 */
const int Version2   = 2;

/*!
  File offsets
 */
const int NVG_VIEWBOX_WIDTH_OFS = 44;
const int NVG_VIEWBOX_HEIGHT_OFS = 48;
const int NVG_VIEWBOX_X_OFS = 36;
const int NVG_VIEWBOX_Y_OFS = 40;
const int NVG_PATHDATATYPE_OFS = 26;
const int NVG_SCALE_OFS = 28;
const int NVG_BIAS_OFS = 32;
const int NVG_COMMANDSECTION_OFS = 2;
const int NVG_RGBA_OFS = 4;
const int NVG_HEADERSIZE_OFS = 4;
const int NVG_PAINTSECTION_LINEARGRAD_TRANSFORM_OFFSET = 20;
const int NVG_PAINTSECTION_RADIALGRAD_TRANSFORM_OFFSET = 24;
const int NVG_VERSION_OFS         = 3;

/*!
  NVG-CS commands
 */ 
const int CMD_SET_FILL_PAINT           = 4  << 24;
const int CMD_SET_COLOR_RAMP           = 6  << 24;
const int CMD_DRAW_PATH                = 7  << 24;
const int CMD_SET_TRANSFORMATION       = 8  << 24;
const int CMD_SET_STROKE_PAINT         = 5  << 24;
const int CMD_SET_STROKE_WIDTH         = 9  << 24;
const int CMD_SET_STROKE_LINE_JOIN_CAP = 10 << 24;
const int CMD_SET_STROKE_MITER_LIMIT   = 11 << 24;

/*!
 Stroke cap style
 */
const int CAP_BUTT        = 1;
const int CAP_SQUARE      = 2;
const int CAP_ROUND       = 3;

/*!
 Stroke join style
 */
const int LINE_JOIN_BEVEL = 1;
const int LINE_JOIN_MITER = 2;
const int LINE_JOIN_ROUND = 3;

/*!
 Fill paint type
 */
const int PAINT_FLAT      = 1;
const int PAINT_LGRAD     = 2;
const int PAINT_RGRAD     = 3;

/*!
 Stroke paint type
 */
const int STROKE_LGRAD        = 2;
const int STROKE_RGRAD        = 3;
const int STROKE_COLOR_RAMP   = 4;

/*!
 Transform encoding values
 */
const int TRANSFORM_COMPLETE    = 0;
const int TRANSFORM_SCALING     = 2;
const int TRANSFORM_SHEARING    = 4;
const int TRANSFORM_ROTATION    = 8;
const int TRANSFORM_TRANSLATION = 16;

const VGfloat IdentityMatrix[] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f , 1.0f
};

    /**
     * @fn      Constructor
     * @Version
     * @parameter: buffer - None
     *
     * @return: None
     */

HbNvgCsIcon::HbNvgCsIcon()
        :   mPaintFill(VG_INVALID_HANDLE),
        mPaintStroke(VG_INVALID_HANDLE),
        mVgPath(VG_INVALID_HANDLE),
        mLastPathDataType(0),
        mDoFill (VG_FALSE),
        mDoStroke(VG_FALSE),
        mCreatingNvgIcon(0),
        mPreserveAspectSetting(HbNvgEngine::NvgPreserveAspectRatioXmidYmid),
        mSmilFitSetting(HbNvgEngine::NvgMeet),
        mNvgIconData(0),
        mOpenVgHandles(0),
        mLastFillPaintType(0),
        mLastStrokePaintType(0),
        mLastFillPaintColor(0),
        mLastStrkePaintColor(0),
        mResetFillPaint(0),
        mResetStrokePaint(0),
        mMirroringMode(false)
{
}

void HbNvgCsIcon::setIconData(const QByteArray &buffer)
{
    mNvgIconData = new HbNvgIconData(buffer.size());
    Q_CHECK_PTR(mNvgIconData);
    mOpenVgHandles = new HbOpenVgHandleStore();
    Q_CHECK_PTR(mNvgIconData);
}

HbNvgCsIcon::~HbNvgCsIcon()
{
    if (mPaintFill) {
        vgDestroyPaint(mPaintFill);
    }
    if (mPaintStroke) {
        vgDestroyPaint(mPaintStroke);
    }
    if (mVgPath) {
        vgDestroyPath(mVgPath);
    }
    
    vgSetPaint(VG_INVALID_HANDLE, VG_FILL_PATH);
    vgSetPaint(VG_INVALID_HANDLE, VG_STROKE_PATH);
    
    delete mNvgIconData;
    delete mOpenVgHandles;
}


void HbNvgCsIcon::setViewBox(float x, float y, float w, float h)
{
    mViewBoxX = x;
    mViewBoxY = y;
    mViewBoxW = w;
    mViewBoxH = h;
}

void  HbNvgCsIcon::setPreserveAspectRatio(HbNvgEngine::NvgAlignStatusType preserveAspectSetting,
        HbNvgEngine::NvgMeetOrSliceType smilFitSetting)
{
    mPreserveAspectSetting = preserveAspectSetting;
    mSmilFitSetting = smilFitSetting;
}

void HbNvgCsIcon::setMirroringMode(bool mirroringMode)
{
    mMirroringMode = mirroringMode;
}

void HbNvgCsIcon::rotate(float angle, float xValue, float yValue)
{
     mRotationAngle  = angle;
     mRotationX      = xValue;
     mRotationY      = yValue;
}

void HbNvgCsIcon ::directDraw(const QByteArray &buffer, const QSize &targetSize)
{
    drawCommandSection(buffer, targetSize, 0);
}

void HbNvgCsIcon::create(const QByteArray &buffer, const QSize& targetSize)
{
    drawCommandSection(buffer, targetSize, 1);
}

void HbNvgCsIcon::drawCommandSection(const QByteArray &buffer, const QSize & targetSize,
        int objectCaching)
{       
    mCreatingNvgIcon = objectCaching;
 
    HbDereferencer iconData(buffer);
    qint16 lHeaderSize  = iconData.derefInt16(NVG_HEADERSIZE_OFS);
    quint8 nvgVersion   = iconData.derefInt8(NVG_VERSION_OFS);
 
    HbNvgEngine::NvgErrorType errInit = HbNvgEngine::NvgErrNone;
        
    errInit = initializeGc();
    if (HbNvgEngine::NvgErrNone != errInit){
       throw HbNvgException(errInit);
    }
        
    qint16 lPathDataType = iconData.derefInt16(NVG_PATHDATATYPE_OFS);
    float lScale         = iconData.derefReal32(NVG_SCALE_OFS);
    float lBias          = iconData.derefReal32(NVG_BIAS_OFS);

    errInit = createPathHandle(lPathDataType, lScale, lBias);
    if (HbNvgEngine::NvgErrNone != errInit){
        throw HbNvgException(errInit);
    }

    VGfloat lCurrentPathMatrix[9];
    vgGetMatrix(lCurrentPathMatrix);
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);

    // We use the caller's base batrix regardless of which mode the caller was.
    // The caller may have set the matrix in VG_MATRIX_IMAGE_USER_TO_SURFACE mode
    // as it thinks it is drawing images (textures).
    // But even though the texture gets stroked instead, we must use the caller's
    // transformation matrix.
    // Everything gets restored to the original values before we return.
    vgLoadMatrix(lCurrentPathMatrix);

//    applyScissoring(lCurrentPathMatrix, targetSize);

    /*
     * set the rotation angle if available
     */
           
    setRotation();
    
    if (mMirroringMode) {
	    vgTranslate((VGfloat)(targetSize.width()), 0); 
	    vgScale(-1.0f, 1.0f);
	    }
    

#ifdef __MIRROR_
     vgScale(1.0f, -1.0f);
     vgTranslate(0, (VGfloat)(-targetSize.height()));
#endif

    //viewbox parameters
    float lViewboxX   = iconData.derefReal32(NVG_VIEWBOX_X_OFS);
    float lViewboxY   = iconData.derefReal32(NVG_VIEWBOX_Y_OFS);
    float lViewboxW   = iconData.derefReal32(NVG_VIEWBOX_WIDTH_OFS);
    float lViewboxH   = iconData.derefReal32(NVG_VIEWBOX_HEIGHT_OFS);

    applyViewboxToViewPortTransformation(targetSize, lViewboxX, lViewboxY, lViewboxW, lViewboxH);

    quint32 offsetSectionLength = iconData.getLength() - lHeaderSize;
    quint8 * offsetPtr = iconData.derefInt8Array(offsetSectionLength, lHeaderSize);
    
    HbDereferencer offsetSection(offsetPtr, offsetSectionLength);
    quint16 lOffsetVectorCount = offsetSection.derefInt16();

    offsetPtr = iconData.derefInt8Array(offsetSectionLength - sizeof(quint16), lHeaderSize + sizeof(quint16));
    HbDereferencer offsetVector(offsetPtr, offsetSectionLength - sizeof(quint16));

    qint32 commandSectionOffset = lOffsetVectorCount * sizeof(quint16);
    HbDereferencer commandSection((offsetVector.getPtr() + commandSectionOffset), 
            iconData.getLength() - commandSectionOffset - lHeaderSize - sizeof(quint16));
            
    // from version 2 onwards command section will start on word boundary
    if (nvgVersion >= Version2 && ((lOffsetVectorCount  & 0x01) == 0))
        {
        commandSection.skip(2);
        }
         
    quint16 lCmdCount = commandSection.derefInt16();
    commandSection.skip(NVG_COMMANDSECTION_OFS);   
    
    /*
     * from version 2 onwards there will be a padding added
     * after the command count to make it word aligned
     */
    if (nvgVersion >= Version2)
        {
        commandSection.skip(2);
        }
        
    executeNVGCSCommandLoop(lCmdCount, &iconData, &offsetVector, &commandSection, nvgVersion);
}

void HbNvgCsIcon::executeNVGCSCommandLoop(quint16 commandCount, HbDereferencer *iconData, 
        HbDereferencer * offsetVector,HbDereferencer * commandSection, quint8 nvgVersion)
    {
    quint32 transVal;

    VGfloat lCurrentPathMatrix[9];
    
    vgGetMatrix(lCurrentPathMatrix);
        
    qint32 lOffsetIx = 0;
    for (int i=0; i < commandCount; i++)
        {
        quint32 currentCommand = commandSection->derefInt32();
        lOffsetIx = currentCommand & 0x0000ffff;
                
        switch (currentCommand & 0xff000000)
            {
            case CMD_SET_FILL_PAINT:
                {
                mFillAlpha        = (currentCommand & 0x00ff0000) >> 16;
                quint16 offset    = offsetVector->derefInt16(lOffsetIx * sizeof(quint16));
                
                HbDereferencer section = getCommandSection(offset, iconData, nvgVersion);
                                
                setFillPaint(&section); 
                }
                break;    
            case CMD_SET_COLOR_RAMP:
                {
                quint16 offset = offsetVector->derefInt16(lOffsetIx * sizeof(quint16));

                HbDereferencer section = getCommandSection(offset, iconData, nvgVersion);

                setColorRamp(&section);
                }
                break;
            case CMD_DRAW_PATH:
                {
                if ((currentCommand & 0x00010000)) 
                    {
                    mDoStroke = VG_TRUE;
                    }
                
                if ((currentCommand & 0x00020000))
                    {
                    mDoFill = VG_TRUE;
                    }
                quint16 offset = offsetVector->derefInt16(lOffsetIx * sizeof(quint16));

                HbDereferencer section = getCommandSection(offset, iconData, nvgVersion);

                drawPath(&section);
                }
                break;
            case CMD_SET_TRANSFORMATION:
                {
                setTransform(commandSection, transVal, lCurrentPathMatrix);
                commandSection->skip(transVal * sizeof(quint32));
                }
                break;
            case CMD_SET_STROKE_PAINT:
                {
                mStrokeAlpha = (currentCommand & 0x00ff0000) >> 16;
                quint16 offset = offsetVector->derefInt16(lOffsetIx * sizeof(quint16));

                HbDereferencer section = getCommandSection(offset, iconData, nvgVersion);

                setStrokePaint(&section); 
                }
                break;
            case CMD_SET_STROKE_WIDTH:
                {
                float lStrokeWidth;
                commandSection->skip(sizeof(quint32));
                
                /*
                 * check for alignment and copy data if not aligned, else directly convert
                 * version 2 or above guarantees that is always word aligned
                 */
                quint8 * cptr = commandSection->derefInt8Array(sizeof(float), 0); 
                if (nvgVersion < Version2 && !isAligned4(cptr))
                    {
                    
                    memcpy(reinterpret_cast<void *>(&lStrokeWidth),
                              reinterpret_cast<void *>(cptr), sizeof(lStrokeWidth));
                    }
                else
                    {
                    lStrokeWidth = commandSection->derefReal32();
                    }
                
                COND_COM_OC(mCreatingNvgIcon,
                        addSetStrokeWidthCommand(lStrokeWidth),
                        vgSetf(VG_STROKE_LINE_WIDTH, lStrokeWidth));
                }
                break;
            case CMD_SET_STROKE_MITER_LIMIT:
                {
                float lMiterLimit;
                commandSection->skip(sizeof(float));

                /*
                 * check for alignment and copy data if not aligned, else directly convert
                 * version 2 or above guarantees that is always word aligned
                 */
                quint8 * cptr = commandSection->derefInt8Array(sizeof(float), 0); 

                if (nvgVersion < Version2 && !isAligned4(cptr))
                    {
                    memcpy(reinterpret_cast<void *>(&lMiterLimit),
                              reinterpret_cast<void *>(cptr), sizeof(lMiterLimit));
                    }
                else
                    {
                    lMiterLimit = commandSection->derefReal32();
                    }
                
                COND_COM_OC(mCreatingNvgIcon,
                        addSetStrokeMiterLimitCommand(lMiterLimit),
                        vgSetf(VG_STROKE_MITER_LIMIT, lMiterLimit));
                }
                break;
            case CMD_SET_STROKE_LINE_JOIN_CAP:
                {
                quint8 lJoinType =(currentCommand & 0x0000ff00)>>8;
                quint8 lCapType = (currentCommand & 0x000000ff);
                
                VGCapStyle capStyle;
                switch(lCapType)
                    {
                    case CAP_SQUARE:
                        capStyle = VG_CAP_SQUARE;
                        break;
                    case CAP_ROUND:
                        capStyle = VG_CAP_ROUND;
                        break;
                    case CAP_BUTT:
                    default:
                        capStyle = VG_CAP_BUTT;
                        break;
                    }
   
                VGJoinStyle lineJoinStyle;
                switch(lJoinType)
                    {
                    case LINE_JOIN_BEVEL:
                        lineJoinStyle = VG_JOIN_BEVEL;
                        break;
                    case LINE_JOIN_ROUND:
                        lineJoinStyle = VG_JOIN_ROUND;
                        break;
                    case LINE_JOIN_MITER:
                    default:
                        lineJoinStyle = VG_JOIN_MITER;
                        break;
                    }
                
                COND_COM_OC(mCreatingNvgIcon,
                        addStrokeLineJoinCapCommand(capStyle, lineJoinStyle),
                        vgSeti(VG_STROKE_CAP_STYLE, capStyle);
                        vgSeti(VG_STROKE_JOIN_STYLE, lineJoinStyle););
                }
                break;
            default:
                {
                throw HbNvgException(HbNvgEngine::NvgErrCorrupt);			
                }
            }
        
        // go to the next command
        commandSection->skip(sizeof(quint32));
        }
    
    }

HbDereferencer HbNvgCsIcon::getCommandSection(quint16 offset, HbDereferencer * iconData, int nvgVersion)
    {
    // the max length that the command section can have
    int commandSectionLength = iconData->getLength() - offset;

    if (commandSectionLength <= 0){
        throw HbNvgException(HbNvgEngine::NvgErrCorrupt);
    }

    HbDereferencer section(iconData->derefInt8Array(commandSectionLength, offset), commandSectionLength);

    /*
     * all the section are expected to be word aligned
     * all of the nvg-cs icon will be version 2 or above
     * the else won't be there as nvg version will always be greater than 2
     */
    if (nvgVersion >= Version2) {
        if (!isAligned4(offset)) {
            throw HbNvgException(HbNvgEngine::NvgErrCorrupt);
        }
    }else{
        /*
         * no need to do anything here as once the nvgdecoder release
         * its version will be always greater than 2
         * infact the check for version will be removed
         */ 
    }
    
    return section;
    }

void HbNvgCsIcon::applyViewboxToViewPortTransformation(const QSize& targetSize, 
        float viewboxX, float viewboxY, float viewboxW, float viewboxH)
    {
    HbNvgFitToViewBoxImpl *viewBoxTx = new HbNvgFitToViewBoxImpl();
    Q_CHECK_PTR(viewBoxTx);
    QScopedPointer<HbNvgFitToViewBoxImpl> viewboxTrnsfr(viewBoxTx);

    /*
     * this is bit unreadable,
     * need to find a better design to separate the object caching solution from normal rendering,
     */
    
    COND_COM_OC_NOC(
            {
            if (mCreatingNvgIcon) {
                setViewBox(viewboxX, viewboxY, viewboxW, viewboxH);
                }
            else{
                viewboxTrnsfr->setAlign(mPreserveAspectSetting);
                viewboxTrnsfr->setScaling(mSmilFitSetting);
                
                if (viewboxW > 0 && viewboxH > 0){
                    viewboxTrnsfr->setViewBox(viewboxX, viewboxY, viewboxW, viewboxH);
                }
                
                int lWidth = aTargetSize.width();
                int lHeight = aTargetSize.height();
                
                viewboxTrnsfr->setWindowViewportTrans(QRect(0, 0, lWidth, lHeight), QSize(0, 0));
            }
           },
            {
            viewboxTrnsfr->setAlign(mPreserveAspectSetting);
            viewboxTrnsfr->setScaling(mSmilFitSetting);
            
            if (viewboxW > 0 && viewboxH > 0){
                viewboxTrnsfr->setViewBox(viewboxX, viewboxY, viewboxW, viewboxH);
                }
            
            int lWidth = targetSize.width();
            int lHeight = targetSize.height();
            
            viewboxTrnsfr->setWindowViewportTrans(QRect(0, 0, lWidth, lHeight), QSize(0, 0));
            });
    }

void HbNvgCsIcon::applyScissoring(VGfloat *aMatrix, const QSize& targetSize)
    {
    /*
     * calculate the rectangle with respect to the transformation applied
     * and set the scissoring rect
     */ 
    QPoint leftBottom  = getTranslatedPoint(aMatrix, QPoint(0, 0));
    QPoint leftTop     = getTranslatedPoint(aMatrix, QPoint(0, targetSize.height()));
    QPoint rightBottom = getTranslatedPoint(aMatrix, QPoint(targetSize.width(), 0));
    QPoint rightTop    = getTranslatedPoint(aMatrix, QPoint(targetSize.width(),targetSize.height()));

    VGfloat minX = leftBottom.x();
    VGfloat minY = leftBottom.y();
    VGfloat maxX = leftBottom.x();
    VGfloat maxY = leftBottom.y();

    minX = minVal4(leftBottom.x(), leftTop.x(), rightBottom.x(), rightTop.x());
    minY = minVal4(leftBottom.y(), leftTop.y(), rightBottom.y(), rightTop.y());

    maxX = maxVal4(leftBottom.x(), leftTop.x(), rightBottom.x(), rightTop.x());
    maxY = maxVal4(leftBottom.y(), leftTop.y(), rightBottom.y(), rightTop.y());

    VGfloat newW = maxX - minX;
    VGfloat newH = maxY - minY;

    VGint clipRect[] = {minX, minY, newW, newH};

    vgSeti(VG_SCISSORING, VG_TRUE);
    vgSetiv(VG_SCISSOR_RECTS, 4, clipRect);
    }


HbNvgEngine::NvgErrorType HbNvgCsIcon::initializeGc()
{
    if (mPaintFill == VG_INVALID_HANDLE) {
        mPaintFill = vgCreatePaint();
        if (mPaintFill == VG_INVALID_HANDLE) {
            return openVgErrorToHbNvgError(vgGetError());
        }
    }
    
    vgSetPaint(mPaintFill, VG_FILL_PATH);
    
    if (mPaintStroke == VG_INVALID_HANDLE) {
        mPaintStroke = vgCreatePaint();
        if (mPaintStroke == VG_INVALID_HANDLE) {
            return openVgErrorToHbNvgError(vgGetError());
        }
    }

    vgSetPaint(mPaintStroke, VG_STROKE_PATH);

    return HbNvgEngine::NvgErrNone;
}

HbNvgEngine::NvgErrorType HbNvgCsIcon::createPathHandle(qint16 pathDataType, float scale, float bias)
{
    (void) scale;
    (void) bias;

    HbNvgEngine::NvgErrorType error = HbNvgEngine::NvgErrNone;

    if (mLastPathDataType != pathDataType) {
        if (mVgPath != VG_INVALID_HANDLE) {
            vgDestroyPath(mVgPath);
            mVgPath = VG_INVALID_HANDLE;
        }
    }

    if (mVgPath == VG_INVALID_HANDLE) {
        switch (pathDataType) {
        case NvgEightBitEncoding: {
            mVgPath = vgCreatePath(VG_PATH_FORMAT_STANDARD,
                                   VG_PATH_DATATYPE_S_16, 1.0f / 2.0f, 0.0f, 0, 0,
                                   VG_PATH_CAPABILITY_APPEND_TO);
            break;
            }
            
        case NvgSixteenBitEncoding: {
            mVgPath = vgCreatePath(VG_PATH_FORMAT_STANDARD,
                                   VG_PATH_DATATYPE_S_16, 1.0f / 16.0f, 0.0f, 0, 0,
                                   VG_PATH_CAPABILITY_APPEND_TO);
            break;
            }
            
        case NvgThirtyTwoBitEncoding: {
            mVgPath = vgCreatePath(VG_PATH_FORMAT_STANDARD,
                                   VG_PATH_DATATYPE_S_32, 1.0f / 65536.0f, 0.0f, 0, 0,
                                   VG_PATH_CAPABILITY_APPEND_TO);
            break;
            }

        default: {
            return HbNvgEngine::NvgErrCorrupt;
            }
        }
    }

    if (mVgPath == VG_INVALID_HANDLE) {
        // Get the Symbian error code
        error = openVgErrorToHbNvgError(vgGetError());

        if (error == HbNvgEngine::NvgErrNoMemory) {
            NVG_DEBUGP1("NVG Error OOMs: vgCreatePaint");
            resetNvgState();
        }

        NVG_DEBUGP1("NVG Error: vgCreatePaint");
        return error;
    }

    mLastPathDataType   = pathDataType;

    return error;
}

void HbNvgCsIcon::resetNvgState()
{
    if (mVgPath != VG_INVALID_HANDLE) {
        vgDestroyPath(mVgPath);
        mVgPath = VG_INVALID_HANDLE;
    }

    if (mPaintFill != VG_INVALID_HANDLE) {
        vgSetPaint(VG_INVALID_HANDLE, VG_FILL_PATH);
        vgDestroyPaint(mPaintFill);
        mPaintFill = VG_INVALID_HANDLE;
    }

    if (mPaintStroke != VG_INVALID_HANDLE) {
        vgSetPaint(VG_INVALID_HANDLE, VG_STROKE_PATH);
        vgDestroyPaint(mPaintStroke);
        mPaintStroke = VG_INVALID_HANDLE;
    }
}

    /**
     * @fn setFillPaint
     * @brief  SetPaint gradient or solid in OpenVG
     * @version
     * @param   buffer - buffer containing OpenVG data
     * @return  None
     */

void HbNvgCsIcon::setFillPaint(HbDereferencer *iconData)
{
    COND_COM_OC_OOC(register int drawingMode = mCreatingNvgIcon);
    
    quint32 lCommonData  = iconData->derefInt32();
    quint32 lPaintType   = lCommonData & 0x07;
    quint16 lSpecifcData = (lCommonData >> 16) & 0xff;

    switch (lPaintType) {
        case PAINT_LGRAD: {
            mGradPaintFill = mPaintFill;
            COND_COM_OC_OOC(
                if (mCreatingNvgIcon) {
                // CNVGCSIcon will destroy the paint handle
                mGradPaintFill = vgCreatePaint();
                if (mGradPaintFill == VG_INVALID_HANDLE){
                    throw HbNvgException(openVgErrorToHbNvgError(vgGetError()));
                }
            });

            // gradient data, the data will be word aligned
            float* lGradData =  (float*)iconData->derefInt8Array(4 * sizeof(VGfloat), sizeof(float));
            
            vgSetParameteri(mGradPaintFill, VG_PAINT_TYPE, VG_PAINT_TYPE_LINEAR_GRADIENT);
            vgSetParameterfv(mGradPaintFill, VG_PAINT_LINEAR_GRADIENT, 4, lGradData);
            vgSeti(VG_MATRIX_MODE, VG_MATRIX_FILL_PAINT_TO_USER);

            if (lSpecifcData & 0x1) {
                float* lGradMatrix1 = (float*) iconData->derefInt8Array(6 * sizeof (VGfloat),
                                                                   NVG_PAINTSECTION_LINEARGRAD_TRANSFORM_OFFSET);
                               
                float lGradMatrix[9] = {lGradMatrix1[0], lGradMatrix1[3], 0.0f,
                                        lGradMatrix1[1], lGradMatrix1[4], 0.0f,
                                        lGradMatrix1[2], lGradMatrix1[5], 1.0f
                                       };
    
                COND_COM_OC(drawingMode,
                            addLinearGradientCommand(4, lGradData, lGradMatrix, mGradPaintFill),
                            vgLoadMatrix(lGradMatrix););
                Q_UNUSED(IdentityMatrix);
            } else {
                COND_COM_OC(drawingMode,
                            addLinearGradientCommand(4, lGradData, (VGfloat*)IdentityMatrix, mGradPaintFill),
                            vgLoadIdentity());
            }

            COND_COM_OC(drawingMode, ; ,
                        vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE));
        }
        break;
        case PAINT_RGRAD: {
            mGradPaintFill = mPaintFill;
    
            COND_COM_OC_OOC(
            if (mCreatingNvgIcon) {
                mGradPaintFill = vgCreatePaint();
                if (mGradPaintFill == VG_INVALID_HANDLE){
                    throw HbNvgException(openVgErrorToHbNvgError(vgGetError()));
                }                
            });
    
            // gradient data, the data will be word aligned
            float* lGradData = (float*)iconData->derefInt8Array(4 * sizeof(VGfloat), sizeof(quint32));
                       
            vgSetParameteri(mGradPaintFill, VG_PAINT_TYPE, VG_PAINT_TYPE_RADIAL_GRADIENT);
            vgSetParameterfv(mGradPaintFill, VG_PAINT_RADIAL_GRADIENT, 5, lGradData);
            vgSeti(VG_MATRIX_MODE, VG_MATRIX_FILL_PAINT_TO_USER);
    
    
            if (lSpecifcData & 0x1) {
                float* lGradMatrix1 = (float*)iconData->derefInt8Array(6 * sizeof (VGfloat),
                                        NVG_PAINTSECTION_RADIALGRAD_TRANSFORM_OFFSET);
                float lGradMatrix[9] = {lGradMatrix1[0], lGradMatrix1[3], 0.0f,
                                        lGradMatrix1[1], lGradMatrix1[4], 0.0f,
                                        lGradMatrix1[2], lGradMatrix1[5], 1.0f
                                       };
    
                COND_COM_OC(drawingMode,
                            addRadialGradientCommand(5, lGradData, lGradMatrix, mGradPaintFill),
                            vgLoadMatrix(lGradMatrix));
                Q_UNUSED(IdentityMatrix);
            } else {
                COND_COM_OC(drawingMode,
                            addRadialGradientCommand(5, lGradData, (VGfloat*)IdentityMatrix, mGradPaintFill),
                            vgLoadIdentity());
            }
    
            COND_COM_OC(drawingMode, ; ,
                        vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE));
        }
        break;
        case PAINT_FLAT:
            {
            quint32 lRgba = iconData->derefInt32(NVG_RGBA_OFS);
    
            lRgba = (lRgba & 0xffffff00) | mFillAlpha;
            
            COND_COM_OC(drawingMode,
                    addSetColorCommand(lRgba),
                    vgSetParameteri(mPaintFill, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
                    vgSetColor(mPaintFill, lRgba));            
            }
            break;
        default: {
            throw HbNvgException(HbNvgEngine::NvgErrCorrupt);
        }
    }
}

void HbNvgCsIcon::setColorRamp(HbDereferencer *iconData)
{
    quint32 lCommonData = iconData->derefInt32();

    int lStopCount = (lCommonData >> 16) & 0x00ff;
    float* lStopData = (float*) iconData->derefInt8Array(lStopCount * 5 * sizeof(float), sizeof(quint32));
    
    VGfloat *crs = new VGfloat[lStopCount * 5];
    Q_CHECK_PTR(crs);
    QScopedArrayPointer<VGfloat> colorRamps(crs); 
    
    if (mFillAlpha == 0xff) {
        vgSetParameteri(mGradPaintFill, VG_PAINT_COLOR_RAMP_SPREAD_MODE, VG_COLOR_RAMP_SPREAD_PAD);
        vgSetParameterfv(mGradPaintFill, VG_PAINT_COLOR_RAMP_STOPS, lStopCount*5, lStopData);
    } else {
        // Copy color ramps and modify alpha
        memcpy(colorRamps.data(), lStopData, lStopCount*5*sizeof(VGfloat));
        VGfloat lAlphaInFloat = mFillAlpha * 0.003921568627450f; //(1.0f/255.0f);
        VGfloat* lAlphaValue = &(colorRamps[4]);
        for (int i = 0; i < lStopCount; i++) {
            *lAlphaValue *= lAlphaInFloat;
            lAlphaValue += 5;
        }

        vgSetParameteri(mGradPaintFill, VG_PAINT_COLOR_RAMP_SPREAD_MODE, VG_COLOR_RAMP_SPREAD_PAD);
        vgSetParameterfv(mGradPaintFill, VG_PAINT_COLOR_RAMP_STOPS, lStopCount * 5, colorRamps.data());
    }
}

void HbNvgCsIcon::drawPath(HbDereferencer * iconData)
{
    int numSegments = iconData->derefInt16();
    const VGubyte * pathSegments  = iconData->derefInt8Array(numSegments, sizeof(quint16));
    /*
    * verify that what we got is proper data
    * for that calculate the path co-ordinate length
    * and check that the path data does not overflow
    */    
   int coordinateCount = 0;
   for (int i = 0; i < numSegments; ++i)
       {
       switch (pathSegments[i])
           {
           case VG_HLINE_TO:
           case VG_VLINE_TO:
               coordinateCount += 1;
               break;
           case VG_MOVE_TO:
           case VG_LINE_TO:
           case VG_SQUAD_TO:
               coordinateCount += 2;
               break;                
           case VG_QUAD_TO:
           case VG_SCUBIC_TO:
               coordinateCount += 4;
               break;
           case VG_SCCWARC_TO:
           case VG_SCWARC_TO:
           case VG_LCCWARC_TO:
           case VG_LCWARC_TO:
               coordinateCount += 5;
               break;
           case VG_CUBIC_TO:
               coordinateCount += 6;
               break;
           default:
               break;
           }
       }

   // this one is just to check the alignment
   quint8* pathData = iconData->derefInt8Array(sizeof(float), sizeof(quint16) + numSegments);
   
   /*
    * path data need to be word aligned
    * alignment are done according to the path format
    */
   quint32 sizeofpathdata = sizeof(float);
   quint32 alignSkip = 0;
   quint8 * alignedPtr = 0;
   if (mLastPathDataType == NvgSixteenBitEncoding)
       {
       alignedPtr = Align2(pathData);
       sizeofpathdata = sizeof( quint16);
       }
   else if (mLastPathDataType == NvgThirtyTwoBitEncoding)
       {        
       alignedPtr = Align4(pathData);
       }
   else
       {
       throw HbNvgException(HbNvgEngine::NvgErrCorrupt);
       }
   
   alignSkip = alignedPtr - pathData; 
   
   /*
    * check to see whether we have enough path data
    */
   iconData->isSafe(coordinateCount * sizeofpathdata + alignSkip, sizeof(quint16) + numSegments);
   
   pathData = alignedPtr;
   
   VGint paintMode = (mDoFill ? VG_FILL_PATH : 0)|(mDoStroke ? VG_STROKE_PATH : 0);
   if (paintMode == 0)
       {
       paintMode = VG_FILL_PATH;
       }
   
   COND_COM_OC(mCreatingNvgIcon,
           {
               VGPath path = createPath();
               
               if (path != VG_INVALID_HANDLE)
                   {
                   vgAppendPathData(path, numSegments, pathSegments, pathData);
                   }
               else
                   {
                   addPathData(numSegments, pathSegments, pathData);
                   }        
                   addDrawPathCommand(path, paintMode);
           }, 
           {
               vgClearPath(mVgPath, VG_PATH_CAPABILITY_APPEND_TO);
               
               vgAppendPathData(mVgPath, numSegments, pathSegments, pathData);
               vgDrawPath(mVgPath, paintMode);
           });
    mDoStroke   = VG_FALSE;
    mDoFill     = VG_FALSE;
}

void HbNvgCsIcon::setTransform(HbDereferencer * iconData, quint32 & counter, const VGfloat* currentMatrix)
{
    COND_COM_OC(mCreatingNvgIcon, ;, vgLoadMatrix(currentMatrix));

    quint32 lCommonData =  iconData->derefInt32();
    quint32 lTransformType = (lCommonData & 0x00ff0000) >> 16 ;

    VGfloat matrixTemp[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    counter = 0;

    if (lTransformType != 1) {
        if (lTransformType == TRANSFORM_COMPLETE) {
            matrixTemp[0] = iconData->derefReal32((++counter) * sizeof (VGfloat));
            matrixTemp[4] = iconData->derefReal32((++counter) * sizeof (VGfloat));
            matrixTemp[3] = iconData->derefReal32((++counter) * sizeof (VGfloat));
            matrixTemp[1] = iconData->derefReal32((++counter) * sizeof (VGfloat));
            matrixTemp[6] = iconData->derefReal32((++counter) * sizeof (VGfloat));
            matrixTemp[7] = iconData->derefReal32((++counter) * sizeof (VGfloat));
        } else {
            if (lTransformType & TRANSFORM_ROTATION) {
                //vgScale
                matrixTemp[0] = iconData->derefReal32((++counter) * sizeof (VGfloat));
                matrixTemp[4] = iconData->derefReal32((++counter) * sizeof (VGfloat));
                
                //vgShear
                matrixTemp[3] = iconData->derefReal32((++counter) * sizeof (VGfloat));
                matrixTemp[1] = iconData->derefReal32((++counter) * sizeof (VGfloat));
            } else {
                if (lTransformType & TRANSFORM_SCALING) {
                    //vgScale
                    matrixTemp[0] = iconData->derefReal32((++counter) * sizeof (VGfloat));
                    matrixTemp[4] = iconData->derefReal32((++counter) * sizeof (VGfloat));
                }

                if (lTransformType & TRANSFORM_SHEARING) {
                    //vgShear
                    matrixTemp[3] = iconData->derefReal32((++counter) * sizeof (VGfloat));
                    matrixTemp[1] = iconData->derefReal32((++counter) * sizeof (VGfloat));;
                }
            }

            if (lTransformType & TRANSFORM_TRANSLATION) {
                //vgTranslate
                matrixTemp[6] = iconData->derefReal32((++counter) * sizeof (VGfloat));;
                matrixTemp[7] = iconData->derefReal32((++counter) * sizeof (VGfloat));;
            }
        }

        COND_COM_OC(mCreatingNvgIcon,
                    addSetTransformCommand(matrixTemp, 1),
                    vgMultMatrix(matrixTemp));
    } else {
        COND_COM_OC(mCreatingNvgIcon,
                    addSetTransformCommand(matrixTemp, 0), ;);
    }
}

void HbNvgCsIcon::setStrokePaint(HbDereferencer * iconData)
{
    COND_COM_OC_OOC(register int drawingMode = mCreatingNvgIcon;);
    
    quint32 lCommonData = iconData->derefInt32();
    quint32 lStrokeType = lCommonData & 0x07;
    quint16 lSpecifcData = (lCommonData >> 16) & 0xff;

    switch (lStrokeType) {
    case STROKE_LGRAD: {
        mGradPaintStroke = mPaintStroke;
        
        COND_COM_OC_OOC(
                   if (mCreatingNvgIcon)
                       {
                       mGradPaintStroke = vgCreatePaint();
                       if (mGradPaintStroke == VG_INVALID_HANDLE) {
                           throw HbNvgException(HbNvgEngine::NvgErrBadHandle);
                       }
                       });
              
        // gradient data, the data will be word aligned
        float* lGradData = (float*)iconData->derefInt8Array(4 * sizeof(VGfloat), sizeof(float));
                   
        COND_COM_OC(drawingMode, ; ,
                    vgSetParameteri(mGradPaintStroke, VG_PAINT_TYPE, VG_PAINT_TYPE_LINEAR_GRADIENT);
                    vgSetParameterfv(mGradPaintStroke, VG_PAINT_LINEAR_GRADIENT, 4, lGradData);
                    vgSeti(VG_MATRIX_MODE, VG_MATRIX_STROKE_PAINT_TO_USER));

        if (lSpecifcData & 0x1) {
            float* lGradMatrix1 = (float*)iconData->derefInt8Array(6 * sizeof(VGfloat),
                    4 + 4 * sizeof(VGfloat));
                            
            float lGradMatrix[9] = {lGradMatrix1[0], lGradMatrix1[3], 0.0f,
                                    lGradMatrix1[1], lGradMatrix1[4], 0.0f,
                                    lGradMatrix1[2], lGradMatrix1[5], 1.0f
                                   };

            COND_COM_OC(drawingMode,
                        addStrokeLinearGradientCommand(4, lGradData, lGradMatrix, mGradPaintStroke),
                        vgLoadMatrix(lGradMatrix));
            Q_UNUSED(IdentityMatrix);
        } else {
            COND_COM_OC(drawingMode,
                        addStrokeLinearGradientCommand(4, lGradData, (VGfloat*)IdentityMatrix, mGradPaintStroke),
                        vgLoadIdentity());
        }
        vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    } 
    break;
    case STROKE_RGRAD: {
        mGradPaintStroke = mPaintStroke;
        
        COND_COM_OC_OOC(
                    if (mCreatingNvgIcon)
                        {
                        mGradPaintStroke = vgCreatePaint();
                        if (mGradPaintStroke == VG_INVALID_HANDLE) {
                            throw HbNvgException(HbNvgEngine::NvgErrBadHandle);
                        }
                        });
        // gradient data, the data will be word aligned
        float* lGradData = (float*)iconData->derefInt8Array(5 * sizeof(VGfloat), sizeof(quint32));

        COND_COM_OC(drawingMode, ; ,
                    vgSetParameteri(mGradPaintStroke, VG_PAINT_TYPE, VG_PAINT_TYPE_RADIAL_GRADIENT);
                    vgSetParameterfv(mGradPaintStroke, VG_PAINT_RADIAL_GRADIENT, 5, lGradData);
                    vgSeti(VG_MATRIX_MODE, VG_MATRIX_STROKE_PAINT_TO_USER));

        if (lSpecifcData & 0x1) {
            float* lGradMatrix1 = (float*)iconData->derefInt8Array(6 * sizeof(VGfloat),
                4 + 5 * sizeof(VGfloat));
            float lGradMatrix[9] = {lGradMatrix1[0], lGradMatrix1[3], 0.0f,
                                    lGradMatrix1[1], lGradMatrix1[4], 0.0f,
                                    lGradMatrix1[2], lGradMatrix1[5], 1.0f
                                   };

            COND_COM_OC(drawingMode,
                        addStrokeRadialGradientCommand(4, lGradData, lGradMatrix, mGradPaintStroke),
                        vgLoadMatrix(lGradMatrix));
            Q_UNUSED(IdentityMatrix);
        } else {
            COND_COM_OC(drawingMode,
                        addStrokeRadialGradientCommand(4, lGradData, (VGfloat*)IdentityMatrix, mGradPaintStroke),
                        vgLoadIdentity());
        }
        vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
        }
        break;

    case STROKE_COLOR_RAMP: {
        int lStopCount = lSpecifcData;
        float* lStopData = (float*) iconData->derefInt8Array(lStopCount * 5 * sizeof(VGfloat), 4);

        if (mStrokeAlpha == 0xff) {
            vgSetParameteri(mGradPaintStroke, VG_PAINT_COLOR_RAMP_SPREAD_MODE, VG_COLOR_RAMP_SPREAD_PAD);
            vgSetParameterfv(mGradPaintStroke, VG_PAINT_COLOR_RAMP_STOPS, lStopCount*5, lStopData);
        } else {
            VGfloat *crs = new VGfloat[lStopCount * 5];
            Q_CHECK_PTR(crs);
            QScopedArrayPointer<VGfloat> colorRamps(crs);
            // Copy color ramps and modify alpha
            memcpy(colorRamps.data(), lStopData, lStopCount*5*sizeof(VGfloat));
            VGfloat lAlphaInFloat = mStrokeAlpha * 0.003921568627450f; //(1.0f/255.0f);
            VGfloat* lAlphaValue = &colorRamps[4];
            for (int i = 0; i < lStopCount; i++) {
                *lAlphaValue *= lAlphaInFloat;
                lAlphaValue += 5;
            }

            vgSetParameteri(mGradPaintStroke, VG_PAINT_COLOR_RAMP_SPREAD_MODE, VG_COLOR_RAMP_SPREAD_PAD);
            vgSetParameterfv(mGradPaintStroke, VG_PAINT_COLOR_RAMP_STOPS, lStopCount*5, colorRamps.data());
            }
        }
        break;

    default: {
        quint32 lRgba = iconData->derefInt32(NVG_RGBA_OFS);
        lRgba = (lRgba & 0xffffff00) | mStrokeAlpha; // replace alpha

        COND_COM_OC(drawingMode,
                    addStrokeSetColorCommand(lRgba),
                    vgSetParameteri(mPaintStroke, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
                    vgSetColor(mPaintStroke, lRgba));
        }
        break;
    }
}

#ifdef    OPENVG_OBJECT_CACHING
VGPath HbNvgCsIcon::createPath()
{
    VGPath path = VG_INVALID_HANDLE;
    switch (mLastPathDataType) {
    case NvgEightBitEncoding: {
        path = vgCreatePath(VG_PATH_FORMAT_STANDARD,
                            VG_PATH_DATATYPE_S_16, 1.0f / 2.0f, 0.0f, 0, 0,
                            VG_PATH_CAPABILITY_APPEND_TO);
    }
    break;

    case NvgSixteenBitEncoding: {
        path = vgCreatePath(VG_PATH_FORMAT_STANDARD,
                            VG_PATH_DATATYPE_S_16, 1.0f / 16.0f, 0.0f, 0, 0,
                            VG_PATH_CAPABILITY_APPEND_TO);
    }
    break;

    case NvgThirtyTwoBitEncoding: {
        path = vgCreatePath(VG_PATH_FORMAT_STANDARD,
                            VG_PATH_DATATYPE_S_32, 1.0f / 65536.0f, 0.0f, 0, 0,
                            VG_PATH_CAPABILITY_APPEND_TO);
    }
    break;
    default: {

    }
    break;
    }
    return path;
}
#endif

void HbNvgCsIcon::addPathData(VGint numSegments, const VGubyte * pathSegments, const void * pathData)
{
    mNvgIconData->encodeInt32(NvgPathData);
    mNvgIconData->encodeInt32(numSegments);
    mNvgIconData->encodeData(pathSegments, numSegments);

    int coordinateCount = 0;
    for (int i = 0; i < numSegments; ++i) {
        switch (pathSegments[i]) {
        case VG_HLINE_TO:
        case VG_VLINE_TO:
            coordinateCount += 1;
            break;
        case VG_MOVE_TO:
        case VG_LINE_TO:
        case VG_SQUAD_TO:
            coordinateCount += 2;
            break;
        case VG_QUAD_TO:
        case VG_SCUBIC_TO:
            coordinateCount += 4;
            break;
        case VG_SCCWARC_TO:
        case VG_SCWARC_TO:
        case VG_LCCWARC_TO:
        case VG_LCWARC_TO:
            coordinateCount += 5;
            break;
        case VG_CUBIC_TO:
            coordinateCount += 6;
            break;
        default:
            break;
        }
    }
    mNvgIconData->encodeInt16(coordinateCount);
    mNvgIconData->encodeData(pathData, coordinateCount * 4);
}

void HbNvgCsIcon::addDrawPathCommand(VGPath path, VGbitfield paintMode)
{
    mOpenVgHandles->addPath(path);
    mNvgIconData->encodeInt32(NvgPath);
    mNvgIconData->encodeInt32(path);
    mNvgIconData->encodeInt32(paintMode);
}

void HbNvgCsIcon::addLinearGradientCommand(VGint count, VGfloat* gradientData, VGfloat* gradientMatrix, VGPaint paint)
{
    mOpenVgHandles->addPaint(paint);
    mNvgIconData->encodeInt32(NvgPaint);
    addLinearGradientCommandData(paint, count, gradientData, gradientMatrix);
}

void HbNvgCsIcon::addRadialGradientCommand(VGint count, VGfloat* gradientData, VGfloat* gradientMatrix, VGPaint paint)
{
    mOpenVgHandles->addPaint(paint);
    mNvgIconData->encodeInt32(NvgPaint);
    addRadialGradientCommandData(paint, count, gradientData, gradientMatrix);
}

void HbNvgCsIcon::addSetColorCommand(VGuint rgba)
{
    mNvgIconData->encodeInt32(NvgPaint);
    mNvgIconData->encodeInt32(VG_PAINT_TYPE_COLOR);
    mNvgIconData->encodeInt32(rgba);
}

void HbNvgCsIcon::addColorRampCommand(VGPaint paint)
{
    mNvgIconData->encodeInt32(NvgColorRamp);
    mNvgIconData->encodeInt32(paint);
}

void HbNvgCsIcon::addSetTransformCommand(const VGfloat* transformMatrix, int aFlag)
{
    mNvgIconData->encodeInt32(NvgTransform);
    mNvgIconData->encodeData(transformMatrix, 9 * sizeof(VGfloat));
    mNvgIconData->encodeInt32(aFlag);
}

void HbNvgCsIcon::addSetStrokeWidthCommand(VGfloat strokeWidth)
{
    mNvgIconData->encodeInt32(NvgStrokeWidth);
    mNvgIconData->encodeReal32(strokeWidth);
}

void HbNvgCsIcon::addSetStrokeMiterLimitCommand(VGfloat miterLimit)
{
    mNvgIconData->encodeInt32(NvgStrokeMiterLimit);
    mNvgIconData->encodeReal32(miterLimit);
}

void HbNvgCsIcon::addStrokeLineJoinCapCommand(VGint capStyle, VGint joinStyle)
{
    mNvgIconData->encodeInt32(NvgStrokeLineJoinCap);
    mNvgIconData->encodeInt32(capStyle);
    mNvgIconData->encodeInt32(joinStyle);
}

void HbNvgCsIcon::addStrokeLinearGradientCommand(VGint count, VGfloat* gradientData, VGfloat* gradientMatrix, VGPaint paint)
{
    mOpenVgHandles->addPaint(paint);
    mNvgIconData->encodeInt32(NvgStrokePaint);
    addLinearGradientCommandData(paint, count, gradientData, gradientMatrix);
}

void HbNvgCsIcon::addStrokeRadialGradientCommand(VGint count, VGfloat* gradientData, VGfloat* gradientMatrix, VGPaint paint)
{
    mOpenVgHandles->addPaint(paint);
    mNvgIconData->encodeInt32(NvgStrokePaint);
    addRadialGradientCommandData(paint, count, gradientData, gradientMatrix);
}

void HbNvgCsIcon::addStrokeSetColorCommand(VGuint rgba)
{
    mNvgIconData->encodeInt32(NvgStrokePaint);
    addSetColorCommandData(rgba);
}

void HbNvgCsIcon::addStrokeColorRampCommand(VGPaint paint)
{
    mNvgIconData->encodeInt32(NvgStrokeColorRamp);
    mNvgIconData->encodeInt32(paint);
}

void HbNvgCsIcon::addLinearGradientCommandData(VGPaint paint, VGint count, VGfloat* gradientData, VGfloat* gradientMatrix)
{
    mNvgIconData->encodeInt32(VG_PAINT_TYPE_LINEAR_GRADIENT);
    mNvgIconData->encodeInt32(paint);
    mNvgIconData->encodeInt32(count);
    mNvgIconData->encodeData(gradientData, count * sizeof(VGfloat));
    mNvgIconData->encodeData(gradientMatrix, 9 * sizeof(VGfloat));
}

void HbNvgCsIcon::addRadialGradientCommandData(VGPaint paint, VGint count, VGfloat* gradientData, VGfloat* gradientMatrix)
{
    mNvgIconData->encodeInt32(VG_PAINT_TYPE_RADIAL_GRADIENT);
    mNvgIconData->encodeInt32(paint);
    mNvgIconData->encodeInt32(count);
    mNvgIconData->encodeData(gradientData, count * sizeof(VGfloat));
    mNvgIconData->encodeData(gradientMatrix, 9 * sizeof(VGfloat));
}

void HbNvgCsIcon::addSetColorCommandData(VGuint rgba)
{
    mNvgIconData->encodeInt32(VG_PAINT_TYPE_COLOR);
    mNvgIconData->encodeInt32(rgba);
}

HbNvgEngine::NvgErrorType HbNvgCsIcon::draw(const QSize &size)
{
    NVG_DEBUGP2("DRAWING NVGCSIcon %s, ", __FUNCTION__);

    HbNvgEngine::NvgErrorType error = HbNvgEngine::NvgErrNone;

    // Get Matrix modes and all caller matrices (must be restored afterwards)
    updateClientMatrices();

    //Exception handling has to happen
    error =  doDraw(size);

    // restore everything as we may have changed matrix mode
    restoreClientMatrices();

    return error;
}

HbNvgEngine::NvgErrorType HbNvgCsIcon::doDraw(const QSize &size)
{
    HbNvgEngine::NvgErrorType ret = HbNvgEngine::NvgErrNone;

    vgSetPaint(mPaintFill,   VG_FILL_PATH);
    vgSetPaint(mPaintStroke, VG_STROKE_PATH);
    mLastFillPaintColor     = 0;
    mLastStrkePaintColor    = 0;
    mLastFillPaintType      = 0;
    mLastStrokePaintType    = 0;

    VGfloat lCurrentPathMatrix[9];
    vgGetMatrix(lCurrentPathMatrix);

    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgLoadMatrix(lCurrentPathMatrix);
    setRotation();
#ifdef __MIRROR_  
    vgScale(1.0f, -1.0f);
    vgTranslate(0, (VGfloat)(-size.height()));
#endif

    setViewBoxToViewTransformation(size);

    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);

    VGfloat currentMatrix[9];

    vgGetMatrix(currentMatrix);
    
    mNvgIconData->beginRead();

    while (!mNvgIconData->eof()) {
        switch (mNvgIconData->readInt32()) {
        case NvgPath: {
            VGPath path = (VGPath)mNvgIconData->readInt32();
            VGPaintMode paintMode = (VGPaintMode)mNvgIconData->readInt32();

            if (path == VG_INVALID_HANDLE) {
                vgDrawPath(mVgPath, paintMode);
            } else {
                vgDrawPath(path, paintMode);
            }
            break;
        }
        case NvgPathData: {
            if (mVgPath != VG_INVALID_HANDLE) {
            
                VGint numSegments = mNvgIconData->readInt32();
                
                VGubyte *pSegArry = new VGubyte[numSegments];
                Q_CHECK_PTR(pSegArry);
                QScopedArrayPointer<VGubyte> pathSegments(pSegArry);
                mNvgIconData->read(pathSegments.data(), numSegments);
                
                VGint coordinateCount = mNvgIconData->readInt32();
                
                VGubyte *pDataArry = new VGubyte[coordinateCount * 4];
                Q_CHECK_PTR(pDataArry);
                QScopedArrayPointer<VGubyte> pathData(pDataArry);
                mNvgIconData->read(pathData.data(), coordinateCount * 4);
                
                vgClearPath(mVgPath, VG_PATH_CAPABILITY_APPEND_TO);
                vgAppendPathData(mVgPath, numSegments, pathSegments.data(), pathData.data());
            }
            break;
            }
        case NvgPaint: {
            drawPaint(mPaintFill, VG_MATRIX_FILL_PAINT_TO_USER, mLastFillPaintType, mLastFillPaintColor, VG_FILL_PATH);
            break;
            }
        case NvgColorRamp: {
            mNvgIconData->readInt32();
            break;
            }
        case NvgTransform: {
            int flag;
            VGfloat transformMatrix[9];

            mNvgIconData->read((quint8 *)transformMatrix, 9 * sizeof(VGfloat));
            flag = mNvgIconData->readInt32();

            vgLoadMatrix(currentMatrix);
            if (flag) {
                vgMultMatrix(transformMatrix);
            }
            
            break;
            }
        case NvgStrokeWidth: {
            VGfloat strokeWidth = mNvgIconData->readReal32();
            vgSetf(VG_STROKE_LINE_WIDTH, strokeWidth);
            break;
            }
        
        case NvgStrokeMiterLimit: {
            VGfloat miterLimit = mNvgIconData->readReal32();
            vgSetf(VG_STROKE_MITER_LIMIT, miterLimit);
            break;
            }
        
        case NvgStrokeLineJoinCap: {
            VGint lineJoin = mNvgIconData->readInt32();
            VGint cap = mNvgIconData->readInt32();

            vgSeti(VG_STROKE_JOIN_STYLE, (VGJoinStyle)lineJoin);
            vgSeti(VG_STROKE_CAP_STYLE, (VGCapStyle)cap);
            break;
            }
        case NvgStrokePaint: {
            drawPaint(mPaintStroke, VG_MATRIX_STROKE_PAINT_TO_USER, mLastStrokePaintType, mLastStrkePaintColor, VG_STROKE_PATH);
            break;
            }
        case NvgStrokeColorRamp: {
            mNvgIconData->readInt32();
            break;
            }
        default: {  
            throw HbNvgException(HbNvgEngine::NvgErrCorrupt);
            }
        }
    }

    mNvgIconData->endRead();

    return ret;
}

void HbNvgCsIcon::drawColorRamp(VGPaint paint)
{
    int stopCount = mNvgIconData->readInt32();
    
    VGfloat *crs = new VGfloat[stopCount];
    Q_CHECK_PTR(crs);
    QScopedArrayPointer<VGfloat> colorRamps(crs);

    mNvgIconData->read((quint8 *)colorRamps.data(), stopCount * sizeof(VGfloat));
    vgSetParameteri(paint, VG_PAINT_COLOR_RAMP_SPREAD_MODE, VG_COLOR_RAMP_SPREAD_PAD);
    vgSetParameterfv(paint, VG_PAINT_COLOR_RAMP_STOPS, stopCount, colorRamps.data());
}

void HbNvgCsIcon::drawPaint(VGPaint paint, VGMatrixMode matrixMode, quint32 & lastPaintType, quint32 &lastPaintColor, VGPaintMode paintMode)
{
    VGPaintType paintType = (VGPaintType)mNvgIconData->readInt32();

    if (paintType == VG_PAINT_TYPE_LINEAR_GRADIENT ||
            paintType == VG_PAINT_TYPE_RADIAL_GRADIENT) {
        VGPaintParamType paintPType = VG_PAINT_LINEAR_GRADIENT;
        if (paintType == VG_PAINT_TYPE_RADIAL_GRADIENT) {
            paintPType = VG_PAINT_RADIAL_GRADIENT;
        }

        VGPaint paintHandle = mNvgIconData->readInt32();
        int count = mNvgIconData->readInt32();
        VGfloat gradientData[5];
        VGfloat gradientMatrix[9];

        mNvgIconData->read((quint8 *)gradientData, count * sizeof(VGfloat));
        mNvgIconData->read((quint8 *)gradientMatrix, 9 * sizeof(VGfloat));

        if (paintHandle) {
            vgSetPaint(paintHandle,   paintMode);
            vgSeti(VG_MATRIX_MODE, matrixMode);
            vgLoadMatrix(gradientMatrix);
            if (paintMode == VG_FILL_PATH) {
                mResetFillPaint = 1;
            } else {
                mResetStrokePaint = 1;
            }
        } else {
            if (lastPaintType != (quint32)paintType) {
                vgSetParameteri(paint, VG_PAINT_TYPE, paintType);
            }
            vgSetParameterfv(paint, paintPType, count, gradientData);

            vgSeti(VG_MATRIX_MODE, matrixMode);
            vgLoadMatrix(gradientMatrix);
        }
        vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    } else if (paintType == VG_PAINT_TYPE_COLOR) {
        if (paintMode == VG_FILL_PATH && mResetFillPaint) {
            mResetFillPaint = 0;
            vgSetPaint(paint, paintMode);
        } else if (paintMode == VG_STROKE_PATH && mResetStrokePaint) {
            mResetStrokePaint = 0;
            vgSetPaint(paint, paintMode);
        }
        quint32 color = static_cast<quint32>(mNvgIconData->readInt32());
        if (lastPaintType != (quint32)paintType) {
            vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
            vgSetColor(paint, color);
        } else {
            if (lastPaintColor != color) {
                vgSetColor(paint, color);
            }
        }
        lastPaintColor = color;
    } else {
        throw HbNvgException(HbNvgEngine::NvgErrCorrupt);
    }
    lastPaintType = paintType;
}

void HbNvgCsIcon::setViewBoxToViewTransformation(const QSize &size)
{
    HbNvgFitToViewBoxImpl *viewBoxTx = new HbNvgFitToViewBoxImpl();
    Q_CHECK_PTR(viewBoxTx);
    QScopedPointer<HbNvgFitToViewBoxImpl> fitToViewBoxImpl(viewBoxTx);

    fitToViewBoxImpl->setAlign(mPreserveAspectSetting);
    fitToViewBoxImpl->setScaling(mSmilFitSetting);
    fitToViewBoxImpl->setViewBox(mViewBoxX, mViewBoxY, mViewBoxW, mViewBoxH);
    fitToViewBoxImpl->setWindowViewportTrans(QRect(0, 0, size.width(), size.height()), QSize(0, 0));
}

void HbNvgCsIcon::setRotation()
{
    if (mRotationAngle) {    
        vgTranslate(mRotationX, mRotationY);             
        vgRotate(mRotationAngle);
        vgTranslate(-mRotationX, -mRotationY);
    }
}

void HbNvgCsIcon::updateClientMatrices()
{
    mMatrixMode = vgGeti(VG_MATRIX_MODE);
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgGetMatrix(mPathMatrix);
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
    vgGetMatrix(mImageMatrix);
    vgSeti(VG_MATRIX_MODE, mMatrixMode);
}

void HbNvgCsIcon::restoreClientMatrices()
{
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgLoadMatrix(mPathMatrix);
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
    vgLoadMatrix(mImageMatrix);
    vgSeti(VG_MATRIX_MODE, mMatrixMode);
}

QPoint HbNvgCsIcon::getTranslatedPoint(VGfloat *trMatrix, const QPoint &point)
{
    QPoint trPoint;

    trPoint.setX(trMatrix[0] * point.x() + trMatrix[3] * point.y() + trMatrix[6]);
    trPoint.setY(trMatrix[1] * point.x() + trMatrix[4] * point.y() + trMatrix[7]);

    return trPoint;
}

VGfloat HbNvgCsIcon::minVal4(VGfloat x1, VGfloat x2, VGfloat x3, VGfloat x4)
{
    VGfloat min = x1;

    if (min > x2) {
        min = x2;
    }
    if (min > x3) {
        min = x3;
    }
    if (min > x4) {
        min = x4;
    }

    return min;
}

VGfloat HbNvgCsIcon::maxVal4(VGfloat x1, VGfloat x2, VGfloat x3, VGfloat x4)
{
    VGfloat max = x1;

    if (max < x2) {
        max = x2;
    }
    if (max < x3) {
        max = x3;
    }
    if (max < x4) {
        max = x4;
    }

    return max;
}

