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

#ifndef HBINDEXFEEDBACKPRIVATE_H
#define HBINDEXFEEDBACKPRIVATE_H

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

#include "hbindexfeedback.h"
#include <hbwidget_p.h>

#define HB_INDEXFEEDBACK_TYPE "indexfeedback"
#define EFFECT_IFAPPEAR "appear"
#define EFFECT_IFDISAPPEAR "disappear"

QT_BEGIN_NAMESPACE
class QGraphicsWidget;
class QGraphicsItem;
class QRectF;
QT_END_NAMESPACE

class HbIndexFeedbackPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC( HbIndexFeedback )

public:
    HbIndexFeedbackPrivate();
    virtual ~HbIndexFeedbackPrivate();
    void init();

    void showIndexFeedback();

    void scrollBarPressed();
    void scrollBarReleased();

    void connectScrollBarToIndexFeedback(HbScrollBar* scrollBar);
    void disconnectItemView();

    void _q_scrollPositionChanged(qreal value, Qt::Orientation orientation);
    void _q_hideIndexFeedback();
    void _q_itemViewDestroyed();
    void _q_hideIndexFeedbackNow();

    void updatePrimitives();
    void createPrimitives();

    void calculatePopupRects();
    qreal textHeight() const;
    qreal textWidth() const;

    QString displayText(const QVariant &data) const;

public:
    int mIndexFeedbackPressTimeout;
    int mIndexFeedbackDwellTimeout;
    int mIndexFeedbackReleaseTimeout;
    HbIndexFeedback::IndexFeedbackPolicy mIndexFeedbackPolicy;

    // TODO::The values are storred here is a work around for a bug in hbscrollbar.
    // the bug is that the value changed signal is emitted when the thumb
    // is pressed, and again when it is released, regaurdless of if there was a value change.
    // once that bug is fixed, this should be removed.
    qreal mVerticalScrollBarValue;
    qreal mHorizontalScrollBarValue;

    QTimer *mIndexFeedbackTimer;
    QTimer *mDisappearTimer;

    QString mPopupContent;
    QGraphicsItem *mTextItem;
    QGraphicsItem *mPopupItem;

    HbAbstractItemView *mItemView;

    QRectF mPopupTextRect;
    QRectF mPopupBackgroundRect;
    QRectF mItemViewContentsRect;

    qreal mOneCharHeight;
    qreal mThreeCharHeight;
    qreal mThreeCharWidth;
    qreal mStringOffset;

    QList<QGraphicsItem*> mPopupItemList;
};

#endif //HBINDEXFEEDBACKPRIVATE_H
