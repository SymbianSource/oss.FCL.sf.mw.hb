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

#include <hbxmlloaderabstractactions_p.h>

// Uncomment the following in order to get additional debug prints
//#define HB_DOCUMENTLOADER_DEBUG
 
#ifndef HB_DOCUMENTLOADER_DEBUG
#define HB_DOCUMENTLOADER_PRINT(a) 
#else
#include <QDebug>
#define HB_DOCUMENTLOADER_PRINT(a) qDebug() << QString(a);
#endif // HB_DOCUMENTLOADER_DEBUG

class HbDocumentLoaderPrivate;

class HbDocumentLoaderActions : public HbXmlLoaderAbstractActions
{
    public:    
    
        enum PropertyAvailableFlag {
            propertyMin     = 0x01,
            propertyMax     = 0x02,
            propertyPref    = 0x04,
            propertyFixed   = 0x08,
            propertySpacing = 0x10
        };

        HbDocumentLoaderActions( HbDocumentLoaderPrivate *ref );
        virtual ~HbDocumentLoaderActions();
                
        
        QObject *createObject(const QString &type, const QString &name, const QString &plugin);
        QObject *createObjectWithFactory(const QString& type, const QString &name);
        
        bool pushObject( const QString& type, const QString &name );
        bool pushWidget( const QString& type, const QString &name, const QString &role, const QString &plugin );
        bool pushSpacerItem( const QString &name, const QString &widget );
        bool pushConnect( const QString &srcName, const QString &signalName, const QString &dstName, const QString &slotName );
        bool pushProperty( const QString &propertyName, const QVariant &value );
        bool pushRef( const QString &name, const QString &role );
        
        bool setContentsMargins( qreal left, qreal top, qreal right, qreal bottom );

        bool setSizePolicy( const QSizePolicy::Policy *horizontalPolicy, 
                            const QSizePolicy::Policy *verticalPolicy, 
                            const int *horizontalStretch,
                            const int *verticalStretch );
    
        bool setSizeHint(Qt::SizeHint hint, qreal *hintWidth, qreal *hintHeight, bool fixed);
        bool setZValue( qreal zValue );
        bool setToolTip( const QString &tooltip );

        bool createAnchorLayout( const QString &widget );
        bool addAnchorLayoutEdge( const QString &src, const QString &srcEdge, 
                                    const QString &dst, const QString &dstEdge, qreal spacing, const QString &spacer = QString() );
        
        bool createGridLayout( const QString &widget, qreal *spacing );
        bool addGridLayoutCell( const QString &src, const QString &row, 
                                const QString &column, const QString &rowspan, const QString &columnspan,
                                const QString &alignment );
        bool setGridLayoutRowProperties( const QString &row, const QString &rowStretchFactor, const QString &alignment );
        bool setGridLayoutColumnProperties( const QString &column, const QString &columnStretchFactor, const QString &alignment );
        bool setGridLayoutRowHeights( const QString &row, const qreal minHeight, const qreal maxHeight, 
                                      const qreal prefHeight, const qreal fixedHeight, 
                                      const qreal rowSpacing,  const int flagsPropertyAvailable);
        bool setGridLayoutColumnWidths( const QString &column, const qreal minWidth, const qreal maxWidth, 
                                        const qreal prefWidth, const qreal fixedWidth,
                                        const qreal columnSpacing, const int flagsPropertyAvailable);

        bool createLinearLayout( const QString &widget, const QString &orientation, qreal *spacing );
        bool addLinearLayoutItem( const QString &itemname, const QString &index, const QString &stretchfactor, 
                                  const QString &alignment, qreal *spacing );
        bool addLinearLayoutStretch( const QString &index, const QString &stretchfactor );
        bool setLayoutContentsMargins( qreal left, qreal top, qreal right, qreal bottom );

        bool createStackedLayout( const QString &widget );
        bool addStackedLayoutItem( const QString &itemname, const QString &index );
                                
        bool createNullLayout( const QString &widget );

        bool createContainer();
        bool appendPropertyToContainer( const QVariant &value );
    
    private:
        Q_DISABLE_COPY(HbDocumentLoaderActions)
        bool setWidgetRole(QGraphicsWidget *parent, QGraphicsWidget *child, const QString &role);
        bool setObjectRole(QObject *parent, QObject *child, const QString &role);
        

    private:

        HbDocumentLoaderPrivate *d;
        HbDocumentLoaderFactory factory;
};

#endif // HBDOCUMENTLOADERACTIONS_P_H
