/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbUtils module of the UI Extensions for Mobile.
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

#include "hbdocumentloaderfactory_p.h"

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QGraphicsWidget>
#if QT_VERSION >= 0x040600 && defined(HBUTILS_WEBKIT)
#include <QGraphicsWebView>
#endif // QT_VERSION

#include <hbwidget.h>
#include <hbpushbutton.h>
#include <hblineedit.h>
#include <hbdatetimeedit.h>
#include <hbsearchpanel.h>
#include <hbmenu.h>
#include <hbview.h>
#include <hbprogressbar.h>
#include <hbscrollbar.h>
#include <hbstackedwidget.h>
#include <hbslider.h>
#include <hbsliderpopup.h>
#include <hbtoolbar.h>
#include <hblabel.h>
#include <hbdialog.h>
#include <hbtooltip.h>
#include <hblistview.h>
#include <hblistwidget.h>
#include <hbaction.h>
#include <hbzoomsliderpopup.h>
#include <hbvolumesliderpopup.h>
#include <hbscrollarea.h>
#include <hbgridview.h>
#include <hbradiobuttonlist.h>
#include <hbtextedit.h>
#include <hbratingslider.h>
#include <hbmessagebox.h>
#include <hbcheckbox.h>
#include <hbprogressslider.h>
#include <hbinputdialog.h>
#include <hbcolordialog.h>
#include <hbselectiondialog.h>
#include <hbcombobox.h>
#include <hbgroupbox.h>
#include <hbtreeview.h>
#include <hbtransparentwindow.h>
#include <hbdataform.h>
#include <hbabstractviewitem.h>
#include <hblistviewitem.h>
#include <hbdatetimepicker.h>
#include <hbgridviewitem.h>
#include <hbtreeviewitem.h>
#include <hbdataformviewitem.h>

enum HbDocumentLoaderFactoryWidgetRoles {
    HbWidgetRoleUnknown, // needs to be the first one = 0
    HbWidgetRoleViewWidget,
    HbWidgetRoleViewToolBar,
    HbWidgetRoleViewMenu,
    HbWidgetRoleMenuSubmenu,
    HbWidgetRolePopupHeadingWidget,
    HbWidgetRolePopupContentWidget,
    HbWidgetRoleScrollAreaContents,
    HbWidgetRoleListViewPrototype,  // deprecated
    HbWidgetRoleGroupBoxContentWidget,
    HbWidgetRoleAbstractViewPrototype,
    HbWidgetRoleStackedContentWidget,
    HbWidgetRoleAmount // needs to be the last one
};

enum HbDocumentLoaderFactoryObjectRoles {
    HbObjectRoleUnknown, // needs to be the first one = 0
    HbObjectRoleDialogPrimaryAction, // deprecated
    HbObjectRoleDialogSecondaryAction, // deprecated
    HbObjectRoleMenuAction, // deprecated
    HbObjectRoleToolBarAction, // deprecated
    HbObjectRoleWidgetAction,
    HbObjectRoleAmount // needs to be the last one
};

struct HbDocumentLoaderFactoryKnownValue
{
    const char *name;
    int id;
};

// Used in binary seacrh, so keep name fields in alphabetical order.
const HbDocumentLoaderFactoryKnownValue widgetRoles[HbWidgetRoleAmount - 1] =
{
    { "HbAbstractView:prototype", HbWidgetRoleAbstractViewPrototype },
    { "HbDialog:contentWidget", HbWidgetRolePopupContentWidget },
    { "HbDialog:headingWidget", HbWidgetRolePopupHeadingWidget },
    { "HbGroupBox:contentWidget", HbWidgetRoleGroupBoxContentWidget },
    { "HbListView:prototype", HbWidgetRoleListViewPrototype },  // deprecated
    { "HbMenu:menu", HbWidgetRoleMenuSubmenu },
    { "HbScrollArea:contents", HbWidgetRoleScrollAreaContents },
    { "HbStackedWidget:contentWidget", HbWidgetRoleStackedContentWidget },
    { "HbView:menu", HbWidgetRoleViewMenu },
    { "HbView:toolBar", HbWidgetRoleViewToolBar },
    { "HbView:widget", HbWidgetRoleViewWidget }
};

// Used in binary seacrh, so keep name fields in alphabetical order.
const HbDocumentLoaderFactoryKnownValue objectRoles[HbObjectRoleAmount - 1] =
{
    { "HbDialog:primaryAction", HbObjectRoleDialogPrimaryAction }, // deprecated
    { "HbDialog:secondaryAction", HbObjectRoleDialogSecondaryAction }, // deprecated
    { "HbMenu:addAction", HbObjectRoleMenuAction }, // deprecated
    { "HbToolBar:addAction", HbObjectRoleToolBarAction }, // deprecated
    { "HbWidget:addAction", HbObjectRoleWidgetAction }
};

