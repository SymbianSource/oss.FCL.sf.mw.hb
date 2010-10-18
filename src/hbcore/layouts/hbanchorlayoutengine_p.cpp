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

#include "hbanchorlayoutengine_p.h"

//Uncomment next define in order to get more debug prints.
//Similar define exists also in the layout side.
//#define HBANCHORLAYOUT_DEBUG


#ifdef HBANCHORLAYOUT_DEBUG
#include <QtDebug>
#endif

static const qreal EPSILON = 0.01f;
static const qreal MAX_SIZE = 0xffffff;

// declared but never referenced:
//static inline bool myFuzzyCompare(double p1, double p2) //krazy:exclude=typedefs
//{
//    return (qAbs(p1 - p2) <= 0.0001);
//}

static inline bool myFuzzyCompare(float p1, float p2) //krazy:exclude=typedefs
{
    return (qAbs(p1 - p2) <= (float)EPSILON); //krazy:exclude=typedefs
}


// declared but never referenced:
//static inline qreal minSlope(const Variable &var, qreal coef )
//{
//    return var.sizeProp.pref + ( var.sizeProp.min - var.sizeProp.pref ) * coef;
//}

// declared but never referenced:
//static inline qreal maxSlope(const Variable &var, qreal coef )
//{
//    return var.sizeProp.pref + ( var.sizeProp.max - var.sizeProp.pref ) * coef;
//}

static inline bool differentSignOrZero( qreal val1, qreal val2 )
{
    return ( ( val1 < EPSILON ) && ( val2 > -EPSILON ) ) ||
           ( ( val1 > -EPSILON ) && ( val2 < EPSILON ) );
}


AnchorLayoutEngine::AnchorLayoutEngine()
{
}

AnchorLayoutEngine::~AnchorLayoutEngine()
{
}

AnchorLayoutEngine *AnchorLayoutEngine::instance()
{
    static AnchorLayoutEngine theAnchorLayoutSimplyfier;
    return &theAnchorLayoutSimplyfier;
}

bool AnchorLayoutEngine::processItems(
    QList<GraphEdge*> *edges,
    QList<GraphVertex*> *vertices,
    VariableSet *vs,
    QList<Expression*> *el )
{
    bool result = true;

    int oldEdgesNumber;
    int oldVerticesNumber;

    bool inLoop = true;

    enum {
        COMMON_ACTIONS,
        SPLIT_1,
        SPLIT_2,
        ERROR,
        DONE
    } state;

    state = COMMON_ACTIONS;
    while( inLoop ) {
        switch( state ) {
            case COMMON_ACTIONS:
                if( !findBranches( edges, vertices ) ) {
                    state = ERROR;
                    break;
                }

                oldEdgesNumber = edges->size();
                oldVerticesNumber = vertices->size();

                if( ! findSerialChains( edges, vertices, el ) ) {
                    state = ERROR;
                    break;
                }

                if( ! findParallelChains( edges, vertices, vs, el ) ) {
                    state = ERROR;
                    break;
                }

                if( ready( vertices ) ) {
                    state = DONE;
                    break;
                }

                if( ( oldEdgesNumber == edges->size() ) &&
                        ( oldVerticesNumber == vertices->size() ) ) {
                    state = SPLIT_1;
                    break;
                }

                break;
            case SPLIT_1:
                if( splitVertices( edges, vertices, 1 ) ) {
                    state = COMMON_ACTIONS;
                    break;
                }
                state = SPLIT_2;
                break;
            case SPLIT_2:
                if( splitVertices( edges, vertices, 2 ) ) {
                    state = COMMON_ACTIONS;
                    break;
                }
                state = ERROR;
                break;
            case ERROR:
                result = false;
                // cleanup;
            case DONE:
                inLoop = false;
                break;
            default:
                state = ERROR;
                break;
        }
    }

    return result;
}

GraphVertex *AnchorLayoutEngine::nextVertex(
    GraphVertex *currentVertex, GraphEdge *currentEdge, int *sign )
{
    if( currentEdge->startVertex == currentVertex ) {
        *sign = 1;
        return currentEdge->endVertex;
    }
    *sign = -1;
    return currentEdge->startVertex;
}

GraphEdge *AnchorLayoutEngine::nextEdge( GraphVertex *currentVertex, GraphEdge *currentEdge )
{
    if( currentVertex->edges.at(0) == currentEdge ) {
        return currentVertex->edges.at(1);
    }
    return currentVertex->edges.at(0);
}

