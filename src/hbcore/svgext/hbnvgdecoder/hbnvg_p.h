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
#ifndef HB_NVG_P_H
#define HB_NVG_P_H

#include <hbglobal.h>

#include <VG/openvg.h>
#include <VG/vgu.h>

#include <QtGlobal>
#include <QSize>
#include <QColor>

QT_BEGIN_NAMESPACE
class QByteArray;
QT_END_NAMESPACE

class HbNvgIcon;
class HbVgImageBinder;
class HbNvgEnginePrivate;

class HB_CORE_PRIVATE_EXPORT HbNvgEngine
{
public :

    enum HbNvgAlignType {
        NvgPreserveAspectRatioNone,
        NvgPreserveAspectRatioXminYmin,
        NvgPreserveAspectRatioXmidYmin,
        NvgPreserveAspectRatioXmaxYmin,
        NvgPreserveAspectRatioXminYmid,
        NvgPreserveAspectRatioXmidYmid,
        NvgPreserveAspectRatioXmaxYmid,
        NvgPreserveAspectRatioXminYmax,
        NvgPreserveAspectRatioXmidYmax,
        NvgPreserveAspectRatioXmaxYmax
    };

    enum HbNvgMeetType  {
        NvgMeet,
        NvgSlice
    };

    enum HbNvgErrorType {
        NvgErrEof = -10,
        NvgErrUnknown = -9,
        NvgErrPermissionDenied = -8,
        NvgErrInUse = -7,
        NvgErrAlreadyExists = -6,
        NvgErrArgument = -5,
        NvgErrNoMemory = -4,
        NvgErrCorrupt = -3,
        NvgErrNotSupported = -2,
        NvgErrBadHandle = -1,
        NvgErrNone = 0
    };

    HbNvgEngine();

    virtual ~HbNvgEngine();

    void rotate(float angle, float xval, float yval);

    void setPreserveAspectRatio(HbNvgAlignType preserveAspectSetting,
                                HbNvgMeetType smilFitSetting);

    QSize contentDimensions(const QByteArray &buffer)const;

    HbNvgIcon * createNvgIcon(const QByteArray &buffer, const QSize &size);

    HbNvgErrorType drawNvg(const QByteArray &buffer, const QSize &size);

    void setVgImageBinder(HbVgImageBinder *imageBinder);

    HbNvgErrorType error() const;

    void setBackgroundColor(const QColor &rgba8888Color);

    void clearBackground();

    void enableMirroring(bool mirroringMode);

private :
    HbNvgEnginePrivate * const d_ptr;

private:
    Q_DISABLE_COPY(HbNvgEngine)
    Q_DECLARE_PRIVATE_D(d_ptr, HbNvgEngine)
};

HbNvgEngine::HbNvgErrorType openVgErrorToHbNvgError(qint32 error);

#endif


