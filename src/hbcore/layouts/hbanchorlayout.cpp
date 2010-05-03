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

#include "hbanchorlayout.h"
#include "hbanchor_p.h"
#include "hbanchorlayoutdebug_p.h"
#include "hbanchorlayoutengine_p.h"

#include <QLayout>

#include "hblayoututils_p.h"

//Uncomment next define in order to get more debug prints.
//Similar define exists also in the engine side.
//#define HBANCHORLAYOUT_DEBUG

#include <QDebug>

/*!
    @stable
    @hbcore
    \class HbAnchorLayout
    \brief HbAnchorLayout manages geometries of its child items with anchors that
    that connect the layout items with each other.

    The anchors have a start edge, an end edge and a value. The start and
    end edges are defined by (layout item, edge) pairs. See setAnchor() for
    more details.

    If anchors set allow ambiguos positioning of items, then layout tries to set items size as close to preferred as possible.

    Example code:
    \snippet{anchorlayoutsample.cpp,1}

    The picture below illustrates the anchors defined by the above example code.

    \image html hbanchorlayout.png
*/

/*!
    \enum Hb::Edge

    This enum defines the edges of a layout item.
*/

/*!
    \var HbAnchorLayout::Left
    Left edge.
*/

/*!
    \var HbAnchorLayout::Top
    Top edge.
*/

/*!
    \var HbAnchorLayout::Right
    Right edge.
*/

/*!
    \var HbAnchorLayout::Bottom
    Bottom edge.
*/

/*!
    \enum EdgeType
    \internal
*/
enum EdgeType {
    Horizontal = 0,
    Vertical
};


/*!
    \internal
*/
HbAnchor::HbAnchor()
    : mStartItem(0),
    mStartEdge(Hb::LeftEdge),
    mEndItem(0),
    mEndEdge(Hb::LeftEdge),
    mValue(0)
{
}

HbAnchor::HbAnchor(const HbAnchor &anchor)
: mStartItem(anchor.mStartItem),
  mStartEdge(anchor.mStartEdge),
  mEndItem(anchor.mEndItem),
  mEndEdge(anchor.mEndEdge),
  mValue(anchor.mValue)
{
}

HbAnchor::HbAnchor( QGraphicsLayoutItem *startItem,
                   HbAnchorLayout::Edge startEdge,
                   QGraphicsLayoutItem *endItem,
                   HbAnchorLayout::Edge endEdge,
                   qreal value )
    : mStartItem(startItem),
    mStartEdge(startEdge),
    mEndItem(endItem),
    mEndEdge(endEdge),
    mValue(value)
{
}


HbAnchor &HbAnchor::operator=(const HbAnchor &anchor)
{
    if (this != &anchor) {
        mStartItem = anchor.mStartItem;
        mStartEdge = anchor.mStartEdge;
        mEndItem = anchor.mEndItem;
        mEndEdge = anchor.mEndEdge;
        mValue = anchor.mValue;
    }
    return *this;
}


class HbAnchorLayoutPrivate
{
public:
    Q_DECLARE_PUBLIC( HbAnchorLayout )

    HbAnchorLayoutPrivate();
    ~HbAnchorLayoutPrivate();

    void addItemIfNeeded( QGraphicsLayoutItem *item );
    EdgeType edgeType( const Hb::Edge edge ) const;
    HbAnchor* getAnchor( QGraphicsLayoutItem *startItem,
                         Hb::Edge startEdge,
                         QGraphicsLayoutItem *endItem,
                         Hb::Edge endEdge );

    void setItemGeometries();

    void createEquations( EdgeType type );
    void setSizeProp( SizeProperty *v, QGraphicsLayoutItem *item, EdgeType type );
    GraphVertex *createCenterEdge( EdgeType type, QGraphicsLayoutItem *item,  Hb::Edge edge );
    void defineNextGeometry( const int itemIndexStart, const int itemIndexEnd, const int anchorIndex, const int definedItemIndex );

    QSizeF sizeHint( Qt::SizeHint which );

public:
    HbAnchorLayout * q_ptr;
    QList<QGraphicsLayoutItem*> mItems;
    QList<HbAnchor*> mAnchors;
    bool mEquationsDirty; // if true, we needed to re-create the equations (e.g. when new anchor is set)
    bool mValid;          // result of the calculations. false, if the equations cannot be solved.
    bool mWrongAnchors;    // need to recreate anchors, these ones are unsolvable with any layout geometry
    bool mInvalidateCalled; // set true in ::invalidate() and cleared after geometry is set in ::setGeometry


    QRectF mUsedRect;

    QList<GraphEdge*> mEdgesVertical;
    QList<GraphEdge*> mEdgesHorizontal;
    QList<GraphVertex*> mVerticesVertical;
    QList<GraphVertex*> mVerticesHorizontal;

    QList<Expression*> mEquationsHorizontal;
    QList<Expression*> mEquationsVertical;
    VariableSet mVariablesHorizontal;
    VariableSet mVariablesVertical;

    Variable *mLayoutVarH;
    Variable *mLayoutVarV;

    QVector<bool> mAnchorsVisited;
    QVector< bool > mGeometryDefinedH;
    QVector< bool > mGeometryDefinedV;
    typedef struct {
        qreal x1, y1, x2, y2;
    } ItemGeometry;

    QVector< ItemGeometry > mItemsGeometry;

    Solution mSolutionHorizontal;
    Solution mSolutionVertical;
};