inline bool operator<(
        const QString &name, const HbDocumentLoaderFactoryKnownValue &prop)
{
    return QString::compare(name, QLatin1String(prop.name), Qt::CaseSensitive) < 0;
}

inline bool operator<(
        const HbDocumentLoaderFactoryKnownValue &prop, const QString &name)
{
    return QString::compare(QLatin1String(prop.name), name, Qt::CaseSensitive) < 0;
}

inline int findKnownValue(
    const QString &name,
    const HbDocumentLoaderFactoryKnownValue *start,
    int numValues)
{
    const HbDocumentLoaderFactoryKnownValue *end = &start[numValues - 1];
    const HbDocumentLoaderFactoryKnownValue *prop = qBinaryFind(start, end, name);
    if (prop == end) {
        return 0;
    }
    return prop->id;
}

/*
    \class HbDocumentLoaderFactory
    \internal
    \proto

    This \c HbDocumentLoaderFactory class is used to perform widget specific actions,
    such as creation of the widgets, which cannot be handled using QT's meta object system.
*/

HbDocumentLoaderFactory::HbDocumentLoaderFactory()
{
}

HbDocumentLoaderFactory::~HbDocumentLoaderFactory()
{
}

/*
    Creates instance of specified type and given object name.
    \param type type of the instance.
    \param name object name of the instance.
    \return newly created object.
*/
QObject *HbDocumentLoaderFactory::create(const QString& type, const QString& name)
{
    QObject *result = 0;

    const QByteArray typeArray = type.toUtf8();
    const char *typeString = typeArray.constData();

    #define CHECK_OBJECT_0(T) else if (!qstrcmp(typeString, #T)) { result = new T(); }

    if (result) {
        // unreachable, intentional
    }
    CHECK_OBJECT_0(QObject)
    CHECK_OBJECT_0(QGraphicsWidget)
    CHECK_OBJECT_0(HbWidget)
    CHECK_OBJECT_0(HbPushButton)
    CHECK_OBJECT_0(HbLabel)
    CHECK_OBJECT_0(HbMenu)
    CHECK_OBJECT_0(HbView)
    CHECK_OBJECT_0(HbSlider)
    CHECK_OBJECT_0(HbLineEdit)
    CHECK_OBJECT_0(HbDateTimeEdit)
    CHECK_OBJECT_0(HbSearchPanel)
    CHECK_OBJECT_0(HbToolBar)
    CHECK_OBJECT_0(HbScrollBar)
    CHECK_OBJECT_0(HbListView)
    CHECK_OBJECT_0(HbListWidget)
    CHECK_OBJECT_0(HbDialog)
    CHECK_OBJECT_0(HbProgressBar)
    CHECK_OBJECT_0(HbStackedWidget)
    CHECK_OBJECT_0(HbAction)
    CHECK_OBJECT_0(HbRadioButtonList)
    CHECK_OBJECT_0(HbZoomSliderPopup)
    CHECK_OBJECT_0(HbVolumeSliderPopup)
    CHECK_OBJECT_0(HbSliderPopup)
    CHECK_OBJECT_0(HbScrollArea)
    CHECK_OBJECT_0(HbGridView)
    CHECK_OBJECT_0(HbTextEdit)
    CHECK_OBJECT_0(HbCheckBox)
    CHECK_OBJECT_0(HbProgressSlider)
    CHECK_OBJECT_0(HbColorDialog)
    CHECK_OBJECT_0(HbSelectionDialog)
    CHECK_OBJECT_0(HbRatingSlider)
    CHECK_OBJECT_0(HbInputDialog)
    CHECK_OBJECT_0(HbMessageBox)
    CHECK_OBJECT_0(HbComboBox)
    CHECK_OBJECT_0(HbGroupBox)
    CHECK_OBJECT_0(HbTreeView)
    CHECK_OBJECT_0(HbTransparentWindow)
    CHECK_OBJECT_0(HbDataForm)
    CHECK_OBJECT_0(HbListViewItem)
    CHECK_OBJECT_0(HbDateTimePicker)
    CHECK_OBJECT_0(HbGridViewItem)
    CHECK_OBJECT_0(HbDataFormViewItem)
    CHECK_OBJECT_0(HbTreeViewItem)
#if QT_VERSION >= 0x040600 && defined(HBUTILS_WEBKIT)
    CHECK_OBJECT_0(QGraphicsWebView)
#endif
    else {
    }

    if (result) {
        result->setObjectName(name);
    }
    return result;
}

