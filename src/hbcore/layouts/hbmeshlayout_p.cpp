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
    void setVariables( Variable *v, QGraphicsLayoutItem *item, EdgeType type );

    void createEquations( EdgeType type );
    void calculateSizeHint( EdgeType type );
    void createEquationsCommon( EdgeType type );
    
    int getEdgeIndex(QGraphicsLayoutItem *item, HbAnchorLayout::Edge edge);

    bool hasAnchorSpacing(const HbMeshAnchor &anchor, qreal *spacing = 0) const;
    bool findEndItem(
        HbMeshEndItemResult &result,
        const HbMeshAnchor &anchor, 
        const HbMeshItemMapInverse &inverse,
        QStringList &ids) const;
    QList<HbAnchor> resolveAnchors();

    bool setAnchor(const HbMeshAnchor &anchor);
    int actualItemsIndexOf(QGraphicsLayoutItem *item) const;
    
    QSizeF sizeHint(Qt::SizeHint which);
    
public:
    HbMeshLayout * q_ptr;

    bool mEquationsDirty; // if true, we needed to re-create the equations (e.g. when new anchor is set)
    bool mValid;          // result of the calculations. false, if the equations cannot be solved.
    bool mSizeHintDirty;    // set true in ::invalidate() and cleared after re-creation of equations in ::sizeHint
    bool mInvalidateCalled; // set true in ::invalidate() and cleared after geometry is set in ::setGeometry
    
    QList<HbAnchor> mAnchors;
    Expression mSx, mSy;
    QSizeF mMinSH, mPrefSH, mMaxSH;

    // mesh layout data
    QList<QGraphicsLayoutItem*> mItems; // for addItem
    QList<QGraphicsLayoutItem*> mActualItems; // layouted items
    HbMeshItemMap mMeshMap;
    QList<HbMeshAnchor> mMeshAnchors;
    QMap<HbMeshKey, qreal> mMeshSpacings;

    // variables needed to solve item geometries using resolved anchors
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
QList<HbAnchor> HbMeshLayoutDebug::getAnchors( HbMeshLayout* layout )
{
    return layout->d_ptr->resolveAnchors();
}

/*
    \class HbMeshLayoutPrivate
    \internal
*/
HbMeshLayoutPrivate::HbMeshLayoutPrivate() : mEquationsDirty(false), mValid(true), mSizeHintDirty( true ), mInvalidateCalled( false ),
                                            mMinSH( 0, 0 ), mPrefSH( 100, 100 ), mMaxSH( 1000, 1000 ) 
{
}

/*
    \internal
*/
HbMeshLayoutPrivate::~HbMeshLayoutPrivate()
{
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
    mAnchors = resolveAnchors();
        
#ifdef HBMESHLAYOUT_DEBUG
    QGraphicsWidget* w = HbLayoutUtils::parentWidget( q );
    if ( w ) {
        qDebug() << "MeshLayout: Mesh anchors for" << w->metaObject()->className();
    }
    const QString parentId = 
        mMeshMap.contains(q) ? mMeshMap.value(q) : QString();
    qDebug() << "-- count: " << mAnchors.size() << ", parent: " << parentId;
    foreach (const HbAnchor &item, mAnchors) {
        const QString itemTemplate("-- (%1, %2) - (%3, %4) = %5");
        qDebug() << 
            itemTemplate
            .arg(mMeshMap.value(item.mStartItem))
            .arg(edgeAsText(item.mStartEdge))
            .arg(mMeshMap.value(item.mEndItem))
            .arg(edgeAsText(item.mEndEdge))
            .arg(item.mValue).toAscii().data();
    }
    qDebug() << "-- ";
#endif // HBMESHLAYOUT_DEBUG

    // HbMeshLayout will only touch items that have anchors defined.
    mActualItems.clear();
    for (QList<HbAnchor>::const_iterator it = mAnchors.constBegin(); 
         it != mAnchors.constEnd(); 
         ++it) {
    
        const HbAnchor& item = *it;

        if (item.mStartItem != q && !mActualItems.contains(item.mStartItem)) {
            mActualItems.append(item.mStartItem);
        }
        if (item.mEndItem != q && !mActualItems.contains(item.mEndItem)) {
            mActualItems.append(item.mEndItem);
        }
    }    
    
}

/*
    \internal
*/
void HbMeshLayoutPrivate::setVariables( Variable *v, QGraphicsLayoutItem *item, EdgeType type )
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


