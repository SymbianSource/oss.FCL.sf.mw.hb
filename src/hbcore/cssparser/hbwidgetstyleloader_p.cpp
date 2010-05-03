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

#include <hbstyle.h>

#include "hbinstance_p.h"
#include "hbstyle_p.h"

#include "hbwidgetstyleloader_p.h"
#include "hbwidgetloader_p.h"
#include "hblayeredstyleloader_p.h"
#include "hbstyleloader.h"
#include "hbcolortheme_p.h"
#include "hbwidget.h"

//#define WIDGETSTYLELOADER_DEBUG

#ifdef WIDGETSTYLELOADER_DEBUG
#include <QDebug>
#endif

static const QString WIDGETML_EXTENSION = ".widgetml";
static const QString CSS_EXTENSION = ".css";
static const QString PATTERN_EXTENSION = "%w";
static const QString HBWIDGETBASE_CLASSNAME = "HbWidgetBase";

/*!
    \proto
    \class HbWidgetStyleLoader
    \brief allows collections of widget-specific CSS files to be automatically loaded when needed

	HbWidgetStyleLoader allows widget-specific CSS files to be automatically loaded and unloaded
	into the layered CSS system on demand. CSS style rules should be split into separate files
	based on the widget they apply to, and named after the widget they apply to (in lower case).
	
	Multiple FileSets can be active at any given time, and apply to any layer of any stack.
	
	A list of class names that CSS has been loaded for is maintained for each FileSet, and the parent 
	hierarchy is navigated up to HbWidget to load CSS for all parent classes.
	
	\note When a FileSet is removed, the corresponding layer of the CSS stack will be cleared.
	Any other FileSets active on the same layer will have their loaded list cleared, and required
	CSS will be reloaded at the next polish event, but any CSS files manually loaded to the layer 
	will be cleared without warning. It is therefore recommended to only load CSS either manually 
	or via this class for any given layer of the stack.

	\sa HbLayeredStyleLoader

*/


Q_GLOBAL_STATIC(HbWidgetStyleLoader, globalWidgetStyleLoader)

/*!
    Returns a static instance of a stack related to the given concern
    
    \return The global HbWidgetStyleLoader instance
 */
HbWidgetStyleLoader* HbWidgetStyleLoader::instance()
{
	return globalWidgetStyleLoader();
}



/*!
    Add file
 */
bool HbWidgetStyleLoader::addFilePath(const QString &filePath,
                                  const HbLayeredStyleLoader::Concern concern,
                                  const HbLayeredStyleLoader::LayerPriority priority)
{
    if ( filePath.isEmpty() ) {
        return false;
    }
    FileSetType type(FileSetType_Unknown);
    if (filePath.endsWith(CSS_EXTENSION)) {
        QFileInfo fileInfo(filePath);
        if ( !fileInfo.exists() ) {
            return false;
        }
        type = FileSetType_CSS;
    } else if (filePath.endsWith(WIDGETML_EXTENSION)) {
        QFileInfo fileInfo(filePath);
        if ( !fileInfo.exists() ) {
            return false;
        }
        type = FileSetType_WidgetML;
    } else if (filePath.contains(PATTERN_EXTENSION)) {
        type = FileSetType_Pattern;
    } else {
        QDir fileDir(filePath);
        if( fileDir.exists() ) {
            // go through all the style files in the directory
            QStringList nameFilters;
            nameFilters << "*.css";
            nameFilters << "*.widgetml";
            QFileInfoList files = fileDir.entryInfoList(nameFilters);
            if (files.count()) {
                bool success( true );
                foreach(QFileInfo file, files) {
                    success &= addFilePath( file.absoluteFilePath(), concern, priority );
                }
                return success;
            }
        }
        return false;
    }
    return doAddFileSet(filePath, type, concern, priority);
}

/*!
    Remove file
 */
