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

#ifndef WMIHELPER_H
#define WMIHELPER_H

#ifndef Q_CC_MINGW
#include <QObject>
#include <QVariant>
#include <QString>
#include <Wbemidl.h>

#include <QStringList>

class WMIHelper : public QObject
{
public:
    WMIHelper(QObject *parent = 0);
    ~WMIHelper();
    QVariant getWMIData();
    QVariant getWMIData(const QString &wmiNamespace,const QString &className, const QStringList &classProperties);
    QList <QVariant> wmiVariantList;
    void setWmiNamespace(const QString &wmiNamespace);
    void setClassName(const QString &className);
    void setClassProperty(const QStringList &classProperties);
    void setConditional(const QString &conditional); //see WQL SQL for WMI)

private:
    IWbemLocator *wbemLocator;
    IWbemServices *wbemServices;
    IWbemClassObject *wbemCLassObject;

    QString m_className;
    QStringList m_classProperties;
    QString m_conditional;
    QString m_wmiNamespace;
    QVariant  msVariantToQVariant(VARIANT msVariant, CIMTYPE variantType);
    void initializeWMI(const QString &wmiNamespace);
    QHash <QString, bool> initializedNamespaces;
};
#endif

#endif // WMIHELPER_H
