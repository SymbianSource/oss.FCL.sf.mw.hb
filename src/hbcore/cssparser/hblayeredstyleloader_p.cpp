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
#include "hblayeredstyleloader_p.h"
#include "hbwidgetstyleloader_p.h"
#ifndef HB_BIN_CSS
#include <hbwidget.h>
#endif

#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QtDebug>

//#define LAYEREDSTYLELOADER_DEBUG
#ifndef HB_BIN_CSS
#define HB_USETHEMESERVER
#endif

#include "hbvector_p.h"
#include "hbmemorymanager_p.h"
#include "hbmemoryutils_p.h"
#ifdef HB_USETHEMESERVER
#include "hbthemeclient_p.h"
#endif
#include "hbtheme.h"
#ifndef HB_BIN_CSS
#include "hbtheme_p.h"
#endif
#include "hbthemeperf_p.h"
#include "hbcssformatter_p.h"

const QString CSS_FILTER = "*.css";
const QString GLOBAL_SELECTOR = "*";

/*!
    \proto
    \class HbLayeredStyleLoader
    \brief allows discrete collections of stylesheet definitions to be layered using a priority 
    system to enforce the sequence of stylesheet definitions

    HbLayeredStyleLoader allows stylesheet files to be loaded and unloaded into separate 
    layers, specified by a priority value. Any stylesheet definitions loaded will be then be 
    accessed by subsequent attempts to apply widget styles using HbStyle, (in cases where 
    the style depends on the defined stylesheet properties).

    For any stylesheet definitions loaded that correspond to similar properties defined using other 
    priority values, the properties loaded with the largest numerical value will take precedence 
    according to the normal rules for stylesheet precedence. For example, selectors that are at
    least equally specific and define the same properties, and are in a layer with higher priority, 
    will override those equivalent property definitions with a lower priority value.

    When a file containing a stylesheet is loaded, a handle is returned, which can then 
    be used to unload the file. The client should store this handle in a member variable 
    if it needs to unload the stylesheet whilst running. If not, the stylesheet will automatically 
    be destroyed on application exit.

    \note that clients of the HbStyleLoader api will automatically be loaded into this class 
    using the HbLayeredStyleLoader::Priority_Application priority. That api is provided as a 
    wrapper to this API, to enforce that application styles are layered correctly alongside 
    platform, theme, and opertor layers, as appropriate.

    \sa HbStyleLoader

*/

Q_GLOBAL_STATIC(HbLayeredStyleLoader::ConcernStacks, globalConcernStacks)

/*!
    Returns a static instance of a stack related to the given concern
    
    \param con Concern identifying the stack to retrieve
    \return The HbLayeredStyleLoader stack for the specified concern
 */
HbLayeredStyleLoader *HbLayeredStyleLoader::getStack(Concern con)
{
    HbLayeredStyleLoader *result = 0;
    
    ConcernStacks *stacks = globalConcernStacks();
    if (stacks) {
        if (!stacks->contains(con)) {
            (*stacks)[con].mConcern = con;
        }
        result = &((*stacks)[con]);
    }
    
    return result;
}


/*!
    Loads the specified file, and appends the resulting selectors to 
    the appropriate list. 
    
    \note Effects will not be noticed until the stylesheets are 
    subequently accessed.

    \note The effect is cumulative, so that attempting to load the same file multiple 
    times will cause unnecessary wasted memory, as the previous definitions will still be 
    loaded in memory, but will never be successfully selected.

    \param fileName, name of the styleSheet file to be loaded
    \param the priority of the layer to be used, selected from LayerPriority
    \return the handle to be used to unload the corresponding stylesheet
 */
