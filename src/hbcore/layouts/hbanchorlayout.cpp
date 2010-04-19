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

#include <QDebug>
#include <QLayout>

#include "hblayoututils_p.h"

//Uncomment next define in order to get more debug prints.
//Similar define exists also in the engine side.
//#define HBANCHORLAYOUT_DEBUG

/*!
    @beta
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
    \enum HbAnchorLayout::Edge

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

class HbAnchorLayoutPrivate
{
public:
    Q_DECLARE_PUBLIC( HbAnchorLayout )

    HbAnchorLayoutPrivate();
    ~HbAnchorLayoutPrivate();

    void addItemIfNeeded( QGraphicsLayoutItem *item );
    EdgeType edgeType( HbAnchorLayout::Edge edge );
    HbAnchor* getAnchor( QGraphicsLayoutItem *startItem,
                         HbAnchorLayout::Edge startEdge,
                         QGraphicsLayoutItem *endItem,
                         HbAnchorLayout::Edge endEdge );

    void setItemGeometries();

    void createEquations( EdgeType type );
    void calculateSizeHint( EdgeType type );
    void createEquationsCommon( EdgeType type );
    void setVariables( Variable *v, QGraphicsLayoutItem *item, EdgeType type );

    int getEdgeIndex( QGraphicsLayoutItem *item, HbAnchorLayout::Edge edge );
    
    QSizeF sizeHint( Qt::SizeHint which );
    
public:
    HbAnchorLayout * q_ptr;
    QList<QGraphicsLayoutItem*> mItems;
    QList<HbAnchor*> mAnchors;
    bool mEquationsDirty; // if true, we needed to re-create the equations (e.g. when new anchor is set)
    bool mValid;          // result of the calculations. false, if the equations cannot be solved.
    bool mSizeHintDirty;    // set true in ::invalidate() and cleared after re-creation of equations in ::sizeHint
    bool mInvalidateCalled; // set true in ::invalidate() and cleared after geometry is set in ::setGeometry

    Expression mSx, mSy;
    
    QSizeF mMinSH, mPrefSH, mMaxSH;
    

    QList<Equation> mEquationsHorizontal;
    QList<Equation> mEquationsVertical;
    VariableSet mVariablesHorizontal;
    VariableSet mVariablesVertical;
    DataGrid mDataHorizontal;
    DataGrid mDataVertical;
    Solution mSolutionHorizontal;
    Solution mSolutionVertical;

    QRectF mUsedRect;
};

/*!
    \internal
*/
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

/*!
    \internal
*/
HbAnchorLayoutPrivate::HbAnchorLayoutPrivate() : mEquationsDirty( true ), mValid( true ), mSizeHintDirty( true ), mInvalidateCalled(false), 
                                            mMinSH( 0, 0 ), mPrefSH( 100, 100 ), mMaxSH( 1000, 1000 )
{
}

/*!
    \internal
*/
HbAnchorLayoutPrivate::~HbAnchorLayoutPrivate()
{   
    qDeleteAll( mAnchors );
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
EdgeType HbAnchorLayoutPrivate::edgeType( HbAnchorLayout::Edge edge )
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
    HbAnchorLayout::Edge startEdge,
    QGraphicsLayoutItem *endItem,
    HbAnchorLayout::Edge endEdge )
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

