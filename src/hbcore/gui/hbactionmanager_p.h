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
#ifndef HBACTIONMANAGER_H
#define HBACTIONMANAGER_H

#include <QObject>

#include "hbaction.h"
#include "hbview.h"
class HbMainWindow;
class QAction;

class HB_AUTOTEST_EXPORT HbActionManager : public QObject
{
    Q_OBJECT

public:

    class TemplateItem 
    {
    public:
        TemplateItem() :
            role(HbAction::NoRole)
        {
        }
        TemplateItem(HbAction::CommandRole role) :
            role(role)
        {
        }
        bool operator==(const TemplateItem &other) const
        { 
            return (role == other.role); 
        }
        HbAction::CommandRole role;
    };

    HbActionManager(HbView::ActionContainer container, QObject *parent, HbMainWindow *mainWindow);
    ~HbActionManager();

    int position(QAction *action, QList<QAction *> actions, int maxPos = -1) const;
    void addItem(HbAction::CommandRole role);
    QAction *orderedAction(int index, QList<QAction *> actions);

public slots:
    void orientationAboutToChange();

protected:
    bool eventFilter(QObject *object, QEvent *event);

private:
    void createTemplate();
    int templateIndex(HbAction *action) const;

    Q_DISABLE_COPY(HbActionManager)

private:
    QList<TemplateItem> actionTemplate;
    HbView::ActionContainer container;
    Qt::Orientation orientation;
    QList<HbAction *> orderedActions;
};

#endif // HBACTIONMANAGER_H
