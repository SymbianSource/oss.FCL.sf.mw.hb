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

#ifndef HBSTYLE_P_H
#define HBSTYLE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Hb API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <QHash>
#include <QVector>
#include <hbstyle.h>
#include "hbcssparser_p.h"

class HbWidget;
class HbPluginLoader;
class HbStyleInterface;
class HbWidgetBasePrivate;

class HbStylePluginInfo
{
public:    
    int primitiveBaseId;
    int primitiveCount;
    int refCount;
};

class HbStyleInterfaceInfo
{
public: 
    HbPluginLoader* loader;
    int primitiveBaseId;
};

static QHash<const QGraphicsWidget*, QString> widgetLayoutNames;

class HbStylePrivate
{
    Q_DECLARE_PUBLIC( HbStyle )
public:
    HbStylePrivate();
    virtual ~HbStylePrivate();

    QString logicalName(HbStyle::Primitive primitive, const QStyleOption *option) const;
    QIcon::Mode iconMode(QStyle::State state) const;
    QIcon::State iconState(QStyle::State state) const;
	HbStyleInterface *stylePluginInterface( HbStyle::Primitive primitive ) const;

    void polishItem(const HbVector<HbCss::StyleRule> &styleRules, HbWidget *widget,
		QGraphicsItem *item, const QString &name, bool layoutDefined) const;
    void updateThemedItems( const HbVector<HbCss::StyleRule> &styleRules,
		QGraphicsItem *item ) const;

    void ensureLayoutParameters(const HbDeviceProfile &profile) const;

    void clearStyleSheetCaches();

    HbWidgetBasePrivate *widgetBasePrivate(HbWidgetBase *widgetBase) const;

    HbStyle* q_ptr;

    mutable QHash<int, HbStyleInterfaceInfo*> customPrimitives;
    mutable QHash<QString, HbStylePluginInfo*> registeredPlugins;
    mutable QHash<QString, QString> pluginStylePaths;
    mutable int nextAvailableId;

    mutable QHash<QString, HbCss::Declaration> layoutParameters;
    mutable QString layoutParametersProfileName;

    mutable QHash<QString, HbVector<HbCss::StyleRule> > styleRulesCache;
};

#endif // HBSTYLE_P_H
