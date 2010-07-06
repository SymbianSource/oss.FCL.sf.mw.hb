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

#ifndef HBICONENGINE_P_H
#define HBICONENGINE_P_H

#include <QIcon>
#include <QIconEngine>
#include <QDataStream>
#include "hbthemecommon_p.h"
#include "hbglobal.h"
#include "hbicon.h"

class HbIconImpl;
class HbIconEnginePrivate;
class HbIconAnimation;
class HbBadgeIconInfo;

class HbIconEngine : public QObject, public QIconEngineV2
{
    Q_OBJECT

public:
    HbIconEngine();
    HbIconEngine(const QString &iconName);
    HbIconEngine(QDataStream &stream);
    ~HbIconEngine();

    HbIconEngine(const HbIconEngine &other);

    void externalize(QDataStream &stream);

    QString iconName() const;
    void setIconName(const QString &iconName);

    QString iconName(QIcon::Mode mode, QIcon::State state) const;
    void setIconName(const QString &iconName, QIcon::Mode mode, QIcon::State state);

    bool isNull() const;
    bool isBadged() const;
    void clear();

    HbIcon::Flags flags() const;
    void setFlags(HbIcon::Flags flags);

    HbIcon::MirroringMode mirroringMode() const;
    void setMirroringMode(HbIcon::MirroringMode mode);

    QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state);
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state);

    QSizeF defaultSize() const;
    QSizeF size() const;
    void setSize(const QSizeF &size);

    void setColor(const QColor &color);
    QColor color() const;

    void setThemedColor(const QColor &color);
    QColor themedColor() const;

    void paint(QPainter *painter,
               const QRect &rect,
               QIcon::Mode mode,
               QIcon::State state);

    void paint(QPainter *painter,
               const QRectF &rect,
               Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio,
               Qt::Alignment alignment = Qt::AlignCenter,
               QIcon::Mode mode = QIcon::Normal,
               QIcon::State state = QIcon::Off);

    void resetIconImpl() const;
    HbIconAnimator *animator() const;
    void setAnimator(HbIconAnimator *animator);
    bool isMirrored() const;
    void addBadge(Qt::Alignment alignment,
                  const HbIcon &badge,
                  int z,
                  const QSizeF &sizeFactor,
                  Qt::AspectRatioMode aspectRatio);
    bool removeBadge(const HbIcon &badge);
    void removeAllBadges();
    const QList<HbBadgeIconInfo> badges() const;
    HbIconFormatType iconFormatType() const;

private:
    void ensureSignalConnections();
    QPixmap getPixmapFromAnimation() const;

    HbIconImpl *paintHelper(const QSizeF &size,
                            Qt::AspectRatioMode aspectRatioMode,
                            QIcon::Mode,
                            QIcon::State);

    bool loadFailed(QIcon::Mode mode, QIcon::State state) const;

    HbIconAnimation *animation() const;

public:
    enum ClearingFlag {
        ResetIconSize = 0x01,
        KeepDefaultSize = 0x02,
        UnloadedByServer = 0x04
    };
    Q_DECLARE_FLAGS(ClearingFlags, ClearingFlag)

public slots:
    void clearStoredIconContent(ClearingFlags flags = 0);
    void clearStoredNonAnimIconContent();

private slots:
    void themeChange(const QStringList &updatedFiles);
    void handleLayoutDirectionChanged();
    void handleDefaultSizeAdjustmentChanged();
    void handleAnimationUpdated();
    void handleAnimationStarted();
    void handleAnimationStopped();
    void handleAnimationFinished();

private:
    HbIconEnginePrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(HbIconEngine::ClearingFlags)

#endif // HBICONENGINE_P_H
