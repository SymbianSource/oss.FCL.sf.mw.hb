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

#define EPSILON 0.01f

static inline bool myFuzzyCompare(double p1, double p2)
{
    return (qAbs(p1 - p2) <= 0.000000000001);
}

static inline bool myFuzzyCompare(float p1, float p2)
{
    return (qAbs(p1 - p2) <= 0.00001f);
}

/*!
    \internal
*/
DataGrid::DataGrid() : mResult(), mSize( 0 )
{
}

/*!
    \internal
*/
DataGrid::~DataGrid()
{
    clear();
}

/*!
    \internal
*/
void DataGrid::clear()
{
    mGrid.clear();
    mResult.clear();
    mSize = 0;
}

/*!
    \internal
*/
void DataGrid::setExpression( const SimpleExpression ex, int i, int j )
{
    Expression e;
    e.plusSimpleExpression( ex );
    setExpression( e, i, j );
}

/*!
    \internal
*/
void DataGrid::setExpression( const Expression ex, int i, int j )
{
    Expression ex2 = ex;
    if ( mGrid[i].contains( j ) ) {
        Equation eq( mGrid.value(i).value(j), ex );
        if ( ( ! mResult.contains( eq ) ) && ( !eq.isTrivial() ) ) {
            mResult.append( eq );
#ifdef HBANCHORLAYOUT_DEBUG
            qDebug( "Create extra equation on cell %d %d: %s", i, j, eq.print().toLatin1().data() );
#endif
            return;
        }
    }

    mGrid[i][j] = ex2;
    ex2.minus();
    mGrid[j][i] = ex2;


    if (mSize  < i + 1) {
        mSize = i + 1;
    }
    if (mSize  < j + 1) {
        mSize = j + 1;
    }
}

/*!
    \internal
*/
qreal DataGrid::value( int i, int j, Solution *solution, bool &ok )  const
{
    ok = true;
    if( mGrid.value(i).contains( j ) ) {
        return mGrid.value(i).value(j).value( solution );
    }
    ok = false;
    return 0;
}

/*!
    \internal
*/
QList<Equation> DataGrid::calculate()
{

#ifdef HBANCHORLAYOUT_DEBUG
    qDebug( "\nbefore:" );

    for( int j = 0; j < mSize; j++ ) {
        QString line;
        line.sprintf( "%d  ", j );
        for( int i = 0; i < mSize; i++ ) {
            if( mGrid.value(i).contains( j ) ) {
                line.append( "X   " );
            } else {
                line.append( "_   " );
            }
        }
        line.append( '\n' );
        qDebug() << line;
    }
#endif // HBANCHORLAYOUT_DEBUG


    for( int i = 1; i < mSize; i++ ) {
        calculateLine( i );
    }

    for( int i = mSize - 1; i > 0; i-- ) {
        if( ! mGrid.value( i ).contains( 0 ) ) {
            calculateLine( i );
        }
    }

    for( int i = 1; i < mSize; i++ ) {
        if( ! mGrid.value( i ).contains( 0 ) ) {
            calculateLine( i );
        }
    }

#ifdef HBANCHORLAYOUT_DEBUG
    qDebug( "\nafter:" );

    for( int j = 0; j < mSize; j++ ) {
        QString line;
        line.sprintf( "%d  ", j );
        for( int i = 0; i < mSize; i++ ) {
            if( mGrid.value(i).contains( j ) ) {
                line.append( "X   " );
            } else {
                line.append( "_   " );
            }
        }
        line.append( '\n' );
        qDebug() << line;
    }

    qDebug( "number of equations: %d", mResult.size() );
    for( int i = 0; i < mResult.size(); i++ ) {
        qDebug( "equation %d: %s", i, mResult.at(i).print().toLatin1().data() );
    }
#endif // HBANCHORLAYOUT_DEBUG

    return mResult;
}

