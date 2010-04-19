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

#include <QtGlobal>

#ifndef Q_CC_MINGW
#include <objbase.h>
#include <QDebug>

#include "hbwmihelper_win_p.h"

WMIHelper::WMIHelper(QObject * parent)
        : QObject(parent)
{
   m_conditional = QString();
}

WMIHelper::~WMIHelper()
{
    //wbemServices->Release();
    //wbemLocator->Release();
    CoUninitialize();
}

QVariant WMIHelper::getWMIData()
{
   if (!m_wmiNamespace.isEmpty() && !m_className.isEmpty() && !m_classProperties.isEmpty()) {
      return getWMIData(m_wmiNamespace, m_className, m_classProperties);
   }
   return QVariant();
}

void WMIHelper::initializeWMI(const QString &wmiNamespace)
{
    HRESULT hres;
    wbemLocator = 0;

    hres = CoCreateInstance(CLSID_WbemLocator,0,CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator, (LPVOID *) &wbemLocator);

    if (hres == CO_E_NOTINITIALIZED) { // COM was not initialized
        CoInitializeEx(0, COINIT_MULTITHREADED);
        hres = CoCreateInstance(CLSID_WbemLocator,0,CLSCTX_INPROC_SERVER,
                                IID_IWbemLocator, (LPVOID *) &wbemLocator);
    }

    if (hres != S_OK) {
       qWarning() << "Failed to create IWbemLocator object." << hres;
        return ;
    }
    wbemServices = 0;
    hres = wbemLocator->ConnectServer(::SysAllocString((const OLECHAR *)wmiNamespace.utf16()),0,0,0,0,0,0,&wbemServices);

    if (hres != WBEM_S_NO_ERROR){
        qWarning() << "Could not connect";
        return ;
    }

    hres = CoSetProxyBlanket( wbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, 0,
                              RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, 0, EOAC_NONE );

    if (hres != S_OK) {
       qWarning() << "Could not set proxy blanket" << hres;
        return ;
    }

    if (!initializedNamespaces.contains(wmiNamespace)) {
        initializedNamespaces.insert(wmiNamespace, true);
    } else {
        initializedNamespaces[wmiNamespace] = true;
    }
}

QVariant WMIHelper::getWMIData(const QString &wmiNamespace, const QString &className, const QStringList &classProperty)
{
    if(!initializedNamespaces.contains(wmiNamespace)) {
        initializeWMI(wmiNamespace);
    }

    HRESULT hres;
    QVariant returnVariant;

    IEnumWbemClassObject* wbemEnumerator = 0;
    if (!m_conditional.isEmpty()) {
        if (m_conditional.left(1) != " ") {
            m_conditional.prepend(" ");
        }
    }

    QString aString = "SELECT * FROM " + className + m_conditional;
    BSTR bstrQuery;
    bstrQuery = ::SysAllocString((const OLECHAR *)aString.utf16());

    hres = wbemServices->ExecQuery(L"WQL", bstrQuery,
            WBEM_FLAG_BIDIRECTIONAL | WBEM_FLAG_RETURN_IMMEDIATELY,0,&wbemEnumerator);

    if (hres != WBEM_S_NO_ERROR){
        qWarning() << "WMI Query failed.";
        wbemLocator->Release();
        wbemEnumerator->Release();
        return returnVariant;
    }

    ::SysFreeString(bstrQuery);

    wbemCLassObject = 0;
    ULONG result = 0;

    wmiVariantList.clear();
    while (wbemEnumerator) {
        HRESULT hr = wbemEnumerator->Next(WBEM_INFINITE, 1,&wbemCLassObject, &result);
        if(0 == result){
            break;
        }

        foreach (QString property, classProperty) {
            VARIANT msVariant;
            CIMTYPE variantType;
            hr = wbemCLassObject->Get((LPCWSTR)property.utf16(), 0, &msVariant, &variantType, 0);
            returnVariant = msVariantToQVariant(msVariant, variantType);
            wmiVariantList << returnVariant;

            VariantClear(&msVariant);
        }
        wbemCLassObject->Release();
    }

    wbemEnumerator->Release();
    return returnVariant;
}

QVariant WMIHelper::msVariantToQVariant(VARIANT msVariant, CIMTYPE variantType)
{
    QVariant returnVariant;
    switch(variantType) {
    case CIM_STRING:
    case CIM_CHAR16:
        {
            QString str((QChar*)msVariant.bstrVal, wcslen(msVariant.bstrVal));
            QVariant vs(str);
            returnVariant = vs;
        }
        break;
    case CIM_BOOLEAN:
        {
            QVariant vb(msVariant.boolVal);
            returnVariant = vb;
        }
        break;
    case CIM_UINT8:
        {
            QVariant vb(msVariant.uintVal);
            returnVariant = vb;
        }
        break;
    case CIM_UINT16:
        {
            QVariant vb(msVariant.uintVal);
            returnVariant = vb;
        }
            case CIM_UINT32:
        {
            QVariant vb(msVariant.uintVal);
            returnVariant = vb;
        }
        break;
    };
    VariantClear(&msVariant);
    return returnVariant;
}

void WMIHelper::setWmiNamespace(const QString &wmiNamespace)
{
    m_wmiNamespace = wmiNamespace;
}

void WMIHelper::setClassName(const QString &className)
{
    m_className = className;
}

void WMIHelper::setClassProperty(const QStringList &classProperties)
{
    m_classProperties = classProperties;
}

void WMIHelper::setConditional(const QString &conditional)
{
    m_conditional = conditional;
}

#endif
