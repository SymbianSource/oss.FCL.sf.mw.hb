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

#ifndef HBSTYLEINTERFACE_H
#define HBSTYLEINTERFACE_H

#include <hbglobal.h>
#include <hbstyle.h>
class HbWidgetBase;

// DEPRECATED API (style plugins are deprecated)
// Interface used with HbStyle plugins, see HbStyle documentation for more information
class HbStyleInterface
{
public:
    virtual ~HbStyleInterface() {}
    
    virtual int primitiveCount() const = 0;
    virtual QGraphicsItem *createPrimitive( HbStyle::Primitive primitive, QGraphicsItem *parent = 0 ) const = 0;
    virtual void updatePrimitive( QGraphicsItem *item, HbStyle::Primitive primitive, const QStyleOption *option ) const  = 0;
    virtual QString layoutPath() const = 0;
    void deprecated();
};

Q_DECLARE_INTERFACE(HbStyleInterface, "com.nokia.Hb.StyleInterface")

#endif // HBSTYLEINTERFACE_H