int HbLayeredStyleLoader::load(
    const QString &fileName, LayerPriority priority)
{

    HB_START_TIME();
#ifdef LAYEREDSTYLELOADER_DEBUG
    qDebug() << "HbLayeredStyleLoader::load called for" << fileName;
#endif
    Layer &layer = mStyleLayers[priority];
    HbCss::StyleSheet* styleSheet = 0;

    // Check for the availability of the file, as QFile::Exists takes more time this method is used
    QFile file(fileName);
    bool fileExists = file.open(QIODevice::ReadOnly);
    file.close();
    //if file doesn't exist no need to proceed further
    if (fileExists) {
        //if sharing is required based on Layer Priority, e.g. Core, Operator, Theme, 
        //get the shared stylesheet from the theme client, which in turn fetches it 
        // from the server.
        if (sharingNeeded(priority)) {
            
#ifdef LAYEREDSTYLELOADER_DEBUG
            QTime time;
            time.start();
#endif
#ifdef HB_USETHEMESERVER
            styleSheet = HbThemeClient::global()->getSharedStyleSheet(fileName,priority);
#endif
#ifdef LAYEREDSTYLELOADER_DEBUG
            qDebug() << "Time elapsed in getting the shared stylesheet "<< fileName << " is : %d ms" <<time.elapsed();
#endif
        }
        // fallback incase of themeserver could not open the file or OOM condition in SharedMemory.
        if (!styleSheet) {
            //if sharing of stylesheet is not required, it means stylesheet is app specific
            //so it won't be loaded from the server
            styleSheet = HbMemoryUtils::create<HbCss::StyleSheet>(HbMemoryManager::HeapMemory);

            HbCss::Parser parser;
            parser.init(fileName, true);
            if (!parser.parse(styleSheet)) {
                HbMemoryUtils::release<HbCss::StyleSheet>(styleSheet);
                styleSheet = 0;
            }
        }
    }

    int handle = 0;
    if (styleSheet) {
        layer.styleSelector.addStyleSheet(styleSheet);
        handle = reinterpret_cast<qptrdiff>(layer.styleSelector.styleSheets.last());
    }

    HB_END_TIME("loadtime for filename: ",fileName);
#ifdef LAYEREDSTYLELOADER_DEBUG
    qDebug("Handle returned: %x", handle);
#endif

    updateLayersListIfRequired(priority);
    return handle;
}

/*!
    Loads the contents of the specified device, and appends the resulting selectors to 
    the appropriate list. 

    \sa load(const QString &fileName, int priority)

    \param device, device containing the styleSheet file to be loaded
    \param the priority of the layer to be used, selected from LayerPriority
    \return the handle to be used to unload the corresponding stylesheet
*/
int HbLayeredStyleLoader::load(QIODevice *device, LayerPriority priority)
{
    int handle = 0;

    if (device->isReadable()) {
        Layer &layer = mStyleLayers[priority];
        QTextStream stream(device);
        QString contents = stream.readAll();

        HbCss::Parser parser;
        parser.init(contents, false);
        HbCss::StyleSheet* styleSheet = 
            HbMemoryUtils::create<HbCss::StyleSheet>(HbMemoryManager::HeapMemory);

        if (parser.parse(styleSheet)) {
            layer.styleSelector.addStyleSheet(styleSheet);
            handle = reinterpret_cast<qptrdiff>(layer.styleSelector.styleSheets.last());
        }
    }

    updateLayersListIfRequired(priority);
    return handle;
}

/*!
    Unloads the stylesheet that was loaded using the supplied handle. If the handle cannot 
    be found, no action is taken.

    \note If other loaded stylesheets, either preceding or following the specified sheet, are still 
    loaded, then they will continue to operate.

    \note The effects will not be noticed until other related stylesheets are subsequently parsed.

    \param handle the handle to be used to unload the corresponding stylesheet, must be a value 
    that was returned from a corresponding load() operation using the same value of \a priority
    \param the priority of the layer that was used during load()
    \return true if the handle was found the stylesheet successfully unloaded
 */
bool HbLayeredStyleLoader::unload(int handle, LayerPriority priority)
{
#ifdef LAYEREDSTYLELOADER_DEBUG
    qDebug("HbLayeredStyleLoader::unload called for handle %x", handle);
#endif
    bool removed = false;
    if (mStyleLayers.contains(priority)) {
        Layer &layer = mStyleLayers[priority];
        for (int i=0; i<layer.styleSelector.styleSheets.count(); i++) {
            int curHandle = reinterpret_cast<qptrdiff>(layer.styleSelector.styleSheets.at(i));
            if (curHandle == handle) {
                HbCss::StyleSheet* sheet = layer.styleSelector.styleSheets.at(i);
                layer.styleSelector.removeStyleSheet(sheet);
                removed = true;
                break;
            }
        }
    }
    return removed;
}


