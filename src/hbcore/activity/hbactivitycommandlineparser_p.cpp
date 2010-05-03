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

#include "hbactivitycommandlineparser_p.h"
#include <QUrl>

/*!
	@alpha
    @hbcore
    \class HbActivityCommandLineParser
    \brief The HbActivityCommandLineParser class is responsible for parsing 
    activity URI passed as command line argument.
*/

/*!
    Searches \a commandLineArguments for pair of "-activity" marker and 
    activity URI, which should have following syntax:
    
    appto://UID3?activityname=activity-name-value&key1=value
    
    If both marker and URI are found, \a reason is set to 
    Hb::ActivationReasonActivity, and \a activityId and \a parameters are
    filled with parsed values.    
*/
void HbActivityCommandLineParser::parseUri(const QStringList &commandLineArguments, Hb::ActivationReason &reason, QString &id, QVariantHash &params)
{
    int activityMarkerIndex = commandLineArguments.indexOf("-activity");
    if (activityMarkerIndex != -1 && commandLineArguments.count() - 1 > activityMarkerIndex) {
        QUrl activityUri(commandLineArguments.at(activityMarkerIndex+1));        
        if (activityUri.scheme() == "appto") {
            QList<QPair<QString, QString> > parameters = activityUri.queryItems();            
            for (QList<QPair<QString, QString> >::const_iterator i = parameters.constBegin(); i != parameters.constEnd(); ++i) {
                params.insert(i->first, i->second);
            }
            
            if (params.contains("activityname") && !params.value("activityname").toString().isEmpty()) {
                // all necessary data is present
                reason = Hb::ActivationReasonActivity;
                id = params.value("activityname").toString();
            } else {
                params.clear();
            }
        }
    }
}
