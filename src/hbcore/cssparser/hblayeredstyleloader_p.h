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

#ifndef HBLAYEREDSTYLELOADER_P_H
#define HBLAYEREDSTYLELOADER_P_H

#include <hbglobal.h>
#include <hbcssparser_p.h>
#include <hbstyleselector_p.h>

#include <QVector>
#include <QDir>

class HB_AUTOTEST_EXPORT HbLayeredStyleLoader
{
public:
    enum LayerPriority {
        Priority_Core,
        Priority_Theme,
        Priority_AppTheme,
        Priority_Application,
        Priority_Operator
    };

    struct Layer {
		HbStyleSelector styleSelector;
    };
    
    enum Concern {
        Concern_Layouts = 0,
        Concern_Colors = 1,
        // Concern_All must be last!
        Concern_All = 0xFFFF
    };
    
    typedef QMapIterator<LayerPriority, HbLayeredStyleLoader::Layer> LayerIterator;
    typedef QMap<LayerPriority, Layer> PriorityLayerMap;
    typedef QMap<Concern, HbLayeredStyleLoader> ConcernStacks;
    
public:
    static HbLayeredStyleLoader *getStack(Concern con);
    inline static bool sharingNeeded(LayerPriority layerPriority){
        return (layerPriority >= Priority_AppTheme && layerPriority <=  Priority_Application )? false : true;
    }

    int load(const QString &fileName, LayerPriority priority, bool enableBinarySupport =false);
    int load(QIODevice *device, LayerPriority priority);
    QVector<int> loadAll(const QStringList &files, LayerPriority priority);
    QVector<int> loadDir(const QString &dirPath, QDir::SortFlags sort, LayerPriority priority);

    bool unload(int handle, LayerPriority priority);
    void unload(const QVector<int> &handles,  LayerPriority priority);

    void clear();
    void clear(LayerPriority priority);
    bool findInDefaultVariables( const QString& variableName, HbCss::Value &val ) const;

public:
    bool hasOrientationSpecificStyleRules(HbStyleSelector::NodePtr node) const;
    HbVector<HbCss::Declaration> declarationsForNode(HbStyleSelector::NodePtr node,
            const Qt::Orientation orientation, const char *extraPseudo = 0) const;
    HbVector<HbCss::StyleRule> styleRulesForNode(HbStyleSelector::NodePtr node,
			const Qt::Orientation orientation) const;
    HbVector<HbCss::Declaration> variableRuleSets() const;
    
private:
    QVector<LayerPriority> LayerList() const;
    bool loadBinary(const QString& fileName,HbCss::StyleSheet *styleSheet);
    bool saveBinary(const QString& fileName,HbCss::StyleSheet *styleSheet);
    void saveDeclarations(QDataStream & stream,HbVector<HbCss::Declaration>* decls );
    HbVector<HbCss::Declaration> loadDeclarations(QDataStream & stream );
    void saveStyleRules(QDataStream &stream, const HbVector<HbCss::StyleRule> *rules);
    void loadStyleRules(QDataStream &stream, HbVector<HbCss::StyleRule> &rules);
    bool isBinaryFile(const QString& fileName);
    QString getBinaryFileName(const QString& fileName);

private:
    PriorityLayerMap mStyleLayers;
    Concern mConcern;
    mutable QMap<QString,HbCss::Value> defaultVariablesMap;
};

#endif // HBLAYEREDSTYLELOADER_P_H
