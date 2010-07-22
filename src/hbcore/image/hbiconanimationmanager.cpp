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

#include "hbiconanimationmanager.h"
#include "hbiconanimationmanager_p.h"
#include "hbiconanimationparser_p.h"
#include "hbiconloader_p.h"
#include "hbforegroundwatcher_p.h"
#include "hbinstance.h"
#include "hbmainwindow.h"
#include "hbiconanimator.h"
#include "hbiconanimator_p.h"
#include "hbtheme.h"
#include "hbiconanimation_p.h"
#include "hbimagetraces_p.h"

// Private class

/*!
\internal
*/
HbIconAnimationManagerPrivate::HbIconAnimationManagerPrivate() :
    viewChangeConnected(false)
{
    fgWatcher = HbForegroundWatcher::instance();
    connect(fgWatcher, SIGNAL(resumeAnimation()), SLOT(handleAnimationResume()));
    connect(fgWatcher, SIGNAL(stopAnimation()), SLOT(handleAnimationPause()));
    connect(hbInstance->theme(), SIGNAL(changed()), SLOT(handleThemeChanged()));
    connect(hbInstance->theme(), SIGNAL(changeFinished()), SLOT(handleThemeChangeFinished()));
}

/*!
\internal
*/
HbIconAnimationManagerPrivate::~HbIconAnimationManagerPrivate()
{
}

/*!
\internal
*/
bool HbIconAnimationManagerPrivate::addDefinitionFile(const QString &definitionFileName)
{
    // Stop right away if the file has already been added.
    QList<QString> keys = animations.keys();
    foreach(const QString & iconName, keys) {
        if (animations.operator[](iconName).definitionFileName == definitionFileName) {
            return true;
        }
    }

    // Check if there is a file with the given name in the current theme.
    // Attach the .axml suffix if needed, just for the theme lookup.
    QString fileNameWithSuffix = definitionFileName;
    if (!fileNameWithSuffix.contains('.')) {
        fileNameWithSuffix.append(QLatin1String(".axml"));
    }
    QString pathInTheme = HbIconLoader::global()->findSharedResource(fileNameWithSuffix, Hb::AnimationResource);
#ifdef HB_ICON_TRACES
    qDebug() << definitionFileName << " => " << pathInTheme;
#endif

    // In case of theme usage definitionFileName will just be a simple name and
    // realFileName will point to the name with the full path.
    const QString *realFileName = pathInTheme.isEmpty() ? &definitionFileName : &pathInTheme;

    // Parse the filename and add entries in the hash table, first try the themeserver.
    bool ret = parser.parseDefinitionFileShared(definitionFileName,
               animations, *realFileName);
    if (!ret) {
        // If themeserver did not return anything then try the file locally.
        ret = parser.parseDefinitionFile(definitionFileName,
                                         animations, *realFileName);
    }

    if (ret) {
        allDefNames.insert(definitionFileName);
    }

    return ret;
}

/*!
\internal
*/
void HbIconAnimationManagerPrivate::addDefinition(
    const QString &iconName,
    const HbIconAnimationDefinition &definition)
{
    // Remove old definition
    animations.remove(iconName);
    // Add new definition
    HbIconAnimationData data;
    data.definitionFileName.clear();
    data.def = definition;
    animations.insert(iconName, data);
}

/*!
\internal
*/
void HbIconAnimationManagerPrivate::removeDefinitionFile(const QString &definitionFileName)
{
    AnimHash::iterator i = animations.begin();
    while (i != animations.end()) {
        const HbIconAnimationData &data = *i;
        if (data.definitionFileName == definitionFileName) {
            // Remove definition from hash and move iterator to next item
            i = animations.erase(i);
        } else {
            ++i;
        }
    }
}

/*!
\internal
*/
void HbIconAnimationManagerPrivate::removeDefinition(const QString &iconName)
{
    animations.remove(iconName);
}

/*!
\internal
*/
HbIconAnimationDefinition HbIconAnimationManagerPrivate::getDefinition(const QString &iconName)
{
    // This returns default constructed list if not found in hash    
    return animations.operator[](iconName).def;
}

