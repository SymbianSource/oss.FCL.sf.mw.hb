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
#include <QDebug>
#include <QAction>

#include "hbinstance.h"
#include "hbmenu.h"
#include "hbactionmanager_p.h"
#include "hbactionmanagerxmlparser_p.h"
#include "hbmainwindow.h"


/*
     @proto
     @hbcore
    \class HbActionManager
    \brief HbActionManager is the class that calculates the position of 
    an action inside of a container (options menu or toolbar). 
    Calcutation is based the UI commands container template that is read
    automatically.

    \internal
*/

/* 
    Constructor.
*/
HbActionManager::HbActionManager(HbView::ActionContainer container, 
                                 QObject *parent, 
                                 HbMainWindow *mainWindow) :
    QObject(parent), container(container), orientation(Qt::Vertical)
{
    if (mainWindow) {
        connect(mainWindow, SIGNAL(aboutToChangeOrientation()),
                this, SLOT(orientationAboutToChange()));
        orientation = mainWindow->orientation();
    }

    createTemplate();

    if (container == HbView::OptionsMenu) {
        parent->installEventFilter(this);
    }
}

/* 
    Destructor.
*/
HbActionManager::~HbActionManager()
{
    actionTemplate.clear();
}

/* 
    Calculates and returns the position of the action 
    based on the container template.
*/
int HbActionManager::position(QAction *aAction, QList<QAction *> actions) const
{
   if (actions.count() == 0) {
        return 0;
    }

    // try to downcast aAction. If that fails, return a fallback value
    HbAction* action = qobject_cast<HbAction *>(aAction);
    if (!action){
        return actions.count();
    }

    int index = templateIndex(action);
    if (index == -1) {
        return actions.count();
    }

    int pos(0);
    foreach (QAction *qaction, actions) {
        HbAction *containerAction = qobject_cast<HbAction *>(qaction);
        int containerIndex = templateIndex(containerAction);
        if (index < containerIndex) {
            break;
        }
        else {
            pos++;
        }
    }
    return pos;
}

/*
    Adds a new item to the container template. Used by HbActionManagerXmlParser.

    \internal
 */
void HbActionManager::addItem(HbAction::CommandRole role)
{ 
    TemplateItem item(role);
    actionTemplate.append(item);
}

/* 
    Calculates and returns the action based on the container template.
*/
QAction *HbActionManager::orderedAction(const int index, QList<QAction *> actions)
{
    QAction *action = 0;

    if (orderedActions.isEmpty()) {
        foreach (QAction *qaction, actions) {
            HbAction *containerAction = qobject_cast<HbAction *>(qaction);
            if (orderedActions.isEmpty()) {
                orderedActions.append(containerAction);
            }
            else {
                int containerIndex = templateIndex(containerAction);
                int i = 0;
                for (i = 0; i < orderedActions.size(); i++) {
                    int index = templateIndex(orderedActions[i]);
                    if (containerIndex < index) {
                        orderedActions.insert(i, containerAction);
                        break;
                    }
                }
                if (i == orderedActions.size()) {
                    orderedActions.append(containerAction);
                }
            }
        }
    }

    if (index >= 0 && index < orderedActions.size()) {
        action = orderedActions[index];
    }

    return action;
}

/*
   This signal is emitted when orientation change starts.

    \sa HbMainWindow aboutToChangeOrientation
 */
void HbActionManager::orientationAboutToChange()
{
    if (orientation == Qt::Vertical) {
        orientation = Qt::Horizontal;
    }
    else {
        orientation = Qt::Vertical;
    }

    actionTemplate.clear();
    createTemplate();
}

bool HbActionManager::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);
    if ((event->type() == QEvent::ActionAdded || event->type() == QEvent::ActionRemoved) &&
        !orderedActions.isEmpty()) {
        orderedActions.clear();
    }
    return false;
}

void HbActionManager::createTemplate()
{
    QFile file;
    QString path(":/actionmanager/");
    if (container == HbView::ToolBar) {
        if (orientation == Qt::Vertical) {
            file.setFileName(path + "action_template_toolbar_vertical.xml");
        }
        else {
            file.setFileName(path + "action_template_toolbar_horizontal.xml");
        }
    }
    else {
        if (orientation == Qt::Vertical) {
            file.setFileName(path + "action_template_menu_vertical.xml");
            }
        else {
            file.setFileName(path + "action_template_menu_horizontal.xml");
        }
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("HbActionManager::createTemplate: opening file failed");
     } 
    else {
        HbActionManagerXmlParser reader(this);
        if (!reader.read(&file)) {
            qWarning("HbActionManager::createTemplate: reading file failed");
        }        
    }
}

int HbActionManager::templateIndex(HbAction *action) const
{
    TemplateItem item(action->commandRole());
    int index = actionTemplate.indexOf(item);
    return index;
}