/*!
    Loads the contents of the specified directory, and appends the resulting selectors to 
    the appropriate list using the specified sort order.

    \sa load(const QString &fileName, int priority)

    \param dirPath, path containing the styleSheet files to be loaded
    \param sort the sort order for the specified directory
    \param the priority of the layer to be used, selected from LayerPriority
    \return the handles to be used to unload the corresponding stylesheets
*/
QVector<int> HbLayeredStyleLoader::loadDir(const QString &dirPath, QDir::SortFlags sort,
        LayerPriority priority)
{
    QVector<int> fileList;

    QDir dir(dirPath, CSS_FILTER, sort, QDir::Readable | QDir::Files);
    QStringList files = dir.entryList();
    for (QStringList::Iterator it = files.begin(); it != files.end(); ++it) {
        QString cssPath = dirPath;
        if (!dirPath.endsWith(QDir::separator())) {
            cssPath.append(QDir::separator());
        }
        cssPath.append(*it);
        int result = load(cssPath, priority);
        if (result) {
            fileList.append(result);
        }
    }

    return fileList;
}


/*!
    Loads the contents of each filename specified in the string list, and
    appends the resulting selectors to the appropriate list.

    \sa load(const QString &fileName, int priority)

    \param files, list of fully qualified paths of styleSheet files to be loaded
    \param the priority of the layer to be used, selected from LayerPriority
    \return the handles to be used to unload the corresponding stylesheets
*/
QVector<int> HbLayeredStyleLoader::loadAll(const QStringList &files, LayerPriority priority)
{
    QVector<int> fileList;

    QStringListIterator iter(files);
    while (iter.hasNext()) {
        int result = load(iter.next(), priority);
        if (result) {
            fileList.append(result);
        }
    }

    return fileList;
}

/*!
    Unloads all stylesheets that are specified from the layer with a specific priority.

    \note The effects will not be noticed until other related stylesheets are subsequently parsed.

    \note If this is not performed, the stylesheets will be automatically deleted on application 
    shutdown.

    \sa unload(int handle, int priority)

    \param handles handles that were returned from a call to load() or loadDir() with the same 
    value for \a priority
    \param priority the priority of the layer from which to unload the specified stylesheets
 */
void HbLayeredStyleLoader::unload(const QVector<int> &handles, LayerPriority priority)
{
    if (mStyleLayers.contains(priority)) {
        int files = handles.count();
        for (int file=0; file<files; file++) {
            int handle = handles.at(file);
            unload(handle, priority);
        }
    }
}

/*!
    Unload all stylesheets from all layers

    \note If this is not performed, all loaded stylesheets will be deleted on destruction.
*/
void HbLayeredStyleLoader::clear()
{
    mStyleLayers.clear();

    HbWidgetStyleLoader *loader = HbWidgetStyleLoader::instance();
    loader->clearConcernFileList(mConcern);
}


/*!
    Unload all stylesheets from the layer with the specified priority.

    \sa clear()

    \param priority the priority of the layer to unload
*/
void HbLayeredStyleLoader::clear(LayerPriority priority)
{
    if (mStyleLayers.contains(priority)) {
        mStyleLayers.remove(priority);

        HbWidgetStyleLoader *loader = HbWidgetStyleLoader::instance();
        loader->clearLayerFileList(mConcern, priority);
    }
}


static inline bool qcss_selectorStyleRuleLessThan(
    const QPair<int, HbCss::StyleRule> &lhs, const QPair<int, HbCss::StyleRule> &rhs)
{
    return lhs.first < rhs.first;
}

static inline bool qcss_selectorDeclarationLessThan(
    const QPair<int, HbCss::Declaration> &lhs, const QPair<int, HbCss::Declaration> &rhs)
{
    return lhs.first < rhs.first;
}


