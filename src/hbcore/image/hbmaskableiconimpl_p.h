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

#ifndef HB_MASKABLEICONIMPL_H
#define HB_MASKABLEICONIMPL_H

#include <QBitmap>
#include <QPixmap>
#include <QPainter>
#include <QColor>
#include "hbiconimpl_p.h"

class HB_AUTOTEST_EXPORT HbMaskableIconImpl
{
public:
    HbMaskableIconImpl(HbIconImpl *icon) {
        this->icon = icon;
        data = 0;
        maskApplied = false;
    }

    QPixmap pixmap() {
        return icon->pixmap();
    }

    void paint(QPainter *painter, const QRectF &childRect,
               Qt::Alignment alignment,
               const QPainterPath &clipPath = QPainterPath()) {
        icon->paint(painter, childRect, alignment, clipPath, this);
        maskApplied = false;
    }

    HbIconImpl *iconImpl() {
        return icon;
    }

    bool maskChanged() {
        return maskApplied;
    }

    void setPixmap(QPixmap pixmap) {
        maskBitmap = pixmap;
    }

    QSize defaultSize() const {
        return icon->defaultSize();
    }

    void setMask(const QBitmap &mask) {
        if (!mask.isNull()) {
            maskBitmap = mask;
            maskApplied = true;
        }
    }

    QBitmap mask() {
        return maskBitmap;
    }

    QSize size() {
        return icon->size();
    }

    void setColor(const QColor &color) {
        icon->setColor(color);
    }

    QColor color() {
        return icon->color();
    }

    HbSharedIconInfo iconData() const {
        return icon->iconData();
    }

    QString iconFileName() const {
        return icon->iconFileName();
    }

    QSizeF keySize() const {
        return icon->keySize();
    }

    Qt::AspectRatioMode iconAspectRatioMode() const {
        return icon->iconAspectRatioMode();
    }

    QIcon::Mode iconMode() const {
        return icon->iconMode();
    }

    bool isMirrored() const {
        return icon->isMirrored();
    }

    bool isCreatedOnServer() const {
        return icon->isCreatedOnServer();
    }

    void incrementRefCount() {
        icon->incrementRefCount();
    }

    uint refCount() {
        return icon->refCount();
    }

    void decrementRefCount() {
        icon->decrementRefCount();
    }

    void dispose() {
        icon->destroyMaskedData(data);
        icon->dispose();
        delete this;
    }

    void setImplData(HbIconMaskedData *data) {
        this->data = data;
    }

    HbIconMaskedData *implData() const {
        return data;
    }

private:
    ~HbMaskableIconImpl() {
    }

    HbIconImpl        *icon;
    HbIconMaskedData  *data;
    QBitmap            maskBitmap;
    bool               maskApplied;
};

#endif