/*!
    \internal
*/
void DataGrid::calculateLine( int x )
{
    QHash< int, Expression > vertical = mGrid.value( x );
    bool flag = false;

    QList<int>::const_iterator i;
    QList<int> keys = vertical.keys();

    for ( i = keys.constBegin(); i != keys.constEnd(); ++i ) {
        if ( ( *i <= x ) && ( *i != 0 ) ) {
            if ( checkLine( x, *i ) ) {
                flag = true;
            }
        }
    }

    if ( flag ) {
        return;
    }

    for ( i = keys.constBegin(); i != keys.constEnd(); ++i ) {
        if ( *i > x ) {
            checkLine( x, *i );
        }
    }
}

/*!
    \internal
*/
bool DataGrid::checkLine( int x, int y )
{
    for ( int i = 0; i < mSize; i++ ) {
        if ( ( mGrid.value(i).contains( y ) ) && ( mGrid.value(i).contains( 0 ) ) && ( i != x ) ) {
            calculateRect( x, 0, i, y );
            return true;
        }
    }
    return false;
}

/*!
    \internal
*/
void DataGrid::calculateRect( int ax, int ay, int cx, int cy )
{
    Expression exp;

    /*
    A ---  B
    --------
    D ---  C

    A = B + D - C
    */

    exp.minusExpression( mGrid.value(cx).value(cy) );
    exp.plusExpression( mGrid.value(cx).value(ay) );
    exp.plusExpression( mGrid.value(ax).value(cy) );

    if ( mGrid.value(ax).contains( ay ) ) {
        Equation eq( mGrid.value(ax).value(ay), exp );
        if ( ( ! mResult.contains( eq ) ) && ( !eq.isTrivial() ) ) {
            mResult.append( eq );
        }
    } else {
        mGrid[ax][ay] = exp;
    }

}

/*!
    \internal
*/
Equation::Equation( const Expression &exp1, const Expression &exp2 ) : mFormula()
{
    mFormula.plusExpression( exp1 );
    mFormula.minusExpression( exp2 );
}

/*!
    \internal
*/
Equation::~Equation()
{
}

/*!
    \internal
*/
bool Equation::isTrivial() const
{
    if ( mFormula.mExpression.count() == 0 ) {
        return true;
    }
    return false;
}

/*!
    \internal
*/
bool Equation::operator== ( const Equation &first ) const
{
    return EquationSolver::compareEquations( first, *this );
}

QString Equation::print() const
{
    return QString( mFormula.print() ) + QString( " = 0" );
}


static inline qreal minSlope(const Variable &var, qreal coef )
{
    return var.mPref + ( var.mMin - var.mPref )* coef;
}

static inline qreal maxSlope(const Variable &var, qreal coef )
{
    return var.mPref + ( var.mMax - var.mPref )* coef;
}