bool HbLayeredStyleLoader::hasOrientationSpecificStyleRules(HbStyleSelector::NodePtr node) const
{
    HbWidget* widget = (HbWidget*) node.ptr;
    HbWidgetStyleLoader *loader = HbWidgetStyleLoader::instance();
    loader->loadCss(widget);
    
    QVector<QVector<HbCss::WeightedRule> > weightedRulesList;
    HbLayeredStyleLoader *allStack = mConcern == Concern_All ? 0 : getStack(Concern_All);

    QListIterator<LayerPriority> iter(mUsedLayers);
    while (iter.hasNext()) {
        LayerPriority priority = iter.next();
        QMap<LayerPriority, Layer>::const_iterator it = mStyleLayers.constFind(priority);
        if (it != mStyleLayers.constEnd()) {
            if (it->styleSelector.hasOrientationSpecificStyleRules(node)) {
                return true;
            }
        }
        if (allStack) {
            QMap<LayerPriority, Layer>::const_iterator allIt = 
                allStack->mStyleLayers.constFind(priority);
            if (allIt != allStack->mStyleLayers.constEnd()) {
                if (allIt->styleSelector.hasOrientationSpecificStyleRules(node)) {
                    return true;
                }
            }
        }
    }
    return false;
}

/*!
    Provides the declarations matching the specified node, or optionally a subcontrol of the node

    \param node the node 
    \param extraPseudo a string corresponding to the subcontrol of the node
    \return declarations
*/
HbVector<HbCss::Declaration> HbLayeredStyleLoader::declarationsForNode(
    HbStyleSelector::NodePtr node,
    const Qt::Orientation orientation, 
    const char *extraPseudo) const
{
    HbWidget* widget = (HbWidget*) node.ptr;
    HbWidgetStyleLoader *loader = HbWidgetStyleLoader::instance();
    loader->loadCss(widget);
    
    QVector<QVector<HbCss::WeightedDeclaration> > weightedDeclsList;
    HbLayeredStyleLoader *allStack = mConcern == Concern_All ? 0 : getStack(Concern_All);

    QListIterator<LayerPriority> iter(mUsedLayers);
    while (iter.hasNext()) {
        LayerPriority priority = iter.next();
        QMap<LayerPriority, Layer>::const_iterator it = mStyleLayers.constFind(priority);
        if (it != mStyleLayers.constEnd()) {
            weightedDeclsList.append(
                it->styleSelector.weightedDeclarationsForNode(node, orientation, extraPseudo));
        }
        if (allStack) {
            QMap<LayerPriority, Layer>::const_iterator allIt = 
                allStack->mStyleLayers.constFind(priority);
            if (allIt != allStack->mStyleLayers.constEnd()) {
                weightedDeclsList.append(
                    allIt->styleSelector.weightedDeclarationsForNode(
                        node, 
                        orientation, 
                        extraPseudo));
            }
        }
    }
    
    QVector<HbCss::WeightedDeclaration> weightedDecls;
    int count = 0;
    for (int i=0; i<weightedDeclsList.count(); i++) {
        count += weightedDeclsList.at(i).count();
    }
    weightedDecls.reserve(count);
    for (int i=0; i<weightedDeclsList.count(); i++) {
        weightedDecls += weightedDeclsList.at(i);
    }
    qStableSort(weightedDecls.begin(), weightedDecls.end(), qcss_selectorDeclarationLessThan);

    HbVector<HbCss::Declaration> decls;
    for (int j = 0; j < weightedDecls.count(); j++)
        decls += weightedDecls.at(j).second;
    return decls;
}