/*!
    \internal
*/
HbAnchorLayoutPrivate::HbAnchorLayoutPrivate() : mEquationsDirty( false ), mValid( true ), mWrongAnchors( false ), mInvalidateCalled(false),
                                                    mLayoutVarH( 0 ), mLayoutVarV( 0 )
{
}

/*!
    \internal
*/
HbAnchorLayoutPrivate::~HbAnchorLayoutPrivate()
{
    qDeleteAll( mAnchors );


    qDeleteAll( mEdgesVertical );
    qDeleteAll( mEdgesHorizontal );
    qDeleteAll( mVerticesVertical );
    qDeleteAll( mVerticesHorizontal );

    qDeleteAll( mEquationsHorizontal );
    qDeleteAll( mEquationsVertical );
}

/*!
    \internal
*/
void HbAnchorLayoutPrivate::addItemIfNeeded( QGraphicsLayoutItem *item )
{
    Q_Q(HbAnchorLayout);
    if ( item != q && !mItems.contains(item) ) {
        HbLayoutUtils::addChildItem(q, item);

        mItems.append( item );
    }
}

/*!
    \internal
*/
EdgeType HbAnchorLayoutPrivate::edgeType( const Hb::Edge edge ) const
{
    EdgeType type( Horizontal );
    if ( edge == Hb::TopEdge || edge == Hb::BottomEdge || edge == Hb::CenterVEdge) {
        type = Vertical;
    }
    return type;
}

/*!
    \internal
*/
HbAnchor* HbAnchorLayoutPrivate::getAnchor(
    QGraphicsLayoutItem *startItem,
    Hb::Edge startEdge,
    QGraphicsLayoutItem *endItem,
    Hb::Edge endEdge )
{
    for ( int i = mAnchors.count()-1 ; i >= 0; i-- ) {
        HbAnchor* anchor = mAnchors.at(i);
        if ( anchor->mStartItem == startItem &&
             anchor->mStartEdge == startEdge &&
             anchor->mEndItem == endItem &&
             anchor->mEndEdge == endEdge ) {
                return anchor;
        }
    }
    return 0;
}

void HbAnchorLayoutPrivate::defineNextGeometry( const int itemIndexStart, const int itemIndexEnd, const int anchorIndex, const int definedItemIndex )
{
    ItemGeometry *knownItemGeom, *unKnownItemGeom;
    Hb::Edge knownEdge, unKnownEdge;
    int sign;
    qreal itemSize;
    bool isHorizontal;
    HbAnchor *anchor = mAnchors.at( anchorIndex );
    qreal leftPoint(0), rightPoint(0), sourcePoint(0), dstPointLeft(0);

    mAnchorsVisited[ anchorIndex ] = true;

    if( edgeType( anchor->mStartEdge ) == Horizontal ) {
        isHorizontal = true;
    } else {
        isHorizontal = false;
    }

    if( itemIndexEnd != definedItemIndex ) {
        knownEdge = anchor->mStartEdge;
        unKnownEdge =  anchor->mEndEdge;

        knownItemGeom = &mItemsGeometry[itemIndexStart];
        unKnownItemGeom = &mItemsGeometry[itemIndexEnd];

        if( isHorizontal ) {
            mGeometryDefinedH[itemIndexEnd] = true;
            itemSize = mSolutionHorizontal.value( mVariablesHorizontal.findVariable( mItems.at(itemIndexEnd) ) );
        } else {
            mGeometryDefinedV[itemIndexEnd] = true;
            itemSize = mSolutionVertical.value( mVariablesVertical.findVariable( mItems.at(itemIndexEnd) ) );
        }

        sign = 1;
    } else {
        knownEdge =  anchor->mEndEdge;
        unKnownEdge = anchor->mStartEdge;

        knownItemGeom = &mItemsGeometry[itemIndexEnd];
        unKnownItemGeom = &mItemsGeometry[itemIndexStart];

        if( isHorizontal ) {
            mGeometryDefinedH[itemIndexStart] = true;
            itemSize = mSolutionHorizontal.value( mVariablesHorizontal.findVariable( mItems.at(itemIndexStart) ) );
        } else {
            mGeometryDefinedV[itemIndexStart] = true;
            itemSize = mSolutionVertical.value( mVariablesVertical.findVariable( mItems.at(itemIndexStart) ) );
        }

        sign = -1;
    }

    if( isHorizontal ) {
        leftPoint = knownItemGeom->x1;
        rightPoint = knownItemGeom->x2;
    } else {
        leftPoint = knownItemGeom->y1;
        rightPoint = knownItemGeom->y2;
    }

    switch( knownEdge ) {
        case Hb::LeftEdge:
        case Hb::TopEdge:
        {
            sourcePoint = leftPoint;
            break;
        }
        case Hb::CenterHEdge:
        case Hb::CenterVEdge:
        {
            sourcePoint = ( leftPoint + rightPoint ) / 2;
            break;
        }
        case Hb::RightEdge:
        case Hb::BottomEdge:
        {
            sourcePoint = rightPoint;
            break;
        }
    }

    switch( unKnownEdge ) {
        case Hb::LeftEdge:
        case Hb::TopEdge:
        {
            dstPointLeft = sourcePoint + sign * anchor->mValue;
            break;
        }
        case Hb::CenterHEdge:
        case Hb::CenterVEdge:
        {
            dstPointLeft = sourcePoint + sign * anchor->mValue - itemSize / 2;
            break;
        }
        case Hb::RightEdge:
        case Hb::BottomEdge:
        {
            dstPointLeft = sourcePoint + sign * anchor->mValue - itemSize;
            break;
        }
    }



    if( isHorizontal ) {
        unKnownItemGeom->x1 = dstPointLeft;
        unKnownItemGeom->x2 = dstPointLeft + itemSize;
    } else {
        unKnownItemGeom->y1 = dstPointLeft;
        unKnownItemGeom->y2 = dstPointLeft + itemSize;
    }

}

