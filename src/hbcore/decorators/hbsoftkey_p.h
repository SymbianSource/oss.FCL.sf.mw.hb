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

#ifndef HBSOFTKEY_P_H
#define HBSOFTKEY_P_H

#include "hbnamespace_p.h"

#include <hbglobal.h>
#include <hbdecorator_p.h>
#include <hbnamespace.h>

class HbAction;
class HbSoftKeyPrivate;
class HbStyleOptionSoftKey;

class HB_CORE_PRIVATE_EXPORT HbSoftKey : public HbDecorator
{
    Q_OBJECT

public:
    explicit HbSoftKey(HbPrivate::SoftKeyId, QGraphicsItem *parent = 0);
    virtual ~HbSoftKey();

    HbAction *action() const;
    void addAction(HbAction *action);
    void removeAction(HbAction *action);

    HbPrivate::SoftKeyId softKeyId();

    enum { Type = HbPrivate::ItemType_SoftKey };
    int type() const { return Type; }

public slots:
    virtual void updatePrimitives();
    void buttonPressed();
    void buttonReleased();

protected:
    HbSoftKey(HbSoftKeyPrivate &dd, HbPrivate::SoftKeyId key, QGraphicsItem *parent = 0);    
    void resizeEvent ( QGraphicsSceneResizeEvent * event );
    virtual bool event(QEvent *e);
    void initStyleOption(HbStyleOptionSoftKey *option) const;

private:  
    Q_DECLARE_PRIVATE_D(d_ptr, HbSoftKey)
    Q_DISABLE_COPY(HbSoftKey)

};

#endif // HBSOFTKEY_P_H
