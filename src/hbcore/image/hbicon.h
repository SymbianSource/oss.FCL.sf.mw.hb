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

#ifndef HBICON_H
#define HBICON_H

#include <hbglobal.h>

#include <QIcon> //krazy:exclude=qclasses
#include <QMetaType>
#include <QString>
#include <QExplicitlySharedDataPointer>

class HbIconPrivate;
class HbIconAnimator;

class HB_CORE_EXPORT HbIcon
{
public:
    HbIcon();
    HbIcon(const QString &iconName);
    HbIcon(const QIcon &icon);

    HbIcon(const HbIcon &other);
    HbIcon &operator=(const HbIcon &other);

    bool operator==(const HbIcon &other) const;
    bool operator!=(const HbIcon &other) const;

    ~HbIcon();

    bool isNull() const;
    bool isBadged() const;
    void clear();

    QPixmap pixmap();

    void setColor(const QColor &color);
    QColor color() const;

    QString iconName() const;
    void setIconName(const QString &iconName);

    QString iconName(QIcon::Mode mode, QIcon::State state) const;
    void setIconName(const QString &iconName, QIcon::Mode mode, QIcon::State state);

    QSizeF defaultSize() const;
    QSizeF size() const;
    void setSize(const QSizeF &size);

    void setHeight(qreal height);
    void setWidth(qreal width);

    qreal width() const;
    qreal height() const;

    enum MirroringMode {
        Default = 0,
        Forced = 1,
        Prevented = 2,
        LayoutDirection = 3
    };

    MirroringMode mirroringMode() const;
    void setMirroringMode(MirroringMode mode);

    enum Flag {
        DoNotCache = 0x01,
        ResolutionCorrected = 0x02,
        NoAutoStartAnimation = 0x04,
        Colorized = 0x08,
        NonThemeable = 0x10
    };

    Q_DECLARE_FLAGS(Flags, Flag)

    Flags flags() const;
    void setFlags(Flags flags);

    void paint(QPainter *painter,
                const QRectF &rect,
                Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio,
                Qt::Alignment alignment = Qt::AlignCenter,
                QIcon::Mode mode = QIcon::Normal,
                QIcon::State state = QIcon::Off) const;

    operator QVariant() const;

    QIcon &qicon() const;

    bool addBadge(Qt::Alignment alignment,
                      const HbIcon& badge,
                      int z=0);
    bool removeBadge(const HbIcon& badge);
    void removeAllBadges();

private:
    QExplicitlySharedDataPointer<HbIconPrivate> d;

    friend HB_CORE_EXPORT QDataStream &operator<<(QDataStream &, const HbIcon &);
    friend HB_CORE_EXPORT QDataStream &operator>>(QDataStream &, HbIcon &);
    friend class HbIconAnimator;
    friend class HbIconPrivate;
};

HB_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const HbIcon &icon);
HB_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, HbIcon &icon);

Q_DECLARE_METATYPE(HbIcon)
Q_DECLARE_OPERATORS_FOR_FLAGS(HbIcon::Flags)

#endif // HBICON_H
