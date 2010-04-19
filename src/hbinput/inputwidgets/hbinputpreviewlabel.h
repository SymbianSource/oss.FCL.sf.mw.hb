/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbInput module of the UI Extensions for Mobile.
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

#ifndef HB_PREVIEWLABEL_H
#define HB_PREVIEWLABEL_H

#include <hbwidget.h>
#include <hbtextitem.h>
#include "hbinputdef.h"

class HB_INPUT_EXPORT HbPreviewLabel : public HbWidget
{
    Q_OBJECT
public:
    explicit HbPreviewLabel(QString previewSymbols, QGraphicsItem *parent = 0);
    ~HbPreviewLabel();
    void setTextGeometry(qreal width, qreal height);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *  event);

signals:
    void selected();
    void hidePreview();
    void hideAccentedPreview();
    void showAccentedPreview(const QString &text, const QRectF &sceneBoundingRect);
private:
    HbTextItem* mTextItem;
};

inline HbPreviewLabel* hbpreviewlabel_cast(QGraphicsItem *item)
{
    if( item->isWidget() && qobject_cast<HbPreviewLabel *>(static_cast<QGraphicsWidget*>(item)) ) {
        return static_cast<HbPreviewLabel *>(item);
    }
    return 0;
}
#endif // HB_PREVIEWLABEL_H