/*
    \internal
*/
void HbMeshLayoutPrivate::setItemGeometries()
{
    Q_Q(HbMeshLayout);
    const QRectF newRect = q->geometry();
#ifdef HBMESHLAYOUT_DEBUG
    QGraphicsWidget* w = HbLayoutUtils::parentWidget( q );
    if ( w ) {
        qDebug() << "MeshLayout: Setting geometries for" << w->metaObject()->className() << ", rect " << newRect;
    }    
#endif

    if ( (newRect != mUsedRect) || mInvalidateCalled ) {

        mValid = true;
        if ( mEquationsDirty ) {
            // Resolve anchors from mesh layout data.
            updateAnchorsAndItems();

            // Remainder of this method is similar to \c HbAnchorLayout.
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
#ifdef HBMESHLAYOUT_DEBUG
            qDebug() << "MeshLayout: Horizontal anchors couldn't be solved";
#endif
            return;
        }

        mValid = EquationSolver::solveEquation(
            mEquationsVertical,
            mVariablesVertical,
            &mSolutionVertical );
        if ( !mValid ) {
#ifdef HBMESHLAYOUT_DEBUG
            qDebug() << "MeshLayout: Vertical anchors couldn't be solved";
#endif
            return;
        }
    }

    mUsedRect = newRect;
   
    Qt::LayoutDirection layoutDir = HbLayoutUtils::visualDirection(q);
    for ( int i=0; i<mActualItems.count(); i++ ) {

        qreal l, t, r, b;
        l = mUsedRect.left() + mDataHorizontal.value( 2*i+1, 0, &mSolutionHorizontal, mValid );
        if ( !mValid ) {
#ifdef HBMESHLAYOUT_DEBUG
            qDebug() << "MeshLayout: Invalid left coordinate";
#endif
        	return;
    	}
        r = mUsedRect.left() + mDataHorizontal.value( 2*i+2, 0, &mSolutionHorizontal, mValid );
        if ( !mValid ) {
#ifdef HBMESHLAYOUT_DEBUG
            qDebug() << "MeshLayout: Invalid right coordinate";
#endif
        	return;
    	}
        t = mUsedRect.top() + mDataVertical.value( 2*i+1, 0, &mSolutionVertical, mValid );
        if ( !mValid ) {
#ifdef HBMESHLAYOUT_DEBUG
            qDebug() << "MeshLayout: Invalid top coordinate";
#endif
        	return;
    	}
        b = mUsedRect.top() + mDataVertical.value( 2*i+2, 0, &mSolutionVertical, mValid );
        if ( !mValid ) {
#ifdef HBMESHLAYOUT_DEBUG
            qDebug() << "Invalid bottom coordinate";
#endif
        	return;
    	}
        QRectF geom(QPointF(l, t), QPointF(r, b));
        HbLayoutUtils::visualRect(layoutDir, geom, mUsedRect);
        QGraphicsLayoutItem *lItem = mActualItems.at(i);
        lItem->setGeometry(geom);

        // left edge -> 2*i+1
        // right edge -> 2*i+2

    }
}

/*
    \internal
*/
int HbMeshLayoutPrivate::getEdgeIndex( QGraphicsLayoutItem *item, HbAnchorLayout::Edge edge )
{
    Q_Q(HbMeshLayout);
    int edgeIndex(0);
    if ( item == q ) {
        if ( edge == Hb::RightEdge || edge == Hb::BottomEdge ) {
            edgeIndex = 2 * mActualItems.count()+1;
        } // else -> 0
    } else {
        edgeIndex = 2 * actualItemsIndexOf( item )+1;
        if ( edge == Hb::RightEdge || edge == Hb::BottomEdge ) {
            edgeIndex += 1;
        }
    }
    return edgeIndex;
}

/*
    \internal
*/
void HbMeshLayoutPrivate::createEquations( EdgeType type )
{
    
    Q_Q(HbMeshLayout);
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
    dg->setExpression( se, 2*mActualItems.count()+1, 0 );

    if ( type == Horizontal ) {
        mEquationsHorizontal = dg->calculate();
        mSolutionHorizontal.clear();
    } else {
        mEquationsVertical = dg->calculate();
        mSolutionVertical.clear();
    }
}


/*
    \internal
*/
void HbMeshLayoutPrivate::calculateSizeHint( EdgeType type )
{
    Q_Q(HbMeshLayout);
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
         
        if( dg->mGrid.value( 2 * mActualItems.count() + 1 ).contains( 0 ) ) {
            mSx = dg->mGrid.value( 2 * mActualItems.count() + 1 ).value( 0 );
        } else {
            mSx.clear();
        }
                
    } else {
        dg->calculate();
        
        if( dg->mGrid.value( 2 * mActualItems.count() + 1 ).contains( 0 ) ) {
            mSy = dg->mGrid.value( 2 * mActualItems.count() + 1 ).value( 0 );
        } else {
            mSy.clear();
        }
    }
    
}


