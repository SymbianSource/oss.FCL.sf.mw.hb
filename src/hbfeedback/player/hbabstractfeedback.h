/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbFeedback module of the UI Extensions for Mobile.
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

#ifndef HBABSTRACTFEEDBACK_H
#define HBABSTRACTFEEDBACK_H

#include <hbfeedbacknamespace.h>

#include <QWidget>
#include <QRect>
#include <QPointer>

class QGraphicsItem;

class QGraphicsView;
class HbAbstractFeedbackPrivate;

class HB_FEEDBACK_EXPORT HbAbstractFeedback
{

public:

    HbAbstractFeedback();
    virtual ~HbAbstractFeedback();

    virtual HbFeedback::Type type() const = 0;

    virtual bool isValid() const = 0;
    bool isLocated() const;

    QRect rect() const;
    QWidget* window() const;
    HbFeedback::Modalities modalities() const;

    void setRect(QRect rect);
    void setRect(const QWidget* widget);
    void setRect(const QGraphicsItem* graphicsItem, const QGraphicsView* graphicsView);
    void setOwningWindow(const QWidget* widget);
    void setModalities(HbFeedback::Modalities modalities = HbFeedback::All);

    virtual HbAbstractFeedback &operator=(const HbAbstractFeedback &feedback);
    virtual bool operator==(const HbAbstractFeedback &feedback) const;
    virtual bool operator!=(const HbAbstractFeedback &feedback) const;

private:
    HbAbstractFeedbackPrivate *d;
};

#endif // HBABSTRACTFEEDBACK_H
