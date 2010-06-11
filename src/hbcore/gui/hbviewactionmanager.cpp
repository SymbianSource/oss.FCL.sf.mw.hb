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

#include <QFile>

#include "hbinstance.h"
#include "hbmenu.h"
#include "hbtoolbar.h"
#include "hbviewactionmanager_p.h"
#include "hbactionmanagerxmlparser_p.h"

/*!
    @proto
    @hbcore
    \class HbViewActionManager
    \brief HbViewActionManager is the class that decides switch to container
    the action is placed (options menu or toolbar).
    Decision is based the UI commands distribution guide that is read
    automatically.

    \internal
*/

/*!
    Constructor.
*/
HbViewActionManager::HbViewActionManager(HbView *view) :
    QObject(view), view(view), toolBarMaxCount(99),
    defaultContainer(HbView::OptionsMenu), orientation(Qt::Vertical)
{
    if (view) {
        HbMainWindow *mainWin = view->mainWindow();
        if (mainWin) {
            connect(mainWin, SIGNAL(orientationChanged(Qt::Orientation)),
                    this, SLOT(orientationChanged(Qt::Orientation)));
            orientation = mainWin->orientation();
        }
    }

    createTemplate();
}

/*!
    Destructor.
*/
HbViewActionManager::~HbViewActionManager()
{

}

/*!
    The function adds \a action to either menu's or toolbar's list of actions.
    The ownership of \a action is not transferred.

    \sa removeAction
*/
void HbViewActionManager::addAction(QAction *action,
                                    HbView::ActionContainer preferredContainer)
{
    HbView::ActionContainer container(preferredContainer);

    // Find the right container from the guide
    if (preferredContainer == HbView::NotSpecified) {
        container = actualContainer(action, preferredContainer);
    }

    // Add action to right container
    if (container == HbView::OptionsMenu) {
        view->menu()->addAction(action);
    } else if (container == HbView::ToolBar) {
        view->toolBar()->addAction(action);
        if (view->toolBar()->actions().count() > toolBarMaxCount) {
            moveActionToMenu();
        }
    }

    // Insert data to private structure
    Placement placement(container, preferredContainer);
    distributedActions.insert(action, placement);
}

/*!
    The function removes \a action either menu's or toolbar's list of actions.

    The ownership of \a action is not transferred.
 */
void HbViewActionManager::removeAction(QAction *action)
{
    removeAction(action, true);
}

/*!
    The function replaces an old \a action either menu's or
    toolbar's list of actions.

    The ownership of \a action is transferred to the caller.
 */
void HbViewActionManager::replaceAction(QAction *oldAction, QAction *newAction)
{
    QMap<QAction *, Placement>::const_iterator i =
        distributedActions.find(oldAction);
    if (i != distributedActions.end()) {
        HbViewActionManager::Placement placement = i.value();
        QList<QAction *> list = containerActions(placement.container);
        int index = list.indexOf(oldAction);
        if (index >= 0) {
            if (placement.container == HbView::OptionsMenu) {
                view->menu()->insertAction(oldAction, newAction);
                view->menu()->removeAction(oldAction);
            } else if (placement.container == HbView::ToolBar) {
                view->toolBar()->insertAction(oldAction, newAction);
                view->toolBar()->removeAction(oldAction);
            }
            // Update private structure
            distributedActions.remove(oldAction);
            distributedActions.insert(newAction, placement);
        }
    }
}

/*!
    Returns a list of all actions added with HbView's addAction.

    \sa QAction addAction
 */
QList<QAction *> HbViewActionManager::actions() const
{
    return distributedActions.keys();
}

/*!
    Sets a toolbar max count. Used by HbActionManagerXmlParser.

    \internal
 */
void HbViewActionManager::setToolBarmaxCount(int count)
{
    toolBarMaxCount = count;
}

/*!
    Sets a default container. Used by HbActionManagerXmlParser.

    \internal
 */
void HbViewActionManager::setDefaultContainer(const QString &containerString)
{
    if (containerString == "ToolBar") {
        defaultContainer = HbView::ToolBar;
    } else if (containerString == "OptionsMenu") {
        defaultContainer = HbView::OptionsMenu;
    } else {
        defaultContainer = HbView::NotSpecified;
    }
}

