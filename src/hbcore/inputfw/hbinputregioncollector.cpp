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
#include "hbinputregioncollector_p.h"

#include <QRegion>
#include <QTransform>
#include <QPointer>

#include "hbwidget.h"
#include "hbmainwindow.h"
#include "hbpopup.h"
#include "hbdeviceprofile.h"

/*!
\proto
\class HbInputRegionCollector
\brief Installs a filter on a HbWidget and observes for a change in position, size and visibility
of the attached widget. As soon as it detects a change in size, position or visibility it calculates
total of the region of all the widgets attached and emits updateRegion signal.

It is enabled and disabled by input framework, it is enabled when ever input framework detects
that the application is not having any HbMainWindow.

*/

/// @cond

class HbWidgetFilterList
{
public:
    HbWidgetFilterList(HbWidget *w)
        : mWidget(w), mIsVisible(false) {
    }
    bool operator ==(const HbWidgetFilterList &other) const {
        return mWidget == other.mWidget;
    }
    QPointer <HbWidget> mWidget;
    // visibility is needed as the time when we get show event inside eventFilter
    // widget is not visible.
    bool mIsVisible;
};

class HbInputRegionCollectorPrivate
{
public:
    HbInputRegionCollectorPrivate()
        : mEnabled(false), mModalDialogs(0) {}
    QList < HbWidgetFilterList > mInputWidgets;
    bool mEnabled;
    int mModalDialogs;
};


/*!
Creates a static instance of HbInputRegionCollector.
*/
HbInputRegionCollector *HbInputRegionCollector::instance()
{
    static HbInputRegionCollector regionCollector;
    return &regionCollector;
}

/*!
Constructor.
*/
HbInputRegionCollector::HbInputRegionCollector()
    : d_ptr(new HbInputRegionCollectorPrivate())
{
}

/*!
Destructor.
*/
HbInputRegionCollector::~HbInputRegionCollector()
{
    delete d_ptr;
}

/*!
Installs a even filter on the passed widget.
*/
void HbInputRegionCollector::attach(HbWidget *widget)
{
    if (widget && !d_ptr->mInputWidgets.contains(widget)) {
        if (d_ptr->mEnabled) {
            widget->installEventFilter(this);
        }
        d_ptr->mInputWidgets.append(HbWidgetFilterList(widget));
    }
}

/*!
Detaches widget from region collection and updates current region if region collection is enabled.
*/
void HbInputRegionCollector::detach(HbWidget *widget)
{
    if (widget && d_ptr->mInputWidgets.contains(widget)) {
        widget->removeEventFilter(this);
        d_ptr->mInputWidgets.removeOne(widget);
        if (d_ptr->mEnabled) {
            update();
        }
    }
}

/*!
Observes size, position and move events of all attached widgets and calls update().
*/
bool HbInputRegionCollector::eventFilter(QObject *obj, QEvent *event)
{
    HbWidget *widget = qobject_cast<HbWidget *>(obj);
    if (widget) {
        switch (event->type()) {
        case QEvent::GraphicsSceneResize:
        case QEvent::GraphicsSceneMove:
            update();
            break;
        case QEvent::Show: {
            // We can not query for HbWidget visiblility at this point
            // so have to set it inside the strcuture variable.
            int pos = d_ptr->mInputWidgets.indexOf(widget);
            if (pos != -1) {
                // Temporary TODO ++
                // TODO write a HbInputWidgetStore class which will hold all the
                // active widgets and will emit signals for example sceneBlocked()
                // sceneUnBlocked(). And then connect to region collector.
                HbPopup *popup = qobject_cast<HbPopup *>(obj);
                // since there is a bug in Qt that QGraphicsItem geenrates two show events
                // once when you do a show() and once when you added it to the scene(),
                // so need a check on visibility.
                if (popup && popup->isModal() && !d_ptr->mInputWidgets[pos].mIsVisible) {
                    d_ptr->mModalDialogs++;
                }
                // Temporary TODO --
                d_ptr->mInputWidgets[pos].mIsVisible = true;
                update();
            }
            break;
        }
        case QEvent::Hide: {
            int pos = d_ptr->mInputWidgets.indexOf(widget);
            if (pos != -1) {
                // Temporary TODO ++
                HbPopup *popup = qobject_cast<HbPopup *>(obj);
                if (popup && popup->isModal()) {
                    d_ptr->mModalDialogs--;
                }
                // Temporary TODO --
                d_ptr->mInputWidgets[pos].mIsVisible = false;
                update();
            }
            break;
        }
        default:
            break;
        };
    }
    return false;
}

/*!
Calculates all the attached HbWidget's rectangle and prepares a region out of it and then
emits updateRegion.
*/
void HbInputRegionCollector::update()
{
    QRegion region;

    // since there is a modal dialog launched we need to mask the entire window.
    if (d_ptr->mModalDialogs) {
        region += QRect(QPoint(0, 0), HbDeviceProfile::current().logicalSize());
        emit updateRegion(region);
        return;
    }

    QList<HbWidgetFilterList> list = d_ptr->mInputWidgets;
    for (int i = 0; i < list.size(); ++i) {
        if (list.at(i).mIsVisible) {
            HbWidget *widget = list.at(i).mWidget;
            if (widget) {
                HbMainWindow *window = widget->mainWindow();
                if (window) {
                    QRectF rect = widget->rect();
                    rect.translate(widget->pos());
                    // we need to check transformation of the QGraphicsView.
                    // for example when there is a orientation switch transformation
                    // is 270 degree. We should map it to get transformed rectangle.
                    QTransform t = window->viewportTransform();
                    QRectF tRect = t.mapRect(rect);
                    QRectF intersection = QRectF(window->geometry()).intersected(tRect);
                    region += intersection.toRect();
                }
            }
        }
    }
    emit updateRegion(region);
}

/*!
Enables region collection, this function is called by input framework.
*/
void HbInputRegionCollector::setEnabled(bool enabled)
{
    d_ptr->mEnabled = enabled;
    QList<HbWidgetFilterList>& list = d_ptr->mInputWidgets;
    for (int i = 0; i < list.size(); ++i) {
        HbWidget *widget = list.at(i).mWidget;
        if (widget) {
            if (enabled) {
                widget->installEventFilter(this);
            } else {
                widget->removeEventFilter(this);
            }
        }
    }
}

//EOF
