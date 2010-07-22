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

#ifndef HBDEVICEPROFILE_H
#define HBDEVICEPROFILE_H

#include <hbglobal.h>
#include <QtGlobal>
#include <QSizeF>
#include <QSize>
#include <QStringList>
#include <QSharedDataPointer>

class HbMainWindow;
class HbTheme;
class HbDeviceProfilePrivate;

QT_BEGIN_NAMESPACE
class QGraphicsItem;
QT_END_NAMESPACE

class HB_CORE_EXPORT HbDeviceProfile
{
public:
    HbDeviceProfile();
    HbDeviceProfile(const HbDeviceProfile &other);
    explicit HbDeviceProfile(const QString &name);
    ~HbDeviceProfile();

    HbDeviceProfile &operator=(const HbDeviceProfile &other);

    bool isNull() const;

    QString name() const;
    QSize logicalSize() const;
    QSizeF physicalSize() const;
    QString alternateProfileName() const;
    bool touch() const;

    Qt::Orientation orientation() const;
    qreal orientationAngle() const;
    qreal unitValue() const;
    qreal ppmValue() const;

    static HbDeviceProfile current();
    static HbDeviceProfile profile(const HbMainWindow *window);
    static HbDeviceProfile profile(const QGraphicsItem *item);

    static QStringList profileNames();
protected:
    QSharedDataPointer<HbDeviceProfilePrivate> d_ptr;
};

#endif // HBDEVICEPROFILE_H
