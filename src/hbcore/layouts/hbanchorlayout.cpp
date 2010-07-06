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
#include "hbanchor.h"
#include "hbanchor_p.h"
#include "hbanchorlayoutengine_p.h"

#include <QLayout>
#include <QDebug>

#include "hblayoututils_p.h"


//Uncomment next define in order to get more debug prints.
//Similar define exists also in the engine side.
//#define HBANCHORLAYOUT_DEBUG

/*!
    \class HbAnchorLayout
    \brief HbAnchorLayout manages geometries of its child items with anchors
    that connect the layout items with each other.

    It also allows layout items to be missing and can fix anchor attachments.
    Here is an example and some simple rules how anchor fixing works (the example is
    only for horizontal direction - the same needs to be done for portrait as well).

    If anchors set allow ambiguos positioning of items, then layout tries to set items
    size as close to preferred as possible.

    \image html hbanchorlayout1.png

    From the image above, we have decided that the green node is always present. This
    means that all the other nodes in the horizontal graph can be optional.

    \image html hbanchorlayout2.png

    Then, we create the anchors starting from the non-optional node and point towards
    the edges of the layout. The anchor layout definition in the WidgetML would look like:

    \code

    <anchoritem srcId="green_item" srcEdge="LEFT" dstId="blue_item" dstEdge="RIGHT" />
    <anchoritem srcId="blue_item" srcEdge="LEFT" dstId="" dstEdge="LEFT" />
    <anchoritem srcId="green_item" srcEdge="RIGHT" dstId="red_item" dstEdge="LEFT" />
    <anchoritem srcId="red_item" srcEdge="RIGHT" dstId="yellow_item" dstEdge="LEFT" />
    <anchoritem srcId="yellow_item" srcEdge="RIGHT" dstId="" dstEdge="RIGHT" />

    \endcode

    As mentioned, the green node needs be present always. In practice, this means that the
    parent widget, which owns this anchor layout, needs to have a child widget with item
    name "green_item". \c HbStyle::setItemName for more details.

    If an optional node is missing, the anchors pointing to the node are
    changed to point to the node after (=towards the parent layout) the missing one.

    \image html hbanchorlayout3.png

    In the picture above, the blue and yellow items are missing. The anchor is fixed by removing
    the anchor definitions starting from the missing nodes.

    \stable
*/

/*!
    \enum HbAnchorLayout::Edge

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
    Type for mapping from layout item to node identifier.
    \internal
*/
typedef QMap<QGraphicsLayoutItem*, QString> ItemToNodeIdMap;
typedef ItemToNodeIdMap::iterator ItemToNodeIdMapIterator;
typedef ItemToNodeIdMap::const_iterator ItemToNodeIdMapConstIterator;


/*
    Result of findEndItem.
*/
struct HbFixedEndItemResult
{
    QGraphicsLayoutItem *mItem;
    HbAnchorLayout::Edge mEdge;
    qreal mMin;
    qreal mPref;
    qreal mMax;
    QSizePolicy::Policy mPolicy;
    HbAnchor::Direction mDirection;
};

class HbAnchorLayoutPrivate
{
public:
    Q_DECLARE_PUBLIC( HbAnchorLayout )

    HbAnchorLayoutPrivate();
    ~HbAnchorLayoutPrivate();

    void addItemIfNeeded(QGraphicsLayoutItem *item);

    static EdgeType edgeType( HbAnchorLayout::Edge edge );
    void setItemGeometries();
    void updateAnchorsAndItems();

    void createEquations( EdgeType type );

    int getEdgeIndex(QGraphicsLayoutItem *item, Hb::Edge edge);

    bool findEndItem(
        QList<HbFixedEndItemResult> &resultList,
        const HbAnchor *anchor,
        QStringList &ids) const;
    void resolveAnchors();
    void removeItemIfNeeded( QGraphicsLayoutItem *item );

    HbAnchor *setAnchor( HbAnchor *anchor );

    void setSizeProp( SizeProperty *v, QGraphicsLayoutItem *item, EdgeType type );
    void setSizeProp( SizeProperty *v, HbAnchor *item );
    GraphVertex *createCenterEdge( EdgeType type, QGraphicsLayoutItem *item,  Hb::Edge edge );
    void defineNextGeometry( const int itemIndexStart, const int itemIndexEnd, const int anchorIndex, const int definedItemIndex );


    QSizeF sizeHint(Qt::SizeHint which);

public:
    HbAnchorLayout * q_ptr;

    bool mEquationsDirty; // if true, we needed to re-create the equations (e.g. when new anchor is set)
    bool mValid;          // result of the calculations. false, if the equations cannot be solved.
    bool mInvalidateCalled; // set true in ::invalidate() and cleared after geometry is set in ::setGeometry
    bool mWrongAnchors;

    QList<HbAnchor*> mAllAnchors; // anchors that are set by user
    QList<HbAnchor*> mResolvedDynamicAnchors; //  references to generated anchors
    QList<HbAnchor*> mResolvedStaticAnchors; // references to anchors, that remains the same after resolving
    QList<HbAnchor*> mResolvedAnchors; // anchors that are passed to engine

    QList<QGraphicsLayoutItem*> mItems; // for addItem
    QList<QGraphicsLayoutItem*> mActualItems; // layouted items
    ItemToNodeIdMap mItemToNodeIdMap;

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