/*!
    Provides the style rules matching the specified node

    \param node the node 
    \return style rules
*/
HbVector<HbCss::StyleRule> HbLayeredStyleLoader::styleRulesForNode(HbStyleSelector::NodePtr node,
        const Qt::Orientation orientation) const
{
    HbWidget* widget = (HbWidget*) node.ptr;
    HbWidgetStyleLoader *loader = HbWidgetStyleLoader::instance();
    loader->loadCss(widget);
    
    QVector<QVector<HbCss::WeightedRule> > weightedRulesList;
    HbLayeredStyleLoader *allStack = mConcern == Concern_All ? 0 : getStack(Concern_All);

    QListIterator<LayerPriority> iter(mUsedLayers);
    while (iter.hasNext()) {
        LayerPriority priority = iter.next();
        QMap<LayerPriority, Layer>::const_iterator it = mStyleLayers.constFind(priority);
        if (it != mStyleLayers.constEnd()) {
            weightedRulesList.append(
                it->styleSelector.weightedStyleRulesForNode(node, orientation));
        }
        if (allStack) {
            QMap<LayerPriority, Layer>::const_iterator allIt = 
                allStack->mStyleLayers.constFind(priority);
            if (allIt != allStack->mStyleLayers.constEnd()) {
                weightedRulesList.append(
                    allIt->styleSelector.weightedStyleRulesForNode(node, orientation));
            }
        }
    }
    
    QVector<HbCss::WeightedRule> weightedRules;
    int count = 0;
    for (int i=0; i<weightedRulesList.count(); i++) {
        count += weightedRulesList.at(i).count();
    }
    weightedRules.reserve(count);
    for (int i=0; i<weightedRulesList.count(); i++) {
        weightedRules += weightedRulesList.at(i);
    }
    qStableSort(weightedRules.begin(), weightedRules.end(), qcss_selectorStyleRuleLessThan);

    HbVector<HbCss::StyleRule> rules;
#ifdef LAYEREDSTYLELOADER_DEBUG
    qDebug() << "Style rules for" << widget->metaObject()->className();
    qDebug("\n%s", HbCssFormatter::weightedStyleRulesToString(weightedRules).toLatin1().constData());
#endif
    rules.reserve(count);
    for (int j = 0; j < weightedRules.count(); j++) {
        rules += weightedRules.at(j).second;
    }
    return rules;
}


/*!
     Provides the variable rule sets for the loaded CSS files.
*/
void HbLayeredStyleLoader::variableRuleSets(QHash<QString, HbCss::Declaration> *variables) const
{
    HbLayeredStyleLoader *allStack = mConcern == Concern_All ? 0 : getStack(Concern_All);
    
    QListIterator<LayerPriority> iter(mUsedLayers);
    while (iter.hasNext()) {
        LayerPriority priority = iter.next();
        QMap<LayerPriority, Layer>::const_iterator it = mStyleLayers.constFind(priority);
        if (it != mStyleLayers.constEnd()) {
            if (priority != HbLayeredStyleLoader::Priority_Core) {
                if (it != mStyleLayers.constEnd()) {
                    it->styleSelector.variableRuleSets(variables);
                }
            } else {
                // These variables are from Core Priority
                // insert it into hash to be used during look up, this happens only once
                // next time onwards instead of comparing each value from list, it's looked from 
                // this hash only.
                if (!mDefaultVariables.count()) {
                    it->styleSelector.variableRuleSets(&mDefaultVariables);
                }
            }
        }
        if (allStack) {
            QMap<LayerPriority, Layer>::const_iterator allIt = 
                allStack->mStyleLayers.constFind(priority);
            if (allIt != allStack->mStyleLayers.constEnd()) {
                allIt->styleSelector.variableRuleSets(variables);
            }
        }
    }
}

/*!
     Finds the variable from core layer. The found varibale is returned in val
     
     \return True if it finds in default colorgroup.css, false otherwise
*/
bool HbLayeredStyleLoader::findInDefaultVariables(
    const QString& variableName, 
    HbCss::Value &val) const
{
    bool found = false;
    if (mDefaultVariables.contains(variableName)) {
        val = mDefaultVariables.value(variableName).values.first();
        found = true;
    }
    return found;
}


/*!
     Updates the cached list of used layers to include the specified layer.
     If this is the All stack, all other stacks' used lists are also updated 
     to include this layer.
     
     \param The LayerPriority to add if not already present
*/
void HbLayeredStyleLoader::updateLayersListIfRequired(LayerPriority priority)
{
    if (mUsedLayers.contains(priority)) {
        return;
    }
    if (mConcern != Concern_All) {
        mUsedLayers.append(priority);
        qSort(mUsedLayers);
    } else {
        ConcernStacks *stacks = globalConcernStacks();
        if (stacks) {
            QMap<Concern, HbLayeredStyleLoader>::iterator iter = stacks->begin();
            while (iter != stacks->end()) {
                if (!iter->mUsedLayers.contains(priority)) {
                    iter->mUsedLayers.append(priority);
                    qSort(iter->mUsedLayers);
                }
                ++iter;
            }
        }
    }
}
