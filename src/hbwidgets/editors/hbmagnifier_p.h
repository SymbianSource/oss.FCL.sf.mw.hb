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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Hb API.  It exists purely as an
// implementation detail.  This file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#ifndef HBMAGNIFIER_P_H
#define HBMAGNIFIER_P_H

#include "hbglobal.h"
#include "hbnamespace_p.h"
#include "hbwidget.h"
#include "hbeffect.h"


QT_FORWARD_DECLARE_CLASS(QGraphicsItem)
QT_FORWARD_DECLARE_CLASS(QStyleOptionGraphicsItem)
QT_FORWARD_DECLARE_CLASS(QPainter)
QT_FORWARD_DECLARE_CLASS(QPointF)
QT_FORWARD_DECLARE_CLASS(QGraphicsSceneResizeEvent)
QT_FORWARD_DECLARE_CLASS(QVariant)


class HB_AUTOTEST_EXPORT HbMagnifierDelegate
{
public:
    virtual void drawContents(QPainter *painter, const QStyleOptionGraphicsItem *option) = 0;
    virtual ~HbMagnifierDelegate() {}
};


class HbMagnifierPrivate;

class HB_AUTOTEST_EXPORT HbMagnifier : public HbWidget
{
    Q_OBJECT

public:

    enum ContentLockStyle {
        ContentLockStyleFrozen, // Content position is frozen regardless of magnifier postion
        ContentLockStyleSmooth  // Content position depends on magnifier postion and scale factor
    };

    explicit HbMagnifier(QGraphicsItem *parent = 0);
    virtual ~HbMagnifier();

    void setBackground(const QString& graphicsName);
    void setMask(const QString& graphicsName);
    void setOverlay(const QString& graphicsName);

    void setContentDelegate(HbMagnifierDelegate * delegate);
    HbMagnifierDelegate * contentDelegate() const;

    void setContentScale(qreal factor);
    qreal contentScale() const;

    void centerOnContent ( const QPointF & pos );

    bool isContentPositionLocked() const;
    void lockContentPosition();
    void unlockContentPosition();
    void setContentLockstyle(ContentLockStyle contentLockStyle);
    HbMagnifier::ContentLockStyle contentLockstyle() const;

    enum { Type = HbPrivate::ItemType_Magnifier };
    int type() const { return Type; }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    void hideWithEffect();
    void showWithEffect();

protected:
    HbMagnifier(HbMagnifierPrivate &dd, QGraphicsItem *parent);
    QVariant itemChange(GraphicsItemChange change, const QVariant & value);

    void resizeEvent(QGraphicsSceneResizeEvent *event);

private:

    Q_PRIVATE_SLOT(d_func(), void _q_hidingEffectFinished(const HbEffect::EffectStatus &status))

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbMagnifier)
    Q_DISABLE_COPY(HbMagnifier)
};



#endif // HBMAGNIFIER_P_H
