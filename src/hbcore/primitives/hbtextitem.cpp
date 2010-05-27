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

#include "hbtextitem.h"
#include "hbtextitem_p.h"
#include "hbstyle.h"
#include "hbtextutils_p.h"
#include "hbcolorscheme.h"
#include "hbevent.h"

#include <QTextLayout>
#include <QGraphicsSceneResizeEvent>
#include <QPainter>
#include <QTextOption>
#include <QApplication>

#ifdef HB_TEXT_MEASUREMENT_UTILITY
#include "hbtextmeasurementutility_p.h"
#include "hbfeaturemanager_p.h"
#endif

#define EPSILON 0.01

bool HbTextItemPrivate::outlinesEnabled = false;

static const QString KDefaultColorThemeName = "qtc_view_normal";
const int MinimumWidth = 5; // minimum width if there is some text.
const int KLayoutCacheLimit = 64;
const qreal KFadeTolerance = 1.0;

HbTextItemPrivate::HbTextItemPrivate () :
    mAlignment(Qt::AlignLeft | Qt::AlignVCenter),
    mElideMode(Qt::ElideNone),
    mInvalidateShownText(true),
    mOffsetPos(0,0),
    mPaintFaded(false),
    mFadeLengthX(30),
    mFadeLengthY(15),
    mPrefHeight(0),
    mMinLines(0),
    mMaxLines(0),
    mNeedToAdjustSizeHint(false),
    mUpdateColor(true)
{
}

void HbTextItemPrivate::init(QGraphicsItem *)
{
    Q_Q(HbTextItem);

    q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    q->setFlag(QGraphicsItem::ItemClipsToShape, false);
    q->setFlag(QGraphicsItem::ItemIsSelectable, false);
    q->setFlag(QGraphicsItem::ItemIsFocusable,  false);

    QTextOption textOption = mTextLayout.textOption();
    textOption.setWrapMode(QTextOption::WordWrap);
    mTextLayout.setTextOption(textOption);
    mTextLayout.setCacheEnabled(true);
}

void HbTextItemPrivate::clear()
{
    // no implementation needed
}

bool HbTextItemPrivate::doLayout(const QString& text, const qreal lineWidth, qreal leading)
{
    bool textTruncated = false;
    mInvalidateShownText = false;

    mTextLayout.setText(text);
    mTextLayout.setFont( q_func()->font() );

    qreal height = 0;
    mTextLayout.beginLayout();
    while (1) {
        QTextLine line = mTextLayout.createLine();
        if (!line.isValid())
            break;
        if( ( mMaxLines > 0 ) && ( mTextLayout.lineCount() > mMaxLines ) ) {
            textTruncated = true;
            break;
        }

        line.setLineWidth(lineWidth);
        height += leading;
        line.setPosition(QPointF(0, height));
        height += line.height();
    }
    mTextLayout.endLayout();

    if( textTruncated ) {
        mTextLayout.setText(text);
        mTextLayout.setFont( q_func()->font() );

        qreal height = 0;
        mTextLayout.beginLayout();
        while ( mTextLayout.lineCount() < mMaxLines ) {
            QTextLine line = mTextLayout.createLine();
            line.setLineWidth(lineWidth);
            height += leading;
            line.setPosition(QPointF(0, height));
            height += line.height();
        }
        mTextLayout.endLayout();
    }

    return textTruncated;
}

void HbTextItemPrivate::setSize(const QSizeF &newSize)
{
    Q_Q(HbTextItem);

    QFont usedFont = q->font();
    QFontMetricsF fontMetrics(usedFont);

    const qreal lineWidth = qRound( newSize.width() + 0.5 ); // round up to integer

    updateTextOption();

    QString tempText(mText);
    if(tempText.indexOf('\n')>=0) {
        // to prevent creation of deep copy if replace has no effect
        tempText.replace('\n', QChar::LineSeparator);
    }

    // function does the layout only when needed
    mTextLayout.setFont(usedFont);
	// Need to call elidedText explicitly to enable multiple length translations.
    tempText = fontMetrics.elidedText(tempText, Qt::ElideNone, lineWidth);
    bool textTruncated = doLayout(tempText, lineWidth, fontMetrics.leading());
    if(mElideMode!=Qt::ElideNone && !tempText.isEmpty()) {
        if( ( mTextLayout.boundingRect().height() - newSize.height() > EPSILON ) ||
            ( mTextLayout.boundingRect().width() - lineWidth > EPSILON ) ||
              textTruncated) {
            // TODO: Multiple length translations with multiline text
            doLayout(elideLayoutedText(newSize, fontMetrics),
                     lineWidth,
                     fontMetrics.leading());
        }
    }
    calculateVerticalOffset();
    calculateFadeRects();
    q->update();
}

/*
    finds index of last line before given Y coordinate
    It is a binary search.
 */
int HbTextItemPrivate::findIndexOfLastLineBeforeY(qreal y) const
{
    int i=0,j=mTextLayout.lineCount();

    if( ( mMaxLines > 0 ) && ( mMaxLines < j ) ){
        j = mMaxLines;
    }

    while(i!=j) {
        int k = (i+j)>>1;
        if(mTextLayout.lineAt(k).naturalTextRect().bottom()>y) {
            j=k;
        } else {
            if(i==k) {
                break;
            }
            i=k;
        }
    }
    return i;
}