bool AnchorLayoutEngine::findSerialChains(
    QList<GraphEdge*> *edges, QList<GraphVertex*> *vertices, QList<Expression*> *el )
{
    bool result = true;

    GraphEdge *newEdge;

    GraphEdge *currentEdge;
    GraphVertex *currentVertex;
    GraphVertex *initialVertex;

    GraphVertex *oldEndVertex, *oldStartVertex;
    GraphEdge *oldEndEdge, *oldStartEdge;

    GraphVertex *vertexToDelete;
    GraphEdge *edgeToDelete;

    GraphEdge *rightEdge, *leftEdge;

    int sign;


    Expression *expr;

    for( int i = 0; i < vertices->size(); i++ ) {

        initialVertex =  vertices->at(i);

        if( ( initialVertex->edges.size() == 2 ) && ( ! initialVertex->special ) ) {

            expr = new Expression();

            rightEdge = initialVertex->edges.at(1);
            leftEdge = initialVertex->edges.at(0);

            // left side
            currentEdge = rightEdge;
            currentVertex = initialVertex;


            edgeToDelete = 0;
            vertexToDelete = 0;

            do {

                vertices->removeOne( currentVertex );
                currentEdge = nextEdge( currentVertex, currentEdge );
                currentVertex = nextVertex( currentVertex, currentEdge, &sign );

                if( sign > 0 ) {
                    expr->minusExpression( currentEdge->expr );
                } else {
                    expr->plusExpression( currentEdge->expr );
                }

                edges->removeOne( currentEdge );

                delete vertexToDelete;

                if( edgeToDelete != leftEdge ) {
                    delete edgeToDelete;
                }

                vertexToDelete = currentVertex;
                edgeToDelete = currentEdge;


#ifdef HBANCHORLAYOUT_DEBUG
                qDebug() << "Adding one more left edge";
#endif //HBANCHORLAYOUT_DEBUG

            } while( ( currentVertex != initialVertex ) &&
                ( currentVertex->edges.size() == 2 ) && ( ! currentVertex->special ) );

            if( currentVertex == initialVertex ) {
                el->append( expr );
                edges->removeOne( currentVertex->edges.at(0) );
                vertices->removeOne( currentVertex );

                qDeleteAll( initialVertex->edges );
                delete currentVertex;
                i--;
                continue;
            }


            oldStartVertex = currentVertex;
            oldStartEdge = currentEdge;


            // right side
            currentEdge = leftEdge;
            currentVertex = initialVertex;

            edgeToDelete = 0;
            vertexToDelete = 0;

            do {
                vertices->removeOne( currentVertex );
                currentEdge = nextEdge( currentVertex, currentEdge );
                currentVertex = nextVertex( currentVertex, currentEdge, &sign );

                edges->removeOne( currentEdge );


                if( sign < 0 ) {
                    expr->minusExpression( currentEdge->expr );
                } else {
                    expr->plusExpression( currentEdge->expr );
                }

                delete vertexToDelete;

                if( edgeToDelete != rightEdge ) {
                    delete edgeToDelete;
                }

                vertexToDelete = currentVertex;
                edgeToDelete = currentEdge;


#ifdef HBANCHORLAYOUT_DEBUG
                qDebug() << "Adding one more right edge";
#endif //HBANCHORLAYOUT_DEBUG

            } while( ( currentVertex->edges.size() == 2 ) && ( ! currentVertex->special ) );


            oldEndVertex = currentVertex;
            oldEndEdge = currentEdge;

            if( oldStartVertex == oldEndVertex ) {
                el->append( expr );

                currentVertex->edges.removeOne( currentEdge );
                currentVertex->edges.removeOne( oldStartEdge );

                edges->removeOne( currentEdge );
                edges->removeOne( oldStartEdge );

                if( ( rightEdge != currentEdge ) && ( leftEdge != currentEdge ) ) {
                    delete currentEdge;
                }
                if( ( rightEdge != oldStartEdge ) && ( leftEdge != oldStartEdge ) ) {
                    delete oldStartEdge;
                }

                delete rightEdge;
                delete leftEdge;

                if( currentVertex->edges.size() == 0 ) {
                    vertices->removeOne( currentVertex );
                    if( !currentVertex->special ) {
                        delete currentVertex;
                    }
                    i--;
                }

                delete initialVertex;
                continue;
            }

            newEdge = new GraphEdge( false );

            newEdge->startVertex = oldStartVertex;
            newEdge->endVertex = oldEndVertex;

            oldStartVertex->edges.removeOne( oldStartEdge );
            oldEndVertex->edges.removeOne( oldEndEdge );

            oldStartVertex->edges.append( newEdge );
            oldEndVertex->edges.append( newEdge );

            newEdge->expr = expr;
            edges->append( newEdge );
            i--;

            if( ( rightEdge != oldStartEdge ) && ( leftEdge != oldStartEdge ) ) {
                delete oldStartEdge;
            }
            if( ( rightEdge != oldEndEdge ) && ( leftEdge != oldEndEdge ) ) {
                delete oldEndEdge;
            }

            delete rightEdge;
            delete leftEdge;
            delete initialVertex;


#ifdef HBANCHORLAYOUT_DEBUG
            qDebug()<< "### Serial New Edge: start= " << newEdge->startVertex->itemSide << ", " << (int)(newEdge->startVertex->itemRef) << ", " << newEdge->startVertex->special;
            qDebug()<< "### Serial New Edge: end= " << newEdge->endVertex->itemSide << ", " << (int)(newEdge->endVertex->itemRef) << ", " << newEdge->endVertex->special;
            qDebug() << "### Serial New Edge size prop: min" << expr->minValue() << " pref:" << expr->prefValue() <<
                    " max:" << expr->maxValue() << " flags" << expr->flags();
#endif //HBANCHORLAYOUT_DEBUG
        }
    }
    return result;
}


