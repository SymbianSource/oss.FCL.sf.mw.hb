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

#include <QHash>
#include <QList>

class Variable
{
public:
    Variable() : mId(-1), mMin(0), mMax(0), mPref(0), mFlags(0), mRef(0) {}
    ~Variable() {}

    bool operator== ( const Variable &var ) const { return ( var.mId == mId ); }

public:
    int mId;
    qreal mMin;
    qreal mMax;
    qreal mPref;
    enum 
    {
        FlagFixed = 1,
        FlagExpanding = 2
    };
    uint mFlags;
    void *mRef; // reference to QGraphicsLayoutItem
};

typedef QHash<Variable, qreal> Solution;

uint qHash ( Variable key );

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

struct SimpleExpression
{
    Variable *mVar;
    qreal mCoef;
};

class Expression
{
public:
    Expression();
    ~Expression();
    void clear();
    void plusSimpleExpression( const SimpleExpression simp_exp );
    void plusExpression( const Expression exp );
    void minusSimpleExpression( const SimpleExpression simp_exp );
    void minusExpression( const Expression exp );
    void minus();

    qreal value( Solution *solution ) const;

    qreal minValue() const;
    qreal prefValue() const;
    qreal maxValue() const;
    qreal minValue( Solution *solution ) const;
    qreal prefValue( Solution *solution ) const;
    qreal prefExpandedValue( Solution *solution ) const;
    qreal maxValue( Solution *solution ) const;

    QString print() const;
    
public:
    QList<SimpleExpression> mExpression;
};


class Equation
{
public:
    Equation( const Expression &exp1, const Expression &exp2 );
    ~Equation();

    bool isTrivial() const;
    bool operator== ( const Equation &first ) const;
    Expression mFormula;
    QString print() const;
};

class EquationSolver
{
public:
    static bool solveEquation( QList<Equation> el, VariableSet &varset, Solution *solution );
    static bool compareExpressions( const Expression &expression1, const Expression &expression2 );
    static bool compareEquations( const Equation &equation1, const Equation &equation2 );

private:
    static int numOfUnknownVars( const Equation *eq, const Solution *solution );
};

class DataGrid
{
public:
    DataGrid();
    ~DataGrid();

    void setExpression( const Expression ex, int i, int j );
    void setExpression( const SimpleExpression ex, int i, int j );

    qreal value( int i, int j, Solution *solution, bool &ok ) const;

    void clear();

    QList<Equation> calculate();

private:
    void calculateLine( int x );
    bool checkLine( int x, int y );
    void calculateRect( int ax, int ay, int cx, int cy );

public:

    QList<Equation> mResult;
    int mSize;

    QHash< int, QHash< int, Expression > > mGrid;
};

#endif // HBANCHORLAYOUTENGINE_P_H