QString HbTextItemPrivate::elideLayoutedText(const QSizeF& size, const QFontMetricsF& metrics) const
{
    int lastVisibleLine =findIndexOfLastLineBeforeY(size.height());
    QTextLine lastLine = mTextLayout.lineAt(lastVisibleLine);

    // all visible lines without last visible line
    QString textToElide = mTextLayout.text();
    QString elidedText = textToElide.left(lastLine.textStart());

    if(!elidedText.isEmpty() && !elidedText.endsWith(QChar::LineSeparator)) {
        // needed to prevent to move "..." to line before last line
        elidedText.append(QChar::LineSeparator);
    }

    int n = lastLine.textLength();
    if(textToElide.at(lastLine.textStart()+n-1)!=QChar::LineSeparator) {
        n = -1;
    }
    elidedText.append(metrics.elidedText(textToElide.mid(lastLine.textStart(), n),
                                         mElideMode, size.width(),textFlagsFromTextOption()));

    return elidedText;
}

void HbTextItemPrivate::updateTextOption()
{
    Q_Q(HbTextItem);

    QTextOption textOpt = mTextLayout.textOption();
    textOpt.setAlignment(QStyle::visualAlignment(q->layoutDirection(), q->alignment()));
    textOpt.setTextDirection(q->layoutDirection());
    mTextLayout.setTextOption(textOpt);
}

void HbTextItemPrivate::calculateVerticalOffset()
{
    Q_Q(HbTextItem);

    mOffsetPos.setY(0);
    Qt::Alignment align = q->alignment();
    if(!align.testFlag(Qt::AlignTop) && (align & Qt::AlignVertical_Mask)!=0 ) {
        int index = mTextLayout.lineCount()-1;
        if(index>=0) {
            qreal diff = q->size().height();
            diff -= mTextLayout.lineAt(index).rect().bottom();
            if(align & Qt::AlignVCenter) {
                diff *=(qreal)0.5;
            }
            if(diff>=0 || mElideMode==Qt::ElideNone) {
                mOffsetPos.setY(diff);
            }
        }
    }
}

int HbTextItemPrivate::textFlagsFromTextOption() const
{
    QTextOption textOption = mTextLayout.textOption();
    int flags = (int)mAlignment;

    switch(textOption.wrapMode()) {
    case QTextOption::NoWrap:
        break;
    case QTextOption::WordWrap:
        flags |= Qt::TextWordWrap;
        break;
    case QTextOption::ManualWrap:
        break;
    case QTextOption::WrapAnywhere:
        flags |= Qt::TextWrapAnywhere;
        break;
    case QTextOption::WrapAtWordBoundaryOrAnywhere:
        flags |= Qt::TextWordWrap | Qt::TextWrapAnywhere;
        break;
    }

    if(q_ptr->flags().testFlag(QGraphicsItem::ItemClipsToShape)) {
        flags |= Qt::TextDontClip;
    }
    if(!q_ptr->isVisible()) { // or ItemHasNoContents?
        flags |= Qt::TextDontPrint;
    }

    return flags;
}

bool HbTextItemPrivate::adjustSizeHint()
{
    Q_Q( HbTextItem );

    mNeedToAdjustSizeHint = false;

    if ( !(q->sizePolicy().verticalPolicy()&QSizePolicy::IgnoreFlag) ) {
        // only calculated if the vertical sizeHint is taken into account

        const QFontMetricsF metrics(q->font());

        if ( mMinLines > 0 && (mMinLines == mMaxLines) ) {
            // if the number of lines if fixed: optimize
            const qreal newPrefHeight = ( metrics.height() + metrics.leading() ) * mMinLines - metrics.leading();
            if( qAbs( mPrefHeight - newPrefHeight ) > EPSILON ) {
                mPrefHeight = newPrefHeight;
                return true;
            }
            return false;
        }

        QSizeF currSize = q->size();
        // do the heavy calculation
        QRectF desiredRect = metrics.boundingRect( QRectF( 0, 0 , currSize.width(), QWIDGETSIZE_MAX ), textFlagsFromTextOption(), mText );

        if( qAbs( desiredRect.height() - mPrefHeight ) > EPSILON ) {
            mPrefHeight = desiredRect.height();
            return true;
        }
    }

    return false;
}

bool HbTextItemPrivate::fadeNeeded(const QRectF& contentRect) const
{
    return (mFadeLengthX!=0 || mFadeLengthY!=0)
            && !contentRect.contains(
                    layoutBoundingRect().adjusted(KFadeTolerance,
                                                  KFadeTolerance,
                                                  -KFadeTolerance,
                                                  -KFadeTolerance));
}

void HbTextItemPrivate::setupGradient(QLinearGradient *gradient, QColor color)
{
    gradient->setColorAt(1.0, color);
    color.setAlpha(color.alpha()>>2); // 1/4 of initial opacity
    gradient->setColorAt(0.5, color); // middle color to improve feeling of fade effect
    color.setAlpha(0); // fully transparent
    gradient->setColorAt(0.0, color);
}