/*
    Sets child widget to specified role in parent widget.
    If role is not specified, then basic parent-child relationship is created.

    \param parent parent widget.
    \param child child widget.
    \param role desired role for child widget.
    \return true upon success, false on failure.
*/
bool HbDocumentLoaderFactory::setWidgetRole(
    QGraphicsWidget *parent, QGraphicsWidget *child, const QString &role)
{
    if (role.isEmpty()) {
        child->setParentItem(parent);
        return true;
    }

    bool success = true;

    switch (findKnownValue(role, widgetRoles, HbWidgetRoleAmount)) {
    case HbWidgetRoleViewWidget:
        {
            HbView *view = qobject_cast<HbView *>(parent);
            success = (view != 0);
            if (view) {
                view->setWidget(child);
            }
        }
        break;

    case HbWidgetRolePopupHeadingWidget:
        {
            HbDialog *popup = qobject_cast<HbDialog *>(parent);
            success = (popup != 0);
            if (popup) {
                popup->setHeadingWidget(child);
            }
        }
        break;
    case HbWidgetRoleGroupBoxContentWidget:
        {
            HbGroupBox *groupBox = qobject_cast<HbGroupBox *>(parent);
            success = (groupBox != 0);
            if(groupBox) {
                groupBox->setContentWidget(qobject_cast<HbWidget *>(child));
            }
        }
        break;

    case HbWidgetRolePopupContentWidget:
        {
            HbDialog *popup = qobject_cast<HbDialog *>(parent);
            success = (popup != 0);
            if (popup) {
                popup->setContentWidget(child);
            }
        }
        break;

    case HbWidgetRoleViewToolBar:
        {
            HbView *view = qobject_cast<HbView *>(parent);
            success = (view != 0);
            if (view) {
                child->setParentItem(parent);
                view->setToolBar(qobject_cast<HbToolBar *>(child));
            }
        }
        break;

    case HbWidgetRoleViewMenu:
        {
            HbView *view = qobject_cast<HbView *>(parent);
            success = (view != 0);
            if (view) {
                view->setMenu(qobject_cast<HbMenu *>(child));
            }
        }
        break;

    case HbWidgetRoleMenuSubmenu:
        {
            HbMenu *menu = qobject_cast<HbMenu *>(parent);
            success = (menu != 0);
            if (menu) {
                menu->addMenu(qobject_cast<HbMenu *>(child));
            }
        }
        break;

    case HbWidgetRoleScrollAreaContents:
        {
            HbScrollArea *scrollArea = qobject_cast<HbScrollArea *>(parent);
            success = (scrollArea != 0);
            if (scrollArea) {
                scrollArea->setContentWidget(child);
            }
        }
        break;

    case HbWidgetRoleAbstractViewPrototype:
        {
            HbAbstractItemView *view = qobject_cast<HbAbstractItemView *>(parent);
            HbAbstractViewItem *item = qobject_cast<HbAbstractViewItem *>(child);
            if (view && item) {
                success = true;
                view->setItemPrototype(item);
            } else {
                success = false;
            }
        }
        break;

    case HbWidgetRoleListViewPrototype:
        {
            HbListView *view = qobject_cast<HbListView *>(parent);
            HbAbstractViewItem * item = qobject_cast<HbAbstractViewItem *>(child);
            if (view && item) {
                success = true;
                view->setItemPrototype(item);
            } else {
                success = false;
            }
        }
        break;

    case HbWidgetRoleStackedContentWidget:
        {
            HbStackedWidget *stacked= qobject_cast<HbStackedWidget *>(parent);
            success = (stacked != 0);
            if (stacked) {
                stacked->addWidget(child);
            }
        }
        break;

    case HbWidgetRoleUnknown:
    default:
        success = false;
        break;
    }

    return success;
}

bool HbDocumentLoaderFactory::setObjectRole(
    QObject *parent, QObject *child, const QString &role)
{
    if (role.isEmpty()) {
        child->setParent(parent);
        return true;
    }

    bool success = true;

    switch (findKnownValue(role, objectRoles, HbObjectRoleAmount)) {
    case HbObjectRoleDialogPrimaryAction:
    case HbObjectRoleDialogSecondaryAction:
    case HbObjectRoleMenuAction:
    case HbObjectRoleToolBarAction:
    case HbObjectRoleWidgetAction:
        {
            HbWidget *widget = qobject_cast<HbWidget*>(parent);
            success = (widget != 0);
            if (widget) {
                widget->addAction(qobject_cast<HbAction *>(child));
            }
        }
        break;

    case HbObjectRoleUnknown:
    default:
        success = false;
        break;
    }

    return success;
}
