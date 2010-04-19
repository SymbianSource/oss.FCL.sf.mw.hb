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
#ifndef HB_NVG_P_P_H
#define HB_NVG_P_P_H

#include "hbnvg_p.h"
#include "hbnvgiconfactory_p.h"

#include <VG/openvg.h>
#include <VG/vgu.h>

class HbNvgIcon;
class HbVgImageBinder;

class NVGIconList
{
public:
    NVGIconList()
    {
        icons[HbNvgIconFactory::NVGCS] = 0;
        icons[HbNvgIconFactory::NVGTLV] = 0;
    }

    void addNVGIcon(HbNvgIconFactory::HbNvgIconType type, HbNvgIcon * nvgICon);
    
    HbNvgIcon * getIcon(HbNvgIconFactory::HbNvgIconType type);
    
    ~NVGIconList();
    
private:
    HbNvgIcon * icons[HbNvgIconFactory::NVGTLV + 1];
};

class HbNvgEnginePrivate
{
public :
    HbNvgEnginePrivate();

    ~HbNvgEnginePrivate();

    void rotate(float angle, float xval, float yval) ;
    
    void setPreserveAspectRatio(HbNvgEngine::NvgAlignStatusType preserveAspectSetting,
            HbNvgEngine::NvgMeetOrSliceType smilFitSetting);

    QSize contentDimensions(const QByteArray &buffer) const;

    HbNvgEngine::NvgErrorType drawNvg(const QByteArray &buffer, const QSize &size);

    HbNvgIcon * createNvgIcon(const QByteArray &buffer, const QSize &size);

    void setVgImageBinder(HbVgImageBinder *imageBinder) 
    {
        mVgImageBinder = imageBinder;
    }

    HbNvgEngine::NvgErrorType error()const 
    {
        return mLastError;
    }
    
    void setMirroringMode(bool mirroringMode )
    {
        mMirroringMode = mirroringMode;
    }
    
    void setBackgroundColor(const QColor &rgba8888Color);
      
    void clearBackground();

private :
    void doDrawNvg(const QByteArray &buffer, const QSize &size);

    int drawTlv(const QByteArray &buffer, const QSize &targetSize);

    int drawCsIcon(const QByteArray &buffer, const QSize &targetSize);

    void updateClientMatrices();
    
    void restoreClientMatrices();

private :

    QSize mCurrentBufferSize;
    
    VGfloat mRotateAngle;
    float   mCentreX;
    float   mCentreY;   
    
    HbNvgEngine::NvgAlignStatusType  mPreserveAspectSetting;
    HbNvgEngine::NvgMeetOrSliceType  mSmilFitSetting;
    
    QColor              mBackgroundColor;
    HbVgImageBinder *   mVgImageBinder;
    
    bool         mCreatingNvgIcon;
    HbNvgIcon * mCurrentNvgIcon;
    
    HbNvgEngine::NvgErrorType mLastError;
    
    bool        mMirroringMode;
    VGint       mMatrixMode;
    VGfloat     mImageMatrix[9];
    VGfloat     mPathMatrix[9];
    NVGIconList mIconList;
};

#endif
//-------------------------------End Of File-------------------------
