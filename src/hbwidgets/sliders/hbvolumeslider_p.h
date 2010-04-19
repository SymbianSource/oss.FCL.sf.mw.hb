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

#ifndef HBVOLUMESLIDER_H
#define HBVOLUMESLIDER_H


#include <hbslider.h>
#include <hbstyleoptionslider.h>
#include "hbnamespace_p.h"

class HbVolumeSliderPrivate;

class HB_AUTOTEST_EXPORT HbVolumeSlider : public HbSlider
{
    Q_OBJECT

public:
    explicit HbVolumeSlider(bool currentValueVisible = false,QGraphicsItem *parent = 0 );
    virtual ~HbVolumeSlider();

    enum { Type = HbPrivate::ItemType_VolumeSlider };
    int type() const { return Type; }

    void setCurrentValueVisible(bool visible);
    bool isCurrentValueVisible()const;

    virtual void setToolTipVisible(bool);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void focusInEvent(QFocusEvent *event);
    void initStyleOption(HbStyleOptionSlider *option) const;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbVolumeSlider)
    Q_DISABLE_COPY(HbVolumeSlider)

    Q_PRIVATE_SLOT(d_func(), void _q_valueChanged(int))
    Q_PRIVATE_SLOT(d_func(), void _q_muteToggled(bool))

};

#endif // HBVOLUMESLIDER_H
