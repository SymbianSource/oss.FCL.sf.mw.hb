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

class HbAnchorLayoutPrivate;
class HbAnchor;

class HB_CORE_EXPORT HbAnchorLayout : public QGraphicsLayout
{
public:
    typedef Hb::Edge Edge;

    explicit HbAnchorLayout(QGraphicsLayoutItem *parent = 0);
    virtual ~HbAnchorLayout();

    bool setAnchor(
        QGraphicsLayoutItem *startItem,
        Hb::Edge  startEdge,
        QGraphicsLayoutItem *endItem,
        Hb::Edge  endEdge,
        qreal value);

    bool removeAnchor(
        QGraphicsLayoutItem *startItem,
        Hb::Edge  startEdge,
        QGraphicsLayoutItem *endItem,
        Hb::Edge  endEdge);

    void removeAt(int index);
    void removeItem(QGraphicsLayoutItem* item);
    void setGeometry(const QRectF &rect);
    int count() const;
    QGraphicsLayoutItem *itemAt(int index) const;
    int indexOf(const QGraphicsLayoutItem* item) const;

    bool isValid() const;

    void invalidate();
    virtual void widgetEvent(QEvent *e);

protected:
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

protected:
    HbAnchorLayoutPrivate * const d_ptr;

private:
    Q_DISABLE_COPY(HbAnchorLayout)
    Q_DECLARE_PRIVATE_D(d_ptr, HbAnchorLayout)

    friend class HbAnchorLayoutDebug;
};

#endif // HBANCHORLAYOUT_H

