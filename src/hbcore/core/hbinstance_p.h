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

#ifndef HBINSTANCE_P_H
#define HBINSTANCE_P_H

#include <hbinstance.h>
#include "hbdeviceprofile.h"

class HbTypefaceInfo;

class TestabilityInterface;

#ifdef Q_OS_SYMBIAN
#include <centralrepository.h>
#include <systemtoneservice.h>

const TUid HBTESTABILITY_CREPO_ID  = {0x2002C3AE};
const TUint32 HbTestabilityKey  = 0x1;
#endif

class HbLocaleChangeNotifier;
class CSystemToneService;

class HbInstancePrivate : public QObject
{
    Q_OBJECT

public:
    HbInstancePrivate();
    ~HbInstancePrivate();
    void addWindow(HbMainWindow *window);
    bool removeWindow(HbMainWindow *window);
    void select(const HbDeviceProfile &display);
    HbDeviceProfile profile();
    CSystemToneService *systemTone();
    void initLibraryPaths();
    void startLocalizationMeasurement();

public slots:
    void updateScenes();
#ifdef HB_SETTINGS_WINDOW
    void showHideSettingsWindow();
#endif
#ifdef HB_CSS_INSPECTOR
    void showHideCssWindow();
#endif
    void doLocalizationMeasurements();

public:
    QList<HbMainWindow *> mWindows;
    mutable HbTypefaceInfo *mTypefaceInfo;
    HbStyle *mStyle;
    HbTheme *mTheme;
    Qt::Orientation mOrientation;
    HbDeviceProfile mCurrentProfile;
    QStringList *mLibraryPaths;
    bool mDropHiddenIconData;

public:
    HbTypefaceInfo *typefaceInfo() const;

    static HbInstancePrivate *d_ptr() {
        return HbInstance::instance()->d;
    }

signals:
    void windowAdded(HbMainWindow *window);
    void windowRemoved(HbMainWindow *window);

private:
    TestabilityInterface *testabilityInterface;
#ifdef Q_OS_SYMBIAN
    CRepository *mRepo;
    bool testabilityEnabled;
    CSystemToneService *mSts;
#endif //Q_OS_SYMBIAN

    HbLocaleChangeNotifier *mLocaleChangeNotifier;

#ifdef HB_TEXT_MEASUREMENT_UTILITY
    HbDeviceProfile mLocalizationMetricsProfile;
    bool mLocalizationMeasurementPending;
#endif

    friend class HbStyle;
};

#endif // HBINSTANCE_P_H