/*!
    \internal
*/
void HbAnchorLayoutPrivate::setItemGeometries()
{
    Q_Q(HbAnchorLayout);
    const QRectF newRect = q->geometry();

    if( mWrongAnchors || ( mItems.isEmpty() ) ) {
        return;
    }


    if ( (newRect != mUsedRect) || mInvalidateCalled ) {

        mUsedRect = newRect;

        if ( mEquationsDirty ) {
            createEquations( Horizontal );
            createEquations( Vertical );
            mEquationsDirty = false;
        }


        mValid = true;

        {

            QList<Expression*> *el = &mEquationsHorizontal;
            VariableSet *vs = &mVariablesHorizontal;
            Solution *solution = &mSolutionHorizontal;
            solution->clear();


            solution->insert( mLayoutVarH, newRect.width() );
#ifdef HBANCHORLAYOUT_DEBUG
            qDebug() << "LayoutH Id = " << mLayoutVarH->mId;
#endif // HBANCHORLAYOUT_DEBUG
            mValid = AnchorLayoutEngine::instance()->solveEquation( el, vs, solution );
            if( !mValid ) {
                return;
            }

#ifdef HBANCHORLAYOUT_DEBUG
            qDebug() << "solution->size() = " << solution->size();

            if( solution->size() > 0 ) {
                QHashIterator<Variable*, qreal>  i(*solution);
                while (i.hasNext()) {
                    i.next();
                    qDebug() << ( ( Variable* )( i.key() ) )->mId << ": " << i.value();
                }
            }
#endif //HBANCHORLAYOUT_DEBUG
        }

        {
            QList<Expression*> *el = &mEquationsVertical;
            VariableSet *vs = &mVariablesVertical;
            Solution *solution = &mSolutionVertical;
            solution->clear();



            solution->insert( mLayoutVarV, newRect.height() );
#ifdef HBANCHORLAYOUT_DEBUG
            qDebug() << "LayoutV Id = " << mLayoutVarV->mId;
#endif //HBANCHORLAYOUT_DEBUG

            mValid = AnchorLayoutEngine::instance()->solveEquation( el, vs, solution );
            if( !mValid ) {
                return;
            }
#ifdef HBANCHORLAYOUT_DEBUG
            qDebug() << "solution->size() = " << solution->size();


            if( solution->size() > 0 ) {
                QHashIterator<Variable*, qreal>  i(*solution);
                while (i.hasNext()) {
                    i.next();
                    qDebug() << ( ( Variable* )( i.key() ) )->mId << ": " << i.value();
                }
            }
#endif //HBANCHORLAYOUT_DEBUG
        }

        {
            for( int i = 0; i < mAnchorsVisited.size(); i++ ) {
                mAnchorsVisited[i] = false;
            }

            for( int i = 0; i < mGeometryDefinedH.size(); i++ ) {
                mGeometryDefinedH[i] = false;
                mGeometryDefinedV[i] = false;
            }

            int layoutIndex = mItems.size();

            mItemsGeometry[ layoutIndex ].x1 = 0;
            mItemsGeometry[ layoutIndex ].x2 = newRect.width();
            mItemsGeometry[ layoutIndex ].y1 = 0;
            mItemsGeometry[ layoutIndex ].y2 = newRect.height();


            for( int i = 0; i < mAnchorsVisited.size(); i++ ) {

                HbAnchor *anchor = mAnchors.at(i);


                if( ( anchor->mStartItem != q ) && ( anchor->mEndItem != q ) ) {
                    continue;
                }

                int startIndex = mItems.indexOf( anchor->mStartItem ); // returns -1 if not found => this is layout
                int endIndex = mItems.indexOf( anchor->mEndItem );

                mAnchorsVisited[i] = true; // Temporary overkill, if both anchors connected to layout. Must be restricted on setAnchor() level

                if( edgeType( anchor->mStartEdge ) == Horizontal ) {
                    if( startIndex > -1 ) {
                        if( ! mGeometryDefinedH.at( startIndex ) ) {
                            defineNextGeometry( startIndex, layoutIndex, i, layoutIndex );
                        }
                    } else if( endIndex > -1 ) {
                        if( ! mGeometryDefinedH.at( endIndex ) ) {
                            defineNextGeometry( layoutIndex, endIndex, i, layoutIndex );
                        }
                    }
                } else {
                    if( startIndex > -1 ) {
                        if( ! mGeometryDefinedV.at( startIndex ) ) {
                            defineNextGeometry( startIndex, layoutIndex, i, layoutIndex );
                        }
                    } else if( endIndex > -1 ) {
                        if( ! mGeometryDefinedV.at( endIndex ) ) {
                            defineNextGeometry( layoutIndex, endIndex, i, layoutIndex );
                        }
                    }
                }
            }



            bool somethingHappens = true;
            bool startDefined, endDefined;
            int startIndex, endIndex;
            while( somethingHappens ) {
                somethingHappens = false;
                for( int i = 0; i < mAnchorsVisited.size(); i++ ) {

                    if( mAnchorsVisited.at(i) ) {
                        continue;
                    }
                    HbAnchor *anchor = mAnchors.at(i);

                    startIndex = mItems.indexOf( anchor->mStartItem );
                    endIndex = mItems.indexOf( anchor->mEndItem );
#ifdef HBANCHORLAYOUT_DEBUG
                    qDebug() << "startIndex:" << startIndex << "  endIndex" << endIndex;
#endif //HBANCHORLAYOUT_DEBUG
                    if( edgeType( anchor->mStartEdge ) == Horizontal ) {
                        startDefined = mGeometryDefinedH.at( startIndex );
                        endDefined = mGeometryDefinedH.at( endIndex );
                    } else {
                        startDefined = mGeometryDefinedV.at( startIndex );
                        endDefined = mGeometryDefinedV.at( endIndex );
                    }

                    if( startDefined && ( !endDefined ) ) {
                        defineNextGeometry( startIndex, endIndex, i, startIndex );
                        somethingHappens = true;
                    } else if( ( !startDefined ) && endDefined ) {
                        defineNextGeometry( startIndex, endIndex, i, endIndex );
                        somethingHappens = true;
                    }
                }
            }


            Qt::LayoutDirection layoutDir = HbLayoutUtils::visualDirection(q);
            for( int i = 0; i < layoutIndex; i++ ) {
                QRectF geom;
                ItemGeometry calcGeom = mItemsGeometry.at(i);
                if( mGeometryDefinedH.at(i) ) {
                    geom.setLeft( mUsedRect.left() + calcGeom.x1 );
                    geom.setRight( mUsedRect.left() + calcGeom.x2 );
                } else {
                    geom.setLeft( mUsedRect.left() );
                    geom.setRight( mUsedRect.left() + mItems.at(i)->preferredWidth() );
                }
                if( mGeometryDefinedV.at(i) ) {
                    geom.setTop( mUsedRect.top() + calcGeom.y1 );
                    geom.setBottom( mUsedRect.top() + calcGeom.y2 );
                } else {
                    geom.setTop( mUsedRect.top() );
                    geom.setBottom( mUsedRect.top() + mItems.at(i)->preferredHeight() );
                }

                HbLayoutUtils::visualRect(layoutDir, geom, newRect);

#ifdef HBANCHORLAYOUT_DEBUG
                qDebug( "Item %d: (%lf, %lf) : (%lf %lf)", i, calcGeom.x1, calcGeom.y1, calcGeom.x2, calcGeom.y2 );
#endif // HBANCHORLAYOUT_DEBUG

                mItems.at(i)->setGeometry( geom );
            }
        }
    }
}

