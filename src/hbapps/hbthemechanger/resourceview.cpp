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

#include <hbmainwindow.h>
#include <hbaction.h>
#include <hbtoolbar.h>
#include <hbdataform.h>
#include <hbdataformmodel.h>
#include <hbdataformmodelitem.h>
#include <QGraphicsLinearLayout>
#include <hbthemeutils_p.h>
#include <hbcombobox.h>
#include <hbmessagebox.h>
#include <hbiconitem.h>
#include <hbthemeutils_p.h>
#include <QDir>
#include "resourceview.h"
#include <hbsearchpanel.h>
#include <hbtumbleview.h>
#include <hblabel.h>
#include <hbcolorscheme.h>
#include <hbiconanimator.h>
#include <hbiconanimationmanager.h>

/**
 * Constructor
 */
ResourceView::ResourceView(HbMainWindow *mainWindow, HbView* parent): HbView(parent)
{
    mMainWindow = mainWindow;
    mParentView = parent;
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);

    this->setTitle(tr("Resource lookup"));

    HbAction *act = new HbAction(Hb::BackNaviAction);
    connect(act, SIGNAL(triggered()), this, SLOT(close()));

    this->setNavigationAction(act);

    act = new HbAction(tr("Icons"));
        connect(act, SIGNAL(triggered()), this, SLOT(iconModeSelected()));
        this->toolBar()->addAction(act);

    act = new HbAction(tr("Colors"));
        connect(act, SIGNAL(triggered()), this, SLOT(colorModeSelected()));
        this->toolBar()->addAction(act);

    act = new HbAction(tr("Effects"));
        connect(act, SIGNAL(triggered()), this, SLOT(effectModeSelected()));
        this->toolBar()->addAction(act);

    act = new HbAction(tr("Animations"));
        connect(act, SIGNAL(triggered()), this, SLOT(animationModeSelected()));
        this->toolBar()->addAction(act);


    loadThemedIcons();
    loadThemedColors();
    loadThemedEffects();
    loadThemedAnimations();
    mAnimator = new HbIconAnimator();
    mResourceItem = new HbLabel(this);
    mResourceItem->setIcon(HbIcon(mThemedIcons.size()?mThemedIcons[0]:""));
    mResourceItem->setAlignment(Qt::AlignCenter);
    mResourceItem->setFontSpec(HbFontSpec(HbFontSpec::Secondary));
    mResourcesList = new HbTumbleView(mThemedIcons, this);
    mResourcesList->setLoopingEnabled(true);

    layout->addItem(mResourceItem);
    layout->addItem(mResourcesList);

    mSearchPanel = new HbSearchPanel(this);
    mSearchPanel->setCancelEnabled(false);
    mSearchPanel->setPlaceholderText(tr("Search"));
    layout->addItem(mSearchPanel);
    this->setLayout(layout);

    connect(mSearchPanel, SIGNAL(criteriaChanged(const QString &)), this, SLOT(criteriaChanged(const QString &)));
    connect(mResourcesList, SIGNAL(itemSelected(int)), this, SLOT(iconItemSelected(int)));
    toggleMode(iconMode);

    mMainWindow->addView(this);
}

/**
 * Destructor
 */
ResourceView::~ResourceView()
{
    delete mAnimator;
}

void ResourceView::close()
{
    mMainWindow->setCurrentView(mParentView);
}

void ResourceView::toggleMode(ListMode mode)
{
    if (mode == mMode) {
        return;
    }

    switch (mMode) {
    case colorMode:
        disconnect(mResourcesList, SIGNAL(itemSelected(int)), this, SLOT(colorItemSelected(int)));
        break;
    case iconMode:
        disconnect(mResourcesList, SIGNAL(itemSelected(int)), this, SLOT(iconItemSelected(int)));
        break;
    case effectMode:
        mResourcesList->setSelected(0);
        disconnect(mResourcesList, SIGNAL(itemSelected(int)), this, SLOT(effectItemSelected(int)));
        break;
    case animationMode:
        mResourcesList->setSelected(0);
        disconnect(mResourcesList, SIGNAL(itemSelected(int)), this, SLOT(animationItemSelected(int)));
        break;
    default:
        break;
    }

    mMode = mode;
    switch (mode) {
    case colorMode:
        connect(mResourcesList, SIGNAL(itemSelected(int)), this, SLOT(colorItemSelected(int)));
        mSearchPanel->setCriteria("");
        mResourcesList->setSelected(0);
        break;
    case iconMode:
        connect(mResourcesList, SIGNAL(itemSelected(int)), this, SLOT(iconItemSelected(int)));
        mSearchPanel->setCriteria("");
        mResourcesList->setSelected(0);
        break;
    case effectMode:
        connect(mResourcesList, SIGNAL(itemSelected(int)), this, SLOT(effectItemSelected(int)));
        mSearchPanel->setCriteria("");
        mResourcesList->setSelected(0);
        break;
    case animationMode:
        connect(mResourcesList, SIGNAL(itemSelected(int)), this, SLOT(animationItemSelected(int)));
        mSearchPanel->setCriteria("");
        mResourcesList->setSelected(0);
        break;
    default:
        break;
    }
}