bool AnchorLayoutEngine::findParallelChains(
    QList<GraphEdge*> *edges,
    QList<GraphVertex*> *vertices,
    VariableSet *vs,
    QList<Expression*> *el )
{
    Q_UNUSED( vertices );
    bool result = true;
    GraphEdge *newEdge = 0;

    GraphVertex *oldStart = 0;
    GraphVertex *oldEnd = 0;

    GraphEdge *current;
    GraphEdge *compared;


    bool found = false;
    int sign;
    int numberOfParallelEdges;
    QList<qreal> expandingPrefValues;

    Expression *expr;

    SizeProperty *newEdgeSizeProp(0);

    SimpleExpression se; //ARM Warning: C2874W: <se.0> may be used before being set

    uint comparedEdgeFlags;

    for( int i = 0; ( i < edges->size() ) && result; i++ ) {
        current = edges->at(i);
        numberOfParallelEdges = 1;
        expandingPrefValues.clear();
        for( int j = i + 1; j <  edges->size(); j++ ) {
            compared = edges->at(j);
            found = false;

            if( ( current->startVertex == compared->startVertex ) &&
                ( current->endVertex == compared->endVertex ) ) {
                found = true;
                sign = 1;
            } else if( ( current->startVertex == compared->endVertex ) &&
                ( current->endVertex == compared->startVertex ) ) {
                found = true;
                sign = -1;
            }

            if( found ) {
                numberOfParallelEdges++;

                j--;
                if( numberOfParallelEdges == 2 ) {
                    newEdge = new GraphEdge();

                    oldStart = current->startVertex;
                    oldEnd = current->endVertex;

                    se.mVar = vs->createVariable( 0 );
                    se.mCoef = 1;

                    newEdgeSizeProp = &( se.mVar->sizeProp );
                    newEdgeSizeProp->min = current->expr->minValue();
                    newEdgeSizeProp->max = current->expr->maxValue();
                    newEdgeSizeProp->flags = current->expr->flags();

                    if( newEdgeSizeProp->flags & SizeProperty::FlagExpanding ) {
                        expandingPrefValues.append( current->expr->prefValue() );
                        newEdgeSizeProp->pref = 0;
                    } else {
                        newEdgeSizeProp->pref = current->expr->prefValue();
                    }



                    oldStart->edges.removeOne( current );
                    oldEnd->edges.removeOne( current );

                    oldStart->edges.append( newEdge );
                    oldEnd->edges.append( newEdge );

                    newEdge->startVertex = oldStart;
                    newEdge->endVertex = oldEnd;

                    edges->removeOne( current );

                    el->append( current->expr->toEquation( se ) );
                }

                oldStart->edges.removeOne( compared );
                oldEnd->edges.removeOne( compared );

                edges->removeOne( compared );

                comparedEdgeFlags = compared->expr->flags();

                if( ~newEdgeSizeProp->flags & SizeProperty::FlagFixed ) {
                    if( comparedEdgeFlags & SizeProperty::FlagFixed ) {
                        newEdgeSizeProp->pref = sign * compared->expr->prefValue();
                        newEdgeSizeProp->flags |= SizeProperty::FlagFixed;
                    } else {
                        if( sign * compared->expr->minValue() <
                                sign * compared->expr->maxValue()  ) {
                            newEdgeSizeProp->min =
                                qMax( newEdgeSizeProp->min, sign * compared->expr->minValue() );
                            newEdgeSizeProp->max =
                                qMin( newEdgeSizeProp->max, sign * compared->expr->maxValue() );
                        } else {
                            newEdgeSizeProp->min =
                                qMax( newEdgeSizeProp->min, sign * compared->expr->maxValue() );
                            newEdgeSizeProp->max =
                                qMin( newEdgeSizeProp->max, sign * compared->expr->minValue() );
                        }
                        if( comparedEdgeFlags & SizeProperty::FlagExpanding ) {
                            expandingPrefValues.append( sign * compared->expr->prefValue() );
                        } else {
                            newEdgeSizeProp->pref += sign * compared->expr->prefValue();
                        }
                    }

                    if( ~comparedEdgeFlags & SizeProperty::FlagExpanding ) {
                        newEdgeSizeProp->flags &= ~SizeProperty::FlagExpanding;
                    }

                    expr = new Expression();
                    if( sign < 0 ) {
                        expr->minusSimpleExpression( se );
                    } else {
                        expr->plusSimpleExpression( se );
                    }
                    expr->minusExpression( compared->expr );

                    el->append( expr );

                } else {
                    if( comparedEdgeFlags & SizeProperty::FlagFixed ) {
                        if( ! myFuzzyCompare( newEdgeSizeProp->pref,
                                              sign * compared->expr->prefValue() ) ) {
#ifdef HBANCHORLAYOUT_DEBUG
                            qDebug()<< "!!!  two different parallel fixed items";
#endif //HBANCHORLAYOUT_DEBUG
                            // ERROR!!!
                            delete compared;
                            delete current;
                            delete newEdge;
                            result = false;
                            break;
                        }
                    } else {
                        expr = new Expression();
                        if( sign < 0 ) {
                            expr->minusSimpleExpression( se );
                        } else {
                            expr->plusSimpleExpression( se );
                        }
                        expr->minusExpression( compared->expr );

                        el->append( expr );
                    }
                }
                delete compared;
            }
        }

        if( ( numberOfParallelEdges > 1 ) && result ) {
            delete current;
            i--; // ???
            newEdge->expr->plusSimpleExpression( se );
            edges->append( newEdge );
            if( newEdgeSizeProp->min - newEdgeSizeProp->max > EPSILON ) {
#ifdef HBANCHORLAYOUT_DEBUG
                qDebug() << "!!! min is bigger than max. min:" << newEdgeSizeProp->min << " max:" << newEdgeSizeProp->max;
#endif //HBANCHORLAYOUT_DEBUG
                // ERROR!!!
                result = false;
                break;
            } else {
                if( ~newEdgeSizeProp->flags & SizeProperty::FlagFixed ) {
                    int expectedPref = 0;
                    if( expandingPrefValues.size() < numberOfParallelEdges ) {
                        expectedPref = static_cast<int>(newEdgeSizeProp->pref /
                            ( numberOfParallelEdges - expandingPrefValues.size() ));
                    }

                    qSort( expandingPrefValues );
                    if( ( !expandingPrefValues.isEmpty() ) &&
                        ( qAbs( expandingPrefValues.last() ) < qAbs( expandingPrefValues.first() ) ) ) {
                        qSort( expandingPrefValues.begin(),
                            expandingPrefValues.end(), qGreater<qreal>() );
                    }

                    while( expandingPrefValues.size() > 0 ) {
                        if( qAbs( expandingPrefValues.last() ) > qAbs( expectedPref ) ) {
                            newEdgeSizeProp->pref += expandingPrefValues.last();
                            expandingPrefValues.removeLast();
                            expectedPref = static_cast<int>(newEdgeSizeProp->pref /
                                ( numberOfParallelEdges - expandingPrefValues.size() ));
                        } else {
                            break;
                        }
                    }
                    newEdgeSizeProp->pref = expectedPref;

                    if( newEdgeSizeProp->pref < newEdgeSizeProp->min ) {
                        newEdgeSizeProp->pref = newEdgeSizeProp->min;
                    }
                    if( newEdgeSizeProp->pref > newEdgeSizeProp->max ) {
                        newEdgeSizeProp->pref = newEdgeSizeProp->max;
                    }
                }
            }
#ifdef HBANCHORLAYOUT_DEBUG
            qDebug()<< "### Parallel New Edge: start= " << newEdge->startVertex->itemSide << ", " << (int)(newEdge->startVertex->itemRef) << ", " << newEdge->startVertex->special;
            qDebug()<< "### Parallel New Edge: end= " << newEdge->endVertex->itemSide << ", " << (int)(newEdge->endVertex->itemRef) << ", " << newEdge->endVertex->special;
            qDebug() << "### Parallel New Edge size prop: min" << newEdgeSizeProp->min << " pref:" << newEdgeSizeProp->pref <<
                    " max:" << newEdgeSizeProp->max << " flags" << newEdgeSizeProp->flags;
#endif //HBANCHORLAYOUT_DEBUG
        }
    }

    return result;
}