void HbAnchorLayoutPrivate::setSizeProp( SizeProperty *v, QGraphicsLayoutItem *item, EdgeType type )
{
    if( type == Vertical ) {
        const QSizePolicy::Policy verticalPolicy = item->sizePolicy().verticalPolicy();

        if ( verticalPolicy & QSizePolicy::ShrinkFlag ) {
            v->min = item->minimumHeight();
        } else {
            v->min = item->preferredHeight();
        }

        if ( verticalPolicy & (QSizePolicy::GrowFlag | QSizePolicy::ExpandFlag) ) {
            v->max = item->maximumHeight();
        } else {
            v->max = item->preferredHeight();
        }

        v->pref = qBound( v->min, item->preferredHeight(), v->max );

        v->flags |= (v->min == v->max) ? SizeProperty::FlagFixed : 0;
        v->flags |= (verticalPolicy & QSizePolicy::ExpandFlag) ? SizeProperty::FlagExpanding : 0;

        if( verticalPolicy & QSizePolicy::IgnoreFlag ) {
            v->pref = v->min;
            v->flags |= SizeProperty::FlagExpanding;
        }
    } else {
        const QSizePolicy::Policy horizontalPolicy = item->sizePolicy().horizontalPolicy();

        if ( horizontalPolicy & QSizePolicy::ShrinkFlag ) {
            v->min = item->minimumWidth();
        } else {
            v->min = item->preferredWidth();
        }

        if ( horizontalPolicy & (QSizePolicy::GrowFlag | QSizePolicy::ExpandFlag) ) {
            v->max = item->maximumWidth();
        } else {
            v->max = item->preferredWidth();
        }

        v->pref = qBound( v->min, item->preferredWidth(), v->max );

        v->flags |= (v->min == v->max) ? SizeProperty::FlagFixed : 0;
        v->flags |= (horizontalPolicy & QSizePolicy::ExpandFlag) ? SizeProperty::FlagExpanding : 0;

        if( horizontalPolicy & QSizePolicy::IgnoreFlag ) {
            v->pref = v->min;
            v->flags |= SizeProperty::FlagExpanding;
        }
    }
}


