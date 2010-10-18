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

#include "hbexpressionparser_p.h"
#include "hbhash_p.h"

#include <QStack>

static bool parseExpressionOperand(
    const QString &str,
    HbExpressionParser::Token &type,
    int &value)
{
    QString valueString = str;
    if (valueString.startsWith("var(") && valueString.endsWith(")")) {
        // remove var( and last )
        type = HbExpressionParser::Variable;
        value = hbHash(valueString.midRef(4, valueString.size()-5));
    } else {
        if (valueString.endsWith(QLatin1String("un"), Qt::CaseInsensitive)) {
            valueString.chop(2);
            type = HbExpressionParser::LengthInUnits;
        } else if (valueString.endsWith(QLatin1String("px"), Qt::CaseInsensitive)) {
            valueString.chop(2);
            type = HbExpressionParser::LengthInPixels;
        } else if (valueString.endsWith(QLatin1String("mm"), Qt::CaseInsensitive)) {
            valueString.chop(2);
            type = HbExpressionParser::LengthInMillimeters;
        } else {
            // assume pixels
            type = HbExpressionParser::LengthInPixels;
        }
        bool ok(true);
        value = HbExpressionParser::toFixed(valueString.toFloat(&ok));
        if (!ok) {
            return false;
        }
    }
    return true;
}

static int precedence(HbExpressionParser::Token op)
{
    switch (op) {
        case HbExpressionParser::Addition:
        case HbExpressionParser::Subtraction:
            return 1;
        case HbExpressionParser::Multiplication:
        case HbExpressionParser::Division:
            return 2;
        case HbExpressionParser::Negation:
        case HbExpressionParser::Ceil:
        case HbExpressionParser::Floor:
        case HbExpressionParser::Round:
            return 3;
        default:
            return 0;
    }
}

static bool isLeft(HbExpressionParser::Token op)
{
    switch (op) {
        case HbExpressionParser::Addition:
        case HbExpressionParser::Subtraction:
        case HbExpressionParser::Multiplication:
        case HbExpressionParser::Division:
            return true;
        case HbExpressionParser::Negation:
        case HbExpressionParser::Ceil:
        case HbExpressionParser::Floor:
        case HbExpressionParser::Round:
        default:
            return false;
    }
}


