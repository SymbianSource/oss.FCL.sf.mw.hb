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

#include <QDebug>
#include <QList>
#include <QWidget>


#include "hbmeshlayout_p.h"
#include "hbmeshlayoutdebug_p.h"

#include "hbanchorlayoutengine_p.h"
#include "hblayoututils_p.h"
#include "hbanchor_p.h"

//Uncomment next define in order to get more debug prints.
//Similar define exists also in the engine side.
//#define HBMESHLAYOUT_DEBUG

/*!
    \class HbMeshLayout
    \brief HbMeshLayout manages geometries of its child items with anchors
    that connect the layout items with each other. This is different from
    \c HbAnchorLayout in such way that this allows layout items to be missing
    and can fix anchor attachments.

    Currently, the HbMeshLayout is an internal class which can be only utilized via the
    WidgetML within a widget.

    The mesh layout is a bit more fragile than the anchor layout. The anchor definitions
    need to support the missing items. Here are some simple rules how the mesh can be
    created (the example is only for horizontal direction - the same needs to be done
    for portrait as well).

    First, we need to find the child item (=node), which is always present i.e. cannot be missing.

    \image html hbmeshlayout1.png

    From the image above, we have decided that the green node is always present. This
    means that all the other nodes in the horizontal graph can be optional.

    \image html hbmeshlayout2.png

    Then, we create the anchors starting from the non-optional node and point towards
    the edges of the layout. The mesh layout definition in the WidgetML would look like:

    \code

    <meshitem src="green_item" srcEdge="LEFT" dst="blue_item" dstEdge="RIGHT" />
    <meshitem src="blue_item" srcEdge="LEFT" dst="" dstEdge="LEFT" />
    <meshitem src="green_item" srcEdge="RIGHT" dst="red_item" dstEdge="LEFT" />
    <meshitem src="red_item" srcEdge="RIGHT" dst="yellow_item" dstEdge="LEFT" />
    <meshitem src="yellow_item" srcEdge="RIGHT" dst="" dstEdge="RIGHT" />

    \endcode

    As mentioned, the green node needs be present always. In practice, this means that the
    parent widget, which owns this mesh layout, needs to have a child widget with item
    name "green_item". \c HbStyle::setItemName for more details.

    If an optional node is missing, the anchors pointing to the node are
    changed to point to the node after (=towards the parent layout) the missing one - this
    is called "fixing the mesh". The fixing only works if the end result can be determined
    i.e. two anchor starting from a missing node is prohibited.

    \image html hbmeshlayout3.png

    In the picture above, the blue and yellow items are missing. The mesh is fixed by removing
    the anchor definitions starting from the missing nodes.

    \proto
    \internal
*/

/*!
    \enum HbMeshLayout::Edge

    This enum defines the edges of a layout item.
*/

/*
    \enum EdgeType
    \internal
*/
enum EdgeType {
    Horizontal = 0,
    Vertical
};

/*
    Single anchor. If mHasSpacing equals to false, then anchor is sort of defined,
    but not really connected.
    \internal
*/
struct HbMeshAnchor
{
    QString mStartId;
    HbMeshLayout::Edge mStartEdge;
    QString mEndId;
    HbMeshLayout::Edge mEndEdge;
    bool mHasSpacing;
    qreal mSpacing;
};

/*
    Key for spacing overrides.
*/
struct HbMeshKey
{
    QString mId;
    HbMeshLayout::Edge mEdge;
};

/*
    Returns true if \a first is less than \a second. Needed for QMap use.
    \internal
*/
inline bool operator<(const HbMeshKey &first, const HbMeshKey &second)
{
    if( first.mId == second.mId ) {
        return int(first.mEdge) < int(second.mEdge);
    } else {
        return first.mId < second.mId;
    }
}

/*
    Type for mapping from layout item to node identifier.
    \internal
*/
typedef QMap<QGraphicsLayoutItem*, QString> HbMeshItemMap;
typedef HbMeshItemMap::iterator HbMeshItemMapIterator;
typedef HbMeshItemMap::const_iterator HbMeshItemMapConstIterator;

/*
    Type for inverse mapping of \c HbMeshItemMap.
    \internal
*/
typedef QMap<QString, QGraphicsLayoutItem*> HbMeshItemMapInverse;

/*
    Result of findEndItem.
*/
struct HbMeshEndItemResult
{
    QGraphicsLayoutItem *mItem;
    HbMeshLayout::Edge mEdge;
    qreal mSpacing;
};

class HbMeshLayoutPrivate
{
public:
    Q_DECLARE_PUBLIC( HbMeshLayout )

    HbMeshLayoutPrivate();
    ~HbMeshLayoutPrivate();

    void addItemIfNeeded(QGraphicsLayoutItem *item);

    static EdgeType edgeType( HbMeshLayout::Edge edge );
    void setItemGeometries();
    void updateAnchorsAndItems();

    void createEquations( EdgeType type );

    int getEdgeIndex(QGraphicsLayoutItem *item, Hb::Edge edge);

