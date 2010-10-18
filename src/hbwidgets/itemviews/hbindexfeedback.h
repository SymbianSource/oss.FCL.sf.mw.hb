/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbWidgets module of the UI Extensions for Mobile.
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


#ifndef HBINDEXFEEDBACK_H
#define HBINDEXFEEDBACK_H

#include <hbglobal.h>
#include <hbwidget.h>

class HbScrollBar;
class HbAbstractItemView;
class HbIndexFeedbackPrivate;
class HbStylePrimitiveData;

class QGraphicsItem;
class QGraphicsObject;

class HB_WIDGETS_EXPORT HbIndexFeedback : public HbWidget
{
    Q_OBJECT

    Q_ENUMS( IndexFeedbackPolicy )

    Q_PROPERTY( IndexFeedbackPolicy IndexFeedbackPolicy READ indexFeedbackPolicy WRITE setIndexFeedbackPolicy)

public:
    enum IndexFeedbackPolicy {
        IndexFeedbackNone = 0,
        IndexFeedbackSingleCharacter,
        IndexFeedbackThreeCharacter,
        IndexFeedbackString
    };

public:
    explicit HbIndexFeedback(QGraphicsItem *parent=0);
    virtual ~HbIndexFeedback();

    enum { Type = Hb::ItemType_IndexFeedback };
    int type() const { return Type; }

    void setIndexFeedbackPolicy( IndexFeedbackPolicy policy);
    IndexFeedbackPolicy indexFeedbackPolicy() const;

    void setItemView(HbAbstractItemView *itemView);
    HbAbstractItemView* itemView() const;

public slots:
    void updatePrimitives();

protected:
    virtual void polish( HbStyleParameters& params);
    virtual bool sceneEventFilter(QGraphicsItem* watched, QEvent* event);
    virtual bool eventFilter(QObject *obj, QEvent *event);
    void initPrimitiveData( HbStylePrimitiveData     *primitiveData, 
                            const QGraphicsObject     *primitive);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbIndexFeedback)
    Q_DISABLE_COPY(HbIndexFeedback)

    Q_PRIVATE_SLOT(d_func(), void _q_scrollPositionChanged(qreal, Qt::Orientation))
    Q_PRIVATE_SLOT(d_func(), void _q_hideIndexFeedback())
    Q_PRIVATE_SLOT(d_func(), void _q_itemViewDestroyed())
    Q_PRIVATE_SLOT(d_func(), void _q_hideIndexFeedbackNow())
};

#endif // HBINDEXFEEDBACK_H