/*
    \internal
*/
void HbMeshLayoutPrivate::createEquationsCommon( EdgeType type )
{
    Q_Q(HbMeshLayout);
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
    for (int i=0; i<mActualItems.count(); i++) {
        QGraphicsLayoutItem *item = mActualItems.at(i);
        se.mVar = vs->createVariable(item);
        // left edge -> 2*i+1
        // right edge -> 2*i+2
        dg->setExpression( se, 2*i+2, 2*i+1 );

        // if no anchors in this dimension...
        bool has_any_anchor = false;
        for (int i=0; i<mAnchors.count(); i++) {
            const HbAnchor& anchor = mAnchors.at(i);
            if( edgeType( anchor.mStartEdge ) == type &&
                ( anchor.mStartItem == item || anchor.mEndItem == item  ) ) {
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
        const HbAnchor& anchor = mAnchors[i];
        if ( edgeType( anchor.mStartEdge ) == type ) {
            Expression ex;
            int startIndex = getEdgeIndex( anchor.mStartItem, anchor.mStartEdge );
            int endIndex = getEdgeIndex( anchor.mEndItem, anchor.mEndEdge );
            
            if( ( anchor.mStartEdge == Hb::CenterHEdge ) || ( anchor.mStartEdge == Hb::CenterVEdge ) ) {
                SimpleExpression startSE;
                startSE.mVar = vs->findVariable( anchor.mStartItem );
                startSE.mCoef = 0.5;
                ex.plusSimpleExpression( startSE );
            }

            if( ( anchor.mEndEdge == Hb::CenterHEdge ) || ( anchor.mEndEdge == Hb::CenterVEdge ) ) {
                SimpleExpression endSE;
                endSE.mVar = vs->findVariable( anchor.mEndItem );
                endSE.mCoef = -0.5;
                ex.plusSimpleExpression( endSE );
            }
            
            se.mCoef = anchor.mValue;
            ex.plusSimpleExpression( se );
            dg->setExpression( ex, endIndex, startIndex );
        }
    }

    se.mCoef = 0;
    se.mVar = v1;
    for( int i = 0; i <= 2*mActualItems.count()+1; i++ ) {
        dg->setExpression( se, i, i );
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
QList<HbAnchor> HbMeshLayoutPrivate::resolveAnchors()
{
    QList<HbAnchor> anchors;
    HbMeshItemMapInverse map = createInverse(mMeshMap);

    for (QList<HbMeshAnchor>::const_iterator it = mMeshAnchors.constBegin();
         it != mMeshAnchors.constEnd();
         ++it) {

        const HbMeshAnchor &anchor = *it;

        QGraphicsLayoutItem *startItem = map.value(anchor.mStartId);

        if (startItem) {            
            HbAnchor item(startItem, anchor.mStartEdge, 0 /*end item*/, anchor.mEndEdge, 0);

            if (hasAnchorSpacing(anchor, &item.mValue)) {
                // anchor really exists

                QGraphicsLayoutItem *endItem = map.value(anchor.mEndId);
                if (endItem) {
                    // this is already valid anchor

                    item.mEndItem = endItem;
                    anchors.append(item);
                } else {
                    // try to "fix" anchor

                    HbMeshEndItemResult result;
                    result.mItem = 0;
                    result.mEdge = Hb::LeftEdge;
                    result.mSpacing = 0;

                    QStringList ids;
                    ids.append(anchor.mStartId);

                    if (findEndItem(result, anchor, map, ids)) {
                        item.mEndEdge = result.mEdge;
                        item.mEndItem = result.mItem;
                        item.mValue = result.mSpacing;
                        anchors.append(item);
                    }
                }            
            }

        } else {
            // Nothing needed.
        }
    }

    return anchors;
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
        d->mEquationsDirty = true;
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
        d->mEquationsDirty = true;
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
        d->mEquationsDirty = true;
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
        d->mEquationsDirty = true;
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

        d->mEquationsDirty = true;
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

    d->mEquationsDirty = true;
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
        d->mEquationsDirty = true;
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
        d->mEquationsDirty = true;
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
        d->mEquationsDirty = true;
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
    return d->mValid;
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

    d->mEquationsDirty = true;
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
    d->mInvalidateCalled = false;
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
    d->mSizeHintDirty = true;
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
    if( mSizeHintDirty ) {
        updateAnchorsAndItems();
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
#ifdef HBMESHLAYOUT_DEBUG
    QString shText;
#endif
    if (which == Qt::MinimumSize) {
#ifdef HBMESHLAYOUT_DEBUG
        shText = "(minimum)";
#endif
        res = mMinSH;
    } else if (which == Qt::PreferredSize ) {
#ifdef HBMESHLAYOUT_DEBUG
        shText = "(preferred)";
#endif
        res = mPrefSH;
    } else {
#ifdef HBMESHLAYOUT_DEBUG
        shText = "(maximum)";
#endif
        res = mMaxSH;
    }
    
#ifdef HBMESHLAYOUT_DEBUG
    Q_Q(HbMeshLayout);
    QGraphicsWidget* w = HbLayoutUtils::parentWidget( q );
    if ( w ) {
        qDebug() << "MeshLayout: Size hint" << shText.toAscii().data() << "for" << w->metaObject()->className() << res;
    }    
#endif    
    return res;
}