void HbTextItemPrivate::calculateFadeRects()
{
    Q_Q(const HbTextItem);

    const QRectF contentRect = q->contentsRect();
    mFadeToRect = contentRect;
    mFadeFromRect = contentRect;

    if(mFadeLengthX>0) {
        mFadeFromRect.moveLeft(mFadeLengthX);
        mFadeFromRect.setRight(contentRect.right()-mFadeLengthX);

        if(mFadeFromRect.width()<0) {
            mFadeFromRect.moveLeft(mFadeFromRect.center().x());
            mFadeFromRect.setWidth(0.0);
        }
    } else {
        mFadeToRect.moveLeft(mFadeLengthX);
        mFadeToRect.setRight(contentRect.right()-mFadeLengthX);
    }

    if(mFadeLengthY>0) { // TODO: alternative direction
        mFadeFromRect.moveTop(mFadeLengthY);
        mFadeFromRect.setBottom(contentRect.bottom()-mFadeLengthY);

        if(mFadeFromRect.height()<0) {
            mFadeFromRect.moveTop(mFadeFromRect.center().y());
            mFadeFromRect.setHeight(0.0);
        }
    } else {
        mFadeToRect.moveTop(mFadeLengthY);
        mFadeToRect.setBottom(contentRect.bottom()-mFadeLengthY);
    }

    qreal dx,dy;
    dx = mFadeFromRect.left() - mFadeToRect.left();
    dy = mFadeFromRect.top()  - mFadeToRect.top();
    if(dx!=0 || dy!=0) {
        // corner gradient vectors
        qreal scale = dx*dy/(dx*dx+dy*dy);
        mCornerFadeX = dy*scale;
        mCornerFadeY = dx*scale;
    } else {
        mCornerFadeX = 1;
        mCornerFadeY = 0;
    }

    mPaintFaded = fadeNeeded(contentRect);
}

/*
    This work-around is needed since there is a problem with pen transformations
    in hardware Open VG renderer. This problem occurs only on s60 hardware.
    On platforms: Linux, Windows and S60 emulator there is no such problem.
    Below flag detects platform which have this problem to activate work-around.
 */
#if defined(Q_WS_S60) && defined(Q_BIG_ENDIAN)
#   warning Work-around is active in fade effect of HbTextItem (see comment)
#   define HB_FADE_EFFECT_WORKAROUND_ON_PHONE
#endif
inline void HbTextItemPrivate::setPainterPen(QPainter *painter,
                         const QPen& pen,
                         const QPointF& lineBegin)
{
#ifdef HB_FADE_EFFECT_WORKAROUND_ON_PHONE
    const QGradient *gradient = pen.brush().gradient();
    if (!gradient || gradient->type()!=QGradient::LinearGradient) {
        painter->setPen(pen);
        return;
    }
    const QLinearGradient* linGrad = static_cast<const QLinearGradient*>(gradient);
    QLinearGradient newGrad(*linGrad);
    newGrad.setStart(newGrad.start()-lineBegin);
    newGrad.setFinalStop(newGrad.finalStop()-lineBegin);

    QBrush newBrush(newGrad);
    QPen newPen;
    newPen.setBrush(newBrush);
    painter->setPen(newPen);
#else
    Q_UNUSED(painter)
    Q_UNUSED(pen)
    Q_UNUSED(lineBegin)
#endif // HB_FADE_EFFECT_WORKAROUND_ON_PHONE
}

/*
    This method paints single piece of text layout.
    If line contains criticalX value then fadePen is used for painting in other
    case normalPen is used.
 */
void HbTextItemPrivate::paintArea(QPainter *painter,
                                  int firstItemToPaint,
                                  int lastItemToPaint,
                                  const QPen& normalPen,
                                  const QPen& fadePen,
                                  qreal criticalX) const
{
    for(int i=firstItemToPaint; i<=lastItemToPaint; ++i) {
        QTextLine line = mTextLayout.lineAt(i);
        QRectF lineRect = line.naturalTextRect();
        lineRect.translate(mOffsetPos);

#ifdef HB_FADE_EFFECT_WORKAROUND_ON_PHONE
        const QPointF gradientOffset(lineRect.left(),
                                     lineRect.top()+line.ascent());

        if (lineRect.left()<criticalX && lineRect.right()>criticalX) {
            setPainterPen(painter, fadePen, gradientOffset);
        } else {
            setPainterPen(painter, normalPen, gradientOffset);
        }
#else
        if (lineRect.left()<criticalX && lineRect.right()>criticalX) {
            painter->setPen(fadePen);
        } else {
            painter->setPen(normalPen);
        }
#endif // HB_FADE_EFFECT_WORKAROUND_ON_PHONE

        line.draw(painter, mOffsetPos);
    } // for i
}

/*
    This method is used to draw center part of lines.
    It is also used to calculate range of lines needed for painting in this range.
 */
int HbTextItemPrivate::paintArea(QPainter *painter,
              int firstItemToPaint,
              const QPen& normalPen,
              qreal lastValidY) const
{
    int i;
    const int n = mTextLayout.lineCount();

#ifndef HB_FADE_EFFECT_WORKAROUND_ON_PHONE
    painter->setPen(normalPen);
#endif

    for(i=firstItemToPaint; i<n; ++i) {
        QTextLine line = mTextLayout.lineAt(i);
        QRectF lineRect = line.naturalTextRect();
        lineRect.translate(mOffsetPos);

        if (lineRect.top()>lastValidY) {
            return i;
        }
#ifdef HB_FADE_EFFECT_WORKAROUND_ON_PHONE
        const QPointF gradientOffset(lineRect.left(),
                                     lineRect.top()+line.ascent());

        setPainterPen(painter, normalPen, gradientOffset);
#endif // HB_FADE_EFFECT_WORKAROUND_ON_PHONE

        line.draw(painter, mOffsetPos);

        if (lineRect.bottom()>lastValidY) {
            return i;
        }
    } // for i
    return n-1;
}