    QVector< bool > mAnchorsVisited;
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
    \internal functions
*/

inline bool idConditionStartStart( HbAnchor *anchor1, HbAnchor *anchor2 )
{
    return ( !anchor1->startNodeId().isNull() ) && ( anchor1->startNodeId() == anchor2->startNodeId() );
}
inline bool idConditionEndEnd( HbAnchor *anchor1, HbAnchor *anchor2 )
{
    return ( !anchor1->endNodeId().isNull() ) && ( anchor1->endNodeId() == anchor2->endNodeId() );
}
inline bool idConditionStartEnd( HbAnchor *anchor1, HbAnchor *anchor2 )
{
    return ( !anchor1->startNodeId().isNull() ) && ( anchor1->startNodeId() == anchor2->endNodeId() );
}
inline bool idConditionEndStart( HbAnchor *anchor1, HbAnchor *anchor2 )
{
    return ( !anchor1->endNodeId().isNull() ) && ( anchor1->endNodeId() == anchor2->startNodeId() );
}

inline bool itemConditionStartStart( HbAnchor *anchor1, HbAnchor *anchor2 )
{
    return ( anchor1->startItem() != 0 ) && ( anchor1->startItem() == anchor2->startItem() );
}
inline bool itemConditionEndEnd( HbAnchor *anchor1, HbAnchor *anchor2 )
{
    return ( anchor1->endItem() != 0 ) && ( anchor1->endItem() == anchor2->endItem() );
}
inline bool itemConditionStartEnd( HbAnchor *anchor1, HbAnchor *anchor2 )
{
    return ( anchor1->startItem() != 0 ) && ( anchor1->startItem() == anchor2->endItem() );
}
inline bool itemConditionEndStart( HbAnchor *anchor1, HbAnchor *anchor2 )
{
    return ( anchor1->endItem() != 0 ) && ( anchor1->endItem() == anchor2->startItem() );
}

inline bool edgeConditionStartStart( HbAnchor *anchor1, HbAnchor *anchor2 )
{
    return anchor1->startEdge() == anchor2->startEdge();
}
inline bool edgeConditionEndEnd( HbAnchor *anchor1, HbAnchor *anchor2 )
{
    return anchor1->endEdge() == anchor2->endEdge();
}
inline bool edgeConditionStartEnd( HbAnchor *anchor1, HbAnchor *anchor2 )
{
    return anchor1->startEdge() == anchor2->endEdge();
}
inline bool edgeConditionEndStart( HbAnchor *anchor1, HbAnchor *anchor2 )
{
    return anchor1->endEdge() == anchor2->startEdge();
}

inline int directionMultiplier( HbAnchor *anchor )
{
    return ( ( anchor->direction() == HbAnchor::Positive )?(1):(-1) );
}


/*!
    Returns list of effective anchors - those which has mappings to QGraphicsItem
    \return list of effective anchors.
*/
QList<HbAnchor*> HbAnchorLayout::effectiveAnchors()
{
    Q_D( HbAnchorLayout );
    d->resolveAnchors();
    return d->mResolvedAnchors;
}

/*!
    Returns list of all anchors set to this layout
    \return list of all anchors.
*/
QList<HbAnchor*> HbAnchorLayout::anchors() const
{
    Q_D( const HbAnchorLayout );
    return d->mAllAnchors;
}

/*
    \class HbAnchorLayoutPrivate
    \internal
*/
HbAnchorLayoutPrivate::HbAnchorLayoutPrivate() : mEquationsDirty(false), mValid(true), mInvalidateCalled( false ), mWrongAnchors( false ),
                                                mUsedRect( 0, 0, 0, 0 ), mLayoutVarH( 0 ), mLayoutVarV( 0 )

{
}

/*
    \internal
*/
HbAnchorLayoutPrivate::~HbAnchorLayoutPrivate()
{

    qDeleteAll( mEdgesVertical );
    qDeleteAll( mEdgesHorizontal );
    qDeleteAll( mVerticesVertical );
    qDeleteAll( mVerticesHorizontal );

    qDeleteAll( mEquationsHorizontal );
    qDeleteAll( mEquationsVertical );

    qDeleteAll( mAllAnchors );
    qDeleteAll( mResolvedDynamicAnchors );
}

/*
    \internal
*/
EdgeType HbAnchorLayoutPrivate::edgeType( HbAnchorLayout::Edge edge )
{
    EdgeType type( Horizontal );
    if( edge == Hb::TopEdge || edge == Hb::BottomEdge || edge == Hb::CenterVEdge ) {
        type = Vertical;
    }
    return type;
}

#ifdef HBANCHORLAYOUT_DEBUG
/*
    Returns string representation of value in \c Edge enumeration.
    \internal
*/
static QString edgeAsText(HbAnchorLayout::Edge edge)
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

static QString itemAsText(QGraphicsLayoutItem* item, QGraphicsLayout *layout)
{
    QString result = "<emtpy>";
    if ( item ) {
        result = (item == layout) ? "HbAnchorLayout" : "<unknown>";
        QGraphicsItem *gItem = item->graphicsItem();
        if (gItem) {
            if (gItem->isWidget()) {
                result = static_cast<QGraphicsWidget*>(gItem)->metaObject()->className();
            }
        }
    }
    return result;
}

#endif // HBANCHORLAYOUT_DEBUG

/*
    \internal
*/
void HbAnchorLayoutPrivate::updateAnchorsAndItems()
{
    Q_Q(HbAnchorLayout);
    resolveAnchors();

#ifdef HBANCHORLAYOUT_DEBUG
    QGraphicsWidget* w = HbLayoutUtils::parentWidget( q );
    if ( w ) {
        qDebug() << "AnchorLayout: Updating anchors for" << w->metaObject()->className();
    }
    const QString parentId =
        mItemToNodeIdMap.contains(q) ? mItemToNodeIdMap.value(q) : QString();
    qDebug() << "-- -- resolved";
    qDebug() << "-- count: " << mResolvedAnchors.size() << ", parent: " << parentId;
    foreach (const HbAnchor *item, mResolvedAnchors) {
        const QString itemTemplate("-- (%1 [%2], %3) - (%4 [%5], %6) = %7");
        qDebug() <<
            itemTemplate
            .arg(item->startNodeId())
            .arg(itemAsText(item->startItem(), q))
            .arg(edgeAsText(item->startEdge()))
            .arg(item->endNodeId())
            .arg(itemAsText(item->endItem(), q))
            .arg(edgeAsText(item->endEdge()))
            .arg(item->preferredLength()).toAscii().data();
    }
    qDebug() << "-- -- all";
    qDebug() << "-- count: " << mAllAnchors.size() << ", parent: " << parentId;
    foreach (const HbAnchor *item, mAllAnchors) {
        const QString itemTemplate("-- (%1 [%2], %3) - (%4 [%5], %6) = %7");
        qDebug() <<
            itemTemplate
            .arg(item->startNodeId())
            .arg(itemAsText(item->startItem(), q))
            .arg(edgeAsText(item->startEdge()))
            .arg(item->endNodeId())
            .arg(itemAsText(item->endItem(), q))
            .arg(edgeAsText(item->endEdge()))
            .arg(item->preferredLength()).toAscii().data();
    }
    qDebug() << "-- ";
#endif // HBANCHORLAYOUT_DEBUG

    // HbAnchorLayout will only touch items that have anchors defined.
    mActualItems.clear();
    for (QList<HbAnchor*>::const_iterator it = mResolvedAnchors.constBegin();
         it != mResolvedAnchors.constEnd();
         ++it) {

        const HbAnchor* item = *it;

        if (item->startItem() != q && !mActualItems.contains(item->startItem())) {
            mActualItems.append(item->startItem());
        }
        if (item->endItem() != q && !mActualItems.contains(item->endItem())) {
            mActualItems.append(item->endItem());
        }
    }

}

void HbAnchorLayoutPrivate::setSizeProp( SizeProperty *v, HbAnchor *item )
{
    const QSizePolicy::Policy policy = item->sizePolicy();

    if ( policy & QSizePolicy::ShrinkFlag ) {
        v->min = item->minimumLength();
    } else {
        v->min = item->preferredLength();
    }

    if ( policy & (QSizePolicy::GrowFlag | QSizePolicy::ExpandFlag) ) {
        v->max = item->maximumLength();
    } else {
        v->max = item->preferredLength();
    }

    v->pref = qBound( v->min, item->preferredLength(), v->max );

    v->flags |= (v->min == v->max) ? SizeProperty::FlagFixed : 0;
    v->flags |= (policy & QSizePolicy::ExpandFlag) ? SizeProperty::FlagExpanding : 0;

    if( policy & QSizePolicy::IgnoreFlag ) {
        v->pref = v->min;
        v->flags |= SizeProperty::FlagExpanding;
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
            qWarning() << "HbAnchorLayout: something wrong " << __LINE__;
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
            qWarning() << "HbAnchorLayout: something wrong " << __LINE__;
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
        qWarning() << "HbAnchorLayout: something wrong " << __LINE__;
        return 0;
    }

    GraphEdge *oldEdge = 0;

    for( int i = 0; i < edges->size(); i++ ) {
        oldEdge = edges->at(i);
        if( ( oldEdge->ref == item ) && ( oldEdge->startVertex == start ) && ( oldEdge->endVertex == end ) ){
            break;
        }
    }

    if( !oldEdge ) {
        qWarning() << "HbAnchorLayout: something wrong " << __LINE__;
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

void HbAnchorLayoutPrivate::defineNextGeometry(
    const int itemIndexStart,
    const int itemIndexEnd,
    const int anchorIndex,
    const int definedItemIndex )
{
    ItemGeometry *knownItemGeom, *unKnownItemGeom;
    Hb::Edge knownEdge, unKnownEdge;
    int sign;
    qreal itemSize;
    qreal anchorSize;
    bool isHorizontal;
    HbAnchor *anchor = mResolvedAnchors.at( anchorIndex );
    qreal leftPoint(0), rightPoint(0), sourcePoint(0), dstPointLeft(0);

    mAnchorsVisited[ anchorIndex ] = true;

    if( edgeType( anchor->startEdge() ) == Horizontal ) {
        isHorizontal = true;
    } else {
        isHorizontal = false;
    }

    if( itemIndexEnd != definedItemIndex ) {
        knownEdge = anchor->startEdge();
        unKnownEdge =  anchor->endEdge();

        knownItemGeom = &mItemsGeometry[itemIndexStart];
        unKnownItemGeom = &mItemsGeometry[itemIndexEnd];

        if( isHorizontal ) {
            mGeometryDefinedH[itemIndexEnd] = true;
            itemSize = mSolutionHorizontal.value( mVariablesHorizontal.findVariable( mActualItems.at(itemIndexEnd) ) );
            anchorSize = mSolutionHorizontal.value( mVariablesHorizontal.findVariable( anchor ) ) * directionMultiplier( anchor );
        } else {
            mGeometryDefinedV[itemIndexEnd] = true;
            itemSize = mSolutionVertical.value( mVariablesVertical.findVariable( mActualItems.at(itemIndexEnd) ) );
            anchorSize = mSolutionVertical.value( mVariablesVertical.findVariable( anchor ) ) * directionMultiplier( anchor );
        }

        sign = 1;
    } else {
        knownEdge =  anchor->endEdge();
        unKnownEdge = anchor->startEdge();

        knownItemGeom = &mItemsGeometry[itemIndexEnd];
        unKnownItemGeom = &mItemsGeometry[itemIndexStart];

        if( isHorizontal ) {
            mGeometryDefinedH[itemIndexStart] = true;
            itemSize = mSolutionHorizontal.value( mVariablesHorizontal.findVariable( mActualItems.at(itemIndexStart) ) );
            anchorSize = mSolutionHorizontal.value( mVariablesHorizontal.findVariable( anchor ) ) * directionMultiplier( anchor );
        } else {
            mGeometryDefinedV[itemIndexStart] = true;
            itemSize = mSolutionVertical.value( mVariablesVertical.findVariable( mActualItems.at(itemIndexStart) ) );
            anchorSize = mSolutionVertical.value( mVariablesVertical.findVariable( anchor ) ) * directionMultiplier( anchor );
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
            dstPointLeft = sourcePoint + sign * anchorSize;
            break;
        }
        case Hb::CenterHEdge:
        case Hb::CenterVEdge:
        {
            dstPointLeft = sourcePoint + sign * anchorSize - itemSize / 2;
            break;
        }
        case Hb::RightEdge:
        case Hb::BottomEdge:
        {
            dstPointLeft = sourcePoint + sign * anchorSize - itemSize;
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
void HbAnchorLayoutPrivate::setItemGeometries()
{
    Q_Q(HbAnchorLayout);
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

            int layoutIndex = mActualItems.size();

            mItemsGeometry[ layoutIndex ].x1 = 0;
            mItemsGeometry[ layoutIndex ].x2 = newRect.width();
            mItemsGeometry[ layoutIndex ].y1 = 0;
            mItemsGeometry[ layoutIndex ].y2 = newRect.height();
            mGeometryDefinedH[ layoutIndex ] = true;
            mGeometryDefinedV[ layoutIndex ] = true;


            for( int i = 0; i < mAnchorsVisited.size(); i++ ) {

                HbAnchor *anchor = mResolvedAnchors.at(i);


                if( ( anchor->startItem() != q ) && ( anchor->endItem() != q ) ) {
                    continue;
                }

                int startIndex = mActualItems.indexOf( anchor->startItem() ); // returns -1 if not found => this is layout
                int endIndex = mActualItems.indexOf( anchor->endItem() );

                mAnchorsVisited[i] = true;

                if( edgeType( anchor->startEdge() ) == Horizontal ) {
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
                    HbAnchor *anchor = mResolvedAnchors.at(i);

                    startIndex = mActualItems.indexOf( anchor->startItem() );
                    endIndex = mActualItems.indexOf( anchor->endItem() );
#ifdef HBANCHORLAYOUT_DEBUG
                    qDebug() << "startIndex:" << startIndex << "  endIndex" << endIndex;
#endif //HBANCHORLAYOUT_DEBUG
                    if( edgeType( anchor->startEdge() ) == Horizontal ) {
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

#ifdef HBANCHORLAYOUT_DEBUG
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

#ifdef HBANCHORLAYOUT_DEBUG
                qDebug( "Item %d: (%lf, %lf) : (%lf %lf)", i, calcGeom.x1, calcGeom.y1, calcGeom.x2, calcGeom.y2 );
#endif // HBANCHORLAYOUT_DEBUG
                mActualItems.at(i)->setGeometry( geom );
            }
        }
    }
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


        for( int i = 0; i < mResolvedAnchors.count(); i++) {
            HbAnchor* anchor = mResolvedAnchors.at(i);
            if ( edgeType( anchor->startEdge() ) == type ) {
                itemStart = 0;
                itemEnd = 0;
                for( int j = 0; j < vertices->size(); j++ ) {
                    if( ( vertices->at(j)->itemRef == anchor->startItem() ) &&
                            ( vertices->at(j)->itemSide == anchor->startEdge() ) ) {
                        itemStart = vertices->at(j);
                    } else if( ( vertices->at(j)->itemRef == anchor->endItem() ) &&
                        ( vertices->at(j)->itemSide == anchor->endEdge() ) ) {
                        itemEnd = vertices->at(j);
                    }
                }

                if( !itemStart ) {
                    itemStart = createCenterEdge( type, anchor->startItem(),  anchor->startEdge() );
                }
                if( !itemEnd ) {
                    itemEnd = createCenterEdge( type, anchor->endItem(),  anchor->endEdge() );
                }

                if( !itemStart ){
                    qWarning() << "HbAnchorLayout: internal error, line " << __LINE__;
                    mWrongAnchors = true;
                    AnchorLayoutEngine::instance()->cleanUp( layoutStart, layoutMiddle, layoutEnd, edges, vertices, el );
                    return;
                }
                if( !itemEnd ) {
                    qWarning() << "HbAnchorLayout: internal error, line " << __LINE__;
                    mWrongAnchors = true;
                    AnchorLayoutEngine::instance()->cleanUp( layoutStart, layoutMiddle, layoutEnd, edges, vertices, el );
                    return;
                }

                newEdge = new GraphEdge();
                itemStart->edges.append( newEdge );
                itemEnd->edges.append( newEdge );

                newEdge->startVertex = itemStart;
                newEdge->endVertex = itemEnd;

                se.mVar = vs->createVariable(anchor);
                se.mCoef = directionMultiplier( anchor );
                setSizeProp( &(se.mVar->sizeProp), anchor );
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
#ifdef HBANCHORLAYOUT_DEBUG
            qDebug() << "FAIL! " << __LINE__;
#endif //HBANCHORLAYOUT_DEBUG
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

        AnchorLayoutEngine::instance()->attachToLayout(
            layoutStart, layoutMiddle, layoutEnd, layoutVar, el );
        AnchorLayoutEngine::instance()->cleanUp(
            layoutStart, layoutMiddle, layoutEnd, edges, vertices, el );


        mAnchorsVisited.resize( mResolvedAnchors.size() * sizeof( bool ) );
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
    Finds new end item for problematic anchor.

    Follows the anchor that have the same start edge
    as the problematic anchor.

    Invariant:
        \a ids must be the exactly same in return. It is the array
        which nodes have already been visited - so in order to avoid
        infinite recursion, don't visit already visited.
*/
bool HbAnchorLayoutPrivate::findEndItem(
    QList<HbFixedEndItemResult> &resultList,
    const HbAnchor *problem,
    QStringList &ids) const
{
    HbFixedEndItemResult result;
    bool found = false;

    for (QList<HbAnchor*>::const_iterator it = mAllAnchors.constBegin();
         it != mAllAnchors.constEnd();
         ++it) {

        const HbAnchor* currentItem = *it;

        if (!currentItem->startNodeId().isNull() &&
            currentItem->startNodeId() == problem->endNodeId() &&
            currentItem->startEdge() == problem->startEdge() &&
            !ids.contains(currentItem->startNodeId())) {

            QGraphicsLayoutItem *item = currentItem->endItem();

            if (item) {
                found = true;
                result.mEdge = currentItem->endEdge();
                result.mItem = item;
                result.mMin = currentItem->minimumLength();
                result.mPref = currentItem->preferredLength();
                result.mMax = currentItem->maximumLength();
                result.mPolicy = currentItem->sizePolicy();
                result.mDirection = currentItem->direction();
                resultList.append( result );
            } else {
                ids.append(currentItem->startNodeId());
                found |= findEndItem(resultList, currentItem, ids);
                ids.takeLast();
            }
        }
    }

    return found;
}

/*
    Resolves anchors to be used in anchor layout calculations.

    For each anchor x with start id, start edge, end id, end edge:

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
void HbAnchorLayoutPrivate::resolveAnchors()
{
    HbAnchor *item;

    qDeleteAll( mResolvedDynamicAnchors );
    mResolvedDynamicAnchors.clear();
    mResolvedStaticAnchors.clear();

    for ( int i = 0; i < mAllAnchors.size(); i++ ) {

        HbAnchor *anchor = mAllAnchors.at(i);

        if( ( anchor->startItem() ) && ( anchor->endItem() ) ) {
            mResolvedStaticAnchors.append( anchor );
            continue;
        }

        if (anchor->startItem() && !anchor->endNodeId().isNull()) {
            QList<HbFixedEndItemResult> resultList;

            QStringList ids;
            ids.append(anchor->startNodeId());

            if (findEndItem(resultList, anchor, ids)) {
                for( int j = 0; j < resultList.size(); j++ ) {
                    item = new HbAnchor( anchor->startItem(), anchor->startEdge(), resultList.at(j).mItem, resultList.at(j).mEdge );
                    item->setMinimumLength( resultList.at(j).mMin );
                    item->setPreferredLength( resultList.at(j).mPref );
                    item->setMaximumLength( resultList.at(j).mMax );
                    item->setSizePolicy( resultList.at(j).mPolicy );
                    item->setDirection( resultList.at(j).mDirection );
                    mResolvedDynamicAnchors.append(item);
                }
            }
        } else {
            // Nothing needed.
        }
    }

    mResolvedAnchors = mResolvedDynamicAnchors + mResolvedStaticAnchors;
}

HbAnchor *HbAnchorLayoutPrivate::setAnchor( HbAnchor *anchor )
{
    Q_Q( HbAnchorLayout );
    // This method is called from HbAnchorLayout::setAnchor.

    if (HbAnchorLayoutPrivate::edgeType(anchor->startEdge()) !=
        HbAnchorLayoutPrivate::edgeType(anchor->endEdge())) {
        qWarning() << "HbAnchorLayout::setAnchor : You can't connect different type of edges";
        return 0;
    }

    if ( ( anchor->startNodeId().isNull() && ( anchor->startItem() == 0 ) ) ||
        ( anchor->endNodeId().isNull() && ( anchor->endItem() == 0 ) ) ){
        qWarning() << "HbAnchorLayout::setAnchor : Both ids must be valid";
        return 0;
    }

    if ( ( anchor->startNodeId() == anchor->endNodeId() ) && ( ( anchor->startItem() == anchor->endItem() ) ) &&
        ( anchor->startEdge() == anchor->endEdge() ) ) {
        qWarning() << "HbAnchorLayout::setAnchor : You cannot set anchor between the same edge";
        return 0;
    }

    const int count = mAllAnchors.size();
    for (int i = 0; i < count; ++i) {
        HbAnchor *item = mAllAnchors.at(i);

        if( ( idConditionStartStart( item, anchor ) || itemConditionStartStart( item, anchor ) ) &&
            ( idConditionEndEnd( item, anchor ) || itemConditionEndEnd( item, anchor ) ) &&
            ( edgeConditionStartStart( item, anchor ) ) &&
            ( edgeConditionEndEnd( item, anchor ) ) ){
                item->setSizePolicy( anchor->sizePolicy() );
                item->setMinimumLength( anchor->minimumLength() );
                item->setPreferredLength( anchor->preferredLength() );
                item->setMaximumLength( anchor->maximumLength() );
                item->setDirection( anchor->direction() );
                delete anchor;
                return item;
        } else if( ( idConditionStartEnd( item, anchor ) || itemConditionStartEnd( item, anchor ) ) &&
            ( idConditionEndStart( item, anchor )  || itemConditionEndStart( item, anchor ) ) &&
            ( edgeConditionStartEnd( item, anchor ) ) &&
            ( edgeConditionEndStart( item, anchor ) ) ){
                item->setSizePolicy( anchor->sizePolicy() );
                item->setMinimumLength( anchor->minimumLength() );
                item->setPreferredLength( anchor->preferredLength() );
                item->setMaximumLength( anchor->maximumLength() );
                item->setDirection( ( anchor->direction() == HbAnchor::Positive )?( HbAnchor::Negative ):( HbAnchor::Positive ) );
                delete anchor;
                return item;
        }
    }

    if( anchor->startItem() != 0 ){
        anchor->d_ptr->mStartId = mItemToNodeIdMap.value( anchor->startItem() );
    } else if( ! anchor->startNodeId().isNull() ) {
        anchor->d_ptr->mStartItem = mItemToNodeIdMap.key( anchor->startNodeId() );
    }

    if( anchor->endItem() != 0 ){
        anchor->d_ptr->mEndId = mItemToNodeIdMap.value( anchor->endItem() );
    } else if( ! anchor->endNodeId().isNull() ) {
        anchor->d_ptr->mEndItem = mItemToNodeIdMap.key( anchor->endNodeId() );
    }

    addItemIfNeeded( anchor->startItem() );
    addItemIfNeeded( anchor->endItem() );

    anchor->d_ptr->mParent = q;

    mAllAnchors.append(anchor);

    return anchor;
}

void HbAnchorLayoutPrivate::removeItemIfNeeded( QGraphicsLayoutItem *item )
{
    Q_Q( HbAnchorLayout );

    if( ( item == 0 ) || ( item == q ) ) {
        return;
    }

    for ( int i = 0; i < mAllAnchors.size(); i++ ) {
        HbAnchor *anchor = mAllAnchors.at(i);
        if ( ( anchor->startItem() == item ) || ( anchor->endItem() == item ) ) {
            return;
        }
    }

    item->setParentLayoutItem( 0 );
    mItems.removeAt(q->indexOf( item ));
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

    The distance between the two edges is defined by \a length.
    If \a length is positive the end edge is to the right or below the start edge.
    If \a length is negative the end edge is to the left or above the start edge.

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
    \param length spacing (in pixels).
    \return created anchor if it was successfully added, or zero otherwise
*/
HbAnchor *HbAnchorLayout::setAnchor( QGraphicsLayoutItem *startItem, Edge startEdge, QGraphicsLayoutItem *endItem, Edge endEdge, qreal length )
{
    Q_D( HbAnchorLayout );


    HbAnchor *anchor = new HbAnchor( startItem, startEdge, endItem, endEdge, length );

    HbAnchor *result = d->setAnchor( anchor );

    if( result ) {
        invalidate();
        return result;
    }

    delete anchor;

    return 0;
}

/*!
    Same as previous, but here it operates with node ids, instead of items itself.

    \param startId start id.
    \param startEdge start edge.
    \param endId end id.
    \param endEdge end edge.
    \param length spacing value for all edges starting from (\a startId, \a startEdge).
    \return created anchor if it was successfully added, or zero otherwise
*/
HbAnchor *HbAnchorLayout::setAnchor( const QString& startId, Edge startEdge, const QString& endId, Edge endEdge, qreal length )
{
    Q_D( HbAnchorLayout );

    HbAnchor *anchor = new HbAnchor( startId, startEdge, endId, endEdge, length );

    HbAnchor *result = d->setAnchor( anchor );

    if( result ) {
        invalidate();
        return result;
    }

    delete anchor;

    return 0;
}


/*!
    Set previously created anchor. Ownership is passed to layout.

    \param anchor anchor, created somewhere outside
    \return reference to updated/created anchor (not necessary the same as in input parameter), or zero if something was wrong.
*/
HbAnchor *HbAnchorLayout::setAnchor( HbAnchor *anchor )
{
    Q_D( HbAnchorLayout );

    HbAnchor *result = d->setAnchor( anchor );

    if( result ) {
        invalidate();
        return result;
    }

    return 0;
}


/*!
    Removes anchor (\a startId, \a startEdge, \a endNodeId, \a endEdge).

    \param startId start id.
    \param startEdge start edge.
    \param endId end id.
    \param endEdge end edge.
    \return true if success, false otherwise.
*/
bool HbAnchorLayout::removeAnchor( const QString& startNodeId, Edge startEdge, const QString& endNodeId, Edge endEdge )
{
    Q_D( HbAnchorLayout );
    bool modified = false;

    for (int i = d->mAllAnchors.size() - 1; i >= 0; --i) {
        HbAnchor* anchor = d->mAllAnchors[i];
        if( ( anchor->startNodeId() == startNodeId && anchor->startEdge() == startEdge &&
                anchor->endNodeId() == endNodeId && anchor->endEdge() == endEdge ) ||
            ( anchor->startNodeId() == endNodeId && anchor->startEdge() == endEdge &&
                anchor->endNodeId() == startNodeId && anchor->endEdge() == startEdge ) ){
            delete d->mAllAnchors.takeAt(i);
            modified = true;
        }
    }

    if (modified) {
        d->removeItemIfNeeded( d->mItemToNodeIdMap.key( startNodeId ) );
        d->removeItemIfNeeded( d->mItemToNodeIdMap.key( endNodeId ) );
        invalidate();
        return true;
    }
    return false;
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
bool HbAnchorLayout::removeAnchor( QGraphicsLayoutItem *startItem, Edge startEdge, QGraphicsLayoutItem *endItem, Edge endEdge )
{
    Q_D( HbAnchorLayout );
    bool modified = false;

    for (int i = d->mAllAnchors.size() - 1; i >= 0; --i) {
        HbAnchor* anchor = d->mAllAnchors[i];
        if( ( anchor->startItem() == startItem && anchor->startEdge() == startEdge &&
                anchor->endItem() == endItem && anchor->endEdge() == endEdge ) ||
                ( anchor->startItem() == endItem && anchor->startEdge() == endEdge &&
                anchor->endItem() == startItem && anchor->endEdge() == startEdge ) ){
            delete d->mAllAnchors.takeAt(i);
            modified = true;
        }
    }

    if (modified) {
        d->removeItemIfNeeded( startItem );
        d->removeItemIfNeeded( endItem );
        invalidate();
        return true;
    }
    return false;
}


/*!

    Removes and deletes an anchor (\a anchor) from layout.

    If layout contains exactly the same anchor, with the same reference, then only this
    one is removed and deleted. Otherwise all anchors with the same start and end points
    are removed and deleted ( \a anchor is not deleted in this case because this instance
    is not in layout ).

    Notice: The item will be removed from the layout if this is the last
    anchor connecting the item.

    \param anchor anchor to be removed.
    \return true if anchor was successfully removed, false otherwise
*/
bool HbAnchorLayout::removeAnchor( HbAnchor *anchor )
{
    Q_D( HbAnchorLayout );

    if( d->mAllAnchors.removeOne( anchor ) ) {
        d->removeItemIfNeeded( anchor->startItem() );
        d->removeItemIfNeeded( anchor->endItem() );
        delete anchor;
        invalidate();
        return true;
    }

    bool modified = true;

    for (int i = d->mAllAnchors.size() - 1; i >= 0; --i) {
        HbAnchor* item = d->mAllAnchors[i];
        if( ( ( idConditionStartStart( item, anchor ) || itemConditionStartStart( item, anchor ) ) &&
            ( idConditionEndEnd( item, anchor ) || itemConditionEndEnd( item, anchor ) ) &&
            ( edgeConditionStartStart( item, anchor ) ) &&
            ( edgeConditionEndEnd( item, anchor ) ) )    // condition for same direction anchor

            ||

            ( ( idConditionStartEnd( item, anchor ) || itemConditionStartEnd( item, anchor ) ) &&
            ( idConditionEndStart( item, anchor )  || itemConditionEndStart( item, anchor ) ) &&
            ( edgeConditionStartEnd( item, anchor ) ) &&
            ( edgeConditionEndStart( item, anchor ) ) ) ){  // condition for opposite direction anchor
            delete d->mAllAnchors.takeAt(i);
            modified = true;
            break;
        }
    }

    if( modified ) {
        d->removeItemIfNeeded( anchor->startItem() );
        d->removeItemIfNeeded( anchor->endItem() );
        invalidate();
        return true;
    }


    return modified;
}

/*!
    Removes all anchors starting or ending to \a nodeId.
    Same is done with associated item

    \param id id to be removed.
    \return true if success, false otherwise.
*/
bool HbAnchorLayout::removeNodeId( const QString& nodeId )
{
    Q_D( HbAnchorLayout );
    bool modified = false;

    // if association, do removal

    for (int i = d->mAllAnchors.size() - 1; i >= 0; --i) {
        HbAnchor *anchor = d->mAllAnchors.at(i);
        if (anchor->startNodeId() == nodeId || anchor->endNodeId() == nodeId) {
            QGraphicsLayoutItem *startItem = anchor->startItem();
            QGraphicsLayoutItem *endItem = anchor->endItem();

            delete d->mAllAnchors.takeAt(i);
            d->removeItemIfNeeded( startItem );
            d->removeItemIfNeeded( endItem );

            modified = true;
        }
    }

    removeMapping( nodeId );

    if (modified) {
        invalidate();
        return true;
    }
    return false;
}

/*!
    Clears all anchors.
    Note that this does not affect on mappings.
*/
void HbAnchorLayout::removeAnchors()
{
    Q_D( HbAnchorLayout );

    if( d->mAllAnchors.size() ) {
        qDeleteAll( d->mResolvedDynamicAnchors );
        qDeleteAll( d->mAllAnchors );
        d->mResolvedDynamicAnchors.clear();
        d->mResolvedStaticAnchors.clear();
        d->mResolvedAnchors.clear();
        d->mAllAnchors.clear();
        invalidate();
    }
}

/*!
    Sets identifier for \a item.
    \param item layout item.
    \param nodeId new id corresponding to \a item.
    \return true if success, false otherwise.
*/
bool HbAnchorLayout::setMapping( QGraphicsLayoutItem *item, const QString& nodeId )
{
    Q_D( HbAnchorLayout );

    bool modified = false;

    if ( !nodeId.isNull() && ( item != 0 ) ) {

        for( int i = 0; i < d->mAllAnchors.size(); i++ ) {
            HbAnchor *anchor = d->mAllAnchors.at(i);
            if( anchor->startItem() == item ) {
                anchor->d_ptr->mStartId = nodeId;
                modified = true;
            } else if( anchor->startNodeId() == nodeId ) {
                anchor->d_ptr->mStartItem = item;
                modified = true;
            }

            if( anchor->endItem() == item ) {
                anchor->d_ptr->mEndId = nodeId;
                modified = true;
            } else if( anchor->endNodeId() == nodeId ) {
                anchor->d_ptr->mEndItem = item;
                modified = true;
            }

        }

        // Remove previous item -> id.
        ItemToNodeIdMapIterator it = d->mItemToNodeIdMap.begin();
        while ( it != d->mItemToNodeIdMap.end() ) {
            if ( it.value() == nodeId ) {
                it = d->mItemToNodeIdMap.erase( it );
            } else {
                ++it;
            }
        }
        d->addItemIfNeeded( item );
        d->mItemToNodeIdMap.insert( item, nodeId );
    } else {
        return false;
    }

    if( modified ){
        invalidate();
    }
    return true;
}

/*!
    Resets mapping for \a item. All anchors are updated after that.

    item <=> "someId"  ---->  item <=> null

    \param item layout item.
    \return true if success, false otherwise.
*/
bool HbAnchorLayout::removeMapping( QGraphicsLayoutItem *item )
{
    Q_D( HbAnchorLayout );

    bool modified = false;

    if( ! item ) {
        return false;
    }

    for( int i = 0; i < d->mAllAnchors.size(); i++ ) {
        HbAnchor *anchor = d->mAllAnchors.at(i);

        if( anchor->startItem() == item ) {
            anchor->d_ptr->mStartId = QString();
            modified = true;
        }

        if( anchor->endItem() == item ) {
            anchor->d_ptr->mEndId = QString();
            modified = true;
        }
    }


    d->mItemToNodeIdMap.remove(item);

    if( modified ){
        invalidate();
    }
    return true;
}

/*!
    Resets mapping for \a nodeId. All anchors are updated after that.

    item <=> "nodeId"  ---->  0 <=> "nodeId"

    \param nodeId node id
    \return true if success, false otherwise.
*/
bool HbAnchorLayout::removeMapping( const QString& nodeId )
{
    Q_D( HbAnchorLayout );

    bool modified = false;

    if( nodeId.isNull() ) {
        return false;
    }

    for( int i = 0; i < d->mAllAnchors.size(); i++ ) {
        HbAnchor *anchor = d->mAllAnchors.at(i);

        if( anchor->startNodeId() == nodeId ) {
            anchor->d_ptr->mStartItem = 0;
            modified = true;
        }

        if( anchor->endNodeId() == nodeId ) {
            anchor->d_ptr->mEndItem = 0;
            modified = true;
        }
    }


    ItemToNodeIdMapIterator it = d->mItemToNodeIdMap.begin();
    while ( it != d->mItemToNodeIdMap.end() ) {
        if ( it.value() == nodeId ) {
            it = d->mItemToNodeIdMap.erase( it );
        } else {
            ++it;
        }
    }

    if( modified ){
        invalidate();
    }
    return true;
}


/*!
    Clears all item id mappings.
*/
void HbAnchorLayout::removeMappings()
{
    Q_D( HbAnchorLayout );
    d->mItemToNodeIdMap.clear();

    for( int i = 0; i < d->mAllAnchors.size(); i++ ) {
        HbAnchor *anchor = d->mAllAnchors.at(i);

        if( !anchor->startNodeId().isNull() ) {
            anchor->d_ptr->mStartItem = 0;
        }

        if( !anchor->endNodeId().isNull() ) {
            anchor->d_ptr->mEndItem = 0;
        }

    }
}

/*!
    Adds \a item.

    \param item item to be added.
    \param id id of this item.
*/
void HbAnchorLayoutPrivate::addItemIfNeeded(QGraphicsLayoutItem *item)
{
    Q_Q(HbAnchorLayout);

    if (!item) {
        return;
    }

    if (item == q) {
        return;
    }

    if (mItems.contains(item)) {
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
void HbAnchorLayout::removeItem(QGraphicsLayoutItem *item)
{
    removeAt(indexOf(item));
}

/*!
    Returns the index of given layout \a item, or -1 if not found.
    \param item item to look for.
    \return index of item or -1 if not found.
*/
int HbAnchorLayout::indexOf(const QGraphicsLayoutItem* item) const
{
    Q_D( const HbAnchorLayout );
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
bool HbAnchorLayout::isValid() const
{
    Q_D( const HbAnchorLayout );
    return ( d->mValid && ( ! d->mWrongAnchors ) );
}


/*!
    Returns node id for given item, or default constructed string if no mapping exist.
    \param item item to check.
    \return node id for given item.
*/
QString HbAnchorLayout::nodeId( QGraphicsLayoutItem *item ) const
{
    Q_D( const HbAnchorLayout );
    if( d->mItemToNodeIdMap.contains( item ) ) {
        return d->mItemToNodeIdMap.value( item );
    }
    return QString();
}

/*!
    Returns list of node ids that are mentioned in anchors list.
    \return list of node ids.
*/
QStringList HbAnchorLayout::nodeIds() const
{
    Q_D( const HbAnchorLayout );
    QStringList list;
    int c = d->mAllAnchors.count();
    while (c--) {
        QString id = d->mAllAnchors.at(c)->startNodeId();
        if (!list.contains(id) && !id.isNull()) {
            list.append(id);
        }
        id = d->mAllAnchors.at(c)->endNodeId();
        if (!list.contains(id) && !id.isNull()) {
            list.append(id);
        }
    }
    return list;
}

/*!
    Returns item reference for given node id, or zero if no mapping exist.
    \param nodeId node id to check.
    \return item reference for given item.
*/
QGraphicsLayoutItem *HbAnchorLayout::itemByNodeId( const QString& nodeId ) const
{
    Q_D( const HbAnchorLayout );
    return d->mItemToNodeIdMap.key( nodeId );
}


/*!
    \reimp
*/
void HbAnchorLayout::removeAt(int index)
{
    Q_D( HbAnchorLayout );
    if ( index < 0 || index > d->mItems.count()-1 ) {
        return;
    }
    QGraphicsLayoutItem *item = itemAt( index );
    if ( item ) {
        for ( int i = d->mAllAnchors.count() - 1; i >= 0; i-- ) {
            if ( ( ( d->mAllAnchors.at(i)->startItem() == item ) && ( d->mAllAnchors.at(i)->startNodeId().isNull() ) ) ||
                 ( ( d->mAllAnchors.at(i)->endItem() == item ) && ( d->mAllAnchors.at(i)->endNodeId().isNull() ) ) ) {
                    delete d->mAllAnchors.takeAt(i);
            }
        }

        removeMapping( d->mItemToNodeIdMap.value(item) );
        item->setParentLayoutItem( 0 );
        d->mItems.removeAt( index );
    }

    invalidate();
}

/*!
    \reimp
*/
void HbAnchorLayout::setGeometry(const QRectF &rect)
{
    Q_D( HbAnchorLayout );
    QGraphicsLayout::setGeometry(rect);
    d->setItemGeometries();
}

/*!
    \reimp
*/
int HbAnchorLayout::count() const
{
    Q_D( const HbAnchorLayout );
    return d->mItems.count();
}

/*!
    \reimp
*/
QGraphicsLayoutItem *HbAnchorLayout::itemAt(int index) const
{
    Q_D( const HbAnchorLayout );
    return d->mItems.value(index);
}

/*!
    \reimp
*/
void HbAnchorLayout::invalidate()
{
    Q_D( HbAnchorLayout );
    d->mInvalidateCalled = true;
    d->mWrongAnchors = false;
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
    From QGraphicsLayoutItem. If size hint for certain set of items cannot be defined,
    then it returns default size hint (0/100/1000)
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

QSizeF HbAnchorLayoutPrivate::sizeHint(Qt::SizeHint which)
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
