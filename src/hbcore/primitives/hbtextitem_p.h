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

#ifndef HBTEXTITEM_P_H
#define HBTEXTITEM_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Hb API.  It exists purely as an
// implementation detail.  This file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QTextLayout>
#include "hbtextitem.h"
#include "hbwidgetbase_p.h"

class QFontMetricsF;
class QLinearGradient;

class HbTextItemPrivate : public HbWidgetBasePrivate
{
    Q_DECLARE_PUBLIC(HbTextItem)

public:

    HbTextItemPrivate();
    void init(QGraphicsItem *parent);
    void clear();

    bool doLayout(const QString& text, const qreal lineWidth, qreal leading);
    void setSize(const QSizeF &newSize);
    void updateTextOption();
    void calculateVerticalOffset();
    void updateLayoutDirection();

    int textFlagsFromTextOption() const;

    int findIndexOfLastLineBeforeY(qreal y) const;

    QString elideLayoutedText(const QSizeF& size, const QFontMetricsF& metrics) const;
    bool adjustSizeHint();

    bool fadeNeeded(const QRectF& contentRect) const;
    static inline void setupGradient(QLinearGradient *gradient, QColor color);

    void calculateFadeRects();

    static inline void setPainterPen(QPainter *painter,
                             const QPen& pen,
                             const QPointF& lineBegin);

    int paintFaded(QPainter *painter,
                    int firstItemToPaint,
                    const QPen& leftPen,
                    const QPen& centerPen,
                    const QPen& rightPen,
                    const QRectF& area ) const;

    void paintWithFadeEffect(QPainter *painter) const;

    void setFadeLengths(qreal xLength, qreal yLength);

    QRectF layoutBoundingRect() const;
    QRectF boundingRect(const QRectF& contentsRect) const;

    QString mText;
    Qt::Alignment mAlignment;
    Qt::TextElideMode mElideMode;
    bool mDontPrint;  // needed to fake text flags
    bool mDontClip;   // needed to fake text flags

    bool mInvalidateShownText;
    QRectF mOldContentsRect;
    QColor mColor;
    mutable QColor mDefaultColor; // color used when no color was set
    QTextLayout mTextLayout;

    QPointF mOffsetPos;

    bool mPaintFaded;
    qreal mFadeLengthX; // distance on which fade efect is performed when text doesn't fit content rectangle
    qreal mFadeLengthY; // distance on which fade efect is performed when text doesn't fit content rectangle
    qreal mCornerFadeX;
    qreal mCornerFadeY;
    QRectF mFadeToRect;
    QRectF mFadeFromRect;

    qreal mPrefHeight;
    int mMinLines;
    int mMaxLines;
    bool mNeedToAdjustSizeHint;
    QSizeF oldSize;

    mutable bool mUpdateColor;
    static bool outlinesEnabled;
};

#endif // HBTEXTITEM_P_H