bool HbTextItemPrivate::setClipPath(QPainter *painter,
                                    const QRectF& rect,
                                    const QPainterPath& initialCliping) const
{
    if (initialCliping.isEmpty()) {
        painter->setClipRect(rect);
    } else {
        QPainterPath newPath(rect.topLeft());
        newPath.addRect(rect);

        if (!initialCliping.intersects(newPath)) {
            return false; // dont paint
        }
        newPath = initialCliping.intersected(newPath);

        painter->setClipPath(newPath);
    }
    return true;
}

void HbTextItemPrivate::paintWithFadeEffect(QPainter *painter) const
{
    Q_Q(const HbTextItem);

    QLinearGradient gradient;
    setupGradient(&gradient, q->textColor());
    const QPainterPath initialClipPath = painter->clipPath();

    const QRectF contentRect = q->contentsRect();
    int i=0;

    const int n = mTextLayout.lineCount();

// #define SEE_FADE_RECTANGLES
#ifdef SEE_FADE_RECTANGLES
    painter->setClipRect(mFadeToRect);
    painter->setBrush(QBrush(QColor(215, 0, 0, 30)));
    painter->drawRect(mFadeToRect);
    painter->setBrush(QBrush(QColor(0, 0, 200, 30)));
    painter->drawRect(mFadeFromRect.adjusted(0,0,-1,-1));
#endif // SEE_FADE_RECTANGLES

    QRectF centerRect(mFadeToRect);
    if(mTextLayout.lineAt(0).y()+mOffsetPos.y()<contentRect.top()) {
        centerRect.setTop(mFadeFromRect.top());

        // top center gradient (==):
        QPointF from(mFadeFromRect.topLeft());
        gradient.setStart(mFadeFromRect.left(),mFadeToRect.top());
        gradient.setFinalStop(from);
        QBrush centerBrush(gradient);
        QPen centerPen;
        centerPen.setBrush(centerBrush);

        if (setClipPath(painter,
                        QRectF(gradient.start(), mFadeFromRect.topRight()),
                        initialClipPath)) {
            i = paintArea(painter, 0, centerPen, from.y());
        } else {
            i = 0;
        }

        // top left gradient (//):
        gradient.setStart(from.x()-mCornerFadeX, from.y()-mCornerFadeY);
        QBrush leftBrush(gradient);
        QPen leftPen;
        leftPen.setBrush(leftBrush);
        if (setClipPath(painter,
                        QRectF(mFadeToRect.topLeft(), mFadeFromRect.topLeft()),
                        initialClipPath)) {
            paintArea(painter, 0, i, centerPen, leftPen, contentRect.left()-KFadeTolerance);
        }

        // top right gradient (\\):
        from = mFadeFromRect.topRight();
        gradient.setStart(from.x()+mCornerFadeX, from.y()-mCornerFadeY);
        gradient.setFinalStop(from);
        QBrush rightBrush(gradient);
        QPen rightPen;
        rightPen.setBrush(rightBrush);

        if (setClipPath(painter,
                        QRectF(mFadeToRect.topRight(), mFadeFromRect.topRight()),
                        initialClipPath)) {
            paintArea(painter, 0, i, centerPen, rightPen, contentRect.right()+KFadeTolerance);
        }
    }

    bool paintBottom = false;
    if(mTextLayout.lineAt(n-1).naturalTextRect().bottom()+mOffsetPos.y()>contentRect.bottom()) {
        // bottom fade is needed here
        centerRect.setBottom(mFadeFromRect.bottom());
        paintBottom = true;
    }

    // paint center part
    {
        int startFrom = i;
        QPen centerPen(q->textColor());
        if (setClipPath(painter,
                        QRectF(QPointF(mFadeFromRect.left(), centerRect.top()),
                               QPointF(mFadeFromRect.right(), centerRect.bottom())),
                        initialClipPath)) {
            // center with no gradient:
            i = paintArea(painter, i, centerPen, centerRect.bottom());
        }

        // left gradient | ||
        gradient.setStart(mFadeToRect.left(), mFadeFromRect.top());
        gradient.setFinalStop(mFadeFromRect.topLeft());
        QBrush leftBrush(gradient);
        QPen leftPen;
        leftPen.setBrush(leftBrush);
        if (setClipPath(painter,
                        QRectF(centerRect.topLeft(),
                               QPointF(mFadeFromRect.left(),
                                       centerRect.bottom())),
                        initialClipPath)) {
            paintArea(painter, startFrom, i, centerPen, leftPen, contentRect.left()-KFadeTolerance);
        }

        // right gradient || |
        gradient.setStart(mFadeToRect.right(), mFadeFromRect.top());
        gradient.setFinalStop(mFadeFromRect.topRight());
        QBrush rightBrush(gradient);
        QPen rightPen;
        rightPen.setBrush(rightBrush);

        if (setClipPath(painter,
                        QRectF(QPointF(mFadeFromRect.right(), centerRect.top()),
                               centerRect.bottomRight()),
                        initialClipPath)) {
            paintArea(painter, startFrom, i, centerPen, rightPen, contentRect.right()+KFadeTolerance);
        }
    }

    // need to draw bottom as faded? is some lines remained?
    if (paintBottom) {
        int startFrom = i;

        // bottom center gradient (==):
        QPointF from(mFadeFromRect.bottomLeft());
        gradient.setFinalStop(from);
        gradient.setStart(mFadeFromRect.left(),mFadeToRect.bottom());
        QBrush centerBrush(gradient);
        QPen centerPen;
        centerPen.setBrush(centerBrush);

        if (setClipPath(painter,
                        QRectF(mFadeFromRect.bottomLeft(),
                               QPointF(mFadeFromRect.right(), mFadeToRect.bottom())),
                        initialClipPath)) {
            // center with no gradient:
            i = paintArea(painter, i, centerPen, mFadeToRect.bottom());
        }

        // bottom left gradient (\\):
        gradient.setStart(from.x()-mCornerFadeX, from.y()+mCornerFadeY);
        QBrush leftBrush(gradient);
        QPen leftPen;
        leftPen.setBrush(leftBrush);

        if (setClipPath(painter,
                        QRectF(mFadeFromRect.bottomLeft(),
                               mFadeToRect.bottomLeft()),
                        initialClipPath)) {
            // center with no gradient:
            paintArea(painter, startFrom, i, centerPen, leftPen, contentRect.left()-KFadeTolerance);
        }

        // bottom right gradient (//):
        from = mFadeFromRect.bottomRight();
        gradient.setStart(from.x()+mCornerFadeX, from.y()+mCornerFadeY);
        gradient.setFinalStop(from);
        QBrush rightBrush(gradient);
        QPen rightPen;
        rightPen.setBrush(rightBrush);

        if (setClipPath(painter,
                        QRectF(mFadeFromRect.bottomRight(),
                               mFadeToRect.bottomRight()),
                        initialClipPath)) {
            // center with no gradient:
            paintArea(painter, startFrom, i, centerPen, rightPen, contentRect.right()+KFadeTolerance);
        }
    }

    // restoring initial clipping region
    painter->setClipPath(initialClipPath);
}