bool HbWidgetStyleLoader::removeFilePath(const QString &filePath,
                                     const HbLayeredStyleLoader::Concern concern,
                                     const HbLayeredStyleLoader::LayerPriority priority)
{
    if ( filePath.isEmpty() ) {
        return false;
    }
    if (filePath.endsWith(CSS_EXTENSION)
        || filePath.endsWith(WIDGETML_EXTENSION)
        || filePath.contains(PATTERN_EXTENSION)) {
            return doRemoveFileSet(filePath, &concern, &priority);
    } else {
        QDir fileDir(filePath);
        if( fileDir.exists() ) {
            // go through all the style files in the directory
            QStringList nameFilters;
            nameFilters << "*.css";
            nameFilters << "*.widgetml";
            QFileInfoList files = fileDir.entryInfoList(nameFilters);
            bool success( true );
            foreach(QFileInfo file, files) {
                success &= removeFilePath( file.absoluteFilePath(), concern, priority );
            }
            return success;
        } else {
            return false;
        }
    }
}



/*!
    Adds a filter for a set of files to the system. Next time declarations (or style rules)
    are requested for a widget, the CSS for that widget (and parent types) will be loaded
    into the specified layer based on the filename pattern given.
    
    The replacement string "%w" will be substituted for the widget class name, and ".css" or
	".widgetml" will be appended when loading appropriate files. For example,
    the pathPattern:
    ":/css/widgets/%w" 
    would become:
    ":/css/widgets/hbpushbutton.css"
    when declarationsForNode (or styleRulesForNode) is called on a HbPushButton. %w can occur
    multiple times in the pattern, but note that it will be replaced by the class name in
    lower case.
    
    \note If an absolute path pattern is specified (without containing "%w"), the same file will 
    be loaded multiple times (for each widget type, whether it applies or not) and cause degraded 
    performance. 
    
    \note The pathPattern is treated as a unique identifier, so only one FileSet can be created
    for any given path.
	
    \param pathPattern, path to load CSS files from, with %w representing the widget class name
	\param concern, the CSS concern stack to load the CSS files into
	\param priority, the layer of the CSS stack to load the files into
    \return whether the addition was successful
 */
/*
bool HbWidgetStyleLoader::addFileSet(const QString &pathPattern, const HbLayeredStyleLoader::Concern concern, 
		const HbLayeredStyleLoader::LayerPriority priority)
{
    return doAddFileSet(pathPattern, FileSetType_Pattern, concern, priority);
}
*/

/*!
    \internal
 */
bool HbWidgetStyleLoader::doAddFileSet(const QString &path,
                                       FileSetType type,
                                       const HbLayeredStyleLoader::Concern concern, 
                                       const HbLayeredStyleLoader::LayerPriority priority)
{
	for(int i=0; i<mFileSets.count(); i++){
		if(mFileSets.at(i).path == path){
#ifdef WIDGETSTYLELOADER_DEBUG
			qDebug() << "Fileset already exists with path" << path;
#endif
            if ( mFileSets.at(i).concern == concern
                && mFileSets.at(i).priority == priority ) {
                mFileSets[i].referenceCount++;
#ifdef WIDGETSTYLELOADER_DEBUG
                qDebug() << "Increased reference count to" << mFileSets[i].referenceCount;
#endif
    			return true;
            } else {
#ifdef WIDGETSTYLELOADER_DEBUG
                qDebug() << "Concern and/or priority do not match";
#endif
                return false;
            }
		}
	}
	
#ifdef WIDGETSTYLELOADER_DEBUG
	qDebug() << "Adding fileset for filename pattern" << path << "with concern" << concern << "and priority" << priority;
#endif
	FileSet set(path, type, concern, priority);
	mFileSets.append(set);

    // Style can be null in hbInstance construction face. 
    if (HbInstancePrivate::d_ptr()->mStyle) {
        HbInstancePrivate::d_ptr()->mStyle->d_func()->clearStyleSheetCaches();
    }
    if ( concern == HbLayeredStyleLoader::Concern_Colors 
         || concern == HbLayeredStyleLoader::Concern_All) {
        HbColorTheme::instance()->flushVariableCache();
    }

#ifdef WIDGETSTYLELOADER_DEBUG
	qDebug() << "WidgetStyleLoader now contains" << mFileSets.count() << "filters";
#endif
	return true;
}




