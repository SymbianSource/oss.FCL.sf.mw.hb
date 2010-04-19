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

#include "hbfeedbackplugin.h"

class HbFeedbackPluginPrivate
{

public:
    HbFeedbackPluginPrivate();
    HbFeedbackPluginPrivate(const QString& featureName);
    ~HbFeedbackPluginPrivate();

public:
    QString m_featureName;
};

HbFeedbackPluginPrivate::HbFeedbackPluginPrivate()
{
}

HbFeedbackPluginPrivate::HbFeedbackPluginPrivate(const QString& featureName) : m_featureName(featureName)
{
}

HbFeedbackPluginPrivate::~HbFeedbackPluginPrivate()
{
}

/*!
    @beta
    @hbcore
    \class HbFeedbackPlugin

    \brief Base class for all feedback plugins.

    Implements abstract interface HbFeedbackPluginInterface and derives from QObject. All plugins
    are expected to derive from HbFeedbackPlugin.
*/

/*!
    \var FeedbackEffectFeedbackFeature[]
    Standard feature name for plugins that implement feedback effects.
*/

/*!
    \var UserLoggerFeedbackFeature[]
    Standard feature name for plugins that implement functionality that collects a log of user's actions.
*/

/*!
    \var ScreenReaderFeedbackFeature[]
    Standard feature name for plugins that implement screenreaders.
*/

/*!
    Constructs a plugin.
*/
HbFeedbackPlugin::HbFeedbackPlugin()
        : d(new HbFeedbackPluginPrivate())
{
}

/*!
    Constructs a plugin with a specific feature name.
    \param featureName the name of the feature implemented by the plugin
*/
HbFeedbackPlugin::HbFeedbackPlugin(const QString& featureName) : d(new HbFeedbackPluginPrivate(featureName))
{
}

HbFeedbackPlugin::~HbFeedbackPlugin()
{
    delete d;
}

/*!
    Returns the name of the feature implemented by the plugin.
*/
QString& HbFeedbackPlugin::featureName() const
{
    return d->m_featureName;
}

/*!
    Sets the name of the feature implemented by the plugin.
*/
void HbFeedbackPlugin::setFeatureName(const QString& featureName)
{
    d->m_featureName = featureName;
}

