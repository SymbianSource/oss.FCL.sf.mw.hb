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
#include <QFile>
#include <QTextStream>
#include <gdi.h>
#include <SVGRendererId.h>
#include <hbtlvwrapper_p.h>
#include "hbtlvwrapper_p_p.h"

/*!
    @proto
    @hbcore
    \class HbTlvWrapper

    \brief HbTlvWrapper provides the NVG-TLV data when run on s60 baseport.
    The TLV data is constructed using the TLV Encoder in s60 SVGEngine.
    HbTlvWrapper is a singleton class. The NVG-TLV is used in the scenarios where the svg
    file can not be converted to nvg, e.g.: svg file containging group-opacity. Since the
    TLV is vector data the rendering would be faster using the sgimage when compared to
    rendering via QPixmap or QPicture.

*/

/*!
    \fn QByteArray HbTlvWrapper::getTLVEncodedData(QString& filename,QSizeF& size, Qt::AspectRatioMode aspectRatioM)

    This function returns the NVG-TLV data in the form of a bytearray. it accepts the
    svg filename, rendersize, aspect ratio as a parameter. This data later can be rendered using the HbNvgEngine's
    drawNvg().
*/

/*!
    \fn QByteArray HbTlvWrapper::getTLVEncodedDataFromDom(quint32 domhandle,QSizeF& size, Qt::AspectRatioMode aspectRatioM)

    This function returns the NVG-TLV data in the form of a bytearray. it accepts the
    handle to dom prepared previously by calling contentDimensions(). Also takes in
    rendersize, aspect ratio as a parameter. This data later can be rendered using the HbNvgEngine's
    drawNvg().
*/
const QString unicodeFormat("fffe");

HbTlvWrapper::HbTlvWrapper(): d_ptr(new HbTlvWrapperPrivate)
{
}

HbTlvWrapper::~HbTlvWrapper()
{
}

HbTlvWrapper *HbTlvWrapper::instance()
{
    static HbTlvWrapper instance;
    return &instance;
}

const QByteArray HbTlvWrapper::getTlvEncodedData(const QString& filename, const QSizeF& size, Qt::AspectRatioMode aspectRatioMode)
{
    return(d_ptr->getTlvEncodedData(filename, size, aspectRatioMode));
}

const QByteArray HbTlvWrapper::getTlvEncodedDataFromDom(const quint32& domhandle, QSizeF& size, Qt::AspectRatioMode aspectRatioMode)
{
    return (d_ptr->getTlvEncodedDataFromDom(domhandle, size, aspectRatioMode));
}

const QSize HbTlvWrapper::contentDimensions(QString& filename, quint32& domhandle)
{
    return(d_ptr->contentDimensions(filename, domhandle));
}

HbTlvWrapperPrivate::HbTlvWrapperPrivate()
{
    // Dummy bitmap for  s60 engine's constructl.
    bitmap = new CFbsBitmap;
    TFontSpec spec;
    TRAPD(err, s60SvgEngine = CSvgEngineInterfaceImpl::NewL(bitmap, 0, spec, ESVGRendererTLV));
}
HbTlvWrapperPrivate::~HbTlvWrapperPrivate()
{
    delete bitmap;
    bitmap = 0;
    
    delete s60SvgEngine;
    s60SvgEngine = 0;
}

