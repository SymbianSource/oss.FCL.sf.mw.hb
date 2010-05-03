/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbUtils module of the UI Extensions for Mobile.
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


#ifndef HBDOCUMENTLOADERACTIONS_P_H
#define HBDOCUMENTLOADERACTIONS_P_H

#include <hbglobal.h>
#include <hbdocumentloader.h>

#include <QPointer>
#include <QGraphicsWidget>
#include <QGraphicsLayout>

#include "hbdocumentloaderfactory_p.h"
#include "hbdocumentloader_p.h"

#include <hbxmlloaderbaseactions_p.h>
#include <hbxmlloaderabstractsyntax_p.h>

// Uncomment the following in order to get additional debug prints
//#define HB_DOCUMENTLOADER_DEBUG
 
#ifndef HB_DOCUMENTLOADER_DEBUG
#define HB_DOCUMENTLOADER_PRINT(a) 
#else
#include <QDebug>
#define HB_DOCUMENTLOADER_PRINT(a) qDebug() << QString(a);
#endif // HB_DOCUMENTLOADER_DEBUG

class HbDocumentLoaderPrivate;

class HbDocumentLoaderActions : public HbXmlLoaderBaseActions
{
    public:    

        HbDocumentLoaderActions( HbDocumentLoaderPrivate *ref, const HbMainWindow *window );
        virtual ~HbDocumentLoaderActions();

        QObject *createObject(const QString &type, const QString &name, const QString &plugin);
        QObject *createObjectWithFactory(const QString& type, const QString &name);

        bool pushObject( const QString& type, const QString &name );
        bool pushWidget( const QString& type, const QString &name, const QString &role, const QString &plugin );
        bool pushSpacerItem( const QString &name, const QString &widget );
        bool pushConnect( const QString &srcName, const QString &signalName, const QString &dstName, const QString &slotName );
        bool pushProperty( const char *propertyName, const HbXmlVariable &variable );
        bool pushRef( const QString &name, const QString &role );
        bool pushContainer( const char *propertyName,
                            HbXmlLoaderAbstractSyntax::DocumentLexems type,
                            const QList<HbXmlVariable*> &container );
        
        bool setContentsMargins( const HbXmlLengthValue &left,
                                 const HbXmlLengthValue &top,
                                 const HbXmlLengthValue &right,
                                 const HbXmlLengthValue &bottom );

        bool setSizePolicy( QSizePolicy::Policy *horizontalPolicy, 
                            QSizePolicy::Policy *verticalPolicy, 
                            int *horizontalStretch,
                            int *verticalStretch );
    
        bool setSizeHint(Qt::SizeHint hint, const HbXmlLengthValue &hintWidth, const HbXmlLengthValue &hintHeight, bool fixed);
        bool setToolTip( const HbXmlVariable &tooltip );

        bool createAnchorLayout( const QString &widget );
        bool addAnchorLayoutEdge( const QString &src, Hb::Edge srcEdge, 
                                  const QString &dst, Hb::Edge dstEdge,
                                  const HbXmlLengthValue &spacing, const QString &spacer = QString() );
        
        bool createGridLayout( const QString &widget, const HbXmlLengthValue &spacing );
        bool addGridLayoutCell( const QString &src, int row, int column,
                                int *rowspan, int *columnspan, Qt::Alignment *alignment );
        bool setGridLayoutRowProperties( int row, int *rowStretchFactor, Qt::Alignment *alignment );
        bool setGridLayoutColumnProperties( int column, int *columnStretchFactor, Qt::Alignment *alignment );
        bool setGridLayoutRowHeights( int row,
                                      const HbXmlLengthValue &minHeight,
                                      const HbXmlLengthValue &maxHeight, 
                                      const HbXmlLengthValue &prefHeight,
                                      const HbXmlLengthValue &fixedHeight, 
                                      const HbXmlLengthValue &rowSpacing );
        bool setGridLayoutColumnWidths( int column,
                                        const HbXmlLengthValue &minWidth,
                                        const HbXmlLengthValue &maxWidth,
                                        const HbXmlLengthValue &prefWidth,
                                        const HbXmlLengthValue &fixedWidth,
                                        const HbXmlLengthValue &columnSpacing );

        bool createLinearLayout( const QString &widget,
                                 Qt::Orientation *orientation, 
                                 const HbXmlLengthValue &spacing );
        bool addLinearLayoutItem( const QString &itemname,
                                  int *index,
                                  int *stretchfactor, 
                                  Qt::Alignment *alignment,
                                  const HbXmlLengthValue &spacing );
        bool addLinearLayoutStretch( int *index,
                                     int *stretchfactor );
        bool setLayoutContentsMargins( const HbXmlLengthValue &left,
                                       const HbXmlLengthValue &top,
                                       const HbXmlLengthValue &right,
                                       const HbXmlLengthValue &bottom );

        bool createStackedLayout( const QString &widget );
        bool addStackedLayoutItem( const QString &itemname, int *index );
                                
        bool createNullLayout( const QString &widget );
    
    private:
        Q_DISABLE_COPY(HbDocumentLoaderActions)
        bool setWidgetRole(QGraphicsWidget *parent, QGraphicsWidget *child, const QString &role);
        bool setObjectRole(QObject *parent, QObject *child, const QString &role);
        
        bool variableToQVariant( const HbXmlVariable &variable, QVariant &variant );

    private:

        HbDocumentLoaderPrivate *d;
        HbDocumentLoaderFactory mFactory;
};

#endif // HBDOCUMENTLOADERACTIONS_P_H
