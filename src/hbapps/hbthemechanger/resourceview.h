/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbApps module of the UI Extensions for Mobile.
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
#ifndef RESOURCEVIEW_H
#define RESOURCEVIEW_H

#include <hbview.h>

class HbMainWindow;
class HbDataForm;
class HbDataFormModelItem;
class HbIconItem;
class HbTumbleView;
class HbSearchPanel;
class HbLabel;
class HbIconAnimator;

class ResourceView : public HbView
{
Q_OBJECT
public:
    ResourceView(HbMainWindow *mainWindow, HbView* parent=0);
    ~ResourceView();

    enum ListMode {
        iconMode,
        colorMode,
        effectMode,
        animationMode
    };

public slots:
    void close();
    void iconModeSelected();
    void colorModeSelected();
    void effectModeSelected();
    void animationModeSelected();
    void iconItemSelected(int index);
    void colorItemSelected(int index);
    void effectItemSelected(int index);
    void animationItemSelected(int index);
    void criteriaChanged(const QString &criteria);
private:
    HbMainWindow *mMainWindow;
    HbView *mParentView;

    QStringList mThemedIcons;
    QStringList mThemedColors;
    QStringList mThemedEffects;
    QStringList mThemedAnimations;
    HbLabel *mResourceItem;
    HbTumbleView *mResourcesList;
    HbSearchPanel *mSearchPanel;
    HbIconAnimator *mAnimator;

    ListMode mMode;
    void loadThemedIcons();
    void loadThemedColors();    
    void loadThemedEffects();
    void loadThemedAnimations();
    void toggleMode(ListMode mode);
};


#endif // RESOURCEVIEW_H
