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

#ifndef HBFRAMEDRAWER_H
#define HBFRAMEDRAWER_H

#include <hbglobal.h>
#include <hbicon.h>

#include <QRectF>

class HbFrameDrawerPrivate;

QT_BEGIN_NAMESPACE
class QString;
class QPainter;
class QGraphicsItem;
QT_END_NAMESPACE

class HB_CORE_EXPORT HbFrameDrawer
{
public:

    enum FrameType {
        Undefined = 0,
        OnePiece,
        ThreePiecesHorizontal,
        ThreePiecesVertical,
        NinePieces
    };

    HbFrameDrawer(bool cacheFlag = true);
    HbFrameDrawer(const QString &frameGraphicsName, FrameType type, bool cacheFlag = true);

    ~HbFrameDrawer();

    bool isNull() const;

    QString frameGraphicsName() const;
    void setFrameGraphicsName(const QString &frameGraphicsName);

    FrameType frameType() const;
    void setFrameType(FrameType type);

    void borderWidths(qreal &left, qreal &top, qreal &right, qreal &bottom) const;
    void setBorderWidths(const qreal left, const qreal top, const qreal right, const qreal bottom);
    void setBorderWidths(const qreal horizontal, const qreal vertical);
    void setBorderWidth(const qreal width);

    bool fillWholeRect() const;
    void setFillWholeRect(bool fill);

    HbIcon::MirroringMode mirroringMode() const;
    void setMirroringMode(HbIcon::MirroringMode mode);

    QStringList fileNameSuffixList() const;
    void setFileNameSuffixList(const QStringList &list);

    void setMask(const QPixmap &mask);
    void setMask(const QBitmap &mask);
    QPixmap mask() const;
    QBitmap maskBitmap() const;

    void setClipPath(const QPainterPath &clipPath);
    QPainterPath clipPath() const;

    QSize frameSize() const;

    void themeChanged();
    void setLayoutDirection(Qt::LayoutDirection direction);

    void setGraphicsItem(QGraphicsItem *item);

    void paint(QPainter *painter, const QRectF &rect) const;

protected:
    QRectF rect() const;
    void setRect(const QRectF &rect);

private:
    Q_DISABLE_COPY(HbFrameDrawer)
    HbFrameDrawerPrivate *d;

    friend class HbStylePrivate;
};

#endif // HBFRAMEDRAWER_H

