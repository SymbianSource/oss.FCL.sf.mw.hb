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

#ifndef HBWIDGETSTYLELOADER_P_H
#define HBWIDGETSTYLELOADER_P_H


#include <hbglobal.h>
#include <hblayeredstyleloader_p.h>

class HbWidget;

class HB_CORE_PRIVATE_EXPORT HbWidgetStyleLoader
{
public:
	
    static HbWidgetStyleLoader *instance();

	bool addFilePath(const QString &filePath,
        const HbLayeredStyleLoader::Concern concern,
        const HbLayeredStyleLoader::LayerPriority priority);
	bool removeFilePath(const QString &filePath,
        const HbLayeredStyleLoader::Concern concern,
        const HbLayeredStyleLoader::LayerPriority priority);


	void clearConcernFileList(const HbLayeredStyleLoader::Concern concern);
	void clearLayerFileList(
        const HbLayeredStyleLoader::Concern concern, 
		const HbLayeredStyleLoader::LayerPriority priority);
	
	void loadCss(const HbWidget *widget);
	
	bool loadWidgetML(
        HbWidget *widget, const QString &layoutName, const QString &section = QString());
	
private:

    enum FileSetType {
        FileSetType_Unknown,
        FileSetType_Pattern,
        FileSetType_CSS,
        FileSetType_WidgetML
    };

    bool doAddFileSet(
        const QString &path,
        FileSetType type,
        const HbLayeredStyleLoader::Concern concern,
		const HbLayeredStyleLoader::LayerPriority priority);

	bool doRemoveFileSet(
        const QString &path,
        const HbLayeredStyleLoader::Concern *concern = 0,
		const HbLayeredStyleLoader::LayerPriority *priority = 0);

    struct FileSet {
		QString path;
        FileSetType type;
		HbLayeredStyleLoader::Concern concern;
		HbLayeredStyleLoader::LayerPriority priority;
		QHash<QString,int> loadedCss;
        QStringList missedCss;
        int referenceCount;
		
		FileSet(const QString &pat,
                FileSetType typ,
                const HbLayeredStyleLoader::Concern con,
				const HbLayeredStyleLoader::LayerPriority pri) {
			path = pat;
            type = typ;
			concern = con;
			priority = pri;
            referenceCount = 1;
		}
	};
	
	QList<FileSet> mFileSets;
    QVector<uint> mFullyLoadedWidgets;
};

#endif // HBWIDGETSTYLELOADER_P_H