/*!
  \internal
*/
void HbIconAnimationManagerPrivate::handleAnimationResume()
{
    // resume all paused animations
    foreach(HbIconAnimation * anim, playingAnims) {
        if (anim->pausedDueToBackground()) {

#ifdef HB_ICON_TRACES
            qDebug() << "Foreground gained - resuming animation:" << anim->iconName();
#endif

            anim->resume();
        }
    }
}

/*!
  \internal
*/
void HbIconAnimationManagerPrivate::handleAnimationPause()
{
    // pause all playing animations
    foreach(HbIconAnimation * anim, playingAnims) {
        if (!anim->paused()) {

#ifdef HB_ICON_TRACES
            qDebug() << "Foreground lost - pausing animation:" << anim->iconName();
#endif

            anim->pause();
            anim->setPausedDueToBackground(true);
        }
    }
}

/*!
  \internal
*/
void HbIconAnimationManagerPrivate::handleThemeChanged()
{
    // The theme is being changed, stop all on-going animations.
    foreach(HbIconAnimation * anim, playingAnims) {
        anim->stop();
    }
}

/*!
  \internal
*/
void HbIconAnimationManagerPrivate::handleThemeChangeFinished()
{
    // Theme has changed, clear name list and animation data, and reload all
    // animation definitions (or at least try to reload; some defs may now be
    // missing if they were present in the previous theme but not in the new
    // one).
    QList<QString> keys = animations.keys();
    foreach(const QString & iconName, keys) {
        // Do not remove animations that were not created from files.
        if (!animations.operator[](iconName).definitionFileName.isEmpty()) {
            animations.remove(iconName);
        }
    }
    // Use allDefNames which contains all the names passed to
    // addDefinitionFile() since the creation of this instance. This
    // enables to work properly also in the following situation:
    //
    // 1. addDefinitionFile(somethingFromTheme)
    //
    // 2. Theme changes to a theme that does _not_ contain somethingFromTheme.
    //
    // 3. Theme changes to another theme that _does_ contain somethingFromTheme.
    //
    foreach(const QString & name, allDefNames) {
        addDefinitionFile(name);
    }
}

void HbIconAnimationManagerPrivate::handleViewChanged(HbView *view)
{
    // Pause all playing animations which do not belong to the new view, resume the ones belonging to it
    foreach(HbIconAnimation * anim, playingAnims) {
        // If the view of the animation is not resolved, do nothing
        if (anim->view()) {
            if (anim->view() == view) {
#ifdef HB_ICON_TRACES
                qDebug() << "View activated - resuming animation:" << anim->iconName();
#endif
                anim->resume();
            } else {
#ifdef HB_ICON_TRACES
                qDebug() << "View deactivated - pausing animation:" << anim->iconName();
#endif
                anim->pause();
            }
        }
    }
}

/*!
  \internal

  Called from HbIconAnimation to register the animation instances that are
  started.
*/
void HbIconAnimationManagerPrivate::animPlaying(HbIconAnimation *anim)
{
    if (!playingAnims.contains(anim)) {
        playingAnims.append(anim);
    }

    // Connect to view change signals if not done yet
    if (!viewChangeConnected) {
        QList<HbMainWindow *> windowList = hbInstance->allMainWindows();
        Q_FOREACH(const HbMainWindow * window, windowList) {
            connect(window, SIGNAL(currentViewChanged(HbView *)), SLOT(handleViewChanged(HbView *)));
        }
        viewChangeConnected = true;
    }

    // Resolve the view where the animation is shown if it is not resolved yet,
    // to be able to pause/resume the animation upon view switches
    if (!anim->view()) {
        HbIconAnimatorPrivate *animatorP = anim->animator()->d;

        // If view has been set explicitly in the animator, use that
        if (animatorP->view) {
            anim->setView(animatorP->view);
        } else {
            QGraphicsItem *item = animatorP->item;
            if (item) {
                QGraphicsScene *scene = item->scene();
                if (scene) {
                    // Resolve the main window having the same scene that the item belongs to
                    QList<HbMainWindow *> windowList = hbInstance->allMainWindows();
                    Q_FOREACH(const HbMainWindow * window, windowList) {
                        if (window->scene() == scene) {
                            anim->setView(window->currentView());
                            break;
                        }
                    }
                }
            }
        }
    }
}

