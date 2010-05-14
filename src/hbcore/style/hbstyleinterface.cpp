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


#include "hbstyleinterface_p.h"


/*!
    \deprecated HbStyleInterface::deprecated()
    is deprecated. Style plugins are deprecated, stop using style plugins.  
*/
void HbStyleInterface::deprecated()
{
}

/*!
    \fn virtual int primitiveCount() const = 0;

    Return the count of primitives created / supported by this plugin.
*/

/*!
    \fn virtual QGraphicsItem *createPrimitive( HbStyle::Primitive primitive, QGraphicsItem *parent = 0 ) const = 0;

    Returns primitives created in this plugin. Note that the primitive parameter range is internal to the plugin. This means that
	you can define your plugin's primitives in your domain (for example app domain). When calling create/updatePrimitive from your custom
	widget use plugin's base id + your primitive's id. See also the documentation from HbStyle::createPrimitive.
*/

/*!
    \fn virtual void updatePrimitive( QGraphicsItem *item, HbStyle::Primitive primitive, const QStyleOption *option ) const  = 0;

	Updates the primitives defined in this plugin. See more documentation from HbStyle::updatePrimitive.
    
*/

/*!
    \fn virtual QString layoutPath() const = 0;

	Returns the path to layout definition files (CSS/WidgetML). The path can define several CSS and WidgetML files. The directory can contain
	several widgets' layout definitions. Subdirectories are not supported. The filenames in the directory must match with the widgets' class
	names. For example class called MyExampleWidget should have myexamplewidget.css and myexamplewidget.widgetml in the directory returned by layoutPath().

    
*/