bool AnchorLayoutEngine::findBranches( QList<GraphEdge*> *edges, QList<GraphVertex*> *vertices )
{
    bool result = true;

    GraphEdge *currentEdge;
    GraphVertex *currentVertex;
    GraphVertex *initialVertex;

    GraphVertex *vertexToDelete;
    GraphEdge *edgeToDelete;


    int sign;

    for( int i = 0; i < vertices->size(); i++ ) {

        initialVertex =  vertices->at(i);

        if( ( initialVertex->edges.size() == 1 ) && ( ! initialVertex->special ) ) {

            currentEdge = initialVertex->edges.at(0);
            currentVertex = nextVertex( initialVertex, currentEdge, &sign );

            vertices->removeOne( initialVertex );
            delete initialVertex;
            edges->removeOne( currentEdge );

            while( ( currentVertex->edges.size() == 2 ) && ( ! currentVertex->special ) ) {
                vertexToDelete = currentVertex;
                edgeToDelete = currentEdge;

                vertices->removeOne( currentVertex );
                currentEdge = nextEdge( currentVertex, currentEdge );
                currentVertex = nextVertex( currentVertex, currentEdge, &sign );

                edges->removeOne( currentEdge );

                delete vertexToDelete;
                delete edgeToDelete;
            }

            currentVertex->edges.removeOne( currentEdge );

            if( currentVertex->edges.size() == 0 ) {
                vertices->removeOne( currentVertex );
                if( !currentVertex->special ) {
                    delete currentVertex;
                }
            }
            delete currentEdge;
            i = 0;
#ifdef HBANCHORLAYOUT_DEBUG
            qDebug()<< "### Branch New Edge";
#endif //HBANCHORLAYOUT_DEBUG
        }
    }

    return result;
}

void AnchorLayoutEngine::attachToLayout(
    GraphVertex *start, GraphVertex *middle, GraphVertex *end, Variable *layoutVar,
                                         QList<Expression*> *el )
{
    GraphEdge *current;
    SimpleExpression se;
    se.mVar = layoutVar;
    bool found;

    int count = 0;

    if( start ) {
        for( int i = 0; i < start->edges.size(); i++ ) {
            current = start->edges.at(i);
            found = false;
            if( ( current->startVertex == start ) && ( current->endVertex == middle ) ) {
                se.mCoef = 0.5;
                found = true;
            } else if( ( current->startVertex == middle ) && ( current->endVertex == start ) ) {
                se.mCoef = -0.5;
                found = true;
            } else if( ( current->startVertex == start ) && ( current->endVertex == end ) ) {
                se.mCoef = 1;
                found = true;
            } else if( ( current->startVertex == end ) && ( current->endVertex == start ) ) {
                se.mCoef = -1;
                found = true;
            }

            if( found ) {
                if( count == 0 ) {
                    layoutVar->sizeProp.min = 0;
                    layoutVar->sizeProp.max = MAX_SIZE;
                    layoutVar->sizeProp.pref = 0;
                    layoutVar->sizeProp.flags = 0;
                }
                count++;

                el->append( current->expr->toEquation( se ) );

                if( current->expr->isFixed() ) {
                    if( ~layoutVar->sizeProp.flags & SizeProperty::FlagFixed ) {
                        layoutVar->sizeProp.flags |= SizeProperty::FlagFixed;
                        layoutVar->sizeProp.pref = current->expr->prefValue() / se.mCoef;
                    }
                    // else some compare and indicate error
                } else if( ~layoutVar->sizeProp.flags & SizeProperty::FlagFixed ) {
                    if( current->expr->minValue() / se.mCoef < current->expr->maxValue() / se.mCoef ) {
                        layoutVar->sizeProp.min =
                            qMax( layoutVar->sizeProp.min, current->expr->minValue() / se.mCoef );
                        layoutVar->sizeProp.max =
                            qMin( layoutVar->sizeProp.max, current->expr->maxValue() / se.mCoef );
                    } else {
                        layoutVar->sizeProp.min =
                            qMax( layoutVar->sizeProp.min, current->expr->maxValue() / se.mCoef );
                        layoutVar->sizeProp.max =
                            qMin( layoutVar->sizeProp.max, current->expr->minValue() / se.mCoef );
                    }
                    layoutVar->sizeProp.pref += qAbs( current->expr->prefValue() / se.mCoef );
                }
                // check for exotic situations
            }
        }
    }

    if( middle ) {
        for( int i = 0; i < middle->edges.size(); i++ ) {
            current = middle->edges.at(i);
            found = false;
            if( ( current->startVertex == middle ) && ( current->endVertex == end ) ) {
                se.mCoef = 0.5;
                found = true;
            } else if( ( current->startVertex == end ) && ( current->endVertex == middle ) ) {
                se.mCoef = -0.5;
                found = true;
            }

            if( found ) {
                if( count == 0 ) {
                    layoutVar->sizeProp.min = 0;
                    layoutVar->sizeProp.max = MAX_SIZE;
                    layoutVar->sizeProp.pref = 0;
                    layoutVar->sizeProp.flags = 0;
                }
                count++;

                el->append( current->expr->toEquation( se ) );

                if( current->expr->isFixed() ) {
                    if( ~layoutVar->sizeProp.flags & SizeProperty::FlagFixed ) {
                        layoutVar->sizeProp.flags |= SizeProperty::FlagFixed;
                        layoutVar->sizeProp.pref = current->expr->prefValue() / se.mCoef;
                    }
                    // else some compare and indicate error
                } else if( ~layoutVar->sizeProp.flags & SizeProperty::FlagFixed ) {
                    if( current->expr->minValue() /
                            se.mCoef < current->expr->maxValue() / se.mCoef ) {
                        layoutVar->sizeProp.min =
                            qMax( layoutVar->sizeProp.min, current->expr->minValue() / se.mCoef );
                        layoutVar->sizeProp.max =
                            qMin( layoutVar->sizeProp.max, current->expr->maxValue() / se.mCoef );
                    } else {
                        layoutVar->sizeProp.min =
                            qMax( layoutVar->sizeProp.min, current->expr->maxValue() / se.mCoef );
                        layoutVar->sizeProp.max =
                            qMin( layoutVar->sizeProp.max, current->expr->minValue() / se.mCoef );
                    }
                    layoutVar->sizeProp.pref += qAbs( current->expr->prefValue() / se.mCoef );
                }
                // check for exotic situations

            }
        }
    }

    if( ( count > 0 ) && ( ~layoutVar->sizeProp.flags & SizeProperty::FlagFixed ) ){
        layoutVar->sizeProp.pref /= count;
        if( layoutVar->sizeProp.pref > layoutVar->sizeProp.max ) {
            layoutVar->sizeProp.pref = layoutVar->sizeProp.max;
        }
        if( layoutVar->sizeProp.pref < layoutVar->sizeProp.min ) {
            layoutVar->sizeProp.pref = layoutVar->sizeProp.min;
        }
    }
}

