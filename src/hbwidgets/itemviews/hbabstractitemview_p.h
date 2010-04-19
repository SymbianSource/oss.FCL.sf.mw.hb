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

#ifndef HBABSTRACTITEMVIEW_P_H
#define HBABSTRACTITEMVIEW_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "hbabstractitemview.h"
#include "hbscrollarea_p.h"
#include <hbeffect.h>

#include <QItemSelectionModel>
#include <QPointer>
#include <QPersistentModelIndex>

QT_BEGIN_NAMESPACE
class QModelIndex;
class QEvent;
class QTimer;
QT_END_NAMESPACE

class HbAbstractItemContainer;
class HbAbstractViewItem;
class HbModelIterator;

class HbAbstractItemViewPrivate : public HbScrollAreaPrivate
{
    Q_DECLARE_PUBLIC(HbAbstractItemView)

public:
    enum ItemViewOption {
        NoOptions = 0x0000,
        PanningActive = 0x0001
    };
    
    enum SelectionSetting{
        None = 0x0000,
        Selection = 0x0001
    };

    Q_DECLARE_FLAGS(ItemViewOptions, ItemViewOption)
    Q_DECLARE_FLAGS(SelectionSettings, SelectionSetting)

public:
    HbAbstractItemViewPrivate();
    virtual ~HbAbstractItemViewPrivate();
    void init(HbAbstractItemContainer *container, HbModelIterator *modelIterator);

    void setModel(QAbstractItemModel *model);
    void initializeNewModel();
    void clearCurrentModel();

    void setSelectionModel(QItemSelectionModel *selectionModel);

    // use ItemPosition as position
    QPointF pixelsToScroll(const HbAbstractViewItem *item, HbAbstractItemView::ScrollHint hint);

    void saveIndexMadeVisibleAfterMetricsChange();

    QItemSelectionModel::SelectionFlags  contiguousSelectionCommand(
            const HbAbstractViewItem *item, const QEvent *event );
    QItemSelectionModel::SelectionFlags multiSelectionCommand(
            const HbAbstractViewItem *item, const QEvent *event );
    QItemSelectionModel::SelectionFlags singleSelectionCommand(
            const HbAbstractViewItem *item, const QEvent *event );

    HbAbstractViewItem *itemAt(const QPointF& position) const;

    virtual bool visible(HbAbstractViewItem* item, bool fullyVisible = true) const;

    HbAbstractViewItem* currentItem() const;
    
    HbAbstractViewItem* viewItem(QGraphicsItem *item) const;
    
    void updateScrollBar(Qt::Orientation orientation);

    void refreshContainerGeometry();

    QRectF itemBoundingRect(const QGraphicsItem *item) const;

    void updateItems();

    virtual void scrollTo(const QModelIndex &index, HbAbstractItemView::ScrollHint hint);
    void revealItem(const HbAbstractViewItem *item, HbAbstractItemView::ScrollHint hint);
    void checkBoundaries(QPointF &newPos);

    void updateScrollBarForUniformSizedItems();
    void updateScrollBarForVariableSizedItems();
    void setScrollBarMetrics(Qt::Orientation orientation);
    bool handleScrollBar(Qt::Orientation orientation);

    void rowsRemoved(const QModelIndex &parent, int start, int end);

    virtual QItemSelectionModel::SelectionFlags selectionFlags( 
                                                    const HbAbstractViewItem *item, 
                                                    const QEvent *event);
    void resetContainer();
    void startAppearEffect(const QModelIndex &parent, int start, int end);
    virtual void ensureVisible(QPointF position, qreal xMargin, qreal yMargin);

    void _q_modelDestroyed();
    void _q_layoutChanged();
    void _q_animationEnabled();
    void _q_animationFinished(const HbEffect::EffectStatus &status);

public:
    QPersistentModelIndex mCurrentIndex;

    HbAbstractItemView::SelectionMode mSelectionMode;

    ItemViewOptions mOptions;

    SelectionSettings mSelectionSettings;

    QPointer<HbAbstractViewItem> mHitItem;

    // mContainer can always be assumed to be valid in the code
    HbAbstractItemContainer *mContainer;

    QModelIndex mVisibleIndex;

    QItemSelectionModel *mSelectionModel;
    QItemSelectionModel::SelectionFlag mContSelectionAction;

    bool mWasScrolling;
    QString mLayoutOptionName;
    bool mFilterRemoved;
    bool mClearingSelection;

    bool mAnimateItems;
    QList< QGraphicsItem * >mItemsAboutToBeDeleted;

    // introduced to solve scrollto problem when view is not visible
    QPersistentModelIndex mPostponedScrollIndex;
    HbAbstractItemView::ScrollHint mPostponedScrollHint;
    QModelIndex mPreviousSelectedIndex;
    QItemSelectionModel::SelectionFlags mPreviousSelectedCommand;
    Hb::InteractionModifiers mInstantClickedModifiers;

    QTimer *mAnimationTimer;
    QList< QPersistentModelIndex > mAppearAnimationIndexes;

    HbModelIterator *mModelIterator;
    HbAbstractItemView::ItemAnimations mEnabledAnimations;

private:
    static HbAbstractItemViewPrivate *d_ptr(HbAbstractItemView *abstractItemView) {
        Q_ASSERT(abstractItemView);
        return abstractItemView->d_func();
    }
    friend class HbGridItemContainer;
    friend class HbAbstractItemContainerPrivate;
};


Q_DECLARE_OPERATORS_FOR_FLAGS( HbAbstractItemViewPrivate::ItemViewOptions )
Q_DECLARE_OPERATORS_FOR_FLAGS( HbAbstractItemViewPrivate::SelectionSettings )

#endif // HBABSTRACTITEMVIEW_P_H
