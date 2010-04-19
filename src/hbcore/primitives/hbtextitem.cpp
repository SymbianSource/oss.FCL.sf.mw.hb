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


#ifdef HB_TEXT_MEASUREMENT_UTILITY
#include "hbtextmeasurementutility_p.h"
#include "hbfeaturemanager_p.h"
#endif


bool HbTextItemPrivate::outlinesEnabled = false;

static const QString KDefaultColorThemeName = "qtc_view_normal";
const int MinimumWidth = 5; // minimum width if there is some text.
const int KLayoutCacheLimit = 64;

HbTextItemPrivate::HbTextItemPrivate () :
    mAlignment(Qt::AlignLeft | Qt::AlignVCenter),
    mElideMode(Qt::ElideRight),
    mDontPrint(false),
    mDontClip(false),
    mTextDirection(Qt::LeftToRight),
    mInvalidateShownText(true),
    mOffsetPos(0,0),
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
    q->setFlag(QGraphicsItem::ItemClipsToShape, !mDontClip);
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

        line.setLineWidth(lineWidth);
        height += leading;
        line.setPosition(QPointF(0, height));
        height += line.height();
        if( ( mMaxLines > 0 ) && ( mTextLayout.lineCount() >= mMaxLines ) ) {
            textTruncated = true;
            break;
        }
    }
    mTextLayout.endLayout();

    return textTruncated;
}

void HbTextItemPrivate::setSize(const QSizeF &newSize)
{
    Q_Q(HbTextItem);

    updateTextOption();

    QFont usedFont = q->font();
    QFontMetricsF fontMetrics(usedFont);

    const qreal lineWidth = newSize.width();

    updateTextOption();

    QString tempText(mText);
    if(tempText.indexOf('\n')>=0) {
        // to prevent creation of deep copy if replace has no efect
        tempText.replace('\n', QChar::LineSeparator);
    }

    // function does the layout only when needed
    mTextLayout.setFont(usedFont);
	// Need to call elidedText explicitly to enable multiple length translations.
    tempText = fontMetrics.elidedText(tempText, Qt::ElideNone, lineWidth);
    bool textTruncated = doLayout(tempText, lineWidth, fontMetrics.leading());
    if(mElideMode!=Qt::ElideNone && !tempText.isEmpty()) {
        if(mTextLayout.boundingRect().height()>newSize.height()
          || mTextLayout.boundingRect().width()>newSize.width() || textTruncated) {
            // TODO: Multiple length translations with multiline text
            doLayout(elideLayoutedText(newSize, fontMetrics),
                     lineWidth,
                     fontMetrics.leading());
        }
    }
    calculateVerticalOffset();
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
    textOpt.setTextDirection (mTextDirection);
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
    QTextOption textOtion = mTextLayout.textOption();
    int flags = (int)mAlignment;

    switch(textOtion.wrapMode()) {
    case QTextOption::NoWrap:
        flags |= Qt::TextSingleLine;
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

    if(mDontClip)  flags |= Qt::TextDontClip;
    if(mDontPrint) flags |= Qt::TextDontPrint;

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
            if( qAbs( mPrefHeight - newPrefHeight ) > 0.01 /* epsilon */ ) {
                mPrefHeight = newPrefHeight;
                return true;
            }
            return false;
        }

        QSizeF currSize = q->size();
        // do the heavy calculation
        QRectF desiredRect = metrics.boundingRect( QRectF( 0, 0 , currSize.width(), QWIDGETSIZE_MAX ), textFlagsFromTextOption(), mText );

        if( qAbs( desiredRect.height() - mPrefHeight ) > 0.01 /* epsilon */ ) {
            mPrefHeight = desiredRect.height();
            return true;
        }
    }

    return false;
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
    Returns the text color used for paiting text.
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
    Returns the text alignment. It suports vertical and horizontal alignment.

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
        bool rightToLeft = HbTextUtils::ImplicitDirectionalityIsRightToLeft(
            txt.utf16(), txt.length(), 0 );
        d->mTextDirection = rightToLeft ? Qt::RightToLeft : Qt::LeftToRight;
        prepareGeometryChange();
        d->mText = txt;
        d->mTextLayout.setCacheEnabled(KLayoutCacheLimit >= d->mText.length());
        bool onlyHorizonalSizeHintChanged = false;
        if ( d->mMinLines > 0 && (d->mMinLines == d->mMaxLines) ) {
            onlyHorizonalSizeHintChanged = true;
        }
        if ( (sizePolicy().horizontalPolicy()&QSizePolicy::IgnoreFlag) && onlyHorizonalSizeHintChanged ) {
            // suppress updageGeometry() and use the same geometry
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
	d->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor, true);
    if (d->mColor != color) {
        d->mColor = color;
        update();
    }
}