void AnchorLayoutEngine::cleanUp(
    GraphVertex *start,
    GraphVertex *middle,
    GraphVertex *end,
    QList<GraphEdge*> *edges,
    QList<GraphVertex*> *vertices,
    QList<Expression*> *el )
{
    if( start ) {
        vertices->removeOne( start );
        delete start;
    }
    if( middle ) {
        vertices->removeOne( middle );
        delete middle;
    }
    if( end ) {
        vertices->removeOne( end );
        delete end;
    }
    qDeleteAll( *edges );
    qDeleteAll( *vertices );
    edges->clear();
    vertices->clear();
    for( int i = 0; i < el->size(); i++ ) {
        if( el->at(i)->isTrivial() ) {
            delete el->takeAt(i);
            i--;
        }
    }
}


bool AnchorLayoutEngine::splitVertices(
    QList<GraphEdge*> *edges,
    QList<GraphVertex*> *vertices,
    int level )
{
    bool result = false;
    GraphEdge *newEdge, *current, *edgeToModify;
    GraphVertex *referenceVertex, *newVertex;

    bool found = false;

    for( int i = 0; i < edges->size(); i++ ) {
        current = edges->at(i);

        if( level == 1 ) {
            if( ! current->expr->isFixed() ) {
                continue;
            }
        }


        found = false;

        if( ( !current->startVertex->special ) && ( current->startVertex->edges.size() > 2 ) ) {
            referenceVertex = current->startVertex;
            found = true;
        } else if( ( !current->endVertex->special ) && ( current->endVertex->edges.size() > 2 ) ) {
            referenceVertex = current->endVertex;
            found = true;
        }


        if( found ) {
            result = true;
#ifdef HBANCHORLAYOUT_DEBUG
                qDebug() << "@@@@ Splitting vertices";
#endif //HBANCHORLAYOUT_DEBUG

            while( referenceVertex->edges.size() > 2 ) {
                edgeToModify = referenceVertex->edges.at( 0 );
                if( edgeToModify == current ) {
                    edgeToModify = referenceVertex->edges.at( 1 );
                }
                newVertex = new GraphVertex();
                newEdge = new GraphEdge();

                referenceVertex->edges.removeOne( edgeToModify );


                // looks like other parameters are useless
                newVertex->special = false;
                newVertex->edges.append( edgeToModify );

                if( ( edgeToModify->startVertex == referenceVertex ) ){
                    edgeToModify->startVertex = newVertex;
                } else {
                    edgeToModify->endVertex = newVertex;
                }

                vertices->append( newVertex );



                // looks like other parameters are useless
                if( referenceVertex == current->startVertex ) {
                    newEdge->startVertex = newVertex;
                    newEdge->endVertex = current->endVertex;
                    current->endVertex->edges.append( newEdge );
                } else {
                    newEdge->startVertex = current->startVertex;
                    newEdge->endVertex = newVertex;
                    current->startVertex->edges.append( newEdge );
                }
                newEdge->expr->plusExpression( current->expr );

                newVertex->edges.append( newEdge );
                edges->prepend( newEdge );
                i++;
            }
        }

    }

    return result;
}

bool AnchorLayoutEngine::ready( QList<GraphVertex*> *vertices )
{
    for( int i = 0; i < vertices->size(); i++ ) {
        if( ! vertices->at(i)->special ) {
            return false;
        }
    }
    return true;
}


