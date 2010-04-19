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

#ifndef HBFEEDBACKPLUGININTERFACE_H
#define HBFEEDBACKPLUGININTERFACE_H

#include <hbglobal.h>

#include <hbfeedbackengine.h>

#include <QtPlugin>
#include <QString>

/*!
    \class HbFeedbackPluginInterface

    \brief Abstract interface for feedback plugins that implement different feedback features. 
    
    See \ref arcFeedback "Feedback Manager Framework" for more information.

    \sa HbFeedbackEngine
*/

struct HbFeedbackPluginInterface {
    virtual ~HbFeedbackPluginInterface() {}

    /*!
    Returns the name of the supported feedback feature
    */
    virtual QString& featureName() const = 0;

    /*!
    Returns the feedback engine that implements the feature
    */
    virtual HbFeedbackEngine& engine() = 0;
};

Q_DECLARE_INTERFACE(HbFeedbackPluginInterface, "com.nokia.Hb.FeedbackPluginInterface/1.0")

#endif // HBFEEDBACKPLUGININTERFACE_H
