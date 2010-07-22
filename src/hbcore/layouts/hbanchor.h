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

#ifndef HBANCHOR_H
#define HBANCHOR_H

#include <hbanchorlayout.h>

class HbAnchorPrivate;

class HB_CORE_EXPORT HbAnchor
{
public:

    enum Direction {
        Positive,
        Negative
    };

    // - always: min < pref < max
    // - no negative lenghts (could be added later, though)
    // - Positive means "to-the-right" or "down"
    // - Negative means "to-the-left" or "up"

    HbAnchor( const QString& startNodeId, Hb::Edge startEdge, const QString& endNodeId, Hb::Edge endEdge, qreal length = 0 );
    HbAnchor( const QString& startNodeId, Hb::Edge startEdge, QGraphicsLayoutItem *endItem, Hb::Edge endEdge, qreal length = 0 );
    HbAnchor( QGraphicsLayoutItem *startItem, Hb::Edge startEdge, const QString& endNodeId, Hb::Edge endEdge, qreal length = 0 );
    HbAnchor( QGraphicsLayoutItem *startItem, Hb::Edge startEdge, QGraphicsLayoutItem *endItem, Hb::Edge endEdge, qreal length = 0 );
    ~HbAnchor();

    HbAnchorLayout *parent() const;

    HbAnchor &operator=(const HbAnchor &other);

    void setSizePolicy( QSizePolicy::Policy policy );
    QSizePolicy::Policy sizePolicy() const;

    void setMaximumLength( qreal length );
    void setPreferredLength( qreal length );
    void setMinimumLength( qreal length );

    qreal minimumLength() const;
    qreal preferredLength() const;
    qreal maximumLength() const;

    qreal lengthHint( Qt::SizeHint which ) const;

    QGraphicsLayoutItem *startItem() const;
    QString startNodeId() const;
    Hb::Edge startEdge() const;

    QGraphicsLayoutItem *endItem() const;
    QString endNodeId() const;
    Hb::Edge endEdge() const;

    Direction direction() const;
    void setDirection( Direction dir );

    QString anchorId() const;
    void setAnchorId( const QString &anchorId );

protected:
    HbAnchorPrivate * const d_ptr;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbAnchor)

    friend class HbAnchorLayout; // for setting/removing parent, for example...
    friend class HbAnchorLayoutPrivate;
};

#endif // HBANCHOR_H

