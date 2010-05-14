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

#ifndef HBWIDGET_H
#define HBWIDGET_H

#include <hbglobal.h>
#include <hbstyle.h>
#include <hbstyleparameters.h>
#include <hbnamespace.h>
#include <hbwidgetbase.h>
#include <hbfeedbacknamespace.h>

class HbEvent;
class HbStyleOption;
class HbWidgetPrivate;
class HbMainWindow;
#ifdef HB_GESTURE_FW
class QGestureEvent;
#endif

class HB_CORE_EXPORT HbWidget: public HbWidgetBase
{
    Q_OBJECT

    Q_ENUMS(FocusDelegation FocusMode)
    Q_PROPERTY(QString toolTipText READ toolTip WRITE setToolTip)
    Q_PROPERTY(HbWidget::FocusDelegation focusDelegation READ focusDelegation WRITE setFocusDelegation)
    Q_PROPERTY(bool focusLooping READ hasFocusLooping WRITE setFocusLooping)
    Q_PROPERTY(HbWidget::FocusMode focusMode READ focusMode WRITE setFocusMode)

public:

    enum FocusDelegation
    {
        FocusDelegationNone,
        FocusDelegationFirstChild,
        FocusDelegationLastChild,
        FocusDelegationBasedOnDirection,
        FocusDelegationFocusedChild
    };

    enum FocusMode
    {
        FocusModeNormal,
        FocusModeEdit
    };

    enum FocusHighlight
    {
        FocusHighlightNone,
        FocusHighlightActive,
        FocusHighlightResidual
    };

    explicit HbWidget(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~HbWidget();
    virtual bool event(QEvent *e);
    void setStyle(HbStyle *style);
    HbStyle *style() const;
    void clearActions();

    enum { Type = Hb::ItemType_Widget };
    int type() const { return Type; }

    int pluginBaseId() const;

    HbMainWindow *mainWindow() const;

    virtual QGraphicsItem *primitive(HbStyle::Primitive primitive) const;
    virtual QGraphicsItem *primitive(const QString &itemName) const;
    virtual QGraphicsLayoutItem *layoutPrimitive(const QString &itemName) const;

    void setFocusOrientation(Qt::Orientations previous, Qt::Orientations next);

    QKeySequence keyForFocusNextChild() const;
    QKeySequence keyForFocusPreviousChild() const;

    void setKeyFocusMode(const QKeySequence &key);
    QKeySequence keyFocusMode() const;

    QGraphicsItem *lastFocusedChild() const;
    void setFocusOrder(QGraphicsItem *first, QGraphicsItem *second);

    void setFocusDelegation(HbWidget::FocusDelegation focusDelegation);
    HbWidget::FocusDelegation focusDelegation() const;

    void setFocusLooping(bool enable);
    bool hasFocusLooping() const;

    void setFocusMode(HbWidget::FocusMode);
    HbWidget::FocusMode focusMode() const;

    void setFocusHighlight(HbStyle::Primitive type, HbWidget::FocusHighlight focusHighlight);
    HbStyle::Primitive focusHighlight(HbWidget::FocusHighlight focusHighlight);

    void setBackgroundItem(HbStyle::Primitive type, int zValue = -1);
    void setBackgroundItem(QGraphicsItem *item, int zValue = -1);
    QGraphicsItem *backgroundItem() const;

public slots:
    virtual void recreatePrimitives();
    virtual void updatePrimitives();

private slots:
    virtual void hideFocusHighlight();

protected:
    virtual void focusInEvent(QFocusEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual void resizeEvent(QGraphicsSceneResizeEvent *event);
    virtual void changeEvent(QEvent *event);
    QVariant itemChange (GraphicsItemChange change, const QVariant &value);
    virtual void polishEvent();
    virtual void polish(HbStyleParameters &params);
    void repolish();

    void setPluginBaseId(int baseId);

protected:
    HbWidget(HbWidgetPrivate &dd, QGraphicsItem *parent, Qt::WindowFlags wFlags=0);
private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbWidget)
    Q_DISABLE_COPY(HbWidget)
    friend class HbGraphicsScenePrivate;
};

#endif // HBWIDGET_H