void HbTextItemPrivate::setFadeLengths(qreal xLength, qreal yLength)
{
    static const qreal KMinDiff = 0.5;
    Q_Q( HbTextItem );

    if(qAbs(mFadeLengthX - xLength)>KMinDiff
       || qAbs(mFadeLengthY - yLength)>KMinDiff) {
        if(mFadeLengthX<0 || xLength<0
           || mFadeLengthY<0 || yLength<0) {
            // in this cases boundingRect will be changed
            q->prepareGeometryChange();
        }
        mFadeLengthX = (qAbs(xLength)<=KMinDiff)? 0.0: xLength;
        mFadeLengthY = (qAbs(yLength)<=KMinDiff)? 0.0: yLength;

        calculateFadeRects();

        q->update();
    }
}

QRectF HbTextItemPrivate::layoutBoundingRect () const
{
    QRectF result;
    for (int i=0, n=mTextLayout.lineCount(); i<n; ++i) {
        result = result.unite(
                mTextLayout.lineAt(i).naturalTextRect());
    }

    result.translate(mOffsetPos);

    return result;
}

QRectF HbTextItemPrivate::boundingRect (const QRectF& contentsRect) const
{
    QRectF result(layoutBoundingRect());
    if(q_ptr->flags().testFlag(QGraphicsItem::ItemClipsToShape)) {
        // clip
        QRectF clippedTo = contentsRect;

        qreal dx = qMin(mFadeLengthX, (qreal)0.0);
        qreal dy = qMin(mFadeLengthY, (qreal)0.0);
        clippedTo.adjust(dx, dy, -dx, -dy);

        result = result.intersected(clippedTo);
    }

    if (HbTextItemPrivate::outlinesEnabled) {
        result = result.united(contentsRect);
    }

    return result;
}

/*!
    @alpha
    @hbcore
    \class HbTextItem
    \brief HbTextItem is a lightweight item for showing text.


    This is mainly used as a primitive in widgets.
    It derives from HbWidgetBase so it can be layouted.
 */

/*!
    Constructor for the class with no content.
 */

HbTextItem::HbTextItem (QGraphicsItem *parent) :
    HbWidgetBase(*new HbTextItemPrivate, parent)
{
    Q_D(HbTextItem);
    d->init(parent);
}


/*!
    Constructs object with a \a text content.
 */
HbTextItem::HbTextItem (const QString &text, QGraphicsItem *parent) :
    HbWidgetBase(*new HbTextItemPrivate, parent)
{
    Q_D(HbTextItem);
    d->init(parent);
    setText(text);
}

/*
    Constructor for internal use only
 */
HbTextItem::HbTextItem (HbTextItemPrivate &dd, QGraphicsItem * parent) :
    HbWidgetBase(dd, parent)
{
    Q_D(HbTextItem);
    d->init(parent);
}

/*!
    Destructor for the class.
 */
HbTextItem::~HbTextItem ()
{
}

/*!
    Returns the text shown by object.

    \sa HbTextItem::setText()
 */
QString HbTextItem::text () const
{
    Q_D( const HbTextItem );
    return d->mText;
}

/*!
    Returns the text color used for painting text.
    If no color was set it returns color based on theme.

    \sa HbTextItem::setTextColor()
 */
QColor HbTextItem::textColor () const
{
    Q_D( const HbTextItem );

    if (d->mColor.isValid()) { // Means user has set text color
        return d->mColor;
    }
    if (!d->mDefaultColor.isValid()) {
        d->mDefaultColor = HbColorScheme::color(KDefaultColorThemeName);
    }
    return d->mDefaultColor;
}


/*!
    Returns the text alignment. It supports vertical and horizontal alignment.

    \sa HbTextItem::setAlignment()
 */
Qt::Alignment HbTextItem::alignment () const
{
    Q_D( const HbTextItem );
    return d->mAlignment;
}