/*!
    Sets the text alignment into \a alignment.
    It suports vertical and horizontal alignment.

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

     HbWidgetBase::paint(painter, option, widget);

    /* Reverted "text layouting optimization"
    if ( d->mInvalidateShownText ) {
        d->setSize( size() );
    }
    */

    if(!d->mDontPrint) {
        painter->setPen(textColor());

        d->mTextLayout.draw(painter,
                            d->mOffsetPos,
                            QVector<QTextLayout::FormatRange>(),
                            d->mDontClip?QRectF():contentsRect());
    }

    if (HbTextItemPrivate::outlinesEnabled){
        painter->setBrush(QBrush(QColor(255, 0, 0, 50)));
        QRectF rect(contentsRect());
        // to see border - bounding rect was cliping bottom and right border
        rect.adjust(0, 0, -1.0, -1.0);
        painter->drawRect(rect);
    }
}

/*!
    \reimp

    Sets geometry of text
 */
void HbTextItem::setGeometry(const QRectF & rect)
{
    /* Reverted "text layouting optimization" */
    Q_D(HbTextItem);

    HbWidgetBase::setGeometry(rect);

    // needed when tere was no size change and some things
    // need to relayout text
    if(d->mInvalidateShownText) {
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

    QRectF result(d->mTextLayout.boundingRect());
    result.translate(d->mOffsetPos);

    if(!d->mDontClip) {
        // clip
        result = result.intersected(contentsRect());
    }

    if (HbTextItemPrivate::outlinesEnabled) {
        result = result.united(contentsRect());
    }

    return result;
}

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
                //optimize single line if the horizonal sizeHint is ignored
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
            d->updateTextOption();
        }
        break;

    case QEvent::FontChange: {
            Q_D(HbTextItem);
            d->mInvalidateShownText = true;
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
    /* Reverted "text layouting optimization"
    d->mInvalidateShownText = true;
    */
    d->setSize(event->newSize());

    if( ( qAbs(event->oldSize().width() - event->newSize().width()) > 0.01 ) &&
        ( ( event->oldSize().width() < preferredWidth() ) || ( event->newSize().width() < preferredWidth() ) ) ){
        if( d->adjustSizeHint() ) {
            updateGeometry();
        }
    }
}

/*!
    @proto
    Sets style of text wrapping. \a mode type will be changed to Hb::TextWraping
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
    returns style of text wrapping. Return value type will be changed to
    Hb::WrappMode after appropriate merge.

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

    \sa HbTextItem::isVisible()
 */
void HbTextItem::setTextVisible(bool isVisible)
{
    Q_D(HbTextItem);
    if( d->mDontPrint == isVisible ) {
        d->mDontPrint = !isVisible;
        update();
    }
}

/*!
    Returns if text is visible.

    \sa HbTextItem::setTextVisible(bool)
 */
bool HbTextItem::isTextVisible() const
{
    Q_D(const HbTextItem);
    return !d->mDontPrint;
}

/*!
    enables (default) od disables text cliping when item geometry is to small.

    \sa HbTextItem::isTextClip()
 */
void HbTextItem::setTextClip(bool cliping)
{
    Q_D(HbTextItem);
    if( d->mDontClip == cliping ) {
        prepareGeometryChange();
        d->mDontClip = !cliping;
        setFlag(QGraphicsItem::ItemClipsToShape, cliping);
        update();
    }
}

/*!
    Returns true if text is cliped when item geometry is to small.

    \sa HbTextItem::setTextClip(bool)
 */
bool HbTextItem::isTextClip() const
{
    Q_D(const HbTextItem);
    return !d->mDontClip;
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

// end of file