GraphVertex *HbAnchorLayoutPrivate::createCenterEdge( EdgeType type, QGraphicsLayoutItem *item,  Hb::Edge edge )
{
    GraphVertex *middle;
    GraphVertex *start = 0;
    GraphVertex *end = 0;

    QList<GraphEdge*> *edges = &mEdgesHorizontal;
    QList<GraphVertex*> *vertices = &mVerticesHorizontal;

    if( type == Vertical ) {
        if( edge != Hb::CenterVEdge ) {
#ifdef HBANCHORLAYOUT_DEBUG
            qDebug() << "something wrong " << __LINE__;
#endif //HBANCHORLAYOUT_DEBUG
            return 0;
        }

        edges = &mEdgesVertical;
        vertices = &mVerticesVertical;

        for( int j = 0; j < vertices->size(); j++ ) {
            GraphVertex *current = vertices->at(j);
            if( current->itemRef == item ) {
                if( current->itemSide == Hb::TopEdge ) {
                    start = current;
                } else if( current->itemSide == Hb::BottomEdge ) {
                    end = current;
                }
            }
        }
    } else {
        if( edge != Hb::CenterHEdge ) {
#ifdef HBANCHORLAYOUT_DEBUG
            qDebug() << "something wrong " << __LINE__;
#endif //HBANCHORLAYOUT_DEBUG
            return 0;
        }

        for( int j = 0; j < vertices->size(); j++ ) {
            GraphVertex *current = vertices->at(j);
            if( current->itemRef == item ) {
                if( current->itemSide == Hb::LeftEdge ) {
                    start = current;
                } else if( current->itemSide == Hb::RightEdge ) {
                    end = current;
                }
            }
        }
    }

    if( !( start && end ) ) {
#ifdef HBANCHORLAYOUT_DEBUG
        qDebug() << "something wrong " << __LINE__;
#endif //HBANCHORLAYOUT_DEBUG
        return 0;
    }

    GraphEdge *oldEdge = 0;

    for( int i = 0; i < edges->size(); i++ ) {
        oldEdge = edges->at(i);
        if( oldEdge->ref == item ) {
            if( ( oldEdge->startVertex == start ) && ( oldEdge->endVertex == end ) ){
/*                edges->removeOne( oldEdge );
                start->edges.removeOne( oldEdge );
                end->edges.removeOne( oldEdge );*/
                break;
            }
        }
    }

    if( !oldEdge ) {
#ifdef HBANCHORLAYOUT_DEBUG
        qDebug() << "something wrong " << __LINE__;
#endif //HBANCHORLAYOUT_DEBUG
        return 0;
    }

    middle = new GraphVertex();
    middle->itemRef = ( void* )item;
    middle->itemSide =  edge;
    middle->special = false;

    GraphEdge *newEdge1 = new GraphEdge();
    GraphEdge *newEdge2 = new GraphEdge();

    newEdge1->startVertex = start;
    newEdge1->endVertex = middle;
    newEdge1->ref = ( void* )item;

    newEdge1->expr->plusExpression( oldEdge->expr );
    newEdge1->expr->multiply( 0.5 );


    newEdge2->startVertex = middle;
    newEdge2->endVertex = end;
    newEdge2->ref = ( void* )item;
    newEdge2->expr->plusExpression( oldEdge->expr );
    newEdge2->expr->multiply( 0.5 );


    middle->edges.append( newEdge1 );
    start->edges.append( newEdge1 );
    middle->edges.append( newEdge2 );
    end->edges.append( newEdge2 );

    edges->append( newEdge1 );
    edges->append( newEdge2 );
    vertices->append( middle );


    return middle;
}