/*!
    Returns the elide mode of the text.
    This option decide how last line of text is truncated.

    \sa HbTextItem::setElideMode()
 */
Qt::TextElideMode HbTextItem::elideMode () const
{
    Q_D( const HbTextItem );
    return d->mElideMode;
}

/*!
    Sets the text into \a text.
 */
void HbTextItem::setText (const QString &text)
{
    Q_D(HbTextItem);

    QString txt( text );

#ifdef HB_TEXT_MEASUREMENT_UTILITY

    if ( HbFeatureManager::instance()->featureStatus( HbFeatureManager::TextMeasurement ) ) {
        if (text.endsWith(QChar(LOC_TEST_END))) {
            int index = text.indexOf(QChar(LOC_TEST_START));
            setProperty( HbTextMeasurementUtilityNameSpace::textIdPropertyName,  text.mid(index + 1, text.indexOf(QChar(LOC_TEST_END)) - index - 1) );
            setProperty( HbTextMeasurementUtilityNameSpace::textMaxLines, d->mMaxLines );
            txt = text.left(index);
        } else {
            setProperty( HbTextMeasurementUtilityNameSpace::textIdPropertyName,  QVariant::Invalid );
        }
    }
#endif //HB_TEXT_MEASUREMENT_UTILITY

    if (d->mText != txt) {
        d->mInvalidateShownText = true;
        prepareGeometryChange();
        d->mText = txt;
        d->mTextLayout.setCacheEnabled(KLayoutCacheLimit >= d->mText.length());
        bool onlyHorizontalSizeHintChanged = false;
        if ( d->mMinLines > 0 && (d->mMinLines == d->mMaxLines) ) {
            onlyHorizontalSizeHintChanged = true;
        }
        if ( (sizePolicy().horizontalPolicy()&QSizePolicy::IgnoreFlag) && onlyHorizontalSizeHintChanged ) {
            // suppress updateGeometry() and use the same geometry
            d->setSize( size() );
        } else {
            updateGeometry();
        }
        update();
    }
}

/*!
    Sets the text color into \a color.
    If invalid color is used color from theme will be used.

    \sa HbTextItem::textColor()
 */
void HbTextItem::setTextColor (const QColor &color)
{
    Q_D(HbTextItem);

    d->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor, color.isValid());
    if (d->mColor != color) {
        d->mColor = color;

        if (!color.isValid()) {
            QGraphicsWidget* cssHandler = parentWidget();
            // check if there is a widget which handles CSS
            if (cssHandler!=NULL) {
                // this is needed to enforce color fetch from CSS
                HbEvent themeEvent(HbEvent::ThemeChanged);
                QApplication::sendEvent(cssHandler, &themeEvent);
            }
        }

        if (!d->mText.isEmpty()) {
            update();
        }
    }
}

/*!
    Sets the text alignment into \a alignment.
    It supports vertical and horizontal alignment.

    \sa HbTextItem::alignment()
 */
void HbTextItem::setAlignment (Qt::Alignment alignment)
{
    Q_D(HbTextItem);
	d->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextAlign, true);
    alignment &= Qt::AlignVertical_Mask | Qt::AlignHorizontal_Mask;
    if (d->mAlignment != alignment) {
        prepareGeometryChange();
        d->mAlignment = alignment;
        d->updateTextOption();
        d->calculateVerticalOffset();

        update();
    }
}

/*!
    Sets the elide mode into \a elideMode.
    The elide mode determines the truncation of the last line of text
    i.e. the "..." usage

    \sa HbTextItem::elideMode()
 */
void HbTextItem::setElideMode (Qt::TextElideMode elideMode)
{
    Q_D(HbTextItem);
    if (elideMode != d->mElideMode) {
        d->mInvalidateShownText = true;
        d->mElideMode = elideMode;
        update();
    }
}

/*!
    \reimp

    Paints text
 */
void HbTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_D(HbTextItem);
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // optimalization:
    if (option->exposedRect.isEmpty()) {
        return;
    }

    // Save painter's state
    QPen oldPen = painter->pen();


    if (HbTextItemPrivate::outlinesEnabled){
        painter->setBrush(QBrush(QColor(255, 0, 0, 50)));
        QRectF rect(contentsRect());
        // to see border - bounding rect was clipping bottom and right border
        rect.adjust(0, 0, -1.0, -1.0);
        painter->drawRect(rect);
    }


    painter->setPen(textColor());

    Q_ASSERT(d->mPaintFaded == d->fadeNeeded(contentsRect()));
    if(d->mPaintFaded ) {
        d->paintWithFadeEffect(painter);
    } else {
        d->mTextLayout.draw(painter,
                            d->mOffsetPos,
                            QVector<QTextLayout::FormatRange>(),
                            flags().testFlag(ItemClipsToShape)?contentsRect():QRectF());
    }


    // Restore painter's state
    painter->setPen(oldPen);
}

/*!
    \reimp

    Sets geometry of text
 */
void HbTextItem::setGeometry(const QRectF & rect)
{
    Q_D(HbTextItem);

    HbWidgetBase::setGeometry(rect);

    // needed when there was no size change and some things
    // need to relayout text
    if(d->mInvalidateShownText) {
        prepareGeometryChange();
        d->setSize(rect.size());
    }
}

/*!
    \reimp

    bounding rectangle.
 */
QRectF HbTextItem::boundingRect () const
{
    Q_D(const HbTextItem);

    return d->boundingRect(contentsRect());
} // boundingRect()