/*!
    \internal
*/
void HbAnchorLayoutPrivate::setItemGeometries()
{
    Q_Q(HbAnchorLayout);
    const QRectF newRect = q->geometry();

    if ( (newRect != mUsedRect) || mInvalidateCalled ) {

        mValid = true;
        if ( mEquationsDirty ) {
            createEquations( Horizontal );
            createEquations( Vertical );
            mEquationsDirty = false;
        }

        Variable *layoutWidth = mVariablesHorizontal.findVariable( q );
        if ( layoutWidth ) {
            layoutWidth->mFlags |= Variable::FlagFixed;
            layoutWidth->mPref = newRect.width();
        }

        Variable *layoutHeight = mVariablesVertical.findVariable( q );
        if ( layoutHeight ) {
            layoutHeight->mFlags |= Variable::FlagFixed;
            layoutHeight->mPref = newRect.height();
        }

        for ( int i = 0; i < mVariablesHorizontal.mVarList.count(); i++ ) {
            Variable *var = mVariablesHorizontal.mVarList.at(i);
            QGraphicsLayoutItem *item = static_cast<QGraphicsLayoutItem*>(var->mRef);
            if ( item && item != q ) {
                setVariables( var, item, Horizontal );
            }
        }

        for ( int i = 0; i < mVariablesVertical.mVarList.count(); i++ ) {
            Variable *var = mVariablesVertical.mVarList.at(i);
            QGraphicsLayoutItem *item = static_cast<QGraphicsLayoutItem*>(var->mRef);
            if ( item && item != q ) {
                setVariables( var, item, Vertical );
            }
        }

        mValid = EquationSolver::solveEquation(
            mEquationsHorizontal,
            mVariablesHorizontal,
            &mSolutionHorizontal );
        if ( !mValid ) {
            return;
        }

#ifdef HBANCHORLAYOUT_DEBUG
        printf("\n\n\nHorisontal solution:\n");
        Solution::iterator i = mSolutionHorizontal.begin();
        while( i != mSolutionHorizontal.end() ) {
            printf( "id = %d  value = %lf\n", i.key().mId, i.value() );
            i++;
        }
        for( int i = 0; i < 2*(mItems.count()+1); i++ ) {
            printf( "distance 0 : %d = %lf\n", i, mDataHorizontal.value( i, 0, &mSolutionHorizontal, mValid ) );
        }
#endif // HBANCHORLAYOUT_DEBUG

        mValid = EquationSolver::solveEquation(
            mEquationsVertical,
            mVariablesVertical,
            &mSolutionVertical );
        if ( !mValid ) {
            return;
        }

#ifdef HBANCHORLAYOUT_DEBUG
        printf("\n\n\nVertical solution:\n");

        i = mSolutionVertical.begin();
        while( i != mSolutionVertical.end() ) {
            printf( "id = %d  value = %lf\n", i.key().mId, i.value() );
            i++;
        }
        for( int i = 0; i < 2*(mItems.count()+1); i++ ) {
            printf( "distance 0 : %d = %lf\n", i, mDataVertical.value( i, 0, &mSolutionVertical, mValid ) );
        }
#endif // HBANCHORLAYOUT_DEBUG

    }

    mUsedRect = newRect;

    Qt::LayoutDirection layoutDir = HbLayoutUtils::visualDirection(q);
    for ( int i=0; i<mItems.count(); i++ ) {

        qreal l, t, r, b;
        l = mUsedRect.left() + mDataHorizontal.value( 2*i+1, 0, &mSolutionHorizontal, mValid );
        if ( !mValid ) {
        	return;
    	}
        r = mUsedRect.left() + mDataHorizontal.value( 2*i+2, 0, &mSolutionHorizontal, mValid );
        if ( !mValid ) {
        	return;
    	}
        t = mUsedRect.top() + mDataVertical.value( 2*i+1, 0, &mSolutionVertical, mValid );
        if ( !mValid ) {
        	return;
    	}
        b = mUsedRect.top() + mDataVertical.value( 2*i+2, 0, &mSolutionVertical, mValid );
        if ( !mValid ) {
        	return;
    	}

#ifdef HBANCHORLAYOUT_DEBUG
        printf( "%d: l=%lf   r=%lf   t=%lf   b=%lf\n", i, l, r, t, b );
#endif // HBANCHORLAYOUT_DEBUG

        QRectF itemRect(QPointF(l, t), QPointF(r, b));
        HbLayoutUtils::visualRect(layoutDir, itemRect, mUsedRect);
        mItems.at(i)->setGeometry(itemRect);

        // left edge -> 2*i+1
        // right edge -> 2*i+2

    }

}

void HbAnchorLayoutPrivate::setVariables( Variable *v, QGraphicsLayoutItem *item, EdgeType type )
{
    if( type == Vertical ) {
        const QSizePolicy::Policy verticalPolicy = item->sizePolicy().verticalPolicy();

        if ( verticalPolicy & QSizePolicy::ShrinkFlag ) {
            v->mMin = item->minimumHeight();
        } else {
            v->mMin = item->preferredHeight();
        }

        if ( verticalPolicy & (QSizePolicy::GrowFlag | QSizePolicy::ExpandFlag) ) {
            v->mMax = item->maximumHeight();
        } else {
            v->mMax = item->preferredHeight();
        }

        v->mPref = qBound( v->mMin, item->preferredHeight(), v->mMax );
        v->mFlags |= (v->mMin == v->mMax) ? Variable::FlagFixed : 0;
        v->mFlags |= (verticalPolicy&QSizePolicy::ExpandFlag) ? Variable::FlagExpanding : 0;

    } else {    
        const QSizePolicy::Policy horizontalPolicy = item->sizePolicy().horizontalPolicy();
    
        if ( horizontalPolicy & QSizePolicy::ShrinkFlag ) {
            v->mMin = item->minimumWidth();
        } else {
            v->mMin = item->preferredWidth();
        }
    
        if ( horizontalPolicy & (QSizePolicy::GrowFlag | QSizePolicy::ExpandFlag) ) {
            v->mMax = item->maximumWidth();
        } else {
            v->mMax = item->preferredWidth();
        }
    
        v->mPref = qBound( v->mMin, item->preferredWidth(), v->mMax );
        v->mFlags |= (v->mMin == v->mMax) ? Variable::FlagFixed : 0;
        v->mFlags |= (horizontalPolicy&QSizePolicy::ExpandFlag) ? Variable::FlagExpanding : 0;
    }    
}