/*!
    Removes a filter for a set of files from the system. This will cause the layer that the
    path pattern applied to to be cleared, and any CSS in that layer will be unloaded. 
    
    \note If any CSS has been manually loaded into the same layer as the specified fileset, it
    will be unloaded without warning. It is therefore recommended to keep CSS files loaded manually
    and via this class in separate layers.
	
    \param pathPattern, path to load CSS files from, with %w representing the widget class name
    \return whether the removal was successful - returns false if the pathPattern is not found
 */
/*
bool HbWidgetStyleLoader::removeFileSet(const QString &pathPattern)
{
    return doRemoveFileSet(pathPattern);
}
*/

/*!
    \internal
 */
bool HbWidgetStyleLoader::doRemoveFileSet(
        const QString &path,
        const HbLayeredStyleLoader::Concern *concern,
		const HbLayeredStyleLoader::LayerPriority *priority)
{
#ifdef WIDGETSTYLELOADER_DEBUG
	qDebug() << "Removing fileset for filename pattern" << path;
#endif
	bool found = false;
	
	for(int i=0; i<mFileSets.count(); i++){
        FileSet &fs = mFileSets[i];
		if(fs.path == path &&
            (!concern || fs.concern == *concern) &&
            (!priority ||fs.priority == *priority)){
            fs.referenceCount--;
            if (!fs.referenceCount) {
#ifdef WIDGETSTYLELOADER_DEBUG
                qDebug() << "Fileset removed";
#endif
			    if(fs.loadedCss.count()){
				    HbLayeredStyleLoader *stack = HbLayeredStyleLoader::getStack(fs.concern);
				    if(stack){
                        QList<int> handles = fs.loadedCss.values();
                        foreach( int handle, handles ) {
                            stack->unload(handle, *priority);
                        }
                    }
			    }
    			mFileSets.removeAt(i);
                if (HbInstancePrivate::d_ptr()->mStyle) {
                    HbInstancePrivate::d_ptr()->mStyle->d_func()->clearStyleSheetCaches();
                }
                if ( concern && (*concern == HbLayeredStyleLoader::Concern_Colors || 
                                 *concern == HbLayeredStyleLoader::Concern_All)) {
                    HbColorTheme *colorThemeInstance = HbColorTheme::instance();
                    if (colorThemeInstance) {  // Check that the instance has not been destroyed.
                        colorThemeInstance->flushVariableCache();
                    }
                }
            }
#ifdef WIDGETSTYLELOADER_DEBUG
            else {
                qDebug() << "Decreased reference count to" << mFileSets[i].referenceCount;
            }
#endif
            found = true;
			break;
		}
	}
	
	return found;
}




/*!
    Attempts to load CSS for the given widget (and all parent classes) based on the currently
    existing filesets. Note that a list of loaded CSS files is stored, so calling this function
    on a widget whose CSS is already loaded will NOT cause duplicate CSS.
	Failed attempts to load a file will also be cached to avoid repeated filesystem access requests.
	
    \param widget, Widget to load CSS for
 */