    bool hasAnchorSpacing(const HbMeshAnchor &anchor, qreal *spacing = 0) const;
    bool findEndItem(
        HbMeshEndItemResult &result,
        const HbMeshAnchor &anchor,
        const HbMeshItemMapInverse &inverse,
        QStringList &ids) const;
    void resolveAnchors( QList<HbAnchor*> *anchors );

    bool setAnchor(const HbMeshAnchor &anchor);
    int actualItemsIndexOf(QGraphicsLayoutItem *item) const;

    void setSizeProp( SizeProperty *v, QGraphicsLayoutItem *item, EdgeType type );
    GraphVertex *createCenterEdge( EdgeType type, QGraphicsLayoutItem *item,  Hb::Edge edge );
    void defineNextGeometry( const int itemIndexStart, const int itemIndexEnd, const int anchorIndex, const int definedItemIndex );


    QSizeF sizeHint(Qt::SizeHint which);

public:
    HbMeshLayout * q_ptr;

    bool mEquationsDirty; // if true, we needed to re-create the equations (e.g. when new anchor is set)
    bool mValid;          // result of the calculations. false, if the equations cannot be solved.
    bool mInvalidateCalled; // set true in ::invalidate() and cleared after geometry is set in ::setGeometry
    bool mWrongAnchors;

    QList<HbAnchor*> mAnchors;

    // mesh layout data
    QList<QGraphicsLayoutItem*> mItems; // for addItem
    QList<QGraphicsLayoutItem*> mActualItems; // layouted items
    HbMeshItemMap mMeshMap;
    QList<HbMeshAnchor> mMeshAnchors;
    QMap<HbMeshKey, qreal> mMeshSpacings;

    QRectF mUsedRect;

