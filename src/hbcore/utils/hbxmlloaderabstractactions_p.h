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


#ifndef HBXMLLOADERABSTRACTACTIONS_P_H
#define HBXMLLOADERABSTRACTACTIONS_P_H

#include <hbxmlloaderabstractsyntax_p.h>

#include <hbglobal.h>
#include <hbnamespace.h>

#include <QGraphicsWidget>
#include <QGraphicsLayout>
#include <QPointer>


// Uncomment the following in order to get additional debug prints
//#define HB_DOCUMENTLOADER_DEBUG
 
#ifndef HB_DOCUMENTLOADER_DEBUG
#define HB_DOCUMENTLOADER_PRINT(a) 
#else
#include <QDebug>
#define HB_DOCUMENTLOADER_PRINT(a) qDebug() << QString(a);
#endif // HB_DOCUMENTLOADER_DEBUG

struct HbXmlLengthValue;

class HB_CORE_PRIVATE_EXPORT HbXmlLoaderAbstractActions
{        
    public:    
    
        HbXmlLoaderAbstractActions();
        virtual ~HbXmlLoaderAbstractActions();
        
        virtual void reset();
        virtual void cleanUp();
        virtual void deleteAll();

        virtual bool pushDocument( const QString& context);
        virtual bool pushObject( const QString& type, const QString &name );
        virtual bool pushWidget(
            const QString &type,
            const QString &name,
            const QString &role,
            const QString &plugin );
        virtual bool pushSpacerItem( const QString &name, const QString &widget );
        virtual bool pushConnect(
            const QString &srcName,
            const QString &signalName,
            const QString &dstName,
            const QString &slotName );
        virtual bool pushProperty( const char *propertyName, const HbXmlVariable &variable );
        virtual bool pushRef( const QString &name, const QString &role );
        virtual bool pushContainer(
            const char *propertyName,
            HbXmlLoaderAbstractSyntax::DocumentLexems type,
            const QList<HbXmlVariable*> &container );
        
        virtual bool pop( const HbXml::ElementType type );

        virtual bool setContentsMargins(
            const HbXmlLengthValue &left,
            const HbXmlLengthValue &top,
            const HbXmlLengthValue &right,
            const HbXmlLengthValue &bottom );
        virtual bool setSizePolicy(
            QSizePolicy::Policy *horizontalPolicy, 
            QSizePolicy::Policy *verticalPolicy, 
            int *horizontalStretch,
            int *verticalStretch );
        virtual bool setSizeHint(
            Qt::SizeHint hint,
            const HbXmlLengthValue &hintWidth,
            const HbXmlLengthValue &hintHeight,
            bool fixed);
        virtual bool setToolTip( const HbXmlVariable &tooltip );

        virtual bool createAnchorLayout( const QString &widget );
        virtual bool addAnchorLayoutEdge(
            const QString &src,
            Hb::Edge srcEdge, 
            const QString &dst,
            Hb::Edge dstEdge,
            const HbXmlLengthValue &spacing,
            const QString &spacer = QString() );
        
        virtual bool createMeshLayout( const QString &widget );
        virtual bool addMeshLayoutEdge(
            const QString &src,
            Hb::Edge srcEdge, 
            const QString &dst,
            Hb::Edge dstEdge,
            const HbXmlLengthValue &spacing,
            const QString &spacer = QString() );

        virtual bool createGridLayout( const QString &widget, const HbXmlLengthValue &spacing );
        virtual bool addGridLayoutCell(
            const QString &src,
            int row, 
            int column,
            int *rowspan,
            int *columnspan,
            Qt::Alignment *alignment );
        virtual bool setGridLayoutRowProperties(
            int row,
            int *rowStretchFactor,
            Qt::Alignment *alignment );
        virtual bool setGridLayoutColumnProperties(
            int column,
            int *columnStretchFactor,
            Qt::Alignment *alignment );
        virtual bool setGridLayoutRowHeights(
            int row,
            const HbXmlLengthValue &minHeight,
            const HbXmlLengthValue &maxHeight, 
            const HbXmlLengthValue &prefHeight,
            const HbXmlLengthValue &fixedHeight, 
            const HbXmlLengthValue &rowSpacing );
        virtual bool setGridLayoutColumnWidths(
            int column,
            const HbXmlLengthValue &minWidth,
            const HbXmlLengthValue &maxWidth,
            const HbXmlLengthValue &prefWidth,
            const HbXmlLengthValue &fixedWidth,
            const HbXmlLengthValue &columnSpacing );

        virtual bool createLinearLayout(
            const QString &widget,
            Qt::Orientation *orientation, 
            const HbXmlLengthValue &spacing );
        virtual bool addLinearLayoutItem(
            const QString &itemname,
            int *index,
            int *stretchfactor, 
            Qt::Alignment *alignment,
            const HbXmlLengthValue &spacing );
        virtual bool addLinearLayoutStretch(
            int *index,
            int *stretchfactor );

        virtual bool setLayoutContentsMargins(
            const HbXmlLengthValue &left,
            const HbXmlLengthValue &top,
            const HbXmlLengthValue &right,
            const HbXmlLengthValue &bottom );

        virtual bool createStackedLayout( const QString &widget );
        virtual bool addStackedLayoutItem( const QString &itemname, int *index );
                                
        virtual bool createNullLayout( const QString &widget );

    private:
        Q_DISABLE_COPY(HbXmlLoaderAbstractActions)
};

#endif // HBXMLLOADERABSTRACTACTIONS_P_H
