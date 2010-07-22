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


#ifndef HBXMLLOADERBINARYACTIONS_P_H
#define HBXMLLOADERBINARYACTIONS_P_H

#include <hbxmlloaderabstractactions_p.h>
#include <hbxmlloaderabstractsyntax_p.h>

#include <hbglobal.h>

#include <QGraphicsWidget>
#include <QGraphicsLayout>
#include <QPointer>


// Uncomment the following in order to get additional debug prints
//#define HB_DOCUMENTLOADER_DEBUG
 
struct HbXmlLengthValue;

class HB_CORE_PRIVATE_EXPORT HbXmlLoaderBinaryActions : public HbXmlLoaderAbstractActions
{        
    public:    
    
        HbXmlLoaderBinaryActions();
        virtual ~HbXmlLoaderBinaryActions();

        void setOutputDevice( QIODevice *device );

    public: // from base class
        void reset();
        void cleanUp();
        void deleteAll();

        bool pushDocument( const QString& context);
        bool pushObject( const QString& type, const QString &name );
        bool pushWidget(
            const QString &type,
            const QString &name,
            const QString &role,
            const QString &plugin );
        bool pushConnect(
            const QString &srcName,
            const QString &signalName,
            const QString &dstName,
            const QString &slotName );
        bool pushProperty( const char *propertyName, const HbXmlVariable &variable );
        bool pushRef( const QString &name, const QString &role );
        bool pushContainer(
            const char *propertyName,
            HbXmlLoaderAbstractSyntax::DocumentLexems type,
            const QList<HbXmlVariable*> &container );
        
        bool pop( const HbXml::ElementType type );

        bool setContentsMargins(
            const HbXmlLengthValue &left,
            const HbXmlLengthValue &top,
            const HbXmlLengthValue &right,
            const HbXmlLengthValue &bottom );
        bool setSizePolicy(
            QSizePolicy::Policy *horizontalPolicy, 
            QSizePolicy::Policy *verticalPolicy, 
            int *horizontalStretch,
            int *verticalStretch );
        bool setSizeHint(
            Qt::SizeHint hint,
            const HbXmlLengthValue &hintWidth,
            const HbXmlLengthValue &hintHeight,
            bool fixed);
        bool setToolTip( const HbXmlVariable &tooltip );

        bool createAnchorLayout( const QString &widget, bool modify );
        bool addAnchorLayoutItem(
            const QString &src,
            const QString &srcId,
            Hb::Edge srcEdge, 
            const QString &dst,
            const QString &dstId,
            Hb::Edge dstEdge,
            const HbXmlLengthValue &minLength,
            const HbXmlLengthValue &prefLength,
            const HbXmlLengthValue &maxLength,
            QSizePolicy::Policy *policy, 
            HbAnchor::Direction *dir,
            const QString &anchorId );
        bool setAnchorLayoutMapping(
            const QString &item,
            const QString &id,
            bool remove);

        bool createGridLayout(
            const QString &widget,
            const HbXmlLengthValue &spacing,
            bool modify );
        bool addGridLayoutCell(
            const QString &src,
            int row, 
            int column,
            int *rowspan,
            int *columnspan,
            Qt::Alignment *alignment );
        bool setGridLayoutRowProperties(
            int row,
            int *rowStretchFactor,
            Qt::Alignment *alignment );
        bool setGridLayoutColumnProperties(
            int column,
            int *columnStretchFactor,
            Qt::Alignment *alignment );
        bool setGridLayoutRowHeights(
            int row,
            const HbXmlLengthValue &minHeight,
            const HbXmlLengthValue &maxHeight, 
            const HbXmlLengthValue &prefHeight,
            const HbXmlLengthValue &fixedHeight, 
            const HbXmlLengthValue &rowSpacing );
        bool setGridLayoutColumnWidths(
            int column,
            const HbXmlLengthValue &minWidth,
            const HbXmlLengthValue &maxWidth,
            const HbXmlLengthValue &prefWidth,
            const HbXmlLengthValue &fixedWidth,
            const HbXmlLengthValue &columnSpacing );

        bool createLinearLayout(
            const QString &widget,
            Qt::Orientation *orientation, 
            const HbXmlLengthValue &spacing,
            bool modify );
        bool addLinearLayoutItem(
            const QString &itemname,
            int *index,
            int *stretchfactor, 
            Qt::Alignment *alignment,
            const HbXmlLengthValue &spacing );
        bool addLinearLayoutStretch(
            int *index,
            int *stretchfactor );

        bool setLayoutContentsMargins(
            const HbXmlLengthValue &left,
            const HbXmlLengthValue &top,
            const HbXmlLengthValue &right,
            const HbXmlLengthValue &bottom );

        bool createStackedLayout( const QString &widget, bool modify );
        bool addStackedLayoutItem( const QString &itemname, int *index );
                                
        bool createNullLayout( const QString &widget );

    private:
        Q_DISABLE_COPY(HbXmlLoaderBinaryActions)
        QDataStream mOut;
};

#endif // HBXMLLOADERBINARYACTIONS_P_H
