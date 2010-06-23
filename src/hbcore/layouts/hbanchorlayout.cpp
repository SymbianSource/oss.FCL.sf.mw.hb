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
#include <QDebug>

#include "hblayoututils_p.h"


//Uncomment next define in order to get more debug prints.
//Similar define exists also in the engine side.
//#define HBANCHORLAYOUT_DEBUG

#ifdef HBANCHORLAYOUT_DEBUG
#ifndef Q_WS_S60
#include "hbspaceritem_p.h"
#endif
#endif

/*!
    \class HbAnchorLayout
    \brief HbAnchorLayout manages geometries of its child items with anchors
    that connect the layout items with each other.

    It also allows layout items to be missing and can fix anchor attachments.
    Here are some simple rules how anchor fixation can be created (the example
    is only for horizontal direction - the same needs to be done for portrait as well).

    If anchors set allow ambiguos positioning of items, then layout tries to set items size as
    close to preferred as possible.

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
    parent widget, which owns this anchor layout, needs to have a child widget with item
    name "green_item". \c HbStyle::setItemName for more details.

    If an optional node is missing, the anchors pointing to the node are
    changed to point to the node after (=towards the parent layout) the missing one - this
    is called "fixing the mesh".

    \image html hbmeshlayout3.png

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
typedef QMap<QGraphicsLayoutItem*, QString> HbMeshItemMap;
typedef HbMeshItemMap::iterator HbMeshItemMapIterator;
typedef HbMeshItemMap::const_iterator HbMeshItemMapConstIterator;


/*
    Result of findEndItem.
*/
struct HbMeshEndItemResult
{
    QGraphicsLayoutItem *mItem;
    HbAnchorLayout::Edge mEdge;
    qreal mValue;
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
        QList<HbMeshEndItemResult> &resultList,
        const HbAnchor *anchor,
        QStringList &ids) const;
    void resolveAnchors();
    void removeItemIfNeeded( QGraphicsLayoutItem *item );

    bool setAnchor( HbAnchor *anchor );

    void setSizeProp( SizeProperty *v, QGraphicsLayoutItem *item, EdgeType type );
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

    // mesh layout data
    QList<QGraphicsLayoutItem*> mItems; // for addItem
    QList<QGraphicsLayoutItem*> mActualItems; // layouted items
    HbMeshItemMap mMeshMap;

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
  mStartId(anchor.mStartId),
  mEndItem(anchor.mEndItem),
  mEndEdge(anchor.mEndEdge),
  mEndId(anchor.mEndId),
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
        mStartId = anchor.mStartId;
        mStartEdge = anchor.mStartEdge;
        mEndItem = anchor.mEndItem;
        mEndId = anchor.mEndId;
        mEndEdge = anchor.mEndEdge;
        mValue = anchor.mValue;
    }
    return *this;
}





/*!
    \internal
*/
QList<HbAnchor*> HbAnchorLayoutDebug::getAnchors( HbAnchorLayout* layout )
{
    layout->d_ptr->resolveAnchors();
    return layout->d_ptr->mResolvedAnchors;
}

