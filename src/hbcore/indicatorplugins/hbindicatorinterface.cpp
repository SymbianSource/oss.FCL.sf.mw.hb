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

#include <hbindicatorinterface.h>
#include <QVariant>

/*!
    \class HbIndicatorInterface
    \brief HbIndicatorInterface is an abstract base class
    for all indicator implementations

    Indicators are displayed in the status indicator area and/or in universal indicator menu.
    Applications can create custom indicators by implementing this abstract class and HbIndicatorPluginInterface.
    Each indicator is identified by its type-string (See indicatorType). Clients use HbIndicator-class to activate
    and deactivate indicators.

    Indicator has a number of data elements associated with it and they can be retrieved by
    specifying a role (see HbIndicatorInterface::DataRole) to the indicatorData()
    function. An indicator should at least contain an icon and text elements.

    Indicator can be interacted by the user, for example indicator text can be clicked.
    (See HbIndicatorInterface::InteractionType).
    The handling of the interaction is indicator's responsibility. (See handleInteraction()).

    Universal indicators are divided into three categories: 
    1) notification indicators, 2) progress indicators and 3) settings indicators. 
    The most important indicators can appear in the status bar, 
    notification indicators on the left side, progress and settings indicators on the right side. 
    The Universal indicator button provides access to the universal indicator menu that contains all active indicators.
    (See HbIndicatorInterface::Cartegory). The category is returned by the function:
    category()

    \sa HbIndicatorPluginInterface, HbIndicator

    \alpha
    \hbcore
*/

/*!
    \deprecated HbIndicatorInterface::GroupPriority
        is deprecated. Please use HbIndicatorInterface::Category instead.
    \enum HbIndicatorInterface::GroupPriority
    Defines indicator group priority. This determines where the indicator is shown: in indicator area or only
    in indicator menu etc.
*/
/*!
    \var HbIndicatorInterface::GroupPriority HbIndicatorInterface::GroupPriorityHigh
    High priority indicators are visible in the status area as long as the process is ongoing.
*/
/*!
    \var HbIndicatorInterface::GroupPriority HbIndicatorInterface::GroupPriorityAverage
    Average priority indicators are used together with notifications (discreet popup)
    The indicator will appear shortly in the status pane.
*/
/*!
    \var HbIndicatorInterface::GroupPriority HbIndicatorInterface::GroupPriorityLow
    Low priority indicators are shown only in universal indicator menu.
*/

/*! 
    \enum HbIndicatorInterface::Category
    Defines indicator group category. This determines where the indicator is shown.
*/
/*!
    \var HbIndicatorInterface::Category HbIndicatorInterface::NotificationCategory
    NotificationCategory indicators are visible in the status bar in the notification area.
    NotificationCategory indicators are shown also in universal indicator menu.
*/
/*!
    \var HbIndicatorInterface::Category HbIndicatorInterface::ProgressCategory
    Aggregated ProgressCategory indicators are visible in the status bar in the progress/settings area.
    ProgressCategory indicators are shown also in universal indicator menu.
*/
/*!
    \var HbIndicatorInterface::Category HbIndicatorInterface::SettingCategory
    SettingCategory indicators are visible in the status bar in the progress/settings area.
    SettingCategory indicators are shown also in universal indicator menu.
*/

/*!
    \enum HbIndicatorInterface::InteractionType
    Defines the interaction type of the indicator. Depending on the interaction type the indicator may look different
    in the indicator menu. Each interaction is handled in handleInteraction().
*/
/*!
    \var HbIndicatorInterface::InteractionType HbIndicatorInterface::NoInteraction
    No interaction.
*/
/*!
    \var HbIndicatorInterface::InteractionType HbIndicatorInterface::InteractionActivated
    Indicator text is a link and can be activated by the user.
*/

/*!
    \enum HbIndicatorInterface::DataRole
    Defines all the data roles an indicator can have.
    An indicator has set of data elements associated with it, each with its own role.
*/
/*!
    \var HbIndicatorInterface::DataRole HbIndicatorInterface::IconNameRole
    Indicator icon path role. This is the primary icon shown in indicator status area. If there's no data
    for this role, the icon path is retrieved using DataRole::DecorationPathRole instead. The icon must be located 
    in a directory, where all possible client applications are able to read it. 
    The icon cannot be located in the plugin's resources. 
*/
/*!
    \var HbIndicatorInterface::DataRole HbIndicatorInterface::DecorationNameRole
    role to the icon path, which is used as a decoration in universal indicator menu.
*/

/*!
    \var HbIndicatorInterface::DataRole HbIndicatorInterface::MonoDecorationNameRole
    role to the icon path, which is used as a decoration in the status bar area.
*/

/*!
    \var HbIndicatorInterface::DataRole HbIndicatorInterface::PrimaryTextRole
    Primary indicator text shown in universal indicator menu.
*/

/*!
    \var HbIndicatorInterface::DataRole HbIndicatorInterface::SecondaryTextRole
    Secondary indicator text shown in universal indicator menu.
*/

/*!
    \var HbIndicatorInterface::DataRole HbIndicatorInterface::TextRole
    Indicator text shown in universal indicator popup.
    \deprecated HbIndicatorInterface::TextRole
        is deprecated. Please use HbIndicatorInterface::PrimaryTextRole instead.
*/

/*!
    \enum HbIndicatorInterface::RequestType
    Client request types. See handleClientRequest()
*/
/*!
    \var HbIndicatorInterface::RequestType HbIndicatorInterface::RequestActivate
    Client requests the activation of the indicator.
*/
/*!
    \var HbIndicatorInterface::RequestType HbIndicatorInterface::RequestDeactivate
    Client requests the deactivation of the indicator.
*/

