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

#ifndef HBANCHORLAYOUTENGINE_P_H
#define HBANCHORLAYOUTENGINE_P_H

#include <hbglobal.h>
#include <hbnamespace.h>
#include <QList>


struct GraphEdge;
struct GraphVertex;
struct SizeProperty;
struct SerialBlockData;
struct ParallelBlockData;

class Variable;
class Expression;

struct SimpleExpression
{
    Variable *mVar;
    qreal mCoef;
};


struct SizeProperty {
    qreal min, pref, max;
    enum
    {
        FlagFixed = 1,
        FlagExpanding = 2
    };
    uint flags;
};




class Variable
{
public:

    inline bool operator== ( const Variable &var ) const { return ( var.mId == mId ); }

    int mId;

    SizeProperty sizeProp;

    void *mRef; // reference to QGraphicsLayoutItem
};

class VariableSet
{
public:
    VariableSet();
    ~VariableSet();

    Variable *createVariable( void *ref );
    bool removeVariable( Variable *var );
    Variable *findVariable( void *ref ) const;

    void clear();

public:
    QList<Variable*> mVarList;
    int mCurr_id;
};


typedef QHash<Variable*, qreal> Solution;


class Expression
{
public:
    void clear();
    void plusSimpleExpression( const SimpleExpression simp_exp );
    void plusExpression( const Expression *exp );
    void minusSimpleExpression( const SimpleExpression simp_exp );
    void minusExpression( const Expression *exp );
    void multiply( qreal multiplier );

    Expression *toEquation( const SimpleExpression simp_exp );
    Expression *toEquation( const Expression *exp );
    Expression *toEquation();

    bool isFixed() const;
    bool isExpanding() const;

    bool isTrivial() const;

    uint flags() const;

    qreal value( Solution *solution ) const;

    qreal minValue() const;
    qreal prefValue() const;
    qreal maxValue() const;
    qreal minValue( Solution *solution ) const;
    qreal prefValue( Solution *solution ) const;
    qreal maxValue( Solution *solution ) const;
    qreal maxExpandedValue( Solution *solution ) const;




    QString print() const;

public:
    QList<SimpleExpression> mExpression;
};



class AnchorLayoutEngine {
private:
    AnchorLayoutEngine();
    ~AnchorLayoutEngine();
public:
    static AnchorLayoutEngine *instance();
    bool processItems( QList<GraphEdge*> *edges, QList<GraphVertex*> *vertices, VariableSet *vs, QList<Expression*> *el );
    void attachToLayout( GraphVertex *start, GraphVertex *middle, GraphVertex *end, Variable *layoutVar, QList<Expression*> *el );
    void cleanUp( GraphVertex *start, GraphVertex *middle, GraphVertex *end, QList<GraphEdge*> *edges, QList<GraphVertex*> *vertices, QList<Expression*> *el );
    bool solveEquation( QList<Expression*> *el, VariableSet *vs, Solution *solution );
private:
    GraphVertex *nextVertex( GraphVertex *currentVertex, GraphEdge *currentEdge, int *sign );
    GraphEdge *nextEdge( GraphVertex *currentVertex, GraphEdge *currentEdge );
    bool findSerialChains( QList<GraphEdge*> *edges, QList<GraphVertex*> *vertices, QList<Expression*> *el );
    bool findParallelChains( QList<GraphEdge*> *edges, QList<GraphVertex*> *vertices, VariableSet *vs, QList<Expression*> *el );
    bool findBranches( QList<GraphEdge*> *edges, QList<GraphVertex*> *vertices );
    bool splitVertices( QList<GraphEdge*> *edges, QList<GraphVertex*> *vertices, int level );
    bool ready( QList<GraphVertex*> *vertices );

    int numOfUnknownVars( const Expression *eq, const Solution *solution );
};

struct GraphEdge {
    GraphEdge( bool initExpr = true );
    ~GraphEdge();
    void *ref;
    GraphVertex *startVertex, *endVertex;
    Expression *expr;
};

struct GraphVertex {
    QList<GraphEdge*> edges;
    bool special;
    Hb::Edge itemSide;
    void *itemRef;
};


#endif // HBANCHORLAYOUTENGINE_P_H