/*!
    \internal
*/
void HbAnchorLayoutPrivate::createEquations( EdgeType type )
{
    Q_Q(HbAnchorLayout);

    {

        VariableSet *vs =  &mVariablesHorizontal;
        QList<Expression*> *el = &mEquationsHorizontal;

        QList<GraphEdge*> *edges = &mEdgesHorizontal;
        QList<GraphVertex*> *vertices = &mVerticesHorizontal;

        Variable *layoutVar;

        if( type == Vertical ) {
            edges = &mEdgesVertical;
            vertices = &mVerticesVertical;
            vs =  &mVariablesVertical;
            el = &mEquationsVertical;
        }

        qDeleteAll( *el );

        vs->clear();
        el->clear();


        GraphVertex *layoutStart = new GraphVertex();
        GraphVertex *layoutMiddle = new GraphVertex();
        GraphVertex *layoutEnd = new GraphVertex();

        GraphVertex *itemStart;
        GraphVertex *itemEnd;

        GraphEdge *newEdge;

        SimpleExpression se;

        vertices->append( layoutStart );
        vertices->append( layoutMiddle );
        vertices->append( layoutEnd );

        layoutStart->itemRef = ( void* )q;
        layoutMiddle->itemRef = ( void* )q;
        layoutEnd->itemRef = ( void* )q;

        layoutStart->special = true;
        layoutMiddle->special = true;
        layoutEnd->special = true;

        if( type == Vertical ) {
            layoutStart->itemSide =  Hb::TopEdge;
            layoutMiddle->itemSide =  Hb::CenterVEdge;
            layoutEnd->itemSide =  Hb::BottomEdge;
        } else {
            layoutStart->itemSide =  Hb::LeftEdge;
            layoutMiddle->itemSide =  Hb::CenterHEdge;
            layoutEnd->itemSide =  Hb::RightEdge;
        }


        for ( int i = 0; i < mItems.count(); i++ ) {
            QGraphicsLayoutItem *item = mItems.at( i );
            itemStart = new GraphVertex();
            itemEnd = new GraphVertex();
            newEdge = new GraphEdge();

            se.mVar = vs->createVariable(item);
            se.mCoef = 1;

            newEdge->expr->plusSimpleExpression( se );

            edges->append( newEdge );
            vertices->append( itemStart );
            vertices->append( itemEnd );

            newEdge->startVertex = itemStart;
            newEdge->endVertex = itemEnd;
            newEdge->ref = ( void* )item;

            setSizeProp( &(se.mVar->sizeProp), item, type );

            itemStart->itemRef = ( void* )item;
            itemEnd->itemRef = ( void* )item;

            itemStart->edges.append( newEdge );
            itemEnd->edges.append( newEdge );

            itemStart->special = false;
            itemEnd->special = false;

            if( type == Vertical ) {
                itemStart->itemSide =  Hb::TopEdge;
                itemEnd->itemSide =  Hb::BottomEdge;
            } else {
                itemStart->itemSide =  Hb::LeftEdge;
                itemEnd->itemSide =  Hb::RightEdge;
            }
        }


        // pseudo variable
        Variable *v1 = vs->createVariable(0);
        v1->sizeProp.pref = 1;
        v1->sizeProp.flags = SizeProperty::FlagFixed;


        for( int i = 0; i < mAnchors.count(); i++) {
            HbAnchor* anchor = mAnchors.at(i);
            if ( edgeType( anchor->mStartEdge ) == type ) {
                itemStart = 0;
                itemEnd = 0;
                for( int j = 0; j < vertices->size(); j++ ) {
                    if( ( vertices->at(j)->itemRef == anchor->mStartItem ) && ( vertices->at(j)->itemSide == anchor->mStartEdge ) ) {
                        itemStart = vertices->at(j);
                    } else if( ( vertices->at(j)->itemRef == anchor->mEndItem ) && ( vertices->at(j)->itemSide == anchor->mEndEdge ) ) {
                        itemEnd = vertices->at(j);
                    }
                }

                if( !itemStart ) {
                    itemStart = createCenterEdge( type, anchor->mStartItem,  anchor->mStartEdge );
                }
                if( !itemEnd ) {
                    itemEnd = createCenterEdge( type, anchor->mEndItem,  anchor->mEndEdge );
                }

                newEdge = new GraphEdge();
                itemStart->edges.append( newEdge );
                itemEnd->edges.append( newEdge );

                newEdge->startVertex = itemStart;
                newEdge->endVertex = itemEnd;
                se.mVar = v1;
                se.mCoef = anchor->mValue;
                newEdge->expr->plusSimpleExpression( se );
                edges->append( newEdge );
            }
        }

        if( layoutStart->edges.isEmpty() ) {
            vertices->removeOne( layoutStart );
            delete layoutStart;
            layoutStart = 0;
        }
        if( layoutMiddle->edges.isEmpty() ) {
            vertices->removeOne( layoutMiddle );
            delete layoutMiddle;
            layoutMiddle = 0;
        }
        if( layoutEnd->edges.isEmpty() ) {
            vertices->removeOne( layoutEnd );
            delete layoutEnd;
            layoutEnd = 0;
        }

#ifdef HBANCHORLAYOUT_DEBUG
        qDebug() << "Before";
        qDebug() << "Vertices:";
        for( int i = 0; i < vertices->size(); i++ ) {
            qDebug() << i << ":  " << vertices->at(i) << "  itemRef: " << vertices->at(i)->itemRef << "  special: " << vertices->at(i)->special;
            for( int j = 0; j < vertices->at(i)->edges.size(); j++ ) {
                qDebug() << "     " << j << "-  start: " << vertices->at(i)->edges.at(j)->startVertex <<
                        " end: " << vertices->at(i)->edges.at(j)->endVertex << " expr: " << vertices->at(i)->edges.at(j)->expr->print();
            }
        }


        qDebug() << "";
        qDebug() << "Edges:";
        for( int j = 0; j < edges->size(); j++ ) {
            qDebug() << "     " << j << "-  start: " << edges->at(j)->startVertex <<
                    " end: " << edges->at(j)->endVertex << " expr: " << edges->at(j)->expr->print();

        }
#endif // HBANCHORLAYOUT_DEBUG

        if( ! AnchorLayoutEngine::instance()->processItems( edges, vertices, vs, el ) ) {
            mWrongAnchors = true;
            AnchorLayoutEngine::instance()->cleanUp( layoutStart, layoutMiddle, layoutEnd, edges, vertices, el );
//#ifdef HBANCHORLAYOUT_DEBUG
            qDebug() << "FAIL line:" << __LINE__;
//#endif //HBANCHORLAYOUT_DEBUG
            return;
        }

#ifdef HBANCHORLAYOUT_DEBUG

        qDebug() << "After";
        qDebug() << "Vertices:";
        for( int i = 0; i < vertices->size(); i++ ) {
            qDebug() << i << ":  " << vertices->at(i) << "  itemRef: " << vertices->at(i)->itemRef << "  special: " << vertices->at(i)->special;
            for( int j = 0; j < vertices->at(i)->edges.size(); j++ ) {
                qDebug() << "     " << j << "-  start: " << vertices->at(i)->edges.at(j)->startVertex <<
                        " end: " << vertices->at(i)->edges.at(j)->endVertex << " var: " << vertices->at(i)->edges.at(j)->expr->print();
            }
        }


        qDebug() << "";
        qDebug() << "Edges:";
        for( int j = 0; j < edges->size(); j++ ) {
            qDebug() << "     " << j << "-  start: " << edges->at(j)->startVertex <<
                    " end: " << edges->at(j)->endVertex << " var: " << edges->at(j)->expr->print();
        }
#endif //HBANCHORLAYOUT_DEBUG

        layoutVar = vs->createVariable( q );
        layoutVar->sizeProp.min = 0;
        layoutVar->sizeProp.max = 1000;
        layoutVar->sizeProp.pref = 100;
        layoutVar->sizeProp.flags = 0;

        AnchorLayoutEngine::instance()->attachToLayout( layoutStart, layoutMiddle, layoutEnd, layoutVar, el );
        AnchorLayoutEngine::instance()->cleanUp( layoutStart, layoutMiddle, layoutEnd, edges, vertices, el );


        mAnchorsVisited.resize( mAnchors.size() * sizeof( bool ) );
        mGeometryDefinedH.resize( mItems.size() * sizeof( bool ) );
        mGeometryDefinedV.resize( mItems.size() * sizeof( bool ) );
        mItemsGeometry.resize( ( mItems.size() + 1 ) * sizeof( ItemGeometry ) );

        if( type == Vertical ) {
            mLayoutVarV = layoutVar;
        } else {
            mLayoutVarH = layoutVar;
        }
    }
}


