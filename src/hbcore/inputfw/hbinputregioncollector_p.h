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

#ifndef HB_INPUT_REGION_COLLECTOR_H
#define HB_INPUT_REGION_COLLECTOR_H

#include "hbinputdef.h"
#include "hbwidget.h"

#include <QObject>
#include <QRegion>
#include <QPointer>

class HbWidget;
class HbInputRegionCollectorPrivate;

class HB_CORE_PRIVATE_EXPORT HbInputRegionCollector : public QObject
{
    Q_OBJECT
public:
    static HbInputRegionCollector *instance();
    ~HbInputRegionCollector();

    void attach(HbWidget *widget);
    void detach(HbWidget *widget);
    void setEnabled(bool enabled);
    void update();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

signals:
    void updateRegion(QRegion region);

private:
    HbInputRegionCollector();
    HbInputRegionCollectorPrivate *const d_ptr;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputRegionCollector)
    Q_DISABLE_COPY(HbInputRegionCollector)
    friend class HbInputMainWindowPrivate;
};

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

#endif // HB_INPUT_REGION_COLLECTOR_H