/*!
    \fn virtual QVariant HbIndicatorInterface::indicatorData(int role) const = 0

    Returns the data stored under the given role. The indicator should at least have data elements for
    HbIndicatorInterface::TextRole and HbIndicatorInterface::DecorationNameRole. If data element for
    HbIndicatorInterface::IconNameRole doesn't exist, the same icon retrieved by \c DecorationNameRole
    is used both in status indicator area and universal indicator popup.  If the indicator is
    low priority indicator, it is shown only in universal indicator popup, and data element for
    \c IconNameRole is ignored

    \a role the data role for the indicator. A value from the HbIndicatorInterface::DataRole - enum.

    For the roles not supported by the plugin, invalid QVariant should be returned.
*/

/*!
    \fn void  HbIndicatorInterface::dataChanged()

    The class should emit this signal, when it's data changes.

    \sa indicatorData()
*/

/*!
    \fn void HbIndicatorInterface::deactivate()
    The class should emit this signal, when it wants to deactivate itself. By default
    this signal is emitted when HbIndicatorInterface::processClientRequest receives
    deactivate-request.

    \sa HbIndicatorInterface::processClientRequest
*/

class HbIndicatorInterfacePrivate
{
public:
    HbIndicatorInterfacePrivate()
    {
    }

    void init(const QString &indicatorType,
        HbIndicatorInterface::Category category,
        HbIndicatorInterface::InteractionTypes interactionTypes)
    {
        this->indicatorType = indicatorType;
        this->category = category;
        this->groupPriority = HbIndicatorInterface::GroupPriorityHigh; //temp
        this->interactionTypes = interactionTypes;
    }

    //deprecated
    void init(const QString &indicatorType,
        HbIndicatorInterface::GroupPriority groupPriority,
        HbIndicatorInterface::InteractionTypes interactionTypes)
    {
        this->indicatorType = indicatorType;
        this->groupPriority = groupPriority;
        this->category = (groupPriority == HbIndicatorInterface::GroupPriorityLow)
                      ? HbIndicatorInterface::SettingCategory
                          : HbIndicatorInterface::NotificationCategory;
        this->interactionTypes = interactionTypes;
    }

public:
    QString indicatorType;
    HbIndicatorInterface::Category category;
    HbIndicatorInterface::GroupPriority groupPriority; //deprecated
    HbIndicatorInterface::InteractionTypes interactionTypes;
};

/*!
    Destructor
*/
HbIndicatorInterface::~HbIndicatorInterface()
{
    delete d;
}

/*!
    returns the the type of the indicator
*/
QString HbIndicatorInterface::indicatorType() const
{
    return d->indicatorType;
}

/*!
    returns the interaction types handled by the class.

    \sa handleInteraction
*/
HbIndicatorInterface::InteractionTypes HbIndicatorInterface::interactionTypes() const
{
    return d->interactionTypes;
}

/*!
    returns the category group the indicator belongs to.
    
*/
HbIndicatorInterface::Category HbIndicatorInterface::category() const
{
    return d->category;
}

/*!
    \deprecated HbIndicatorInterface::groupPriority()
        is deprecated. Please use HbIndicatorInterface::category() instead.
    returns the priority group the indicator belongs to.
   */
HbIndicatorInterface::GroupPriority HbIndicatorInterface::groupPriority() const
{
    return d->groupPriority;
}

/*!
    processes the requests from the client.

    Calls handleClientRequest. Does default handling for the request,
    if the request is not handled: emits deactivate signal when receiving deactivate-request.

    \a type is a type of the request.
    \a parameter is extra parameter from the client. Can be invalid, if not given.


    \sa RequestType, handleInteraction
*/
void HbIndicatorInterface::processClientRequest(RequestType type,
                                                const QVariant &parameter)
{
    if (!handleClientRequest(type, parameter)) {
        //default handling.
        if (type == RequestDeactivate) {
            emit deactivate();
        }
    }
}

/*!
    called by the framework, when the user interacts with the indicator.
    \a type is a type of the interaction.

    The method should be reimplemented, when interaction type is set to other than
    HbIndicatorInterface::NoInteraction.

    should return true, if interaction was handled, false otherwise. Default implementation
    returns false.

    \sa interactionTypes
*/
bool HbIndicatorInterface::handleInteraction(InteractionType /*type*/)
{
    return false; //not handled
}

/*!
    Constructs an indicator.

    \a indicatorType contains the type of the indicator,
    \a Category indicator category the object belongs to,
    \a interactionTypes interaction type flags for the object
*/
HbIndicatorInterface::HbIndicatorInterface(const QString &indicatorType,
    Category category, InteractionTypes interactionTypes) :
    d(new HbIndicatorInterfacePrivate())
{
    d->init(indicatorType, category, interactionTypes);
}

/*!
    Constructs an indicator.

    \a indicatorType contains the type of the indicator,
    \a indicatorGroup indicator group priority the object belongs to,
    \a interactionTypes interaction type flags for the object

    \deprecated HbIndicatorInterface::HbIndicatorInterface(const QString&, GroupPriority, InteractionTypes)
        is deprecated, use the other constructor.
*/
HbIndicatorInterface::HbIndicatorInterface(const QString &indicatorType,
    GroupPriority indicatorGroup, InteractionTypes interactionTypes) :
    d(new HbIndicatorInterfacePrivate())
{
    d->init(indicatorType, indicatorGroup, interactionTypes);
}

/*!
    Handles the requests sent from the client
    \a type is a type of the request.
    \a parameter is extra parameter from the client. Can be invalid, if not given.

    Should return true, when the request is handled.
    Default implementation returns false.

    \sa RequestType, HbIndicator
*/
bool HbIndicatorInterface::handleClientRequest(RequestType /*type*/, const QVariant &/*parameter*/)
{
    return false;
}