/*!
    \internal
*/
QList<HbAnchor*> HbAnchorLayoutDebug::getAnchors( HbAnchorLayout* layout )
{
    return layout->d_ptr->mAnchors;
}

/*!
    Constructor.
*/
HbAnchorLayout::HbAnchorLayout(QGraphicsLayoutItem *parent)
: QGraphicsLayout( parent), d_ptr( new HbAnchorLayoutPrivate )
{
    Q_D( HbAnchorLayout );
    d->q_ptr = this;
}

/*!
    Destructor.
*/
HbAnchorLayout::~HbAnchorLayout()
{
    if (d_ptr) {
        for (int i = count() - 1; i >= 0; --i) {
            QGraphicsLayoutItem *item = itemAt(i);
            // The following lines can be removed, but this removes the item
            // from the layout more efficiently than the implementation of
            // ~QGraphicsLayoutItem.
            removeAt(i);
            if (item) {
                item->setParentLayoutItem(0);
                if (item->ownedByLayout()) {
                    delete item;
                }
            }
        }
    }

    delete d_ptr;
}

/*!
    Creates an anchor, or updates an existing one, between the edges described by
    (\a startItem, \a startEdge) and (\a endItem, \a endEdge).

    The edges can be either horizontal (e.g. left) or vertical (e.g. top)
    and you can connect only the same type of edges with each other.
    That is, it is not allowed to connect e.g. top edge of an item to the
    left edge of another one. Also there are horizontal and vertical center edges.

    The distance between the two edges is defined by \a value.
    If \a value is positive the end edge is to the right or below the start edge.
    If \a value is negative the end edge is to the left or above the start edge.

    Anchors can be created between the parent layout and a child layout item,
    or between two child layout items, or even between two edges of the same
    child layout item (when you are essentially defining the height or width
    of the item).

    There could be only one anchor between two edges, that mean that every next
    call of setAnchor for the same edges will just update existing anchor between them.
    So even though anchors are directional, if you first set anchor between A and B, and
    after that from B to A, then second anchor will just override the first one.


    \a startItem and  \a endItem will automatically be added to the layout.

    \param startItem source item.
    \param startEdge source edge.
    \param endItem target item.
    \param endEdge target edge.
    \param value spacing (in pixels).
    \return true if anchor was successfully added, false otherwise
*/
bool HbAnchorLayout::setAnchor( QGraphicsLayoutItem *startItem,
                             Hb::Edge startEdge,
                             QGraphicsLayoutItem *endItem,
                             Hb::Edge endEdge,
                             qreal value )
{
    Q_D( HbAnchorLayout );
    if ( d->edgeType(startEdge) != d->edgeType(endEdge) ) {
        qWarning() << "HbAnchorLayout::setAnchor : You can't connect different type of edges";
        return false;
    }

    if ( !startItem || !endItem ) {
        qWarning() << "HbAnchorLayout::setAnchor : One of the items is NULL";
        return false;
    }

    if ( ( startItem == endItem ) && ( startEdge == endEdge ) ) {
        qWarning() << "HbAnchorLayout::setAnchor : You cannot set anchor between the same edge";
        return false;
    }

    d->addItemIfNeeded(startItem);
    d->addItemIfNeeded(endItem);

    HbAnchor* anchor = d->getAnchor(startItem, startEdge, endItem, endEdge);
    HbAnchor* anchor2 = d->getAnchor(endItem, endEdge, startItem, startEdge);

    if ( anchor ) {
        anchor->mValue = value;
    } else if ( anchor2 ) {
        anchor2->mValue = -value;
    } else {
        anchor = new HbAnchor(startItem, startEdge, endItem, endEdge, value);
        d->mAnchors.append(anchor);
    }

    invalidate();
    return true;
}

/*!

    Removes an anchor between the edges described by (\a item1, \a edge1) and
    (\a item2, \a edge2).

    Notice: The item will be removed from the layout if this is the last
    anchor connecting the item.

    \param item1 first item.
    \param edge1 first edge.
    \param item2 second item.
    \param edge2 second edge.
    \return true if anchor was successfully removed, false otherwise
*/
bool HbAnchorLayout::removeAnchor( QGraphicsLayoutItem *item1,
                                   Hb::Edge edge1,
                                   QGraphicsLayoutItem *item2,
                                   Hb::Edge edge2 )
{
    Q_D( HbAnchorLayout );
    HbAnchor *anchor = d->getAnchor( item1, edge1, item2, edge2 );
    if( !anchor ) {
        anchor = d->getAnchor( item2, edge2, item1, edge1 );
    }
    if ( anchor ) {
        d->mAnchors.removeAll( anchor );

        // Remove non-anchored items
        bool startFound = false;
        bool endFound = false;
        for ( int i = d->mAnchors.count() - 1; i >= 0; i-- ) {
            if ( d->mAnchors.at(i)->mStartItem == item1 ||
                 d->mAnchors.at(i)->mEndItem == item1 ) {
                    startFound = true;
            }
            if ( d->mAnchors.at(i)->mStartItem == item2 ||
                 d->mAnchors.at(i)->mEndItem == item2 ) {
                    endFound = true;
            }
        }
        if ( !startFound && item1 != this ) {
            item1->setParentLayoutItem( 0 );
            d->mItems.removeAt(indexOf(item1));
        }
        if ( !endFound && item2 != this) {
            item2->setParentLayoutItem( 0 );
            d->mItems.removeAt(indexOf(item2));
        }
        delete anchor;
        invalidate();
        return true;
    } else {
        return false;
    }
}

