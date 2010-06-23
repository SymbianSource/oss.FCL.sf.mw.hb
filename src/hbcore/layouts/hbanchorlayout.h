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

#ifndef HBANCHORLAYOUT_H
#define HBANCHORLAYOUT_H

#include <QGraphicsLayout>

#include <hbglobal.h>
#include <hbnamespace.h>

class HbAnchorLayoutDebug;
class HbAnchorLayoutPrivate;
class HbAnchor;

class HB_CORE_EXPORT HbAnchorLayout : public QGraphicsLayout
{
public:

    typedef Hb::Edge Edge; // need to remove later...

    explicit HbAnchorLayout( QGraphicsLayoutItem *parent = 0 );
    virtual ~HbAnchorLayout();

    bool setAnchor( const QString &startNodeId, Edge startEdge, const QString &endNodeId, Edge endEdge, qreal length = 0 );
    bool setAnchor( QGraphicsLayoutItem *startItem, Edge startEdge, QGraphicsLayoutItem *endItem, Edge endEdge, qreal length = 0 );

    bool removeAnchor( const QString &startNodeId, Edge startEdge, const QString &endNodeId, Edge endEdge );
    bool removeAnchor( QGraphicsLayoutItem *startItem, Edge startEdge, QGraphicsLayoutItem *endItem, Edge endEdge );

    void removeAnchors();

    bool setMapping( QGraphicsLayoutItem *item, const QString& nodeId );
    bool removeMapping( QGraphicsLayoutItem *item );
    bool removeMapping( const QString &nodeId );
    void removeMappings();

    QString nodeId( QGraphicsLayoutItem *item ) const;
    QGraphicsLayoutItem *itemByNodeId( const QString &nodeId ) const;
    QStringList nodeIds() const;

    void removeItem( QGraphicsLayoutItem *item );
    bool removeNodeId( const QString &nodeId );

    int indexOf(const QGraphicsLayoutItem *item) const;


    bool isValid() const;

    virtual void removeAt( int index );
    virtual void setGeometry( const QRectF &rect );
    virtual int count() const;
    virtual QGraphicsLayoutItem *itemAt( int index ) const;
    virtual void invalidate();
    virtual void widgetEvent( QEvent *e );

protected:
    virtual QSizeF sizeHint( Qt::SizeHint which, const QSizeF &constraint = QSizeF() ) const;

protected:
    HbAnchorLayoutPrivate *const d_ptr;

private:
    Q_DISABLE_COPY( HbAnchorLayout )
    Q_DECLARE_PRIVATE_D( d_ptr, HbAnchorLayout )

    friend class HbAnchorLayoutDebug;
};

#endif // HBANCHORLAYOUT_H