bool HbExpressionParser::parse(const QString &str, QList<int> &tokens)
{
    tokens.clear();

    // Parse the infix notation to "tempTokens"
    QList<int> tempTokens;

    const QChar SPACE(' ');
    const QChar OPENPARENTHESIS('(');
    const QChar CLOSEPARENTHESIS(')');
    const QChar PLUS('+');
    const QChar MINUS('-');
    const QChar STAR('*');
    const QChar SLASH('/');

    const QString VAR("var(");
    const QString CEIL("ceil(");
    const QString FLOOR("floor(");
    const QString ROUND("round(");

    const int TokenSpace = -1;
    const int TokenUnknown = -2;

    int position = 0;

    if (str.startsWith("expr(") && str.endsWith(")")) {
        position = 4;
    } else if (str.startsWith("-expr(") && str.endsWith(")")) {
        position = 5;
        tempTokens.append(LengthInPixels);
        tempTokens.append(toFixed(-1.0));
        tempTokens.append(Multiplication);
    }

    int token = -1;
    int begin = -1;
    int nestingLevel = 0; 
    int operatorCount = 1;//there can only be 2 sequental operators if the latter one is negation
    while (position < str.size()) {
        int movePos = 1;
        if (token == Variable) {
            if (str.at(position) == CLOSEPARENTHESIS) {
                token = TokenUnknown;
            }
        } else if (str.at(position) == SPACE) {
            token = TokenSpace;
        } else if (str.at(position) == OPENPARENTHESIS) {
            token = OpenParenthesis;
        } else if (str.at(position) == CLOSEPARENTHESIS) {
            token = CloseParenthesis;
        } else if (str.at(position) == PLUS) {
            token = Addition;
        } else if (str.at(position) == MINUS) {
            token = Subtraction;
        } else if (str.at(position) == STAR) {
            token = Multiplication;
        } else if (str.at(position) == SLASH) {
            token = Division;
        } else if (str.mid(position, VAR.length()) == VAR) {
            token = Variable;
            movePos = VAR.length();
        } else if (str.mid(position, CEIL.length()) == CEIL) {
            token = Ceil;
            movePos = CEIL.length() - 1;
        } else if (str.mid(position, FLOOR.length()) == FLOOR) {
            token = Floor;
            movePos = FLOOR.length() - 1;
        } else if (str.mid(position, ROUND.length()) == ROUND) {
            token = Round;
            movePos = ROUND.length() - 1;
        } else {
            token = TokenUnknown;
        }

        if (begin >= 0 && !(token == TokenUnknown || token == Variable)) {
            // parse operand
            QString valueString = str.mid(begin, position - begin);
            if (operatorCount==0) {
                // an operand without operator
                return false;
            }
            HbExpressionParser::Token type;
            int val;
            if ( !parseExpressionOperand(valueString, type, val ) ) {
                return false;
            }
            tempTokens.append(type);
            tempTokens.append(val);
            operatorCount = 0;
            begin = -1;
        }
        switch (token) {
            case Addition:
            case Multiplication:
            case Division:
                if (operatorCount > 0) {
                    return false;
                }
                tempTokens.append(token);
                operatorCount++;
                break;
            case Subtraction:
                if (operatorCount == 1) {
                    tempTokens.append(Negation);
                } else if (operatorCount > 1) {
                    return false;
                } else {
                    tempTokens.append(Subtraction);
                }
                operatorCount++;
                break;
            case Ceil:
            case Floor:
            case Round:
                if (operatorCount < 1 || operatorCount > 2) {
                    return false;
                }
                tempTokens.append(token);
                operatorCount++;
                break;
            case OpenParenthesis:
                tempTokens.append(OpenParenthesis);
                operatorCount = 1;
                nestingLevel++;
                break;
            case CloseParenthesis:
                if (operatorCount != 0) {
                    // an operator without operand
                    return false;
                }
                tempTokens.append(CloseParenthesis);
                nestingLevel--;
                break;
            case Variable:
            case TokenUnknown:
                if (begin == -1) {
                    begin = position;
                }
                break;
            case TokenSpace:
            default:
                break;
            }
        position += movePos;
    }

    // check for unmatching parentheses
    if (nestingLevel != 0) {
        return false;
    }

    // parse last value
    if (begin >= 0) {
        // parse operand
        if (operatorCount==0) {
            // an operand without operator
            return false;
        }
        QString valueString = str.mid(begin, position - begin);
        HbExpressionParser::Token type;
        int val;
        if ( !parseExpressionOperand(valueString, type, val ) ) {
            return false;
        }
        tempTokens.append(type);
        tempTokens.append(val);
    } else {
        if (operatorCount!=0) {
            // an operator without operand
            return false;
        }
    }

    if (tempTokens.count()<2) {
        return false;
    }

    // Convert the infix notation to RPN with "shunting-yard" algorithm

    QStack<Token> operatorStack;
    for (int i=0; i<tempTokens.count(); i++) {
        Token t = (Token)tempTokens.at(i);
        switch (t) {
            case Variable:
            case LengthInPixels:
            case LengthInUnits:
            case LengthInMillimeters:
                tokens.append(tempTokens.at(i));
                i++;
                tokens.append(tempTokens.at(i));
                break;
            case OpenParenthesis:
                operatorStack.push(t);
                break;
            case CloseParenthesis:
                while (operatorStack.count()) {
                    Token op = operatorStack.pop();
                    if (op == OpenParenthesis) {
                        break;
                    } else {
                        tokens.append(op);
                    }
                }
                break;
            case Addition:
            case Subtraction:
            case Multiplication:
            case Division:
            case Negation:
            case Ceil:
            case Floor:
            case Round:
                while (operatorStack.count()) {
                    Token op = operatorStack.top();
                    if ((isLeft(t) && (precedence(t) <= precedence(op))) ||
                        (!isLeft(t) && (precedence(t) < precedence(op)))) {
                        tokens.append(op);
                        operatorStack.pop();
                    } else {
                        break;
                    }
                }
                operatorStack.push(t);
                break;
            default:
                break;
        }
    }
    while (operatorStack.count()) {
        tokens.append(operatorStack.pop());
    }

    return true;
}

int HbExpressionParser::toFixed(float f)
{
    Q_ASSERT( sizeof(quint32) == sizeof(float) );
    quint32 u;
    memcpy(&u, &f, sizeof(quint32));
    return u;
}

int HbExpressionParser::toFixed(double d)
{
    return toFixed((float)d);
}

qreal HbExpressionParser::fromFixed(int i)
{
    float f;
    memcpy(&f, &i, sizeof(float));
    return f;
}