void HbWidgetStyleLoader::loadCss(const HbWidget *widget)
{
	if(!widget){
		return;
	}
#ifdef WIDGETSTYLELOADER_DEBUG
	qDebug() << "loadCssForWidget called for " << widget->metaObject()->className();
#endif
	for(int i=0; i<mFileSets.count(); i++){
        if ( !(mFileSets.at(i).type == FileSetType_Pattern ||
            mFileSets.at(i).type == FileSetType_CSS ) ) {
                continue;
        }
		HbLayeredStyleLoader *stack = HbLayeredStyleLoader::getStack(mFileSets[i].concern);
		if(!stack){
			continue;
		}
        if (mFileSets[i].type == FileSetType_CSS) {
            if (mFileSets[i].loadedCss.count() || mFileSets[i].missedCss.count()) {
#ifdef WIDGETSTYLELOADER_DEBUG
			    qDebug() << "Already loaded CSS from file" << mFileSets[i].path;
#endif
            } else {
			    int hand = stack->load(mFileSets[i].path, mFileSets[i].priority);
			    if (hand) {
#ifdef WIDGETSTYLELOADER_DEBUG
				    qDebug() << "Successfully loaded CSS from file" << mFileSets[i].path;
#endif
				    mFileSets[i].loadedCss.insert("", hand);
			    } else {
#ifdef WIDGETSTYLELOADER_DEBUG
				    qDebug() << "Failed to load CSS from file" << mFileSets[i].path;
#endif
                    mFileSets[i].missedCss.append("");
			    }
            }
            continue;
        }
        // Else -> pattern file set
		const QMetaObject *meta = widget->metaObject();
		QString className = meta->className();
		while(className != HBWIDGETBASE_CLASSNAME){
			className = meta->className();
			meta = meta->superClass();
			if (!meta) {
				break;
			}
            if (mFileSets[i].loadedCss.contains(className)){
#ifdef WIDGETSTYLELOADER_DEBUG
				qDebug() << "CSS already loaded for" << className;
#endif
				continue;
			}
            if (mFileSets[i].missedCss.contains(className)){
#ifdef WIDGETSTYLELOADER_DEBUG
				qDebug() << "CSS already searched but missed for" << className;
#endif
				continue;
            }			
			QString lowerClass = className.toLower();
			QString filename = mFileSets[i].path;
			filename.replace(PATTERN_EXTENSION, lowerClass);
			filename.append(CSS_EXTENSION);
#ifdef WIDGETSTYLELOADER_DEBUG
			qDebug() << "Loading CSS for widget" << className << "from file" << filename;
#endif
			int hand = stack->load(filename, mFileSets[i].priority);
			if (hand) {
#ifdef WIDGETSTYLELOADER_DEBUG
				qDebug() << "Successfully loaded CSS for" << className;
#endif
				mFileSets[i].loadedCss.insert(className, hand);
			} else {
#ifdef WIDGETSTYLELOADER_DEBUG
				qDebug() << "Failed to load CSS for" << className;
#endif
                mFileSets[i].missedCss.append(className);
			}
		}
	}
}


/*!
 * 
    This function attempts to load the specified layout from a WidgetML file with the matching 
    widget name (matched by the %w substitution in the pathPatterns of loaded filesets). 
	If the file is not found or the layout name does not exist in the file, the pathPattern is 
	re-substituted with the parent widget name and loading is attempted again.
    
    \sa addFileSet
    
    \param widget, Widget to load layout for. If successful, the widget's layout variable will be modified.
    \param layoutName, The layout name (previously extracted from CSS) to look for
    \param sectionName, The section name (previously extracted from CSS) to look for, if not defined then use the first section found
 */