QList<HbAnchor*> HbAnchorLayoutDebug::getOriginalAnchors( HbAnchorLayout* layout )
{
    return layout->d_ptr->mAllAnchors;
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
#ifndef Q_WS_S60
        } else {
            HbSpacerItem *spacer = dynamic_cast<HbSpacerItem *>(item);
            if ( spacer ) {
                result = "HbSpacerItem";
            }
#endif
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
        qDebug() << "MeshLayout: Mesh anchors for" << w->metaObject()->className();
    }
    const QString parentId =
        mMeshMap.contains(q) ? mMeshMap.value(q) : QString();
    qDebug() << "-- -- resolved";
    qDebug() << "-- count: " << mResolvedAnchors.size() << ", parent: " << parentId;
    foreach (const HbAnchor *item, mResolvedAnchors) {
        const QString itemTemplate("-- (%1 [%2], %3) - (%4 [%5], %6) = %7");
        qDebug() <<
            itemTemplate
            .arg(item->mStartId)
            .arg(itemAsText(item->mStartItem, q))
            .arg(edgeAsText(item->mStartEdge))
            .arg(item->mEndId)
            .arg(itemAsText(item->mEndItem, q))
            .arg(edgeAsText(item->mEndEdge))
            .arg(item->mValue).toAscii().data();
    }
    qDebug() << "-- -- all";
    qDebug() << "-- count: " << mAllAnchors.size() << ", parent: " << parentId;
    foreach (const HbAnchor *item, mAllAnchors) {
        const QString itemTemplate("-- (%1 [%2], %3) - (%4 [%5], %6) = %7");
        qDebug() <<
            itemTemplate
            .arg(item->mStartId)
            .arg(itemAsText(item->mStartItem, q))
            .arg(edgeAsText(item->mStartEdge))
            .arg(item->mEndId)
            .arg(itemAsText(item->mEndItem, q))
            .arg(edgeAsText(item->mEndEdge))
            .arg(item->mValue).toAscii().data();
    }
    qDebug() << "-- ";