void HbAnchorLayoutPrivate::createEquationsCommon( EdgeType type )
{
    Q_Q(HbAnchorLayout);
    VariableSet *vs = &mVariablesHorizontal;
    DataGrid *dg = &mDataHorizontal;
    if ( type == Vertical ) {
        vs = &mVariablesVertical;
        dg = &mDataVertical;
    }
    vs->clear();
    dg->clear();

    SimpleExpression se = {0,0};

    // pseudo variable
    Variable *v1 = vs->createVariable(0);
    v1->mPref = 1;
    v1->mFlags |= Variable::FlagFixed;

    vs->createVariable(q);

    se.mCoef = 1;
    for (int i=0; i<mItems.count(); i++) {
        QGraphicsLayoutItem *item = mItems.at(i);
        se.mVar = vs->createVariable(item);
        
        // left edge -> 2*i+1
        // right edge -> 2*i+2
        dg->setExpression( se, 2*i+2, 2*i+1 );

        // if no anchors in this dimension...
        bool has_any_anchor = false;
        for (int i=0; i<mAnchors.count(); i++) {
            HbAnchor* anchor = mAnchors.at(i);
            if( edgeType( anchor->mStartEdge ) == type &&
                ( anchor->mStartItem == item || anchor->mEndItem == item  ) ) {
                has_any_anchor = true;
                break;
            }
        }

        if( !has_any_anchor ) {
            // ...anchor the item with a "fake anchor"
            // to the top or the left edge of the layout.
            se.mVar = v1;
            se.mCoef = 0;
            int startIndex = 0;
            HbAnchorLayout::Edge fakeEdge = ( type == Horizontal )
                ? Hb::LeftEdge
                : Hb::TopEdge;
            int endIndex = getEdgeIndex( item, fakeEdge );
            dg->setExpression( se, endIndex, startIndex );
            se.mCoef = 1;
        }
    }

    se.mVar = v1;
    for (int i=0; i<mAnchors.count(); i++) {
        HbAnchor* anchor = mAnchors.at(i);
        if ( edgeType( anchor->mStartEdge ) == type ) {
            Expression ex;
            int startIndex = getEdgeIndex( anchor->mStartItem, anchor->mStartEdge );
            int endIndex = getEdgeIndex( anchor->mEndItem, anchor->mEndEdge );
            
            if( ( anchor->mStartEdge == Hb::CenterHEdge ) || ( anchor->mStartEdge == Hb::CenterVEdge ) ) {
                SimpleExpression startSE;
                startSE.mVar = vs->findVariable( anchor->mStartItem );
                startSE.mCoef = 0.5;
                ex.plusSimpleExpression( startSE );
            }

            if( ( anchor->mEndEdge == Hb::CenterHEdge ) || ( anchor->mEndEdge == Hb::CenterVEdge ) ) {
                SimpleExpression endSE;
                endSE.mVar = vs->findVariable( anchor->mEndItem );
                endSE.mCoef = -0.5;
                ex.plusSimpleExpression( endSE );
            }
            
            se.mCoef = anchor->mValue;
            ex.plusSimpleExpression( se );
            dg->setExpression( ex, endIndex, startIndex );
        }
    }

    se.mCoef = 0;
    se.mVar = v1;
    for( int i = 0; i <= 2*mItems.count()+1; i++ ) {
        dg->setExpression( se, i, i );
    }
}

void HbAnchorLayoutPrivate::calculateSizeHint( EdgeType type )
{
    Q_Q(HbAnchorLayout);
    VariableSet *vs = &mVariablesHorizontal;
    DataGrid *dg = &mDataHorizontal;
    if ( type == Vertical ) {
        vs = &mVariablesVertical;
        dg = &mDataVertical;
    }
    
    createEquationsCommon( type );

    Variable *vLayout = vs->findVariable( q );

    vLayout->mFlags = 0;
    vLayout->mMin = 0;
    vLayout->mPref = 100;
    vLayout->mMax = 1000; 

    for ( int i = 0; i < vs->mVarList.count(); i++ ) {
        Variable *var = vs->mVarList.at(i);
        QGraphicsLayoutItem *item = static_cast<QGraphicsLayoutItem*>(var->mRef);
        if ( item && item != q ) {
            setVariables( var, item, type );
        }            
    }
    
    if ( type == Horizontal ) {        
        dg->calculate();
         
        if( dg->mGrid.value( 2 * mItems.count() + 1 ).contains( 0 ) ) {
            mSx = dg->mGrid.value( 2 * mItems.count() + 1 ).value( 0 );
        } else {
            mSx.clear();
        }
                
    } else {
        dg->calculate();
        
        if( dg->mGrid.value( 2 * mItems.count() + 1 ).contains( 0 ) ) {
            mSy = dg->mGrid.value( 2 * mItems.count() + 1 ).value( 0 );
        } else {
            mSy.clear();
        }
    }
    
}

