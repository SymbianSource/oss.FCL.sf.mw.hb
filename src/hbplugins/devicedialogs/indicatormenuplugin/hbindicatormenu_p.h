/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbPlugins module of the UI Extensions for Mobile.
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

#ifndef HBINDICATORMENU_P_H
#define HBINDICATORMENU_P_H

#include "hbnamespace_p.h"

#include <QObject>
#include <QVariantMap>

#include <hbdevicedialog.h>
#include <hbdevicedialoginterface.h>
#include <hbdevicedialogplugin.h>

#include "hbdialog.h"

class HbDialogPrivate;
class HbIndicatorInterface;
class HbLabel;

class HbIndicatorMenu : public HbDialog, public HbDeviceDialogInterface
{
    Q_OBJECT

public:
    HbIndicatorMenu(const QVariantMap &parameters);

    HbDeviceDialogPlugin::DeviceDialogGroup deviceDialogGroup() const;
    int deviceDialogPriority() const;
    HbDeviceDialogPlugin::DeviceDialogFlags deviceDialogFlags() const;
    bool setDeviceDialogParameters(const QVariantMap &parameters);
    int deviceDialogError() const;
    void closeDeviceDialog(bool byClient);
    HbPopup *deviceDialogWidget() const;

    enum { Type = HbPrivate::ItemType_IndicatorMenu };
    int type() const { return Type; }

public slots:
    void userActivity();
    void userActivityStarted();
    void userActivityEnded();

    void indicatorsActivated(QList<HbIndicatorInterface*> activatedIndicators);
    void indicatorActivated(HbIndicatorInterface *activatedIndicator);
    void indicatorDeactivated(HbIndicatorInterface *indicator);

private:
    bool constructMenu(const QVariantMap &parameters);
    bool checkProperties(const QVariantMap &parameters);
    void setProperties(const QVariantMap &parameters);
    void resetProperties();
    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);
    void doMenuLayout();

signals:
    void deviceDialogClosed();
    void deviceDialogData(QVariantMap data);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbDialog)
    Q_DISABLE_COPY(HbIndicatorMenu)

    int mLastError;
    bool mShowEventReceived;
};

#endif // HBINDICATORMENU_P_H
