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

#ifndef HB_INPUT_PREDICTION_CALLBACK_H
#define HB_INPUT_PREDICTION_CALLBACK_H

#include <hbinputdef.h>

/*!
@proto
@hbcore
\class HbPredictionCallback
\brief A callback method for prediction engine API.

This callback class can be delivered as a parameter for various prediction engine API methods.
When it is given, the prediction engine can use it for querying editor buffer contents and
cursor position. This information is needed for example in case of context sensitive prediction.

\sa HbPredictionEngine
*/
class HbPredictionCallback
{
public:
    virtual ~HbPredictionCallback() {}

    virtual QString surroundingText() = 0;
    virtual int cursorPosition() = 0;
    virtual QList<HbKeyPressProbability> probableKeypresses() {
        return QList<HbKeyPressProbability>();
    }
};

#endif // HB_INPUT_PREDICTION_CALLBACK_H

// End of file