/*!
    \internal
*/
void HbAnchorLayoutPrivate::createEquations( EdgeType type )
{
    Q_Q(HbAnchorLayout);
    VariableSet *vs = &mVariablesHorizontal;
    DataGrid *dg = &mDataHorizontal;
    if ( type == Vertical ) {
        vs = &mVariablesVertical;
        dg = &mDataVertical;
    }
    vs->clear();
    dg->clear();
    
    createEquationsCommon( type );
    
    SimpleExpression se;
    se.mVar = vs->findVariable( q );
    se.mCoef = 1;
    dg->setExpression( se, 2*mItems.count()+1, 0 );

    if ( type == Horizontal ) {
        mEquationsHorizontal = dg->calculate();
        mSolutionHorizontal.clear();
    } else {
        mEquationsVertical = dg->calculate();
        mSolutionVertical.clear();
    }
}

/*!
    \internal
*/
int HbAnchorLayoutPrivate::getEdgeIndex( QGraphicsLayoutItem *item, HbAnchorLayout::Edge edge )
{
    Q_Q(HbAnchorLayout);
    int edgeIndex(0);
    if ( item == q ) {
        if ( edge == Hb::RightEdge || edge == Hb::BottomEdge ) {
            edgeIndex = 2 * mItems.count()+1;
        } // else -> 0
    } else {
        edgeIndex = 2 * q->indexOf( item )+1;
        if ( edge == Hb::RightEdge || edge == Hb::BottomEdge ) {
            edgeIndex += 1;
        }
    }
    return edgeIndex;
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
                             HbAnchorLayout::Edge startEdge,
                             QGraphicsLayoutItem *endItem,
                             HbAnchorLayout::Edge endEdge,
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

    d->mEquationsDirty = true;
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
                                   HbAnchorLayout::Edge edge1,
                                   QGraphicsLayoutItem *item2,
                                   HbAnchorLayout::Edge edge2 )
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
        d->mEquationsDirty = true;
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

    d->mEquationsDirty = true;
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
    return d->mValid;
}

/*!
    From QGraphicsLayoutItem.
*/
void HbAnchorLayout::invalidate()
{
    Q_D( HbAnchorLayout );
    d->mSizeHintDirty = true;
    d->mInvalidateCalled = true;
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
    if( mSizeHintDirty ) {
        calculateSizeHint( Horizontal );
        calculateSizeHint( Vertical );
        mEquationsDirty = true;        
        mSizeHintDirty = false;
        
        
        
        if( mSx.mExpression.size() > 0 ) {

            mMinSH.setWidth( mSx.minValue() );
            mPrefSH.setWidth( mSx.prefValue() );
            mMaxSH.setWidth( mSx.maxValue() );

            if( mMinSH.width() < 0 ) {
                mMinSH.setWidth( 0 );
            }

            if( mPrefSH.width() < 0 ) {
                mPrefSH.setWidth( 0 );
            }

            if( mMaxSH.width() < 0 ) {
                mMaxSH.setWidth( 0 );
            }

        } else {
            mMinSH.setWidth( 0 );
            mPrefSH.setWidth( 100 );
            mMaxSH.setWidth( 1000 );
        }
        
        if( mSy.mExpression.size() > 0 ) {
            mMinSH.setHeight( mSy.minValue() );
            mPrefSH.setHeight( mSy.prefValue() );            
            mMaxSH.setHeight( mSy.maxValue() );

            if( mMinSH.height() < 0 ) {
                mMinSH.setHeight( 0 );
            }

            if( mPrefSH.height() < 0 ) {
                mPrefSH.setHeight( 0 );
            }

            if( mMaxSH.height() < 0 ) {
                mMaxSH.setHeight( 0 );
            }
        } else {
            mMinSH.setHeight( 0 );
            mPrefSH.setHeight( 100 );
            mMaxSH.setHeight( 1000 );
        }
        
        mSx.clear();
        mSy.clear();
        
    }
    
    
    QSizeF res;
    
    if (which == Qt::MinimumSize) {
        res = mMinSH;
    } else if (which == Qt::PreferredSize ) {
        res = mPrefSH;
    } else {
        res = mMaxSH;
    }
    
    return res;
}

