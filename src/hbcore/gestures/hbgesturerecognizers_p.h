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

#ifndef HBGESTURERECOGNIZERS_P_H
#define HBGESTURERECOGNIZERS_P_H

#include "hbgestures_p.h"
#include "hbpangesturelogic_p.h"
#include "hbtapgesturelogic_p.h"
#include "hbtapandholdgesturelogic_p.h"
#include "hbpinchgesturelogic_p.h"
#include "hbswipegesturelogic_p.h"

#include <QGesture>

class HB_CORE_PRIVATE_EXPORT HbPanGestureRecognizer : public QGestureRecognizer, public HbPanGestureLogic
{
public:
    explicit HbPanGestureRecognizer();
    virtual ~HbPanGestureRecognizer();

    QGesture* create(QObject *);
    QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event);
    void reset(QGesture *state);
};

class HB_CORE_PRIVATE_EXPORT HbTapGestureRecognizer : public QGestureRecognizer, public HbTapGestureLogic
{
public:
    explicit HbTapGestureRecognizer();
    virtual ~HbTapGestureRecognizer();

    QGesture* create(QObject *);
    QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event);
    void reset(QGesture *state);
};

class HB_CORE_PRIVATE_EXPORT HbTapAndHoldGestureRecognizer : public QGestureRecognizer, public HbTapAndHoldGestureLogic
{
public:
    explicit HbTapAndHoldGestureRecognizer();
    virtual ~HbTapAndHoldGestureRecognizer();

    QGesture* create(QObject *);
    QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event);
    void reset(QGesture *state);
};

class HB_CORE_PRIVATE_EXPORT HbPinchGestureRecognizer : public QGestureRecognizer, public HbPinchGestureLogic
{
public:
    HbPinchGestureRecognizer();
    virtual ~HbPinchGestureRecognizer();

    QGesture* create(QObject *);
    QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event);
    void reset(QGesture *state);
};

class HB_CORE_PRIVATE_EXPORT HbSwipeGestureRecognizer : public QGestureRecognizer, public HbSwipeGestureLogic
{
public:
    HbSwipeGestureRecognizer();
    virtual ~HbSwipeGestureRecognizer();

    QGesture* create(QObject *);
    QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event);
    void reset(QGesture *state);
};
#endif // HBGESTURERECOGNIZERS_P_H