bool AnchorLayoutEngine::solveEquation(
    QList<Expression*> *elOriginal,
    VariableSet *vs,
    Solution *solution )
{
    enum State {
        CheckEquationsNum,
        FillRest,
        SolveOne,
        Result,
        CheckEquation,
        Validate,
        FinalizeVariable,
        ExcludeEquation,
        SolveMany
    } state;

    qreal min;
    qreal max;
    qreal pref;
    int currentEquationNum = 0;
    QList<Expression*> el = *elOriginal;

    state = CheckEquationsNum;
    bool inloop = true;
    bool result = false;

    for ( int i = 0; i < vs->mVarList.size(); i++ ) {
        if ( vs->mVarList.at(i)->sizeProp.flags & SizeProperty::FlagFixed ) {
            solution->insert( vs->mVarList.at(i), vs->mVarList.at(i)->sizeProp.pref );
#ifdef HBANCHORLAYOUT_DEBUG
            qDebug( "Solved fixed variable: Id=%d value=%lf", vs->mVarList.at(i)->mId, vs->mVarList.at(i)->sizeProp.pref );
#endif // HBANCHORLAYOUT_DEBUG
        }
    }

#ifdef HBANCHORLAYOUT_DEBUG
    qDebug( "Equation list size = %d", el.size() );
    for( int i = 0; i < el.size(); i++ ) {
        qDebug() << "Equation " << i << ": " << el.at(i)->print() << " = 0";
    }

    QHashIterator<Variable*, qreal>  i(*solution);
    while (i.hasNext()) {
        i.next();
        qDebug() << ( ( Variable* )( i.key() ) )->mId << ": " << i.value();
    }
#endif //HBANCHORLAYOUT_DEBUG



    while ( inloop ) {
        switch ( state ) {
            case CheckEquationsNum:
            {
#ifdef HBANCHORLAYOUT_DEBUG
                qDebug( "state = CheckEquationsNum" );
#endif // HBANCHORLAYOUT_DEBUG
                if ( el.size() == 0 ) {
                    state = FillRest;
               } else {
                    state = CheckEquation;
                }
                break;
            }
            case FillRest:
            {
#ifdef HBANCHORLAYOUT_DEBUG
                qDebug( "state = FillRest" );
#endif // HBANCHORLAYOUT_DEBUG
                for ( int i = 0; i < vs->mVarList.size(); i++ ) {
                    if ( !solution->contains( vs->mVarList.at(i) ) ) {
                        solution->insert( vs->mVarList.at(i), vs->mVarList.at(i)->sizeProp.pref );
                    }
                }

                state = Result;
                result = true;
                break;
            }
            case SolveOne:
            {
#ifdef HBANCHORLAYOUT_DEBUG
                qDebug( "state = SolveOne" );
#endif // HBANCHORLAYOUT_DEBUG
                Expression *currentEquation = el.at(currentEquationNum);

                pref = currentEquation->prefValue( solution );

                // special block for expanding cases
                if( currentEquation->isExpanding() ) {
                    qreal maxExp = currentEquation->maxExpandedValue( solution );

#ifdef HBANCHORLAYOUT_DEBUG
                    qDebug( "pref=%lf  maxExp=%lf", pref, maxExp );
#endif // HBANCHORLAYOUT_DEBUG

                    if( differentSignOrZero( pref, maxExp ) ) {
                        for ( int i = 0; i < currentEquation->mExpression.size(); i++ ) {
                            SimpleExpression se = currentEquation->mExpression.at(i);

                            if ( ( se.mVar->sizeProp.flags & SizeProperty::FlagFixed ) ||
                                 ( se.mVar->sizeProp.flags & SizeProperty::FlagExpanding ) ||
                                    solution->contains( ( se.mVar ) ) ) {
                                continue;
                            }
                            solution->insert( se.mVar, se.mVar->sizeProp.pref );
                        }
                    } else if( ( ( pref < EPSILON ) && ( maxExp > pref ) ) ||
                        ( ( pref > -EPSILON ) && ( maxExp < pref ) ) ) {
                        for ( int i = 0; i < currentEquation->mExpression.size(); i++ ) {
                            SimpleExpression se = currentEquation->mExpression.at(i);

                            if ( ( se.mVar->sizeProp.flags & SizeProperty::FlagFixed ) ||
                                    solution->contains( ( se.mVar ) ) ) {
                                continue;
                            }

                            if( ( se.mVar->sizeProp.flags & SizeProperty::FlagExpanding ) ) {
                                solution->insert( se.mVar, se.mVar->sizeProp.max );
                            }
                        }
                        pref = currentEquation->prefValue( solution );
                    }
                }

                min = currentEquation->minValue( solution );
                max = currentEquation->maxValue( solution );

                qreal coef = 0;


                if ( ( ( min > 0 ) && !myFuzzyCompare(min, 0) ) ||
                     ( ( max < 0 ) && !myFuzzyCompare(max, 0) ) ) {
                    result = false;
                    state = Result;
                    break;
                }

                bool min_is_pref = false;

                if( pref < 0 ) {
                    min_is_pref = true;
                }

                if( myFuzzyCompare( pref, 0 ) ) {
                    coef = 0;
                } else if( min_is_pref ) {
                    coef = myFuzzyCompare( max, 0 ) ? 1 : ( pref / ( pref - max ) );
                } else {
                    coef = myFuzzyCompare( min, 0 ) ? 1 : ( pref / ( pref - min ) );
                }


                for ( int i = 0; i < currentEquation->mExpression.size(); i++ ) {
                    SimpleExpression se = currentEquation->mExpression.at(i);
                    qreal value;

                    if ( ( se.mVar->sizeProp.flags & SizeProperty::FlagFixed ) ||
                        solution->contains( ( se.mVar ) ) ) {
                        continue;
                    }

                    if( min_is_pref ) {

                        if ( se.mCoef > 0 ) {
                            value = se.mVar->sizeProp.pref
                                + ( se.mVar->sizeProp.max - se.mVar->sizeProp.pref )
                                * coef;
                        } else {
                            value = se.mVar->sizeProp.pref
                                + ( se.mVar->sizeProp.min - se.mVar->sizeProp.pref )
                                * coef;
                        }
                    } else {
                        if ( se.mCoef > 0 ) {
                            value = se.mVar->sizeProp.pref
                                + ( se.mVar->sizeProp.min - se.mVar->sizeProp.pref )
                                * coef;
                        } else {
                            value = se.mVar->sizeProp.pref
                                + ( se.mVar->sizeProp.max - se.mVar->sizeProp.pref )
                                * coef;
                        }
                    }

                    solution->insert( ( se.mVar ), value );
                }
                result = true;
                state = ExcludeEquation;
                break;
            }
            case Result:
            {
#ifdef HBANCHORLAYOUT_DEBUG
                qDebug( "state = Result" );
#endif // HBANCHORLAYOUT_DEBUG
                inloop = false;
                break;
            }
            case CheckEquation:
            {
#ifdef HBANCHORLAYOUT_DEBUG
                qDebug( "state = CheckEquation" );
#endif // HBANCHORLAYOUT_DEBUG

                bool real_break = false;
                int minUndefinedVariables = 0xffff;
                currentEquationNum = el.size() - 1;

                for ( int i = el.size() -1; i >= 0; i-- ) {
                    int num;
                    num = numOfUnknownVars( ( el.at(i) ), solution );
                    if ( num == 0 ) {
                        currentEquationNum = i;
                        state = Validate;
                        real_break = true;
                        break;
                    }
                    if ( num == 1 ) {
                        currentEquationNum = i;
                        state = FinalizeVariable;
                        real_break = true;
                        break;
                    }

                    if( num < minUndefinedVariables ) {
                        minUndefinedVariables = num;
                        currentEquationNum = i;
                    }
                }

                if ( real_break ) {
                    break;
                }

                state = SolveMany;
                break;
            }
            case Validate:
            {
#ifdef HBANCHORLAYOUT_DEBUG
                qDebug( "state = Validate" );
#endif // HBANCHORLAYOUT_DEBUG

                if ( !myFuzzyCompare( el.at(currentEquationNum)->value( solution ), 0 ) ) {
                    qDebug( "val = %lf", el.at(currentEquationNum)->value( solution ) );
                    result = false;
                    state = Result;
                } else {
                    state = ExcludeEquation;
                }

                break;
            }
            case FinalizeVariable:
            {
#ifdef HBANCHORLAYOUT_DEBUG
                qDebug( "state = FinalizeVariable" );
#endif // HBANCHORLAYOUT_DEBUG

                Expression exp = *( el.at(currentEquationNum) );
                SimpleExpression se;
                se.mCoef = 0;
                se.mVar = 0;

                for ( int i = 0; i < exp.mExpression.size(); i++ ) {
                    if ( !solution->contains( ( exp.mExpression.at(i).mVar ) ) ) {
                        se = exp.mExpression.at(i);
                        exp.mExpression.removeAt( i );
                        break;
                    }
                }

                qreal value = - exp.value( solution ) / se.mCoef;

                if ( ( value - se.mVar->sizeProp.min < -EPSILON ) ||
                    ( value - se.mVar->sizeProp.max > EPSILON  ) ) {
#ifdef HBANCHORLAYOUT_DEBUG
                    qDebug( "cannot solve: min=%lf, max=%lf, value=%lf", se.mVar->sizeProp.min, se.mVar->sizeProp.max, value );
#endif
                    result = false;
                    state = Result;
                    break;
                }

                solution->insert( ( se.mVar ), value );

                state = ExcludeEquation;
                break;
            }
            case ExcludeEquation:
            {
#ifdef HBANCHORLAYOUT_DEBUG
                qDebug( "state = ExcludeEquation" );
#endif // HBANCHORLAYOUT_DEBUG
                el.removeAt( currentEquationNum );
                state = CheckEquationsNum;
                break;
            }
            case SolveMany:
            {
#ifdef HBANCHORLAYOUT_DEBUG
                qDebug( "state = SolveMany" );
#endif // HBANCHORLAYOUT_DEBUG
                state = SolveOne;
                break;
            }
        }
    }

#ifdef HBANCHORLAYOUT_DEBUG
    qDebug( "result = %d\n", (int)result );
#endif // HBANCHORLAYOUT_DEBUG
    return result;
}