/*!
    From QGraphicsLayoutItem.
    Sets the geometry of all the child items of this layout.

    In case the layout is not valid geometry will not be set
    to any of the child items.

    \param rect rectangle for this layout.
    \sa isValid
*/
void HbAnchorLayout::setGeometry(const QRectF &rect)
{
    Q_D( HbAnchorLayout );

    QGraphicsLayout::setGeometry(rect);
    d->setItemGeometries();
    d->mInvalidateCalled = false;
}

/*!
    From QGraphicsLayoutItem.
    Removes the item at index, \a index, without destroying it.

    Removes all the anchors connected to the removed item.

    \param index index of item to be removed.
*/
void HbAnchorLayout::removeAt(int index)
{
    Q_D( HbAnchorLayout );
    if ( index < 0 || index > d->mItems.count()-1 ) {
        return;
    }
    QGraphicsLayoutItem *item = itemAt( index );
    if ( item ) {
        for ( int i = d->mAnchors.count() - 1; i >= 0; i-- ) {
            if ( d->mAnchors.at(i)->mStartItem == item ||
                 d->mAnchors.at(i)->mEndItem == item ) {
                    d->mAnchors.removeAt(i);
            }
        }

        item->setParentLayoutItem( 0 );
        d->mItems.removeAt(index);
    }

    invalidate();
}

/*!
    Removes given \a item.
    This is a convenience function. It's equivalent to calling removeAt(indexOf(item)).

    \param item item to be removed.
*/
void HbAnchorLayout::removeItem(QGraphicsLayoutItem* item)
{
    removeAt(indexOf(item));
}

/*!
    From QGraphicsLayoutItem.
    Returns the count of the child items in anchor layout.
    \return amount of items in this layout.
*/
int HbAnchorLayout::count() const
{
    Q_D( const HbAnchorLayout );
    return d->mItems.count();
}

/*!
    From QGraphicsLayoutItem.
    Returns a pointer to the item at an index \a index.
    \param index position of desired item.
    \return item at specified index.
*/
QGraphicsLayoutItem *HbAnchorLayout::itemAt(int index) const
{
    Q_D( const HbAnchorLayout );
    return d->mItems.value(index);
}

/*!
    Returns the index of given layout \a item, or -1 if not found.
    \param item item to look for.
    \return index of item or -1 if not found.
*/
int HbAnchorLayout::indexOf(const QGraphicsLayoutItem* item) const
{
    Q_D( const HbAnchorLayout );
    for ( int i=0; i< d->mItems.count(); i++) {
        if ( d->mItems.at(i) == item ) {
            return i;
        }
    }
    return -1;
}

/*!
    Returns true if anchor layout could be "solved" in the
    last setGeometry call. That is, the anchors of the layout do
    not create any contradictions in the geometry calculation.
    \return true if valid layout, false otherwise.
*/
bool HbAnchorLayout::isValid() const
{
    Q_D( const HbAnchorLayout );
    return ( d->mValid && ( ! d->mWrongAnchors ) );
}

/*!
    From QGraphicsLayoutItem.
*/
void HbAnchorLayout::invalidate()
{
    Q_D( HbAnchorLayout );
    d->mWrongAnchors = false;
    d->mInvalidateCalled = true;
    d->mEquationsDirty = true;
    QGraphicsLayout::invalidate();
}

/*!
    \reimp
*/
void HbAnchorLayout::widgetEvent(QEvent *e)
{
    QGraphicsLayout::widgetEvent(e);
}

/*!
    From QGraphicsLayoutItem. If size hint for certain set of items cannot be defined, then it returns default size hint (0/100/1000)
    \param which desired size hint.
    \param constraint optional constraint.
    \return calculated size hint.
*/
QSizeF HbAnchorLayout::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_D( const HbAnchorLayout );
    Q_UNUSED( constraint );

    return const_cast<HbAnchorLayoutPrivate*>(d)->sizeHint( which );
}

QSizeF HbAnchorLayoutPrivate::sizeHint( Qt::SizeHint which )
{
    if ( mEquationsDirty ) {
        mEquationsDirty = false;
        createEquations( Horizontal );
        createEquations( Vertical );
    }


    if( mLayoutVarH && mLayoutVarV ) {

        QSizeF res;

        if( mLayoutVarH->sizeProp.flags & SizeProperty::FlagFixed ) {
            res.setWidth( mLayoutVarH->sizeProp.pref );
        } else {
            if (which == Qt::MinimumSize) {
                res.setWidth( mLayoutVarH->sizeProp.min );
            } else if (which == Qt::PreferredSize ) {
                res.setWidth( mLayoutVarH->sizeProp.pref );
            } else {
                res.setWidth( mLayoutVarH->sizeProp.max );
            }
        }

        if( mLayoutVarV->sizeProp.flags & SizeProperty::FlagFixed ) {
            res.setHeight( mLayoutVarV->sizeProp.pref );
        } else {
            if (which == Qt::MinimumSize) {
                res.setHeight( mLayoutVarV->sizeProp.min );
            } else if (which == Qt::PreferredSize ) {
                res.setHeight( mLayoutVarV->sizeProp.pref );
            } else {
                res.setHeight( mLayoutVarV->sizeProp.max );
            }
        }

        return res;
    } else {
        if (which == Qt::MinimumSize) {
            return QSizeF( 0, 0 );
        } else if (which == Qt::PreferredSize ) {
            return QSizeF( 100, 100 );
        } else {
            return QSizeF( 1000, 1000 );
        }
    }
}

