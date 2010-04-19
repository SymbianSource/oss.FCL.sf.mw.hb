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

#ifndef HBINSTANTFEEDBACK_H
#define HBINSTANTFEEDBACK_H

#include "hbabstractfeedback.h"

class HbInstantFeedbackPrivate;

class HB_FEEDBACK_EXPORT HbInstantFeedback : public HbAbstractFeedback
{

public:
    HbInstantFeedback();
    HbInstantFeedback(HbFeedback::InstantEffect effect);
    ~HbInstantFeedback();

    void play();
    static void play(HbFeedback::InstantEffect effect);

    HbFeedback::Type type() const {
        return HbFeedback::TypeInstant;
    }

    bool isValid() const;

    HbFeedback::InstantEffect instantEffect() const;
    void setInstantEffect(HbFeedback::InstantEffect effect);

    HbInstantFeedback &operator=(const HbInstantFeedback &feedback);
    bool operator==(const HbInstantFeedback &feedback) const;
    bool operator!=(const HbInstantFeedback &feedback) const;

private:
    HbInstantFeedbackPrivate *d;
};

#endif // HBINSTANTFEEDBACK_H