int AnchorLayoutEngine::numOfUnknownVars( const Expression *eq, const Solution *solution )
{
    int result = 0;

    for ( int i = 0; i < eq->mExpression.size(); i++ ) {
        if ( solution->contains( (eq->mExpression.at(i).mVar) ) ) {
            continue;
        }
        result++;
    }

    return result;

}

Expression *Expression::toEquation( const SimpleExpression simp_exp )
{
    Expression *expr = new Expression();
    expr->plusExpression( this );
    expr->minusSimpleExpression( simp_exp );
    return expr;
}

Expression *Expression::toEquation( const Expression *exp )
{
    Expression *expr = new Expression();
    expr->plusExpression( this );
    expr->minusExpression( exp );
    return expr;
}

Expression *Expression::toEquation()
{
    Expression *expr = new Expression();
    expr->plusExpression( this );
    return expr;
}


bool Expression::isTrivial() const
{
    if ( mExpression.count() == 0 ) {
        return true;
    }
    return false;
}

bool Expression::isFixed() const
{
    bool result = true;

    for ( int i = 0; i < mExpression.size(); i++ ) {
        result &= ( mExpression.at(i).mVar->sizeProp.flags & SizeProperty::FlagFixed );
    }

    return result;

}

bool Expression::isExpanding() const
{
    bool result = false;

    for ( int i = 0; i < mExpression.size(); i++ ) {
        result |= (bool)( mExpression.at(i).mVar->sizeProp.flags & SizeProperty::FlagExpanding );
    }

    return result;
}

uint Expression::flags() const
{
    bool isFixed = true;
    bool isExpanding = false;
    uint result = 0;

    for ( int i = 0; i < mExpression.size(); i++ ) {
        isFixed = isFixed && ( mExpression.at(i).mVar->sizeProp.flags & SizeProperty::FlagFixed );
        isExpanding = ( isExpanding ) ||
            ( mExpression.at(i).mVar->sizeProp.flags & SizeProperty::FlagExpanding );
    }

    if( isFixed ) {
        result |= SizeProperty::FlagFixed;
    }
    if( isExpanding ) {
        result |= SizeProperty::FlagExpanding;
    }

    return result;
}

/*!
    \internal
*/
void Expression::clear()
{
    mExpression.clear();
}

/*!
    \internal
*/
void Expression::plusSimpleExpression( const SimpleExpression simp_exp )
{
    SimpleExpression *item;

    if( myFuzzyCompare( simp_exp.mCoef, 0 ) ) {
        return;
    }

    for ( int i = 0; i < mExpression.size(); i++ ) {
        item = &(mExpression[i]);

        if ( item->mVar->mId < simp_exp.mVar->mId ) {
            continue;
        }

        if ( item->mVar->mId == simp_exp.mVar->mId ) {
            item->mCoef += simp_exp.mCoef;
            if ( myFuzzyCompare( item->mCoef, 0 ) ) {
                mExpression.removeAt( i );
            }
            return;
        } else {
            mExpression.insert( i, simp_exp );
            return;
        }
    }

    mExpression.append( simp_exp );
}

/*!
    \internal
*/
void Expression::plusExpression( const Expression *exp )
{
    for ( int i = 0; i < exp->mExpression.size(); i++ ) {
        plusSimpleExpression( exp->mExpression.at(i) );
    }
}

/*!
    \internal
*/
void Expression::minusSimpleExpression( const SimpleExpression simp_exp )
{
    SimpleExpression se = simp_exp;
    se.mCoef = -se.mCoef;
    plusSimpleExpression( se );
}

/*!
    \internal
*/
void Expression::minusExpression( const Expression *exp )
{
    for ( int i = 0; i < exp->mExpression.size(); i++ ) {
        minusSimpleExpression( exp->mExpression.at(i) );
    }
}

/*!
    \internal
*/
void Expression::multiply( qreal multiplier )
{
    if( multiplier == 0 ) {
        clear();
    }

    for ( int i = 0; i < mExpression.size(); i++ ) {
        mExpression[i].mCoef *= multiplier;
    }
}

/*!
    \internal
*/
qreal Expression::value( Solution *solution ) const
{
    qreal result = 0;

    for ( int i = 0; i < mExpression.size(); i++ ) {
        result += solution->value( (mExpression.at(i).mVar) ) * mExpression.at(i).mCoef;
    }

    return result;
}

/*!
    \internal
*/
qreal Expression::minValue() const
{
    qreal result = 0;

    for ( int i = 0; i < mExpression.size(); i++ ) {
        const SimpleExpression currentExpression = mExpression.at(i);
        if ( currentExpression.mVar->sizeProp.flags & SizeProperty::FlagFixed ) {
            result += currentExpression.mVar->sizeProp.pref * currentExpression.mCoef;
        } else {
            result += qMin( currentExpression.mVar->sizeProp.max *
                currentExpression.mCoef,
                currentExpression.mVar->sizeProp.min * currentExpression.mCoef );
        }
    }

    return result;
}

/*!
    \internal
*/
qreal Expression::prefValue() const
{
    qreal result = 0;

    for ( int i = 0; i < mExpression.size(); i++ ) {
         const SimpleExpression currentExpression = mExpression.at(i);
         result += currentExpression.mVar->sizeProp.pref * currentExpression.mCoef;
    }

    return result;
}

