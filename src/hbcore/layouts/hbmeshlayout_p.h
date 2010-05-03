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

#ifndef HBMESHLAYOUT_H
#define HBMESHLAYOUT_H

#include <QGraphicsLayout>

#include <hbglobal.h>
#include <hbnamespace.h>

class HbMeshLayoutPrivate;
class HbMeshLayoutDebug;

class HB_AUTOTEST_EXPORT HbMeshLayout : public QGraphicsLayout
{
    friend class HbMeshLayoutDebug;

public:
    typedef Hb::Edge Edge;

public:
    explicit HbMeshLayout(QGraphicsLayoutItem *parent = 0);
    virtual ~HbMeshLayout();

    bool setAnchor(const QString& startId, Edge startEdge, const QString& endId, Edge endEdge, qreal spacing);
    bool setAnchor(const QString& startId, Edge startEdge, const QString& endId, Edge endEdge);
    bool removeAnchor(const QString& startId, Edge startEdge);
    bool removeAnchors(const QString& id);
    void clearAnchors();

    void overrideSpacing(const QString& startId, Edge startEdge, qreal spacing);
    void resetSpacingOverride(const QString& startId, Edge startEdge);
    void clearSpacingOverrides();

    void setItemId(QGraphicsLayoutItem *item, const QString& id);
    void clearItemIds();

    QString nodeId( QGraphicsLayoutItem *item ) const;
    QStringList nodeIds() const;
    QGraphicsLayoutItem *itemById( const QString& id ) const;

    qreal spacing( const QString& startId, Edge startEdge ) const;

    void removeItem(QGraphicsLayoutItem *item);
    int indexOf(const QGraphicsLayoutItem *item) const;

    bool isValid() const;

    virtual void removeAt(int index);
    virtual void setGeometry(const QRectF &rect);
    virtual int count() const;
    virtual QGraphicsLayoutItem *itemAt(int index) const;
    virtual void invalidate();
    virtual void widgetEvent(QEvent *e);

protected:
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

protected:
    HbMeshLayoutPrivate *const d_ptr;

private:
    Q_DISABLE_COPY(HbMeshLayout)
    Q_DECLARE_PRIVATE_D(d_ptr, HbMeshLayout)
};

#endif // HBMESHLAYOUT_H