    // new items

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
QList<HbAnchor*> HbMeshLayoutDebug::getAnchors( HbMeshLayout* layout )
{
    QList<HbAnchor*> anchors;
    layout->d_ptr->resolveAnchors( &anchors );
    return anchors;
}

/*
    \class HbMeshLayoutPrivate
    \internal
*/
HbMeshLayoutPrivate::HbMeshLayoutPrivate() : mEquationsDirty(false), mValid(true), mInvalidateCalled( false ), mWrongAnchors( false ),
                                                mUsedRect( 0, 0, 0, 0 ), mLayoutVarH( 0 ), mLayoutVarV( 0 )

{
}

/*
    \internal
*/
HbMeshLayoutPrivate::~HbMeshLayoutPrivate()
{

    qDeleteAll( mEdgesVertical );
    qDeleteAll( mEdgesHorizontal );
    qDeleteAll( mVerticesVertical );
    qDeleteAll( mVerticesHorizontal );

    qDeleteAll( mEquationsHorizontal );
    qDeleteAll( mEquationsVertical );

    qDeleteAll( mAnchors );
}

/*
    \internal
*/
EdgeType HbMeshLayoutPrivate::edgeType( HbMeshLayout::Edge edge )
{
    EdgeType type( Horizontal );
    if( edge == Hb::TopEdge || edge == Hb::BottomEdge || edge == Hb::CenterVEdge ) {
        type = Vertical;
    }
    return type;
}

#ifdef HBMESHLAYOUT_DEBUG
/*
    Returns string representation of value in \c Edge enumeration.
    \internal
*/
static QString edgeAsText(HbMeshLayout::Edge edge)
{
    QString result;
    switch (edge) {
    case Hb::LeftEdge:
        result = "LEFT";
        break;

    case Hb::RightEdge:
        result = "RIGHT";
        break;

    case Hb::CenterHEdge:
        result = "CENTERH";
        break;

    case Hb::TopEdge:
        result = "TOP";
        break;

    case Hb::BottomEdge:
        result = "BOTTOM";
        break;

    case Hb::CenterVEdge:
        result = "CENTERV";
        break;

    default:
        result = "<UNDEFINED>";
        break;
    }

    return result;
}

#endif // HBMESHLAYOUT_DEBUG

/*
    \internal
*/
void HbMeshLayoutPrivate::updateAnchorsAndItems()
{
    Q_Q(HbMeshLayout);
    resolveAnchors( &mAnchors );

#ifdef HBMESHLAYOUT_DEBUG
    QGraphicsWidget* w = HbLayoutUtils::parentWidget( q );
    if ( w ) {
        qDebug() << "MeshLayout: Mesh anchors for" << w->metaObject()->className();
    }
    const QString parentId =
        mMeshMap.contains(q) ? mMeshMap.value(q) : QString();
    qDebug() << "-- count: " << mAnchors.size() << ", parent: " << parentId;
    foreach (const HbAnchor *item, mAnchors) {
        const QString itemTemplate("-- (%1, %2) - (%3, %4) = %5");
        qDebug() <<
            itemTemplate
            .arg(mMeshMap.value(item->mStartItem))
            .arg(edgeAsText(item->mStartEdge))
            .arg(mMeshMap.value(item->mEndItem))
            .arg(edgeAsText(item->mEndEdge))
            .arg(item->mValue).toAscii().data();
    }
    qDebug() << "-- ";
#endif // HBMESHLAYOUT_DEBUG

    // HbMeshLayout will only touch items that have anchors defined.
    mActualItems.clear();
    for (QList<HbAnchor*>::const_iterator it = mAnchors.constBegin();
         it != mAnchors.constEnd();
         ++it) {

        const HbAnchor* item = *it;

        if (item->mStartItem != q && !mActualItems.contains(item->mStartItem)) {
            mActualItems.append(item->mStartItem);
        }
        if (item->mEndItem != q && !mActualItems.contains(item->mEndItem)) {
            mActualItems.append(item->mEndItem);
        }
    }

}


void HbMeshLayoutPrivate::setSizeProp( SizeProperty *v, QGraphicsLayoutItem *item, EdgeType type )
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


GraphVertex *HbMeshLayoutPrivate::createCenterEdge( EdgeType type, QGraphicsLayoutItem *item,  Hb::Edge edge )
{
    GraphVertex *middle;
    GraphVertex *start = 0;
    GraphVertex *end = 0;

    QList<GraphEdge*> *edges = &mEdgesHorizontal;
    QList<GraphVertex*> *vertices = &mVerticesHorizontal;

    if( type == Vertical ) {
        if( edge != Hb::CenterVEdge ) {
#ifdef HBMESHLAYOUT_DEBUG
            qDebug() << "something wrong " << __LINE__;
#endif //HBMESHLAYOUT_DEBUG
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
#ifdef HBMESHLAYOUT_DEBUG
            qDebug() << "something wrong " << __LINE__;
#endif //HBMESHLAYOUT_DEBUG
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
#ifdef HBMESHLAYOUT_DEBUG
        qDebug() << "something wrong " << __LINE__;
#endif //HBMESHLAYOUT_DEBUG
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
#ifdef HBMESHLAYOUT_DEBUG
        qDebug() << "something wrong " << __LINE__;
#endif //HBMESHLAYOUT_DEBUG
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

void HbMeshLayoutPrivate::defineNextGeometry( const int itemIndexStart, const int itemIndexEnd, const int anchorIndex, const int definedItemIndex )
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
            itemSize = mSolutionHorizontal.value( mVariablesHorizontal.findVariable( mActualItems.at(itemIndexEnd) ) );
        } else {
            mGeometryDefinedV[itemIndexEnd] = true;
            itemSize = mSolutionVertical.value( mVariablesVertical.findVariable( mActualItems.at(itemIndexEnd) ) );
        }

        sign = 1;
    } else {
        knownEdge =  anchor->mEndEdge;
        unKnownEdge = anchor->mStartEdge;

        knownItemGeom = &mItemsGeometry[itemIndexEnd];
        unKnownItemGeom = &mItemsGeometry[itemIndexStart];

        if( isHorizontal ) {
            mGeometryDefinedH[itemIndexStart] = true;
            itemSize = mSolutionHorizontal.value( mVariablesHorizontal.findVariable( mActualItems.at(itemIndexStart) ) );
        } else {
            mGeometryDefinedV[itemIndexStart] = true;
            itemSize = mSolutionVertical.value( mVariablesVertical.findVariable( mActualItems.at(itemIndexStart) ) );
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


/*
    \internal
*/
void HbMeshLayoutPrivate::setItemGeometries()
{
    Q_Q(HbMeshLayout);
    const QRectF newRect = q->geometry();

    if( mWrongAnchors || ( mActualItems.isEmpty() ) ) {
        return;
    }


    if( (newRect != mUsedRect) || mInvalidateCalled ) {

        mInvalidateCalled = false;
        mUsedRect = newRect;

        if ( mEquationsDirty ) {
            updateAnchorsAndItems();
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
#ifdef HBMESHLAYOUT_DEBUG
            qDebug() << "LayoutH Id = " << mLayoutVarH->mId;
#endif // HBMESHLAYOUT_DEBUG
            mValid = AnchorLayoutEngine::instance()->solveEquation( el, vs, solution );
            if( !mValid ) {
                return;
            }

#ifdef HBMESHLAYOUT_DEBUG
            qDebug() << "solution->size() = " << solution->size();

            if( solution->size() > 0 ) {
                QHashIterator<Variable*, qreal>  i(*solution);
                while (i.hasNext()) {
                    i.next();
                    qDebug() << ( ( Variable* )( i.key() ) )->mId << ": " << i.value();
                }
            }
#endif //HBMESHLAYOUT_DEBUG
        }

        {
            QList<Expression*> *el = &mEquationsVertical;
            VariableSet *vs = &mVariablesVertical;
            Solution *solution = &mSolutionVertical;
            solution->clear();



            solution->insert( mLayoutVarV, newRect.height() );
#ifdef HBMESHLAYOUT_DEBUG
            qDebug() << "LayoutV Id = " << mLayoutVarV->mId;
#endif //HBMESHLAYOUT_DEBUG

            mValid = AnchorLayoutEngine::instance()->solveEquation( el, vs, solution );
            if( !mValid ) {
                return;
            }
#ifdef HBMESHLAYOUT_DEBUG
            qDebug() << "solution->size() = " << solution->size();


            if( solution->size() > 0 ) {
                QHashIterator<Variable*, qreal>  i(*solution);
                while (i.hasNext()) {
                    i.next();
                    qDebug() << ( ( Variable* )( i.key() ) )->mId << ": " << i.value();
                }
            }
#endif //HBMESHLAYOUT_DEBUG
        }

        {
            for( int i = 0; i < mAnchorsVisited.size(); i++ ) {
                mAnchorsVisited[i] = false;
            }

            for( int i = 0; i < mGeometryDefinedH.size(); i++ ) {
                mGeometryDefinedH[i] = false;
                mGeometryDefinedV[i] = false;
            }

            int layoutIndex = mActualItems.size();

            mItemsGeometry[ layoutIndex ].x1 = 0;//newRect.left();
            mItemsGeometry[ layoutIndex ].x2 = newRect.width();//newRect.right();
            mItemsGeometry[ layoutIndex ].y1 = 0;//newRect.top();
            mItemsGeometry[ layoutIndex ].y2 = newRect.height();//newRect.bottom();
            mGeometryDefinedH[ layoutIndex ] = true;
            mGeometryDefinedV[ layoutIndex ] = true;


            for( int i = 0; i < mAnchorsVisited.size(); i++ ) {

                HbAnchor *anchor = mAnchors.at(i);


                if( ( anchor->mStartItem != q ) && ( anchor->mEndItem != q ) ) {
                    continue;
                }

                int startIndex = mActualItems.indexOf( anchor->mStartItem ); // returns -1 if not found => this is layout
                int endIndex = mActualItems.indexOf( anchor->mEndItem );

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

                    startIndex = mActualItems.indexOf( anchor->mStartItem );
                    endIndex = mActualItems.indexOf( anchor->mEndItem );
#ifdef HBMESHLAYOUT_DEBUG
                    qDebug() << "startIndex:" << startIndex << "  endIndex" << endIndex;
#endif //HBMESHLAYOUT_DEBUG
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

#ifdef HBMESHLAYOUT_DEBUG
            QGraphicsWidget* w = HbLayoutUtils::parentWidget( q );
            if ( w ) {
                qDebug() << "Items of " << w->metaObject()->className();
            }
#endif


            Qt::LayoutDirection layoutDir = HbLayoutUtils::visualDirection(q);
            for( int i = 0; i < mActualItems.size(); i++ ) {
                QRectF geom;
                ItemGeometry calcGeom = mItemsGeometry.at(i);
                if( mGeometryDefinedH.at(i) ) {
                    geom.setLeft( mUsedRect.left() + calcGeom.x1 );
                    geom.setRight( mUsedRect.left() + calcGeom.x2 );
                } else {
                    geom.setLeft( mUsedRect.left() );
                    geom.setRight( mUsedRect.left() + mActualItems.at(i)->preferredWidth() );
                }
                if( mGeometryDefinedV.at(i) ) {
                    geom.setTop( mUsedRect.top() + calcGeom.y1 );
                    geom.setBottom( mUsedRect.top() + calcGeom.y2 );
                } else {
                    geom.setTop( mUsedRect.top() );
                    geom.setBottom( mUsedRect.top() + mActualItems.at(i)->preferredHeight() );
                }

                HbLayoutUtils::visualRect( layoutDir, geom, newRect );

#ifdef HBMESHLAYOUT_DEBUG
                qDebug( "Item %d: (%lf, %lf) : (%lf %lf)", i, calcGeom.x1, calcGeom.y1, calcGeom.x2, calcGeom.y2 );
        //        qDebug() << "Item " <<  i << "(" << ((QGraphicsWidget*)mActualItems.at(i))->metaObject()->className() << ")" << " geom " << geom;
#endif // HBMESHLAYOUT_DEBUG
                mActualItems.at(i)->setGeometry( geom );
            }
        }
    }
}

/*!
    \internal
*/
void HbMeshLayoutPrivate::createEquations( EdgeType type )
{
    Q_Q(HbMeshLayout);

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


        for ( int i = 0; i < mActualItems.count(); i++ ) {
            QGraphicsLayoutItem *item = mActualItems.at( i );
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

#ifdef HBMESHLAYOUT_DEBUG
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
#endif // HBMESHLAYOUT_DEBUG

        if( ! AnchorLayoutEngine::instance()->processItems( edges, vertices, vs, el ) ) {
            mWrongAnchors = true;
            AnchorLayoutEngine::instance()->cleanUp( layoutStart, layoutMiddle, layoutEnd, edges, vertices, el );
#ifdef HBMESHLAYOUT_DEBUG
            qDebug() << "FAIL! " << __LINE__;
#endif //HBMESHLAYOUT_DEBUG
            return;
        }

#ifdef HBMESHLAYOUT_DEBUG

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
#endif //HBMESHLAYOUT_DEBUG

        layoutVar = vs->createVariable( q );
        layoutVar->sizeProp.min = 0;
        layoutVar->sizeProp.max = 1000;
        layoutVar->sizeProp.pref = 100;
        layoutVar->sizeProp.flags = 0;

        AnchorLayoutEngine::instance()->attachToLayout( layoutStart, layoutMiddle, layoutEnd, layoutVar, el );
        AnchorLayoutEngine::instance()->cleanUp( layoutStart, layoutMiddle, layoutEnd, edges, vertices, el );


        mAnchorsVisited.resize( mAnchors.size() * sizeof( bool ) );
        mGeometryDefinedH.resize( ( mActualItems.size() + 1  ) * sizeof( bool ) );
        mGeometryDefinedV.resize( ( mActualItems.size() + 1 ) * sizeof( bool ) );
        mItemsGeometry.resize( ( mActualItems.size() + 1 ) * sizeof( ItemGeometry ) );

        if( type == Vertical ) {
            mLayoutVarV = layoutVar;
        } else {
            mLayoutVarH = layoutVar;
        }
    }
}

/*
    Creates inverse mapping, i.e. from id -> layout item.
*/
static HbMeshItemMapInverse createInverse(
    const HbMeshItemMap &map)
{
    HbMeshItemMapInverse result;

    HbMeshItemMapConstIterator end = map.constEnd();
    for (HbMeshItemMapConstIterator it = map.constBegin(); it != end; ++it) {
        result.insert(it.value(), it.key());
    }

    return result;
}

/*
    Checks if anchor has spacing value defined. It will be returned in \a spacing
    if it's not NULL.
*/
bool HbMeshLayoutPrivate::hasAnchorSpacing(
    const HbMeshAnchor &anchor,
    qreal *spacing) const
{
    if (spacing) {

        HbMeshKey key;
        key.mId = anchor.mStartId;
        key.mEdge = anchor.mStartEdge;

        QMap<HbMeshKey, qreal>::const_iterator spacingIt =
            mMeshSpacings.constFind(key);

        bool hasSpacing = true;
        if (spacingIt != mMeshSpacings.constEnd()) {
            *spacing = spacingIt.value();
        } else if (anchor.mHasSpacing) {
            *spacing = anchor.mSpacing;
        } else {
            hasSpacing = false;
        }
        return hasSpacing;

    } else {
        if (anchor.mHasSpacing) {
            return true;
        }

        HbMeshKey key;
        key.mId = anchor.mStartId;
        key.mEdge = anchor.mStartEdge;
        return mMeshSpacings.contains(key);
    }
}

/*
    Finds new end item for problematic anchor.

    Follows the anchor that have the same start edge
    as the problematic anchor.

    Invariant:
        \a ids must be the exactly same in return. It is the array
        which nodes have already been visited - so in order to avoid
        infinite recursion, don't visit already visited.
*/
bool HbMeshLayoutPrivate::findEndItem(
    HbMeshEndItemResult &result,
    const HbMeshAnchor &problem,
    const HbMeshItemMapInverse &inverse,
    QStringList &ids) const
{

    for (QList<HbMeshAnchor>::const_iterator it = mMeshAnchors.constBegin();
         it != mMeshAnchors.constEnd();
         ++it) {

        const HbMeshAnchor &currentItem = *it;

        if (currentItem.mStartId == problem.mEndId &&
            currentItem.mStartEdge == problem.mStartEdge &&
            !ids.contains(currentItem.mStartId)) {

            qreal currentSpacing(0.0);

            if (hasAnchorSpacing(currentItem, &currentSpacing)) {
                QGraphicsLayoutItem *item = inverse.value(currentItem.mEndId);
                bool found = false;

                if (item) {
                    found = true;
                    result.mEdge = currentItem.mEndEdge;
                    result.mItem = item;
                    result.mSpacing = currentSpacing;
                } else {
                    ids.append(currentItem.mStartId);
                    found = findEndItem(result, currentItem, inverse, ids);
                    ids.takeLast();
                }
                if (found) {
                    // We have found an end item. There can be multiple end items,
                    // but (for now) the first one is selected.
                    return true;
                }
            }
        }
    }

    return false;
}

/*
    Resolves anchors to be used in anchor layout calculations.

    For each anchor x with start id, start edge, end id, end edge:

    If anchor x does not have spacing defined, anchor is ignored.
    Note that you can define spacing either in anchor or spacing overrides.

    If there is layout items corresponding to both start id and end id,
    anchor is used automatically.
    If there is layout item corresponding to start id, then we try to
    "fix" anchor by looking for a path of anchors (same direction, with spacing defined)
    from anchor x's end id as starting point to such end id that has layout item.
    If found, anchor is fixed by replacing end id with found end id.

    So direction of anchors affect this resolution process, but not in the
    anchor layout calculations.

    \sa findEndItem
*/
void HbMeshLayoutPrivate::resolveAnchors( QList<HbAnchor*> *anchors )
{
    HbMeshItemMapInverse map = createInverse(mMeshMap);

    int count = 0;
    HbAnchor *item;

    bool isAnchorNew = false;

    for (QList<HbMeshAnchor>::const_iterator it = mMeshAnchors.constBegin();
         it != mMeshAnchors.constEnd();
         ++it) {

        const HbMeshAnchor &anchor = *it;

        QGraphicsLayoutItem *startItem = map.value(anchor.mStartId);

        if (startItem) {
            qreal value;

            if (hasAnchorSpacing(anchor, &value)) {
                // anchor really exists

                if( count > anchors->size() - 1 ) {
                    isAnchorNew = true;
                }

                if( isAnchorNew ) {
                    item = new HbAnchor(startItem, anchor.mStartEdge, 0 /*end item*/, anchor.mEndEdge, value);
                } else {
                    item = anchors->at( count );
                    item->mStartItem = startItem;
                    item->mStartEdge = anchor.mStartEdge;
                    item->mEndItem = 0;
                    item->mEndEdge = anchor.mEndEdge;
                    item->mValue = value;
                }

                QGraphicsLayoutItem *endItem = map.value(anchor.mEndId);
                if (endItem) {
                    // this is already valid anchor

                    item->mEndItem = endItem;
                    if( isAnchorNew ) {
                        anchors->append(item);
                    }
                    count++;
                } else {
                    // try to "fix" anchor

                    HbMeshEndItemResult result;
                    result.mItem = 0;
                    result.mEdge = Hb::LeftEdge;
                    result.mSpacing = 0;

                    QStringList ids;
                    ids.append(anchor.mStartId);

                    if (findEndItem(result, anchor, map, ids)) {
                        item->mEndEdge = result.mEdge;
                        item->mEndItem = result.mItem;
                        item->mValue = result.mSpacing;
                        if( isAnchorNew ) {
                            anchors->append(item);
                        }
                        count++;
                    }
                }
            }

        } else {
            // Nothing needed.
        }
    }

    while( count < anchors->size() ) {
        delete anchors->last();
        anchors->removeLast();
    }
}

bool HbMeshLayoutPrivate::setAnchor(const HbMeshAnchor& anchor)
{
    // This method is called from HbMeshLayout::setAnchor.

    if (HbMeshLayoutPrivate::edgeType(anchor.mStartEdge) !=
        HbMeshLayoutPrivate::edgeType(anchor.mEndEdge)) {
        qWarning() << "HbMeshLayout::setAnchor : You can't connect different type of edges";
        return false;
    }

    if (anchor.mStartId.isNull() ||
        anchor.mEndId.isNull()) {
        qWarning() << "HbMeshLayout::setAnchor : Both ids must be valid";
        return false;
    }

    if ((anchor.mStartId == anchor.mEndId) &&
        (anchor.mStartEdge == anchor.mEndEdge)) {
        qWarning() << "HbMeshLayout::setAnchor : You cannot set anchor between the same edge";
        return false;
    }

    bool modified = false;

    const int count = mMeshAnchors.size();
    for (int i = 0; i < count; ++i) {
        HbMeshAnchor& item = mMeshAnchors[i];
        if (item.mStartId == anchor.mStartId &&
            item.mStartEdge == anchor.mStartEdge) {
#ifdef HBMESHLAYOUT_DEBUG
                if (item.mEndId != anchor.mEndId ||
                    item.mEndEdge != anchor.mEndEdge) {
                        qDebug() << "MeshLayout: Replacing existing anchor!"
                            << "Check your layout definition whether this is really intentional."
                            << "There can be only one anchor starting from one edge.";
                }
#endif
            modified = true;

            // Only one anchor starting from single (startId, startEdge).
            item = anchor;
            break;
        }
    }

    if (!modified) {
        mMeshAnchors.append(anchor);
    }

    return true;
}

int HbMeshLayoutPrivate::actualItemsIndexOf(QGraphicsLayoutItem *item) const
{
    for (int i=0; i < mActualItems.count(); i++) {
        if (mActualItems.at(i) == item) {
            return i;
        }
    }
    return -1;
}

/*!
    Constructor.
*/
HbMeshLayout::HbMeshLayout(QGraphicsLayoutItem *parent)
: QGraphicsLayout( parent), d_ptr( new HbMeshLayoutPrivate )
{
    Q_D( HbMeshLayout );
    d->q_ptr = this;
}

/*!
    Destructor.
*/
HbMeshLayout::~HbMeshLayout()
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
    Sets anchor between (startId, startEdge) to (endId, endEdge). Optionally, you can provide
    spacing in \a defaultSpacing parameter. Note that you cannot have more than one anchor from
    (id, edge) pair. So, \c startId shouldn't represent this layout item.

    Note that previously define spacing override is still used as defined for
    (\a startId, \a startEdge).

    \param startId start id.
    \param startEdge start edge.
    \param endId end id.
    \param endEdge end edge.
    \param spacing spacing value for all edges starting from (\a startId, \a startEdge).
    \return true if success, false otherwise.
*/
bool HbMeshLayout::setAnchor(const QString& startId, Edge startEdge, const QString& endId, Edge endEdge, qreal spacing)
{
    Q_D( HbMeshLayout );

    HbMeshAnchor anchor;
    anchor.mStartId = startId;
    anchor.mStartEdge = startEdge;
    anchor.mEndId = endId;
    anchor.mEndEdge = endEdge;
    anchor.mHasSpacing = true;
    anchor.mSpacing = spacing;

    if (d->setAnchor(anchor)) {
        invalidate();
        return true;
    }
    return false;
}

bool HbMeshLayout::setAnchor(const QString& startId, Edge startEdge, const QString& endId, Edge endEdge)
{
    Q_D( HbMeshLayout );

    HbMeshAnchor anchor;
    anchor.mStartId = startId;
    anchor.mStartEdge = startEdge;
    anchor.mEndId = endId;
    anchor.mEndEdge = endEdge;
    anchor.mHasSpacing = false;
    anchor.mSpacing = 0;

    if (d->setAnchor(anchor)) {
        invalidate();
        return true;
    }
    return false;
}

/*!
    Removes anchor starting from (\a startId, \a startEdge). As you cannot have more
    than one anchor starting from a (id, edge) pair, provided (\a startId, \a startEdge)
    pair uniquely defines anchor to be removed.

    Note that this does not affect on spacing override.

    \param startId id.
    \param startEdge edge.
    \param spacing spacing value for all edges starting from (\a startId, \a startEdge).
    \return true if success, false otherwise.
*/
bool HbMeshLayout::removeAnchor(const QString& startId, Edge startEdge)
{
    Q_D( HbMeshLayout );
    bool modified = false;

    for (int i = d->mMeshAnchors.size() - 1; i >= 0; --i) {
        const HbMeshAnchor& anchor = d->mMeshAnchors[i];
        if (anchor.mStartId == startId && anchor.mStartEdge == startEdge) {
            d->mMeshAnchors.removeAt(i);
            modified = true;
            break;
        }
    }

    if (modified) {
        invalidate();
        return true;
    }
    return false;
}

/*!
    Removes all anchors starting or ending to \a id.
    Note that this does not affect on spacing override.

    \param id id to be removed.
    \return true if success, false otherwise.
*/
bool HbMeshLayout::removeAnchors(const QString& id)
{
    Q_D( HbMeshLayout );
    bool modified = false;

    for (int i = d->mMeshAnchors.size() - 1; i >= 0; --i) {
        const HbMeshAnchor& anchor = d->mMeshAnchors[i];
        if (anchor.mStartId == id || anchor.mEndId == id) {
            d->mMeshAnchors.removeAt(i);
            modified = true;
        }
    }

    if (modified) {
        invalidate();
        return true;
    }
    return false;
}

/*!
    Clears all anchors.
    Note that this does not affect on spacing override.
*/
void HbMeshLayout::clearAnchors()
{
    Q_D( HbMeshLayout );
    if (d->mMeshAnchors.size()) {
        d->mMeshAnchors.clear();
        invalidate();
    }
}

/*!
    Overrides spacing for anchor starting from (\a startId, \a startEdge).
    \param startId id.
    \param startEdge edge.
    \param spacing spacing value for anchor starting from (\a startId, \a startEdge).
*/
void HbMeshLayout::overrideSpacing(const QString& startId, Edge startEdge, qreal spacing)
{
    Q_D( HbMeshLayout );

    HbMeshKey key;
    key.mId = startId;
    key.mEdge = startEdge;

    d->mMeshSpacings.insert(key, spacing); // will replace spacing if key already exists

    invalidate();
}

/*!
    Resets spacing override for anchor starting from (\a startId, \a startEdge).
    \param startId id.
    \param startEdge edge.
*/
void HbMeshLayout::resetSpacingOverride(const QString& startId, Edge startEdge)
{
    Q_D( HbMeshLayout );
    HbMeshKey key;
    key.mId = startId;
    key.mEdge = startEdge;

    if(d->mMeshSpacings.remove(key)) {
        invalidate();
    }
}

/*!
    Clears all spacing overrides.
*/
void HbMeshLayout::clearSpacingOverrides()
{
    Q_D( HbMeshLayout );
    if (d->mMeshSpacings.size()) {
        d->mMeshSpacings.clear();
        invalidate();
    }
}

/*!
    Sets identifier for \a item. You can reset this mapping by
    passing \c UndefineId as \a id.
    \param item layout item.
    \param id new id corresponding to \a item.
*/
void HbMeshLayout::setItemId(QGraphicsLayoutItem *item, const QString& id)
{
    Q_D( HbMeshLayout );

    bool modified = false;

    if (!id.isNull()) {

        // Remove previous item -> id.
        HbMeshItemMapIterator it = d->mMeshMap.begin();
        while (it != d->mMeshMap.end()) {
            if (it.value() == id) {
                it = d->mMeshMap.erase(it);
            } else {
                ++it;
            }
        }
        d->addItemIfNeeded(item);
        d->mMeshMap.insert(item, id);
        modified = true;
    } else {
        modified = (d->mMeshMap.remove(item) != 0);
    }

    if (modified) {
        invalidate();
    }
}

/*!
    Clears all item id mappings.
*/
void HbMeshLayout::clearItemIds()
{
    Q_D( HbMeshLayout );
    d->mMeshMap.clear();
}

/*!
    Adds \a item.

    \param item item to be added.
    \param id id of this item.
*/
void HbMeshLayoutPrivate::addItemIfNeeded(QGraphicsLayoutItem *item)
{
    Q_Q(HbMeshLayout);

    if (!item) {
        //qWarning() << "HbMeshLayout::addItemIfNeeded : item is NULL";
        return;
    }

    if (item == q) {
        //qWarning() << "HbMeshLayout::addItemIfNeeded : layout cannot be added";
        return;
    }

    if (mItems.contains(item)) {
        //qWarning() << "HbMeshLayout::addItemIfNeeded : item is already in layout";
        return;
    }

    HbLayoutUtils::addChildItem(q, item);
    mItems.append(item);
}

/*!
    Removes given \a item.
    This is a convenience function. It's equivalent to calling removeAt(indexOf(item)).

    \param item item to be removed.
*/
void HbMeshLayout::removeItem(QGraphicsLayoutItem *item)
{
    removeAt(indexOf(item));
}

/*!
    Returns the index of given layout \a item, or -1 if not found.
    \param item item to look for.
    \return index of item or -1 if not found.
*/
int HbMeshLayout::indexOf(const QGraphicsLayoutItem* item) const
{
    Q_D( const HbMeshLayout );
    for (int i=0; i < d->mItems.count(); i++) {
        if (d->mItems.at(i) == item) {
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
bool HbMeshLayout::isValid() const
{
    Q_D( const HbMeshLayout );
    return ( d->mValid && ( ! d->mWrongAnchors ) );
}


QString HbMeshLayout::nodeId( QGraphicsLayoutItem *item ) const
{
    Q_D( const HbMeshLayout );
    if( d->mMeshMap.contains( item ) ) {
        return d->mMeshMap.value( item );
    }
    return QString();
}

QStringList HbMeshLayout::nodeIds() const
{
    Q_D( const HbMeshLayout );
    QStringList list;
    int c = d->mMeshAnchors.count();
    while (c--) {
        QString id = d->mMeshAnchors.at(c).mStartId;
        if (!list.contains(id)) {
            list.append(id);
        }
        id = d->mMeshAnchors.at(c).mEndId;
        if (!list.contains(id)) {
            list.append(id);
        }
    }
    return list;
}

QGraphicsLayoutItem *HbMeshLayout::itemById( const QString& id ) const
{
    Q_D( const HbMeshLayout );
    return d->mMeshMap.key( id );
}

qreal HbMeshLayout::spacing( const QString& startId, Edge startEdge ) const
{
    Q_D( const HbMeshLayout );
    HbMeshKey key;
    key.mId = startId;
    key.mEdge = startEdge;

    return d->mMeshSpacings.value( key );
}



/*!
    \reimp
*/
void HbMeshLayout::removeAt(int index)
{
    Q_D( HbMeshLayout );
    if ( index < 0 || index > d->mItems.count()-1 ) {
        return;
    }
    QGraphicsLayoutItem *item = itemAt( index );
    if ( item ) {
        setItemId(item, QString());

        item->setParentLayoutItem( 0 );
        d->mItems.removeAt(index);
    }

    invalidate();
}

/*!
    \reimp
*/
void HbMeshLayout::setGeometry(const QRectF &rect)
{
    Q_D( HbMeshLayout );
    QGraphicsLayout::setGeometry(rect);
    d->setItemGeometries();
}

/*!
    \reimp
*/
int HbMeshLayout::count() const
{
    Q_D( const HbMeshLayout );
    return d->mItems.count();
}

/*!
    \reimp
*/
QGraphicsLayoutItem *HbMeshLayout::itemAt(int index) const
{
    Q_D( const HbMeshLayout );
    return d->mItems.value(index);
}

/*!
    \reimp
*/
void HbMeshLayout::invalidate()
{
    Q_D( HbMeshLayout );
    d->mInvalidateCalled = true;
    d->mWrongAnchors = false;
    d->mEquationsDirty = true;
    QGraphicsLayout::invalidate();
}

/*!
    \reimp
*/
void HbMeshLayout::widgetEvent(QEvent *e)
{
    QGraphicsLayout::widgetEvent(e);
}
/*!
    \reimp
*/
QSizeF HbMeshLayout::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_D( const HbMeshLayout );
    Q_UNUSED( constraint );

    return const_cast<HbMeshLayoutPrivate*>(d)->sizeHint( which );
}

QSizeF HbMeshLayoutPrivate::sizeHint(Qt::SizeHint which)
{
    if ( mEquationsDirty ) {
        updateAnchorsAndItems();
        createEquations( Horizontal );
        createEquations( Vertical );
        mEquationsDirty = false;
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

