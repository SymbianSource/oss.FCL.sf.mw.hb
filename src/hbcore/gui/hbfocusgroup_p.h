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

#ifndef HBFOCUSGROUP_P_H
#define HBFOCUSGROUP_P_H
#include "hbwidget.h"
#include <qtimer.h>

class HB_CORE_EXPORT HbFocusGroup : public QGraphicsItem
{
public:
    enum FocusParameter {
        AlwaysOn = 1,
        NeverOn = 2,
        Transient = 4
    };

    Q_DECLARE_FLAGS(FocusParameters, FocusParameter)

    HbFocusGroup(QGraphicsWidget *parent = 0);
    ~HbFocusGroup();

    void registerItem(QGraphicsItem *item);
    void unregisterItem(QGraphicsItem *item);
    void registerChildren(QGraphicsItem *item);
    void unregisterChildren(QGraphicsItem *item);
    bool isItemRegistered(QGraphicsItem *item) const;
    bool isChildrenRegistered() const { return (childList.size() > 0); }
    bool isFocusNextPrevKey(const QKeyEvent &event, bool next) const;
    bool isFocusModeChangeKey(const QKeyEvent &event) const;
    void setFocusOrder(QGraphicsItem *first, QGraphicsItem *second);
    bool focusNextPrevItem(QGraphicsItem *item, bool next) const;
    bool delegateFocus(QFocusEvent *event);


    void startHighlightTimer(HbWidget *widget);
    void stopHighlightTimer(HbWidget *widget, bool disconnect = true);

    QRectF boundingRect() const { return QRectF(); }
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) { }
    
    void setKeyForFocusNextChild(const QKeySequence &key);
    QKeySequence keyForFocusNextChild() const { return mKeyFocusNextChild; }

    void setKeyForFocusPreviousChild(const QKeySequence &key);
    QKeySequence keyForFocusPreviousChild() const { return mKeyFocusPreviousChild; }

    void changeFocusMode();
    void setFocusMode(HbWidget::FocusMode focusMode);
    HbWidget::FocusMode focusMode() const { return mFocusMode; }

    void setKeyForFocusMode(const QKeySequence &key);
    QKeySequence keyFocusMode() const { return mKeyFocusMode; }

    void updateCurrentFocusChild(QGraphicsItem *item);
    QGraphicsItem *currentFocusChild() const { return mCurrentFocusChild; }

    void setFocusDelegation(HbWidget::FocusDelegation);
    HbWidget::FocusDelegation focusDelegation() const { return mFocusDelegation; }

    void setFocusLooping(bool enable);
    bool hasFocusLooping() const { return mFocusLooping; }

    FocusParameters focusParameters() const { return mFocusParameters; }

private:
    QKeySequence mKeyFocusNextChild;
    QKeySequence mKeyFocusPreviousChild;
    QKeySequence mKeyFocusMode;
    QGraphicsItem *mCurrentFocusChild;
    QGraphicsItem *mFocusFirst;
    QGraphicsItem *mFocusLast;

    HbWidget::FocusDelegation mFocusDelegation;
    bool mFocusLooping;
    HbWidget::FocusMode mFocusMode;
    FocusParameters mFocusParameters;

    QTimer mHighlightTimer;

private:
    void removeItem(QGraphicsItem *item);

    struct ChildInfo
    {
        ChildInfo(QGraphicsItem *child = 0) : focusNext(child), focusPrev(child) {}
        QGraphicsItem *focusNext;
        QGraphicsItem *focusPrev;
    };
    ChildInfo findNextPrevItem(QGraphicsItem *item, bool next) const;

    QHash<QGraphicsItem *, ChildInfo> childList;

    class GraphicsItem: public QGraphicsItem
    {
        friend class HbFocusGroup;
    };
};

#endif //HBFOCUSGROUP_P_H