/*!
  \internal

  Called from HbIconAnimation to deregister animation instances that are not
  playing anymore (either because they are stopped manually or because they have
  finished due to reaching the end of the animation).
*/
void HbIconAnimationManagerPrivate::animNotPlaying(HbIconAnimation *anim)
{
    int idx = playingAnims.indexOf(anim);
    if (idx != -1) {
        playingAnims.remove(idx);
    }
}

/*!
    @stable
    @hbcore
    \class HbIconAnimationManager
    \brief HbIconAnimationManager manages definitions for icon animations that are
    constructed from separate frame icons.

    Global definitions are managed by theme server.
    Definitions added in an application are local to the application.
*/

/*!
  \enum HbIconAnimationPlayMode
  \brief Play mode for the animation.

  \b PlayOnce \b (0) The animation is played once.
  When the animation is restarted, it starts from the beginning.

  \b Loop \b (1) The animation starts again from the beginning
  once it has reached its end.
*/

/*!
  \struct HbIconAnimationFrame
  \brief Defines an icon animation frame.

  \member \b iconName The name of the frame icon. This corresponds to the icon name
  parameter passed to HbIcon constructor.

  \member \b duration The duration of the animation frame in milliseconds.
*/

/*!
* Constructor. This is not needed by clients, instead use method HbIconAnimationManager::global()
* to get the global animation manager instance.
*/
HbIconAnimationManager::HbIconAnimationManager()
{
    d = new HbIconAnimationManagerPrivate;
}

/*!
* Destructor. Not needed by clients as this is a singleton class.
*/
HbIconAnimationManager::~HbIconAnimationManager()
{
    delete d;
}

/*!
* Returns the global animation manager instance.
*/
HbIconAnimationManager *HbIconAnimationManager::global()
{
// Global instance
    static HbIconAnimationManager theManager;
    return &theManager;
}

/*!
* Adds a definition file in the system for defining icon animations.
* The file has XML format. See an example of the file contents below.
*
* \code
*
* <animations>
* <icon name="animated_icon" playmode="playonce">
* <frame duration="1000">anim1_frame1</frame>
* <frame duration="1000">anim1_frame2</frame>
* <frame duration="1000">anim1_frame3</frame>
* <frame duration="1000">anim1_frame4</frame>
* </icon>
* <icon name="animated_icon2" playmode="loop">
* <frame duration="1000">anim2_frame1</frame>
* <frame duration="1000">anim2_frame2</frame>
* <frame duration="1000">anim2_frame3</frame>
* <frame duration="1000">anim2_frame4</frame>
* </icon>
* </animations>
*
* \endcode
*
* The frame names e.g. "anim1_frame1" in the example above correspond to the icon name
* parameters passed in HbIcon constructor when the animation frame icons are loaded.
*
* Any existing definitions for the same icons are overridden.
*
* Returns true if at least one animation definition was added.
*/
bool HbIconAnimationManager::addDefinitionFile(const QString &definitionFileName)
{
    return d->addDefinitionFile(definitionFileName);
}

/*!
* Adds an animation definition for a single icon.
* Any existing definition for the same icon is overridden.
* \param iconName Name of the icon.
* \param def adds the HbIconAnimationDefinition.
*/
void HbIconAnimationManager::addDefinition(
    const QString &iconName,
    const HbIconAnimationDefinition &def)
{
    d->addDefinition(iconName, def);
}

/*!
* Removes animation definitions that were added with the file named \a definitionFileName.
*/
void HbIconAnimationManager::removeDefinitionFile(const QString &definitionFileName)
{
    d->removeDefinitionFile(definitionFileName);
}

/*!
* Removes animation definition for the icon with the name \a iconName.
*/
void HbIconAnimationManager::removeDefinition(const QString &iconName)
{
    d->removeDefinition(iconName);
}

/*!
* Returns animation definition for the icon with the name \a iconName.
* If there is no animation definition for the icon, a null definition is returned.
* Normally the client does not need this definition because the animation is loaded
* by the HbIcon instance of the animated icon.
*/
HbIconAnimationDefinition HbIconAnimationManager::getDefinition(const QString &iconName)
{
    return d->getDefinition(iconName);
}

// End of File
