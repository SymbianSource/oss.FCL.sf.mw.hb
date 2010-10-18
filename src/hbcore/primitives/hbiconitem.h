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

#ifndef HBICONITEM_H
#define HBICONITEM_H

#include <hbglobal.h>
#include <hbwidgetbase.h>
#include <hbicon.h>
#include <QBrush>

class HbIconItemPrivate;
class HbIconAnimator;

class HB_CORE_EXPORT HbIconItem: public HbWidgetBase
{
    Q_OBJECT

public:
    explicit HbIconItem(const QString &iconName, QGraphicsItem *parent = 0);
    explicit HbIconItem(const HbIcon &icon, QGraphicsItem *parent = 0);
    explicit HbIconItem(QGraphicsItem *parent = 0);
    virtual ~HbIconItem();

    void setIcon(const HbIcon &icon, bool reserved = false);
    void setSize(const QSizeF &size);
    void setAspectRatioMode(Qt::AspectRatioMode mode);
    void setAlignment(Qt::Alignment alignment);
    void setMode(QIcon::Mode mode);
    void setState(QIcon::State state);
    void setIconName(const QString &iconName);
    void setIconName(const QString &iconName, QIcon::Mode mode, QIcon::State state);
    void setFlags(HbIcon::Flags flags);
    void setMirroringMode(HbIcon::MirroringMode mode);
    void setBrush(const QBrush &brush);
    void setIconScaling(bool enabled = true);
    void setAsync(bool async = true);

    void setColor(const QColor &color);
    QColor color() const;
    HbIcon icon() const;
    QSizeF defaultSize() const;
    QSizeF iconItemSize() const;
    Qt::AspectRatioMode aspectRatioMode() const;
    Qt::Alignment alignment() const;
    QIcon::Mode mode() const;
    QIcon::State state() const;
    QString iconName() const;
    QString iconName(QIcon::Mode mode, QIcon::State state) const;
    HbIcon::Flags flags() const;
    HbIcon::MirroringMode mirroringMode() const;
    QBrush brush() const;
    bool iconScaling() const;
    bool async() const;

    bool isNull() const;

    static const QIcon::Mode defaultMode;
    static const QIcon::State defaultState;
    static const Qt::AspectRatioMode defaultAspectRatioMode;
    static const Qt::Alignment defaultAlignment;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    QRectF boundingRect() const;
    void setGeometry(const QRectF& rect);
    HbIconAnimator &animator();

    enum { Type = Hb::ItemType_IconItem };
    int type() const {
        return Type;
    }

protected:
    HbIconItem(HbIconItemPrivate &dd, QGraphicsItem *parent);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void changeEvent(QEvent *event);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbIconItem)
    Q_DISABLE_COPY(HbIconItem)

    friend class HbStylePrivate;
};

#endif // HBICONITEM_H