bool HbWidgetStyleLoader::loadWidgetML(HbWidget *widget, const QString &layoutName, const QString &sectionName)
{
	if(!widget){
		return false;
	}
	bool loaded = false;
	
#ifdef WIDGETSTYLELOADER_DEBUG
	qDebug() << "Looking for layout" << layoutName << "in section" << sectionName;
#endif

	// Iterate backwards to ensure filesets override correctly (eg landscape delta overrides portrait)
	int c = mFileSets.count();
	while(c-- && !loaded){
        if ( !(mFileSets.at(c).type == FileSetType_Pattern ||
            mFileSets.at(c).type == FileSetType_WidgetML ) ) {
                continue;
        }
		const QMetaObject *meta = widget->metaObject();
		QString className = meta->className();
#ifdef WIDGETSTYLELOADER_DEBUG
		qDebug() << "Checking fileset " << mFileSets[c].path;
#endif
		while(className != HBWIDGETBASE_CLASSNAME){
			QString lowerClass = className.toLower();
			QString filename = mFileSets[c].path;
            bool attemptToLoad(true);
            if ( mFileSets[c].type == FileSetType_Pattern ) {
			    filename.replace(PATTERN_EXTENSION, lowerClass);
			    filename.append(WIDGETML_EXTENSION);
            } else {
                QString classFilename( lowerClass );
                classFilename.append(WIDGETML_EXTENSION);
                if (!filename.endsWith(classFilename)) {
#ifdef WIDGETSTYLELOADER_DEBUG
    				qDebug() << "Not a WidgetML file" << filename << "for class" << className;
#endif
                    attemptToLoad = false;
                }
#ifdef WIDGETSTYLELOADER_DEBUG
                else {
    				qDebug() << "Attempting to load WidgetML from non-pattern file" << filename;
                }
#endif
            }
			
            if ( attemptToLoad ) {
			    static HbWidgetLoader loader;
#ifdef WIDGETSTYLELOADER_DEBUG
			    qDebug() << "Attempting to load file " << filename;
#endif
                HbMemoryManager::MemoryType type =
                    HbLayeredStyleLoader::sharingNeeded(mFileSets[c].priority)
                    ? HbMemoryManager::SharedMemory
                    : HbMemoryManager::HeapMemory;
                if(loader.load(widget, filename, layoutName, sectionName, type)) {
				    loaded = true;
				    break;
			    }
            }
			
			meta = meta->superClass();
			if(!meta){
				break;
			}
			className = meta->className();
		}
	}

	return loaded;
}


/*!
	For any filesets loaded with the given concern, the list of loaded CSS files is cleared causing 
	them to be reloaded on the nextstyleRulesForNode/declarationsForNode call. This should only be 
	called from the LayeredStyleLoader's clear function, to keep this 'cache' up to date.
    
    \param concern, Concern which has been cleared
 */
void HbWidgetStyleLoader::clearConcernFileList(const HbLayeredStyleLoader::Concern concern)
{
#ifdef WIDGETSTYLELOADER_DEBUG
	qDebug() << "Clearing list for stack" << concern;
#endif
	for(int i=0; i<mFileSets.count(); i++){
		if(mFileSets.at(i).concern == concern){
			mFileSets[i].loadedCss.clear();
            mFileSets[i].missedCss.clear();
#ifdef WIDGETSTYLELOADER_DEBUG
	qDebug() << "Cleared fileset" << i;
#endif
		}
	}
}


/*!
	For any filesets loaded with the given concern and priority, the list of loaded CSS files is 
	cleared causing them to be reloaded on the next styleRulesForNode/declarationsForNode call. 
	This should only be called from the LayeredStyleLoader's clear function, to keep this 'cache' 
	up to date.
    
    \param concern, Concern of layer which has been cleared
    \param priority, Priority of layer which has been cleared
 */
void HbWidgetStyleLoader::clearLayerFileList(const HbLayeredStyleLoader::Concern concern, 
		const HbLayeredStyleLoader::LayerPriority priority)
{
#ifdef WIDGETSTYLELOADER_DEBUG
	qDebug() << "Clearing list for stack" << concern << "and layer" << priority;
#endif
	for(int i=0; i<mFileSets.count(); i++){
		if(mFileSets.at(i).concern == concern && mFileSets.at(i).priority == priority){
			mFileSets[i].loadedCss.clear();
            mFileSets[i].missedCss.clear();
#ifdef WIDGETSTYLELOADER_DEBUG
			qDebug() << "Cleared fileset" << i;
#endif
		}
	}
}
