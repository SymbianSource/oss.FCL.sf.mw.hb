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
#ifndef HBABSTRACTVIEWITEM_H
#define HBABSTRACTVIEWITEM_H

#include <hbwidget.h>
#include <hbglobal.h>
#include <hbnamespace.h>
#include <hbeffect.h>

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class HbAbstractViewItemPrivate;
class HbAbstractItemView;
class HbStyleOptionAbstractViewItem;
class HbStyleParameters;
class HbFrameBackground;

class HB_WIDGETS_EXPORT HbAbstractViewItem : public HbWidget
{
    Q_OBJECT

    Q_PROPERTY(Hb::ModelItemType modelItemType READ modelItemType)

public:

    enum StateKey
    { 
        FocusKey,
        CheckStateKey,
        UserKey = 100
    };
    
    explicit HbAbstractViewItem(QGraphicsItem *parent=0);
    virtual ~HbAbstractViewItem();

    enum { Type = Hb::ItemType_AbstractViewItem };
    int type() const;

    virtual HbAbstractViewItem *createItem() = 0;
    virtual void updateChildItems();

    QModelIndex modelIndex() const;
    virtual bool canSetModelIndex(const QModelIndex &index) const;
    virtual void setModelIndex(const QModelIndex &index);

    HbAbstractViewItem *prototype() const;

    HbAbstractItemView *itemView() const;
    void setItemView(HbAbstractItemView *itemView);
    
    bool isFocused() const;
    virtual void receivedFocus();
    virtual void lostFocus();

    virtual QMap<int,QVariant> state() const;
    virtual void setState(const QMap<int,QVariant> &state);
    virtual bool selectionAreaContains(const QPointF &scenePosition) const;

    virtual QGraphicsItem *primitive(HbStyle::Primitive primitive) const;

    void setCheckState(Qt::CheckState state);
    Qt::CheckState checkState() const;

    void setDefaultFrame(const HbFrameBackground &frame);
    HbFrameBackground defaultFrame() const;

    void setPressed(bool pressed, bool animate=true);
    bool isPressed() const;

    Hb::ModelItemType modelItemType() const;

public slots:
    void updatePrimitives();

protected:

    HbAbstractViewItem(const HbAbstractViewItem &source);
    HbAbstractViewItem &operator=(const HbAbstractViewItem &source);

    HbAbstractViewItem( HbAbstractViewItemPrivate &dd, QGraphicsItem *parent );
    void initStyleOption(HbStyleOptionAbstractViewItem *option) const;
    virtual void polish(HbStyleParameters& params);

    virtual bool event(QEvent *e);

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

    virtual void pressStateChanged(bool value, bool animate);

private:
    Q_DECLARE_PRIVATE_D( d_ptr, HbAbstractViewItem )
    Q_PRIVATE_SLOT(d_func(), void _q_animationFinished(const HbEffect::EffectStatus &status))
};

#endif /*HBABSTRACTVIEWITEM_H*/