/*!
    Adds a new item to the distribution guide. Used by HbActionManagerXmlParser.

    \internal
 */
void HbViewActionManager::addGuideItem(const GuideItem &guideItem)
{
    distributionGuide.append(guideItem);
}

/*!
   This signal is emitted when orientation change is done.

   \sa HbMainWindow orientationChanged
 */
void HbViewActionManager::orientationChanged(Qt::Orientation orientation)
{
    this->orientation = orientation;

    // clear the current guide and create it again
    distributionGuide.clear();
    createTemplate();

    // Remove actions from containers
    QMapIterator<QAction *, Placement> i(distributedActions);
    while (i.hasNext()) {
        i.next();
        removeAction(i.key(), false);
    }
    // Add them again with the new guide
    i.toFront();
    while (i.hasNext()) {
        i.next();
        addAction(i.key(), i.value().preferredContainer);
    }
}

void HbViewActionManager::createTemplate()
{
    QFile file;
    QString path(":/actionmanager/");
    if (orientation == Qt::Vertical) {
        file.setFileName(path + "distribution_guide_vertical.xml");
    } else {
        file.setFileName(path + "distribution_guide_horizontal.xml");
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("HbViewActionManager::createTemplate: opening file failed");
    } else {
        HbActionManagerXmlParser reader(this);
        if (!reader.read(&file)) {
            qWarning("HbViewActionManager::createTemplate: reading file failed");
        }
    }
}

HbView::ActionContainer HbViewActionManager::actualContainer(QAction *action,
        HbView::ActionContainer preferredContainer)
{
    HbView::ActionContainer container(preferredContainer);

    HbAction *hbAction = qobject_cast<HbAction *>(action);
    // use the default container, if the downcast failed cause then
    // we are not able to obtain the commandRole from the action
    if (!hbAction) {
        return defaultContainer;
    }

    // Find the container from the distribution guide
    HbViewActionManager::GuideItem guideItem;
    HbActionManager::TemplateItem templateItem(hbAction->commandRole());
    guideItem.setTemplateItem(templateItem);
    int index = distributionGuide.indexOf(guideItem);
    if (index != -1) {
        if (distributionGuide[index].placements.count() > 0) {
            container = distributionGuide[index].placements[0].container;
        }
    }

    // Use the default container if the guide does not specify container
    if (container == HbView::NotSpecified) {
        container = defaultContainer;
    }

    return container;
}

void HbViewActionManager::moveActionToMenu()
{
    // Remove last action from the toolbar
    QList<QAction *> toolBarActions = view->toolBar()->actions();

    QAction *action = toolBarActions[view->toolBar()->actions().count() -1];
    view->toolBar()->removeAction(action);
    distributedActions.remove(action);

    // Add actions to menu if it does not exist already
    if (view->menu()->actions().indexOf(action) < 0) {
        view->menu()->addAction(action);
        // Update private structure
        Placement placement(HbView::OptionsMenu, HbView::ToolBar);
        distributedActions.insert(action, placement);
    }
}

void HbViewActionManager::removeAction(QAction *action, bool removeFromMap)
{
    QMap<QAction *, Placement>::const_iterator i =
        distributedActions.find(action);
    if (i != distributedActions.end()) {
        // Remove from the container
        HbViewActionManager::Placement placement = i.value();
        if (placement.container == HbView::OptionsMenu) {
            view->menu()->removeAction(action);
        } else if (placement.container == HbView::ToolBar) {
            view->toolBar()->removeAction(action);
        }

        if (removeFromMap) {
            // Remove from private structure
            distributedActions.remove(action);
        }
    }
}

QList<QAction *> HbViewActionManager::containerActions(
    HbView::ActionContainer container)
{
    QList<QAction *> list;

    switch (container) {
    case HbView::OptionsMenu:
        list = view->menu()->actions();
        break;
    case HbView::ToolBar:
        list = view->toolBar()->actions();
        break;
    default:
        break;
    }
    return list;
}
