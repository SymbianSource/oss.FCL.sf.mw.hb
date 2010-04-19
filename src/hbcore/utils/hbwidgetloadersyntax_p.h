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

#ifndef HBWIDGETLOADERSYNTAX_P_H
#define HBWIDGETLOADERSYNTAX_P_H

#include "hbxmlloaderabstractsyntax_p.h"
#include "hbxmlloaderabstractactions_p.h"
#include "hbwidgetloaderactions_p.h"
#include <hbglobal.h>

QT_BEGIN_NAMESPACE
class QIODevice;
QT_END_NAMESPACE
class HbWidget;
class HbWidgetLoaderPrivate;

class HB_CORE_PRIVATE_EXPORT HbWidgetLoaderSyntax : public HbXmlLoaderAbstractSyntax
{

public:

    HbWidgetLoaderSyntax( HbWidgetLoaderActions *actions );

    virtual ~HbWidgetLoaderSyntax();
    
    bool load( QIODevice *device, const QString &name, const QString &section );
    
    static QString version();
    
        
public:
    Q_DISABLE_COPY(HbWidgetLoaderSyntax)

    ElementType elementType( QStringRef name ) const;
    bool processLayout();
    bool processDocument();
    bool readLayoutStartItem();
    
public:
    
    HbWidgetLoaderActions *mRealActions;

    QString mLayoutName;

    bool mLayoutFound;
};

#endif // HBWIDGETLOADERSYNTAX_P_H
