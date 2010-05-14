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

#ifndef HBPLUGINLOADER_P_H
#define HBPLUGINLOADER_P_H

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

#include <QPluginLoader>
#include <hbglobal.h>

class HbPluginLoaderPrivate;

class HB_CORE_PRIVATE_EXPORT HbPluginLoader : public QPluginLoader
{
    Q_OBJECT

public:
    HbPluginLoader(QObject *parent = 0);
    HbPluginLoader(const QString &pluginName, QObject *parent = 0);
    ~HbPluginLoader();

    QString pluginName() const;
    void setPluginName(const QString &pluginName);

    QObject *instance();
    bool load();

    bool resolveFileName();

protected:
    HbPluginLoaderPrivate * const d_ptr;

private:
    Q_DISABLE_COPY(HbPluginLoader)
    Q_DECLARE_PRIVATE_D(d_ptr, HbPluginLoader)
};

#endif // HBPLUGINLOADER_P_H