/*!
    \internal
*/
bool EquationSolver::solveEquation( QList<Equation> el, VariableSet &varset, Solution *solution )
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

    state = CheckEquationsNum;
    bool inloop = true;
    bool result = false;

    solution->clear();

    for ( int i = 0; i < varset.mVarList.size(); i++ ) {
        if ( varset.mVarList.at(i)->mFlags&Variable::FlagFixed ) {
            solution->insert( *(varset.mVarList.at(i)), varset.mVarList.at(i)->mPref );
#ifdef HBANCHORLAYOUT_DEBUG
            qDebug( "Solved fixed variable: Id=%d value=%lf", varset.mVarList.at(i)->mId, varset.mVarList.at(i)->mPref );
#endif // HBANCHORLAYOUT_DEBUG
        }
    }



    while ( inloop ) {
        switch ( state ) {
            case CheckEquationsNum:
            {
#ifdef HBANCHORLAYOUT_DEBUG
                qDebug( "state = CheckEquationsNum" );
#endif // HBANCHORLAYOUT_DEBUG
                if ( el.size() == 0 ) {
                    state = FillRest;
//                } else if ( el.size() == 1 ) {
//                    state = SolveOne;
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
                for ( int i = 0; i < varset.mVarList.size(); i++ ) {
                    if ( !solution->contains( *(varset.mVarList.at(i)) ) ) {
                        solution->insert( *(varset.mVarList[i]), varset.mVarList.at(i)->mPref );
                    }
                }

                state = Result;
                result = true;
                break;
            }
            case SolveOne:
            {
                currentEquationNum = el.size() - 1;
#ifdef HBANCHORLAYOUT_DEBUG
                qDebug( "state = SolveOne" );
#endif // HBANCHORLAYOUT_DEBUG
                Equation currentEquation = el.at(currentEquationNum);

                min = currentEquation.mFormula.minValue( solution );
                max = currentEquation.mFormula.maxValue( solution );
                pref = currentEquation.mFormula.prefValue( solution );
                const qreal prefExpanded = currentEquation.mFormula.prefExpandedValue( solution );

                qreal coef = 0;
                
                if ( ( ( min > 0 ) && !myFuzzyCompare(min, 0) ) ||
                     ( ( max < 0 ) && !myFuzzyCompare(max, 0) ) ) {
                    result = false;
                    state = Result;
                    break;
                }

                bool min_is_pref = false;
                bool min_is_prefExp = false;
                
                if( pref < 0 ) {
                    min_is_pref = true;
                } else if ( prefExpanded < 0 ) {
                    min_is_prefExp = true;
                }
                
                if( myFuzzyCompare( pref, 0 ) ) {
                    coef = 0;
                } else if( min_is_pref ) {
                    if( myFuzzyCompare( max, 0 ) ) {
                        coef = 1;
                    } else {
                        coef = ( pref / ( pref - max ) );
                    }
                } else {
                    if( myFuzzyCompare( min, 0 ) ) {
                        coef = 1;
                    } else {
                        if ( min_is_prefExp ) {
                            coef = ( pref / ( pref - prefExpanded ) );
                        } else {
                            coef = ( prefExpanded / ( prefExpanded - min ) );
                        }
                    }
                }


                for ( int i = 0; i < currentEquation.mFormula.mExpression.size(); i++ ) {
                    SimpleExpression se = currentEquation.mFormula.mExpression[i];
                    qreal value;

                    if ( (se.mVar->mFlags&Variable::FlagFixed) || solution->contains( *( se.mVar ) ) ) {
                        continue;
                    }

                    if( min_is_pref ) {

                        if ( se.mCoef > 0 ) {
                            value = maxSlope( *se.mVar, coef );
                        } else {
                            value = minSlope( *se.mVar, coef );
                        }
                    } else {
                        if ( min_is_prefExp ) {
                            if ( se.mVar->mFlags&Variable::FlagExpanding ){
                                if ( se.mCoef > 0 ) {
                                    value = minSlope( *se.mVar, coef );
                                  } else {
                                    value = maxSlope( *se.mVar, coef );
                                } 
                            }else {
                                value = se.mVar->mPref;
                            }

                        } else {
                            if ( se.mVar->mFlags&Variable::FlagExpanding ){
                                if ( se.mCoef > 0 ) {
                                    value = se.mVar->mMin;
                                } else {
                                    value = se.mVar->mMax;
                                }
                            } else {
                                if ( se.mCoef > 0 ) {
                                    value = minSlope( *se.mVar, coef );
                                 } else {
                                    value = maxSlope( *se.mVar, coef );
                                }
                            }
                        }
                    }

                    solution->insert( *( se.mVar ), value );
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

                for ( int i = 0; i < el.size(); i++ ) {
                    int num;
                    num = numOfUnknownVars( &( el.at(i) ), solution );
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

                if ( !myFuzzyCompare( el.at(currentEquationNum).mFormula.value( solution ), 0 ) ) {
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

                Expression exp = el.at(currentEquationNum).mFormula;
                SimpleExpression se;
                se.mCoef = 0;
                se.mVar = 0;

                for ( int i = 0; i < exp.mExpression.size(); i++ ) {
                    if ( !solution->contains( *( exp.mExpression.at(i).mVar ) ) ) {
                        se = exp.mExpression.at(i);
                        exp.mExpression.removeAt( i );
                        break;
                    }
                }

                qreal value = - exp.value( solution ) / se.mCoef;

                if ( ( value - se.mVar->mMin < -EPSILON ) || ( value - se.mVar->mMax > EPSILON ) ) {
#ifdef HBANCHORLAYOUT_DEBUG
                    qDebug( "cannot solve: min=%lf, max=%lf, value=%lf", se.mVar->mMin, se.mVar->mMax, value );
#endif
                    result = false;
                    state = Result;
                    break;
                }

                solution->insert( *( se.mVar ), value );

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

/*!
    \internal
*/
int EquationSolver::numOfUnknownVars( const Equation *eq, const Solution *solution ) {

    int result = 0;


    for ( int i = 0; i < eq->mFormula.mExpression.size(); i++ ) {
        if ( solution->contains( *(eq->mFormula.mExpression.at(i).mVar) ) ) {
            continue;
        }
        result++;
    }

    return result;
}

/*!
    \internal
*/
bool EquationSolver::compareEquations( const Equation &equation1, const Equation &equation2 )
{
    QList<SimpleExpression> exp1 = equation1.mFormula.mExpression;
    QList<SimpleExpression> exp2 = equation2.mFormula.mExpression;
    if ( exp1.count() != exp2.count() ) {
        return false;
    }

    if ( exp1.count() == 0 ) {
        return true;
    }

    qreal global_coef = 1.0 * exp2.at(0).mCoef / exp1.at(0).mCoef;

    for ( int i = 0; i < exp1.count(); i++ ) {
        if ( ( !myFuzzyCompare( global_coef * exp1.at(i).mCoef, exp2.at(i).mCoef ) )
            || ( exp1.at(i).mVar != exp2.at(i).mVar ) ) {
            return false;
        }
    }
    return true;
}

/*!
    \internal
*/
bool EquationSolver::compareExpressions( const Expression &expression1, const Expression &expression2 )
{
    QList<SimpleExpression> exp1 = expression1.mExpression;
    QList<SimpleExpression> exp2 = expression2.mExpression;
    if ( exp1.count() != exp2.count() ) {
        return false;
    }

    for ( int i = 0; i < exp1.count(); i++ ) {
        if ( ( !myFuzzyCompare( exp1.at(i).mCoef, exp2.at(i).mCoef ) )
            || ( exp1.at(i).mVar != exp2.at(i).mVar ) ) {
            return false;
        }
    }

    return true;
}

/*!
    \internal
*/
Expression::Expression() : mExpression()
{
}

/*!
    \internal
*/
Expression::~Expression()
{
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
void Expression::plusExpression( const Expression exp )
{
    for ( int i = 0; i < exp.mExpression.size(); i++ ) {
        plusSimpleExpression( exp.mExpression.at(i) );
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
void Expression::minusExpression( const Expression exp )
{
    for ( int i = 0; i < exp.mExpression.size(); i++ ) {
        minusSimpleExpression( exp.mExpression.at(i) );
    }
}

/*!
    \internal
*/
void Expression::minus()
{
    for ( int i = 0; i < mExpression.size(); i++ ) {
        mExpression[i].mCoef = -mExpression.at(i).mCoef;
    }
}

/*!
    \internal
*/
qreal Expression::value( Solution *solution ) const
{
    qreal result = 0;

    for ( int i = 0; i < mExpression.size(); i++ ) {
        result += solution->value( *(mExpression.at(i).mVar) ) * mExpression.at(i).mCoef;
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
        const SimpleExpression &currentExpression = mExpression.at(i);
        if ( currentExpression.mVar->mFlags&Variable::FlagFixed ) {
            result += currentExpression.mVar->mPref * currentExpression.mCoef;
        } else {
            if ( currentExpression.mCoef > 0 ) {
                result += currentExpression.mVar->mMin * currentExpression.mCoef;
            } else {
                result += currentExpression.mVar->mMax * currentExpression.mCoef;
            }
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
         const SimpleExpression &currentExpression = mExpression.at(i);
         result += currentExpression.mVar->mPref * currentExpression.mCoef;
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
        const SimpleExpression &currentExpression = mExpression.at(i);
        if ( currentExpression.mVar->mFlags&Variable::FlagFixed ) {
            result += currentExpression.mVar->mPref * currentExpression.mCoef;
        } else {
            if ( currentExpression.mCoef < 0 ) {
                result += currentExpression.mVar->mMin * currentExpression.mCoef;
            } else {
                result += currentExpression.mVar->mMax * currentExpression.mCoef;
            }
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
        const SimpleExpression &currentExpression = mExpression.at(i);
        if ( currentExpression.mVar->mFlags&Variable::FlagFixed ) {
            result += currentExpression.mVar->mPref * currentExpression.mCoef;
        } else if ( solution->contains( *( currentExpression.mVar ) ) ) {
            result += solution->value( *( currentExpression.mVar ) ) * currentExpression.mCoef;
        } else {
            if ( currentExpression.mCoef > 0 ) {
                result += currentExpression.mVar->mMin * currentExpression.mCoef;
            } else {
                result += currentExpression.mVar->mMax * currentExpression.mCoef;
            }
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
        const SimpleExpression &currentExpression = mExpression.at(i);
        if ( solution->contains( *( currentExpression.mVar ) ) ) {
            result += solution->value( *( currentExpression.mVar ) ) * currentExpression.mCoef;
        } else {
            result += currentExpression.mVar->mPref * currentExpression.mCoef;
        }
    }

    return result;
}

/*!
    \internal
*/
qreal Expression::prefExpandedValue( Solution *solution ) const
{
    qreal result = 0;

    for ( int i = 0; i < mExpression.size(); i++ ) {
        const SimpleExpression &currentExpression = mExpression.at(i);
        if ( currentExpression.mVar->mFlags&Variable::FlagExpanding ) {
            // use the min value
            if ( currentExpression.mVar->mFlags&Variable::FlagFixed ) {
                result += currentExpression.mVar->mPref * currentExpression.mCoef;
            } else if ( solution->contains( *( currentExpression.mVar ) ) ) {
                result += solution->value( *( currentExpression.mVar ) ) * currentExpression.mCoef;
            } else {
                if ( currentExpression.mCoef > 0 ) {
                    result += currentExpression.mVar->mMin * currentExpression.mCoef;
                } else {
                    result += currentExpression.mVar->mMax * currentExpression.mCoef;
                }
            }
        }
        else if ( solution->contains( *( currentExpression.mVar ) ) ) {
            result += solution->value( *( currentExpression.mVar ) ) * currentExpression.mCoef;
        } else {
            result += currentExpression.mVar->mPref * currentExpression.mCoef;
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
        const SimpleExpression &currentExpression = mExpression.at(i);
        if ( currentExpression.mVar->mFlags&Variable::FlagFixed ) {
            result += currentExpression.mVar->mPref * currentExpression.mCoef;
        } else if ( solution->contains( *( currentExpression.mVar ) ) ) {
            result += solution->value( *( currentExpression.mVar ) ) * currentExpression.mCoef;
        } else {
            if ( currentExpression.mCoef < 0 ) {
                result += currentExpression.mVar->mMin * currentExpression.mCoef;
            } else {
                result += currentExpression.mVar->mMax * currentExpression.mCoef;
            }
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
        res += '(' + QString::number( mExpression.at(i).mCoef ) + ')' + "var[" + QString::number( mExpression.at(i).mVar->mId ) + ']';
    }
    return res;
}

/*!
    \internal
*/
VariableSet::VariableSet() : mVarList()
{
    mCurr_id = 0;
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
    var->mFlags &= ~Variable::FlagFixed;
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

/*!
    \internal
*/
uint qHash ( Variable key ) {
    return key.mId;
}