void ResourceView::loadThemedIcons()
{
    mThemedIcons.clear();
    QString basetheme = HbThemeUtils::getThemeSetting(HbThemeUtils::BaseThemeSetting);
    QDir scalableIconsDir(basetheme + "/scalable");
    QDir pixmapIconsDir(basetheme + "/pixmap");
    QFileInfoList files = scalableIconsDir.entryInfoList(QDir::Files);
    files += pixmapIconsDir.entryInfoList(QDir::Files);
    // Strip the file extensions
    for (int i=0;i<files.count();i++) {
       mThemedIcons += files[i].baseName();
    }
}

void ResourceView::loadThemedEffects()
{
    mThemedEffects.clear();
    QString basetheme = HbThemeUtils::getThemeSetting(HbThemeUtils::BaseThemeSetting);
    basetheme = basetheme.replace("/icons/", "/effects/");
    QDir effectsDir(basetheme);
    QFileInfoList files = effectsDir.entryInfoList(QDir::Files);
    // Strip the file extensions
    for (int i=0;i<files.count();i++) {
       mThemedEffects += files[i].baseName();
    }
}

void ResourceView::loadThemedAnimations()
{
    mThemedAnimations.clear();
    QString basetheme = HbThemeUtils::getThemeSetting(HbThemeUtils::BaseThemeSetting);
    basetheme = basetheme.replace("/icons/", "/animations/");
    QDir animationsDir(basetheme);
    QFileInfoList files = animationsDir.entryInfoList(QDir::Files);
    // Add animation definitions and strip the file extensions
    for (int i=0;i<files.count();i++) {
       HbIconAnimationManager::global()->addDefinitionFile(files[i].fileName());
       mThemedAnimations += files[i].baseName();
    }
}

void ResourceView::loadThemedColors()
{
    mThemedColors.clear();
    QString basetheme = HbThemeUtils::getThemeSetting(HbThemeUtils::BaseThemeSetting);
    basetheme = basetheme.replace("/icons/", "/style/");
    QDir colorsDir(basetheme + "/variables/color");
    QFileInfoList files = colorsDir.entryInfoList(QDir::Files);
    for (int i=0;i<files.count();i++) {
        QFile file(colorsDir.absolutePath() + '/' +files[i].fileName());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);

            while(!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line.startsWith("qtc_")) {
                    // Extract logical name
                    mThemedColors += line.mid(0, line.indexOf(':')).trimmed();
                }
            }
            file.close();
        }
    }
}

void ResourceView::iconModeSelected()
{
    toggleMode(iconMode);
}

void ResourceView::colorModeSelected()
{
    toggleMode(colorMode);
}

void ResourceView::effectModeSelected()
{
    toggleMode(effectMode);
}

void ResourceView::animationModeSelected()
{
    toggleMode(animationMode);
}


void ResourceView::iconItemSelected(int index)
{
    mResourceItem->setIcon(HbIcon(mResourcesList->items()[index]));
}

void ResourceView::animationItemSelected(int index)
{
    mResourceItem->setIcon(HbIcon(mResourcesList->items()[index]));
    mAnimator->setIcon(mResourceItem->icon());
    mAnimator->startAnimation();
}


void ResourceView::colorItemSelected(int index)
{
    QString rgbValue("#");
    rgbValue += QString::number(HbColorScheme::color(mResourcesList->items()[index]).red(), 16);
    rgbValue += QString::number(HbColorScheme::color(mResourcesList->items()[index]).green(), 16);
    rgbValue += QString::number(HbColorScheme::color(mResourcesList->items()[index]).blue(), 16);
    mResourceItem->setPlainText(rgbValue.toUpper());
}

void ResourceView::effectItemSelected(int index)
{
    HbEffect::add(mResourceItem,mResourcesList->items()[index],"selected");
    HbEffect::start(mResourceItem,"selected");
}


void ResourceView::criteriaChanged(const QString &criteria)
{
    QStringList filteredItems;
    switch (mMode) {
    case iconMode:
        filteredItems = mThemedIcons.filter(criteria.toLower());
        break;
    case colorMode:
        filteredItems = mThemedColors.filter(criteria.toLower());
        break;
    case effectMode:
        filteredItems = mThemedEffects.filter(criteria.toLower());
        break;
    case animationMode:
        filteredItems = mThemedAnimations.filter(criteria.toLower());
        break;
    default:
        break;
    }
    mResourcesList->setItems(filteredItems);
}