/*!
    \reimp
 */
QSizeF HbTextItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_D(const HbTextItem);

    QSizeF size(0,0);

    Qt::Orientations effectiveOrientations(0);
    if ( !(sizePolicy().horizontalPolicy()&QSizePolicy::IgnoreFlag) ) {
        effectiveOrientations |= Qt::Horizontal;
    }

    if ( !(sizePolicy().verticalPolicy()&QSizePolicy::IgnoreFlag) ) {
        effectiveOrientations |= Qt::Vertical;
    }   

    if ( !effectiveOrientations ) {
        // if the whole sizeHint is ignored, return ASAP with default values (0<50<QMAX)
        return HbWidgetBase::sizeHint( which, constraint );
    }

    const QFontMetricsF metrics(font());
    QSizeF maxSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    if(constraint.width()>0) {
        maxSize.setWidth(constraint.width());
    }
    if(constraint.height()>0) {
        maxSize.setHeight(constraint.height());
    }

    switch(which) {
    case Qt::MinimumSize: 
        {
            if ( !d->mText.isEmpty() ) {
                size.setWidth( MinimumWidth ); // just to show something  -- should not matter in read use-case

                if( d->mMinLines > 1 ) {
                    size.setHeight( ( metrics.height() + metrics.leading() ) * d->mMinLines - metrics.leading() );
                } else {
                    size.setHeight( metrics.height() );
                }
            }

            break;
        }

    case Qt::PreferredSize: 
        {
            if ( !(effectiveOrientations&Qt::Horizontal) && d->mMinLines > 0 && (d->mMinLines == d->mMaxLines) ) {
                //optimize single line if the horizontal sizeHint is ignored
                size.setHeight( ( metrics.height() + metrics.leading() ) * d->mMinLines - metrics.leading() );
                break;
            }

            // do the heavy calculation
            size = metrics.boundingRect(QRectF(QPointF(),maxSize),
                d->textFlagsFromTextOption(),
                d->mText).size();


            if( ( constraint.width() < 0 ) && ( constraint.height() < 0 ) ) {

                if( ( d->mNeedToAdjustSizeHint ) || ( d->oldSize != size ) ) {
                    const_cast<HbTextItemPrivate*>(d)->adjustSizeHint();
                }

                qreal pref =  d->mPrefHeight;

                if( d->mMaxLines > 0 ) {
                    qreal maxLimit =  ( metrics.height() + metrics.leading() ) * d->mMaxLines - metrics.leading();
                    if( maxLimit < pref ) {
                        pref = maxLimit;
                    }

                }

                const_cast<HbTextItemPrivate*>(d)->oldSize = size;
                size.setHeight( pref );
            }

            break;
        }

    default:
        size = HbWidgetBase::sizeHint( which, constraint );
    }

    return size;
}

 /*!
    \reimp

    Detects: font changes, layout direction changes and theme changes.
 */
void HbTextItem::changeEvent(QEvent *event)
{
    // Listens theme changed event so that item size hint is

    switch(event->type()) {
    case QEvent::LayoutDirectionChange: {
            Q_D(HbTextItem);
            d->mInvalidateShownText = true;
            updateGeometry();
        }
        break;

    case QEvent::FontChange: {
            Q_D(HbTextItem);
            d->mInvalidateShownText = true;
            prepareGeometryChange();
            updateGeometry();
        }
        break;

    default:
        // comparing event->type() with dynamic values:

        if (event->type() == HbEvent::ThemeChanged) {
            Q_D(HbTextItem);
            d->mDefaultColor = QColor(); 
            if(!d->mColor.isValid()) {
                update();
            }
        }
    }
    HbWidgetBase::changeEvent( event );
}

/*!
    \reimp
 */
void HbTextItem::resizeEvent ( QGraphicsSceneResizeEvent * event )
{
    Q_D(HbTextItem);

    HbWidgetBase::resizeEvent(event);

    d->setSize(event->newSize());

    if( ( qAbs(event->oldSize().width() - event->newSize().width()) > EPSILON ) &&
        ( ( event->oldSize().width() < preferredWidth() ) || ( event->newSize().width() < preferredWidth() ) ) ){
        if( d->adjustSizeHint() ) {
            updateGeometry();
        }
    }
}

/*!
    @proto
    Sets style of text wrapping. \a mode type will be changed to Hb::TextWrapping
    after appropriate merge.

    \sa HbTextItem::textWrapping
    \sa QTextOption::setWrapMode
 */
void HbTextItem::setTextWrapping(Hb::TextWrapping mode)
{
    Q_D(HbTextItem);
	d->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextWrapMode, true);
    QTextOption::WrapMode textWrapMode = static_cast<QTextOption::WrapMode>(mode);

    QTextOption textOption = d->mTextLayout.textOption();
    if(textOption.wrapMode()!=textWrapMode) {
        textOption.setWrapMode(textWrapMode);
        d->mTextLayout.setTextOption(textOption);
        if(!d->mText.isEmpty()) {
            d->mInvalidateShownText = true;
            d->mNeedToAdjustSizeHint = true;
            updateGeometry();
        }
    }
}

/*!
    @proto
    returns style of text wrapping.

    \sa HbTextItem::setTextWrapping
    \sa QTextOption::wrapMode
 */
Hb::TextWrapping HbTextItem::textWrapping() const
{
    Q_D(const HbTextItem);
    return static_cast<Hb::TextWrapping>(d->mTextLayout.textOption().wrapMode());
}

