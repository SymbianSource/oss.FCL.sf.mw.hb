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
#ifndef HBVIEWACTIONMANAGER_H
#define HBVIEWACTIONMANAGER_H

#include <QObject>
#include "hbactionmanager_p.h"

class QAction;

class HB_CORE_PRIVATE_EXPORT HbViewActionManager : public QObject
{
    Q_OBJECT

public:

    class Placement
    {
    public:
        Placement(HbView::ActionContainer container,
                  HbView::ActionContainer preferredContainer) :
            container(container), preferredContainer(preferredContainer) {
        }
        Placement(const QString &containerString) :
            container(HbView::NotSpecified), preferredContainer(HbView::NotSpecified) {
            if (containerString == "ToolBar") {
                container = HbView::ToolBar;
                preferredContainer = HbView::ToolBar;
            } else {
                container = HbView::OptionsMenu;
                preferredContainer = HbView::OptionsMenu;
            }
        }
        HbView::ActionContainer container;
        HbView::ActionContainer preferredContainer;
    };

    class GuideItem
    {
    public:
        GuideItem() {
        }
        GuideItem(HbActionManager::TemplateItem templateItem, Placement placement) :
            templateItem(templateItem) {
            placements.append(placement);
        }
        bool operator==(const GuideItem &other) const {
            return (templateItem == other.templateItem);
        }
        void setTemplateItem(HbActionManager::TemplateItem item) {
            templateItem = item;
        }
        void addPlacement(Placement placement) {
            placements.append(placement);
        }
        HbActionManager::TemplateItem templateItem;
        QList<Placement> placements;
    };

public:
    explicit HbViewActionManager(HbView *view);
    ~HbViewActionManager();

    void addAction(QAction *action, HbView::ActionContainer preferredContainer);
    void removeAction(QAction *action);
    void replaceAction(QAction *oldAction, QAction *newAction);
    QList<QAction *> actions() const;
    void setToolBarmaxCount(int count);
    void setDefaultContainer(const QString &containerString);
    void addGuideItem(const GuideItem &guideItem);

public slots:
    void orientationChanged(Qt::Orientation orientation);

private:
    void createTemplate();
    HbView::ActionContainer actualContainer(QAction *action,
                                            HbView::ActionContainer preferredContainer);
    void moveActionToMenu();
    void removeAction(QAction *action, bool removeFromMap);
    QList<QAction *> containerActions(HbView::ActionContainer container);

    Q_DISABLE_COPY(HbViewActionManager)

private:
    HbView *view; // Not owned
    int toolBarMaxCount;
    HbView::ActionContainer defaultContainer;
    Qt::Orientation orientation;
    QList<GuideItem> distributionGuide;
    QMap<QAction *, Placement> distributedActions;
};

#endif // HBVIEWACTIONMANAGER_H