#endif // HBANCHORLAYOUT_DEBUG

    // HbAnchorLayout will only touch items that have anchors defined.
    mActualItems.clear();
    for (QList<HbAnchor*>::const_iterator it = mResolvedAnchors.constBegin();
         it != mResolvedAnchors.constEnd();
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
    bool isHorizontal;
    HbAnchor *anchor = mResolvedAnchors.at( anchorIndex );
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

            mItemsGeometry[ layoutIndex ].x1 = 0;//newRect.left();
            mItemsGeometry[ layoutIndex ].x2 = newRect.width();//newRect.right();
            mItemsGeometry[ layoutIndex ].y1 = 0;//newRect.top();
            mItemsGeometry[ layoutIndex ].y2 = newRect.height();//newRect.bottom();
            mGeometryDefinedH[ layoutIndex ] = true;
            mGeometryDefinedV[ layoutIndex ] = true;


            for( int i = 0; i < mAnchorsVisited.size(); i++ ) {

                HbAnchor *anchor = mResolvedAnchors.at(i);


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
                    HbAnchor *anchor = mResolvedAnchors.at(i);

                    startIndex = mActualItems.indexOf( anchor->mStartItem );
                    endIndex = mActualItems.indexOf( anchor->mEndItem );
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
        //        qDebug() << "Item " <<  i << "(" << ((QGraphicsWidget*)mActualItems.at(i))->metaObject()->className() << ")" << " geom " << geom;
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
            if ( edgeType( anchor->mStartEdge ) == type ) {
                itemStart = 0;
                itemEnd = 0;
                for( int j = 0; j < vertices->size(); j++ ) {
                    if( ( vertices->at(j)->itemRef == anchor->mStartItem ) &&
                            ( vertices->at(j)->itemSide == anchor->mStartEdge ) ) {
                        itemStart = vertices->at(j);
                    } else if( ( vertices->at(j)->itemRef == anchor->mEndItem ) &&
                        ( vertices->at(j)->itemSide == anchor->mEndEdge ) ) {
                        itemEnd = vertices->at(j);
                    }
                }

                if( !itemStart ) {
                    itemStart = createCenterEdge( type, anchor->mStartItem,  anchor->mStartEdge );
                }
                if( !itemEnd ) {
                    itemEnd = createCenterEdge( type, anchor->mEndItem,  anchor->mEndEdge );
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
    QList<HbMeshEndItemResult> &resultList,
    const HbAnchor *problem,
    QStringList &ids) const
{
    HbMeshEndItemResult result;
    bool found = false;

    for (QList<HbAnchor*>::const_iterator it = mAllAnchors.constBegin();
         it != mAllAnchors.constEnd();
         ++it) {

        const HbAnchor* currentItem = *it;

        if (!currentItem->mStartId.isNull() &&
            currentItem->mStartId == problem->mEndId &&
            currentItem->mStartEdge == problem->mStartEdge &&
            !ids.contains(currentItem->mStartId)) {

            qreal currentSpacing = currentItem->mValue;

            QGraphicsLayoutItem *item = currentItem->mEndItem;


            if (item) {
                found = true;
                result.mEdge = currentItem->mEndEdge;
                result.mItem = item;
                result.mValue = currentSpacing;
                resultList.append( result );
            } else {
                ids.append(currentItem->mStartId);
                found |= findEndItem(resultList, currentItem, ids);
                ids.takeLast();
            }
            /*
            if (found) {
                // We have found an end item. There can be multiple end items,
                // but (for now) the first one is selected.
                return true;
            }*/
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

        if( ( anchor->mStartItem ) && ( anchor->mEndItem ) ) {
            mResolvedStaticAnchors.append( anchor );
            continue;
        }

        if (anchor->mStartItem && !anchor->mEndId.isNull()) {
            QList<HbMeshEndItemResult> resultList;

            QStringList ids;
            ids.append(anchor->mStartId);

            if (findEndItem(resultList, anchor, ids)) {
                for( int j = 0; j < resultList.size(); j++ ) {
                    item = new HbAnchor();
                    item->mStartItem = anchor->mStartItem;
                    item->mStartId = anchor->mStartId;
                    item->mStartEdge = anchor->mStartEdge;
                    item->mEndEdge = resultList.at(j).mEdge;
                    item->mEndItem = resultList.at(j).mItem;
                    item->mValue = resultList.at(j).mValue;
                    mResolvedDynamicAnchors.append(item);
                }
            }
        } else {
            // Nothing needed.
        }
    }

    mResolvedAnchors = mResolvedDynamicAnchors + mResolvedStaticAnchors;
}

bool HbAnchorLayoutPrivate::setAnchor( HbAnchor *anchor )
{
    // This method is called from HbAnchorLayout::setAnchor.

    if (HbAnchorLayoutPrivate::edgeType(anchor->mStartEdge) !=
        HbAnchorLayoutPrivate::edgeType(anchor->mEndEdge)) {
        qWarning() << "HbAnchorLayout::setAnchor : You can't connect different type of edges";
        return false;
    }

    if ( ( anchor->mStartId.isNull() && ( anchor->mStartItem == 0 ) ) ||
        ( anchor->mEndId.isNull() && ( anchor->mEndItem == 0 ) ) ){
        qWarning() << "HbAnchorLayout::setAnchor : Both ids must be valid";
        return false;
    }

    if ( ( anchor->mStartId == anchor->mEndId ) && ( ( anchor->mStartItem == anchor->mEndItem ) ) &&
        ( anchor->mStartEdge == anchor->mEndEdge ) ) {
        qWarning() << "HbAnchorLayout::setAnchor : You cannot set anchor between the same edge";
        return false;
    }

    bool modified = false;

    const int count = mAllAnchors.size();
    for (int i = 0; i < count; ++i) {
        HbAnchor *item = mAllAnchors.at(i);


        bool idConditionStartStart = ( !item->mStartId.isNull() ) && ( item->mStartId == anchor->mStartId );
        bool idConditionEndEnd = ( !item->mEndId.isNull() ) && ( item->mEndId == anchor->mEndId );
        bool idConditionStartEnd = ( !item->mStartId.isNull() ) && ( item->mStartId == anchor->mEndId );
        bool idConditionEndStart = ( !item->mEndId.isNull() ) && ( item->mEndId == anchor->mStartId );

        bool itemConditionStartStart = ( item->mStartItem != 0 ) && ( item->mStartItem == anchor->mStartItem );
        bool itemConditionEndEnd = ( item->mEndItem != 0 ) && ( item->mEndItem == anchor->mEndItem );
        bool itemConditionStartEnd = ( item->mStartItem != 0 ) && ( item->mStartItem == anchor->mEndItem );
        bool itemConditionEndStart = ( item->mEndItem != 0 ) && ( item->mEndItem == anchor->mStartItem );

        bool edgeConditionStartStart = item->mStartEdge == anchor->mStartEdge;
        bool edgeConditionEndEnd = item->mEndEdge == anchor->mEndEdge;
        bool edgeConditionStartEnd = item->mStartEdge == anchor->mEndEdge;
        bool edgeConditionEndStart = item->mEndEdge == anchor->mStartEdge;


        if((idConditionStartStart || itemConditionStartStart) &&
            (idConditionEndEnd || itemConditionEndEnd) &&
            (edgeConditionStartStart) &&
            (edgeConditionEndEnd) ){
                modified = true;
                item->mValue = anchor->mValue;
                delete anchor;
                break;
        } else if( (idConditionStartEnd || itemConditionStartEnd) &&
            (idConditionEndStart || itemConditionEndStart) &&
            (edgeConditionStartEnd) &&
            (edgeConditionEndStart) ){
                modified = true;
                item->mValue = -anchor->mValue;
                delete anchor;
                break;
        }
    }

    if (!modified) {
        if( anchor->mStartItem != 0 ){
            anchor->mStartId = mMeshMap.value( anchor->mStartItem );
        } else if( ! anchor->mStartId.isNull() ) {
            anchor->mStartItem = mMeshMap.key( anchor->mStartId );
        }

        if( anchor->mEndItem != 0 ){
            anchor->mEndId = mMeshMap.value( anchor->mEndItem );
        } else if( ! anchor->mEndId.isNull() ) {
            anchor->mEndItem = mMeshMap.key( anchor->mEndId );
        }

        addItemIfNeeded( anchor->mStartItem );
        addItemIfNeeded( anchor->mEndItem );

        mAllAnchors.append(anchor);
    }

    return true;
}

void HbAnchorLayoutPrivate::removeItemIfNeeded( QGraphicsLayoutItem *item )
{
    Q_Q( HbAnchorLayout );

    if( ( item == 0 ) || ( item == q ) ) {
        return;
    }

    for ( int i = 0; i < mAllAnchors.size(); i++ ) {
        HbAnchor *anchor = mAllAnchors.at(i);
        if ( ( anchor->mStartItem == item ) || ( anchor->mEndItem == item ) ) {
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
    \return true if anchor was successfully added, false otherwise
*/
bool HbAnchorLayout::setAnchor( QGraphicsLayoutItem *startItem, Edge startEdge, QGraphicsLayoutItem *endItem, Edge endEdge, qreal length )
{
    Q_D( HbAnchorLayout );

    HbAnchor *anchor = new HbAnchor();
    anchor->mStartItem = startItem;
    anchor->mStartEdge = startEdge;
    anchor->mEndItem = endItem;
    anchor->mEndEdge = endEdge;
    anchor->mValue = length;

    if (d->setAnchor(anchor)) {
        invalidate();
        return true;
    }

    delete anchor;

    return false;
}

/*!
    Same as previous, but here it operates with node ids, instead of items itself.

    \param startId start id.
    \param startEdge start edge.
    \param endId end id.
    \param endEdge end edge.
    \param length spacing value for all edges starting from (\a startId, \a startEdge).
    \return true if success, false otherwise.
*/
bool HbAnchorLayout::setAnchor( const QString& startId, Edge startEdge, const QString& endId, Edge endEdge, qreal length )
{
    Q_D( HbAnchorLayout );

    HbAnchor *anchor = new HbAnchor();
    anchor->mStartId = startId;
    anchor->mStartEdge = startEdge;
    anchor->mEndId = endId;
    anchor->mEndEdge = endEdge;
    anchor->mValue = length;

    if (d->setAnchor(anchor)) {
        invalidate();
        return true;
    }

    delete anchor;

    return false;
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
        if( ( anchor->mStartId == startNodeId && anchor->mStartEdge == startEdge &&
                anchor->mEndId == endNodeId && anchor->mEndEdge == endEdge ) ||
            ( anchor->mStartId == endNodeId && anchor->mStartEdge == endEdge &&
                anchor->mEndId == startNodeId && anchor->mEndEdge == startEdge ) ){
            delete d->mAllAnchors.takeAt(i);
            modified = true;
            break;
        }
    }

    if (modified) {
        d->removeItemIfNeeded( d->mMeshMap.key( startNodeId ) );
        d->removeItemIfNeeded( d->mMeshMap.key( endNodeId ) );
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
        if( ( anchor->mStartItem == startItem && anchor->mStartEdge == startEdge &&
                anchor->mEndItem == endItem && anchor->mEndEdge == endEdge ) ||
                ( anchor->mStartItem == endItem && anchor->mStartEdge == endEdge &&
                anchor->mEndItem == startItem && anchor->mEndEdge == startEdge ) ){
            delete d->mAllAnchors.takeAt(i);
            modified = true;
            break;
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
        if (anchor->mStartId == nodeId || anchor->mEndId == nodeId) {
            QGraphicsLayoutItem *startItem = anchor->mStartItem;
            QGraphicsLayoutItem *endItem = anchor->mEndItem;

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
            if( anchor->mStartItem == item ) {
                anchor->mStartId = nodeId;
                modified = true;
            } else if( anchor->mStartId == nodeId ) {
                anchor->mStartItem = item;
                modified = true;
            }

            if( anchor->mEndItem == item ) {
                anchor->mEndId = nodeId;
                modified = true;
            } else if( anchor->mEndId == nodeId ) {
                anchor->mEndItem = item;
                modified = true;
            }

        }

        // Remove previous item -> id.
        HbMeshItemMapIterator it = d->mMeshMap.begin();
        while ( it != d->mMeshMap.end() ) {
            if ( it.value() == nodeId ) {
                it = d->mMeshMap.erase( it );
            } else {
                ++it;
            }
        }
        d->addItemIfNeeded( item );
        d->mMeshMap.insert( item, nodeId );
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

        if( anchor->mStartItem == item ) {
            anchor->mStartId = QString();
            modified = true;
        }

        if( anchor->mEndItem == item ) {
            anchor->mEndId = QString();
            modified = true;
        }
    }


    d->mMeshMap.remove(item);

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

        if( anchor->mStartId == nodeId ) {
            anchor->mStartItem = 0;
            modified = true;
        }

        if( anchor->mEndId == nodeId ) {
            anchor->mEndItem = 0;
            modified = true;
        }
    }


    HbMeshItemMapIterator it = d->mMeshMap.begin();
    while ( it != d->mMeshMap.end() ) {
        if ( it.value() == nodeId ) {
            it = d->mMeshMap.erase( it );
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
    d->mMeshMap.clear();

    for( int i = 0; i < d->mAllAnchors.size(); i++ ) {
        HbAnchor *anchor = d->mAllAnchors.at(i);

        if( !anchor->mStartId.isNull() ) {
            anchor->mStartItem = 0;
        }

        if( !anchor->mEndId.isNull() ) {
            anchor->mEndItem = 0;
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
        //qWarning() << "HbAnchorLayout::addItemIfNeeded : item is NULL";
        return;
    }

    if (item == q) {
        //qWarning() << "HbAnchorLayout::addItemIfNeeded : layout cannot be added";
        return;
    }

    if (mItems.contains(item)) {
        //qWarning() << "HbAnchorLayout::addItemIfNeeded : item is already in layout";
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
    if( d->mMeshMap.contains( item ) ) {
        return d->mMeshMap.value( item );
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
        QString id = d->mAllAnchors.at(c)->mStartId;
        if (!list.contains(id) && !id.isNull()) {
            list.append(id);
        }
        id = d->mAllAnchors.at(c)->mEndId;
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
    return d->mMeshMap.key( nodeId );
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
            if ( ( ( d->mAllAnchors.at(i)->mStartItem == item ) && ( d->mAllAnchors.at(i)->mStartId.isNull() ) ) ||
                 ( ( d->mAllAnchors.at(i)->mEndItem == item ) && ( d->mAllAnchors.at(i)->mEndId.isNull() ) ) ) {
                    delete d->mAllAnchors.takeAt(i);
            }
        }

        removeMapping( d->mMeshMap.value(item) );
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