const QByteArray HbTlvWrapperPrivate::getTlvEncodedData(const QString& filename, const QSizeF& size, Qt::AspectRatioMode aspectRatioMode)
{
#ifdef __TLV_
    TInt domhandle;
    QFile file(filename);
    if (!file.open(QIODevice::NotOpen | QIODevice::ReadOnly)) {
        return QByteArray();
    }
    
    QByteArray isUniCoded = file.read(2).toHex();
    file.seek(0);
    QByteArray byteArray;
    if (isUniCoded == unicodeFormat) {
        QTextStream in(&file);
        QString data = in.readAll();
        byteArray = data.toUtf8();
    } else {
        byteArray = file.readAll();
    }
    
    file.close();
        
    TPtrC8 fileDataPtr8((TUint8 *)(byteArray.constData()));
    s60SvgEngine->PrepareDom(fileDataPtr8, domhandle);

    TSize bitmapsize(size.width(), size.height());
    s60SvgEngine->UseDom(domhandle, bitmap, 0, bitmapsize, ENone, ENone);
    // Aspect ratio setting
    TSvgPreserveAspectAlignType preserveAspectSetting = ESvgPreserveAspectRatio_XmidYmid;
    TSvgMeetOrSliceType smilFitSetting = ESvgMeetOrSlice_Meet;;
    switch (aspectRatioMode) {

    case Qt::IgnoreAspectRatio: {
        preserveAspectSetting = ESvgPreserveAspectRatio_None;
        break;
    }
    case Qt::KeepAspectRatio: {
        break;
    }
    case Qt::KeepAspectRatioByExpanding: {
        smilFitSetting = ESvgMeetOrSlice_Slice;
        break;
    }
    default: {
        break;
    }
    }
    s60SvgEngine->SetPreserveAspectRatio((CSvgDocumentImpl*)domhandle, preserveAspectSetting, smilFitSetting, ETrue);
    s60SvgEngine->Start();
    const TPtrC8 des = s60SvgEngine->TLVEncodedData();

    s60SvgEngine->DeleteDom(domhandle);

    return (QByteArray((const char *)des.Ptr(), des.Length()));
#else
    return QByteArray();
#endif
}

const QByteArray HbTlvWrapperPrivate::getTlvEncodedDataFromDom(const quint32& domhandle, QSizeF& size, Qt::AspectRatioMode aspectRatioMode)
{
#ifdef __TLV_
    TSize bitmapsize(size.width(), size.height());
    s60SvgEngine->UseDom(domhandle, bitmap, 0, bitmapsize, ENone, ENone);
    // Aspect ratio setting
    TSvgPreserveAspectAlignType preserveAspectSetting = ESvgPreserveAspectRatio_XmidYmid;
    TSvgMeetOrSliceType smilFitSetting = ESvgMeetOrSlice_Meet;
    switch (aspectRatioMode) {

    case Qt::IgnoreAspectRatio: {
        preserveAspectSetting = ESvgPreserveAspectRatio_None;
        break;
    }
    case Qt::KeepAspectRatio: {
        break;
    }
    case Qt::KeepAspectRatioByExpanding: {
        smilFitSetting = ESvgMeetOrSlice_Slice;
        break;
    }
    default: {
        break;
    }
    }
    s60SvgEngine->SetPreserveAspectRatio((CSvgDocumentImpl*)domhandle, preserveAspectSetting, smilFitSetting, EFalse);
    s60SvgEngine->Start();
    const TPtrC8 des = s60SvgEngine->TLVEncodedData();

    s60SvgEngine->DeleteDom(domhandle);
    return (QByteArray((const char *)des.Ptr(), des.Length()));
#else
    return QByteArray();
#endif
}
const QSize HbTlvWrapperPrivate::contentDimensions(QString& filename, quint32& domhandle)
{
#ifdef __TLV_
    TInt handle;
    QFile file(filename);
    if (!file.open(QIODevice::NotOpen | QIODevice::ReadOnly)) {
        return QSize();
    }
    
    QByteArray isUniCoded = file.read(2).toHex();
    file.seek(0);
    QByteArray byteArray;
    if (isUniCoded == unicodeFormat) {
        QTextStream in(&file);
        QString data = in.readAll();
        byteArray = data.toUtf8();
    } else {
        byteArray = file.readAll();
    }
    
    file.close();
    
    TPtrC8 ptr8((TUint8 *)(byteArray.constData()));
    s60SvgEngine->PrepareDom(ptr8, handle);
    TSize dummysize;
    s60SvgEngine->UseDom(handle, bitmap, 0, dummysize, ENone, ENone);
    s60SvgEngine->ChooseViewBoxIfNotSet(handle);
    contentsize = s60SvgEngine->ContentDimensions();
    domhandle = handle;
   
    return(QSize(contentsize.iWidth, contentsize.iHeight));
#else
    return QSize();
#endif
}