/*!
    \internal
*/
qreal Expression::maxValue() const
{
    qreal result = 0;

    for ( int i = 0; i < mExpression.size(); i++ ) {
        const SimpleExpression currentExpression = mExpression.at(i);
        if ( currentExpression.mVar->sizeProp.flags & SizeProperty::FlagFixed ) {
            result += currentExpression.mVar->sizeProp.pref * currentExpression.mCoef;
        } else {
            result += qMax( currentExpression.mVar->sizeProp.max *
                currentExpression.mCoef,
                currentExpression.mVar->sizeProp.min * currentExpression.mCoef );
        }
    }

    return result;
}

/*!
    \internal
*/
qreal Expression::minValue( Solution *solution ) const
{
    qreal result = 0;

    for ( int i = 0; i < mExpression.size(); i++ ) {
        const SimpleExpression currentExpression = mExpression.at(i);
        if ( currentExpression.mVar->sizeProp.flags & SizeProperty::FlagFixed ) {
            result += currentExpression.mVar->sizeProp.pref * currentExpression.mCoef;
        } else if ( solution->contains( ( currentExpression.mVar ) ) ) {
            result += solution->value( ( currentExpression.mVar ) ) * currentExpression.mCoef;
        } else {
            result += qMin( currentExpression.mVar->sizeProp.max *
                currentExpression.mCoef,
                currentExpression.mVar->sizeProp.min * currentExpression.mCoef );
        }
    }

    return result;
}

/*!
    \internal
*/
qreal Expression::prefValue( Solution *solution ) const
{
    qreal result = 0;

    for ( int i = 0; i < mExpression.size(); i++ ) {
        const SimpleExpression currentExpression = mExpression.at(i);
        if ( solution->contains( ( currentExpression.mVar ) ) ) {
            result += solution->value( ( currentExpression.mVar ) ) * currentExpression.mCoef;
        } else {
            result += currentExpression.mVar->sizeProp.pref * currentExpression.mCoef;
        }
    }

    return result;
}

/*!
    \internal
*/
qreal Expression::maxValue( Solution *solution ) const
{
    qreal result = 0;

    for ( int i = 0; i < mExpression.size(); i++ ) {
        const SimpleExpression currentExpression = mExpression.at(i);
        if ( currentExpression.mVar->sizeProp.flags & SizeProperty::FlagFixed ) {
            result += currentExpression.mVar->sizeProp.pref * currentExpression.mCoef;
        } else if ( solution->contains( ( currentExpression.mVar ) ) ) {
            result += solution->value( ( currentExpression.mVar ) ) * currentExpression.mCoef;
        } else {
            result += qMax( currentExpression.mVar->sizeProp.max *
                currentExpression.mCoef,
                currentExpression.mVar->sizeProp.min * currentExpression.mCoef );
        }
    }

    return result;
}


qreal Expression::maxExpandedValue( Solution *solution ) const
{
    qreal result = 0;

    for ( int i = 0; i < mExpression.size(); i++ ) {
        const SimpleExpression &currentExpression = mExpression.at(i);
        if ( currentExpression.mVar->sizeProp.flags & SizeProperty::FlagExpanding ) {
            if ( currentExpression.mVar->sizeProp.flags & SizeProperty::FlagFixed ) {
                result += currentExpression.mVar->sizeProp.pref * currentExpression.mCoef;
            } else if ( solution->contains( currentExpression.mVar ) ) {
                result += solution->value( currentExpression.mVar ) * currentExpression.mCoef;
            } else {
                result += currentExpression.mVar->sizeProp.max * currentExpression.mCoef;
            }
        }
        else if ( solution->contains( currentExpression.mVar ) ) {
            result += solution->value( currentExpression.mVar ) * currentExpression.mCoef;
        } else {
            result += currentExpression.mVar->sizeProp.pref * currentExpression.mCoef;
        }
    }

    return result;
}



QString Expression::print() const
{
    QString res;
    for( int i = 0; i < mExpression.size(); i++ ) {
        if( i > 0 ) {
            res += " + ";
        }
        if( mExpression.at(i).mVar->sizeProp.flags & SizeProperty::FlagFixed ) {
            res += '(' + QString::number( mExpression.at(i).mCoef ) +
                ')' + "var[" + QString::number( mExpression.at(i).mVar->mId ) + ']' + '(' +
                    QString::number( mExpression.at(i).mVar->sizeProp.pref ) + "|f)";
        } else {
            res += '(' + QString::number( mExpression.at(i).mCoef ) +
                ')' + "var[" + QString::number( mExpression.at(i).mVar->mId ) + ']' + '(' +
                    QString::number( mExpression.at(i).mVar->sizeProp.min ) + ',' +
                    QString::number( mExpression.at(i).mVar->sizeProp.pref ) + ',' +
                    QString::number( mExpression.at(i).mVar->sizeProp.max ) + ')';
        }
    }
    return res;
}


/*!
    \internal
*/
VariableSet::VariableSet() : mVarList(), mCurr_id(0)
{
}

/*!
    \internal
*/
VariableSet::~VariableSet()
{
    qDeleteAll(mVarList);
}

/*!
    \internal
*/
Variable *VariableSet::createVariable( void *ref )
{
    Variable *var = new Variable();
    var->mId = mCurr_id;
    var->sizeProp.flags = 0;
    var->mRef = ref;

    mVarList.append( var );

    mCurr_id++;

    return var;
}

/*!
    \internal
*/
bool VariableSet::removeVariable( Variable *var )
{
    if( var ) {
        return mVarList.removeOne( var );
    }
    return false;
}

/*!
    \internal
*/
Variable *VariableSet::findVariable( void *ref ) const
{
    if( ! ref ) {
        return 0;
    }

    QList<Variable*>::const_iterator i;
    for ( i = mVarList.constBegin(); i != mVarList.constEnd(); ++i ) {
        if ( (*i)->mRef == ref ) return *i;
    }
    return 0;
}


/*!
    \internal
*/
void VariableSet::clear()
{
    qDeleteAll( mVarList );
    mVarList.clear();
    mCurr_id = 0;
}


GraphEdge::GraphEdge( bool initExpr )
{
    if( initExpr ) {
        expr = new Expression();
    } else {
        expr = 0;
    }
}

GraphEdge::~GraphEdge()
{
    delete expr;
}