/*!
    Shows (default) or hides text.
    Size hint remains unchanged (same as when text is visible).

    Equvalent of QGraphicsItem::setVisible(bool)
 */
void HbTextItem::setTextVisible(bool isVisible)
{
    setVisible(isVisible);
}

/*!
    Returns if text is visible.

    \sa HbTextItem::setTextVisible(bool)

    Equvalent of QGraphicsItem::isVisible()
 */
bool HbTextItem::isTextVisible() const
{
    return isVisible();
}

/*!
    enables (default) od disables text cliping when item geometry is to small.

    \sa HbTextItem::isTextClip()

    Equvalent of QGraphicsItem::setFlag(QGraphicsItem::ItemClipsToShape, clipping)
 */
void HbTextItem::setTextClip(bool clipping)
{
    setFlag(QGraphicsItem::ItemClipsToShape, clipping);
}

/*!
    Returns true if text is cliped when item geometry is to small.
    \sa HbTextItem::setTextClip(bool)

    Equvalent of QGraphicsItem::flags().testFlag(QGraphicsItem::ItemClipsToShape)
 */
bool HbTextItem::isTextClip() const
{
    return flags().testFlag(ItemClipsToShape);
}

/*!
    Sets minimum number of lines for text item. If minimum number of lines is set,
    then text item will always draw at least this number of lines.

    If you set minimum lines bigger than maximum lines, then maximum lines parameter
    will be automatically increased.

    Pass negative or zero value as an input parameter to unset this constraint

    \sa HbTextItem::minimumLines()
    \sa HbTextItem::setMaximumLines()
    \sa HbTextItem::maximumLines()
 */
void HbTextItem::setMinimumLines( int minLines )
{
    Q_D( HbTextItem );
	d->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin, true);

    if( minLines != d->mMinLines ) {
        if( ( d->mMaxLines > 0 ) && ( minLines > d->mMaxLines ) ) {
            d->mMaxLines = minLines;
        }

        d->mMinLines = minLines;
        updateGeometry();
    }
}

/*!
    Sets maximum number of lines for text item. If maximum number of lines is set,
    then text item will not draw more lines then this maximum.

    Pass negative or zero value as an input parameter to unset this constraint

    If you set maximum lines less than minimum lines, then minimum lines parameter
    will be automatically decreased.

    \sa HbTextItem::maximumLines()
    \sa HbTextItem::setMinimumLines()
    \sa HbTextItem::minimumLines()
 */
void HbTextItem::setMaximumLines( int maxLines )
{
    Q_D( HbTextItem );
	d->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMax, true);

    if( maxLines != d->mMaxLines ) {
        if( ( d->mMinLines > 0 ) && ( maxLines > 0 ) && ( maxLines < d->mMinLines ) ){
            d->mMinLines = maxLines;
        }

        d->mMaxLines = maxLines;
        updateGeometry();
#ifdef HB_TEXT_MEASUREMENT_UTILITY
        if ( HbFeatureManager::instance()->featureStatus( HbFeatureManager::TextMeasurement ) ) {
            setProperty( HbTextMeasurementUtilityNameSpace::textMaxLines, d->mMaxLines );
        }
#endif
    }
}

/*!
    \sa HbTextItem::setMinimumLines()
    \sa HbTextItem::setMaximumLines()
    \sa HbTextItem::maximumLines()
    \return "minimum lines" parameter
 */
int HbTextItem::minimumLines() const
{
    Q_D( const HbTextItem );
    return d->mMinLines;
}

/*!
    \sa HbTextItem::setMaximumLines()
    \sa HbTextItem::setMinimumLines()
    \sa HbTextItem::minimumLines()
    \return "maximum lines" parameter
 */
int HbTextItem::maximumLines() const
{
    Q_D( const HbTextItem );
    return d->mMaxLines;
}

/*!
    @proto

    returns distance which text fades out when reaching border of item.

    \sa HbTextItem::setFadeLengths(qreal, qreal)
*/
QPointF HbTextItem::fadeLengths() const
{
    Q_D( const HbTextItem );
    return QPointF(d->mFadeLengthX, d->mFadeLengthY);
}

/*!
    @proto

    Method provided for convenience.
    Equivalent of setFadeLengths(length, length).

    \sa HbTextItem::setFadeLengths(qreal, qreal)
*/
void HbTextItem::setFadeLength(qreal length)
{
    Q_D( HbTextItem );
    d->setFadeLengths(length, length);
}

/*!
    @proto

    Sets distance on which text will be fade out when reaching border of item.

    Effect is performed only when text should be clipped at specified border.

    Positive value means that fade will end at border of contentsRect()
    and will start at a \a length distance inside of this rectangle.

    Zero value disables the feature.

    Behavior for negative values is undefined.

    Note that text clip (setTextClip) must be set to true to use this effect.

    xLength and yLength values refer to fade effect to horizontal and vertical
    direction respectively.
 */
void HbTextItem::setFadeLengths(qreal xLength, qreal yLength)
{
    Q_D( HbTextItem );
    d->setFadeLengths(xLength, yLength);
}

/*!
    @proto

    Method provided for connivance.
    Equivalent of setFadeLengths(lengths.x(), lengths.y()).

    \sa HbTextItem::setFadeLengths(qreal, qreal)
 */
void HbTextItem::setFadeLengths(const QPointF& lengths)
{
    Q_D( HbTextItem );
    d->setFadeLengths(lengths.x(), lengths.y());
}

// end of file
