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

#ifndef HBFRAMEBACKGROUND_H
#define HBFRAMEBACKGROUND_H

#include <hbframedrawer.h>
#include <QExplicitlySharedDataPointer>

class HbFrameBackgroundPrivate;

QT_BEGIN_NAMESPACE
class QString;
class QPainter;
QT_END_NAMESPACE

class HB_CORE_EXPORT HbFrameBackground
{
public:

    HbFrameBackground();
    HbFrameBackground(const QString &frameGraphicsName, HbFrameDrawer::FrameType type);
    ~HbFrameBackground();

    HbFrameBackground(const HbFrameBackground &other);
    HbFrameBackground &operator=(const HbFrameBackground &other);

    bool operator==(const HbFrameBackground &other) const;
    bool operator!=(const HbFrameBackground &other) const;

    bool isNull() const;

    QString frameGraphicsName() const;
    void setFrameGraphicsName(const QString &frameGraphicsName);

    HbFrameDrawer::FrameType frameType() const;
    void setFrameType(HbFrameDrawer::FrameType type);

    operator QVariant() const;

private:

    QExplicitlySharedDataPointer<HbFrameBackgroundPrivate> d;
};

Q_DECLARE_METATYPE(HbFrameBackground)

#endif // HBFRAMEBACKGROUND_H
