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

#include "hbrichtextitem.h"
#include "hbrichtextitem_p.h"
#include "hbtextutils_p.h"
#include "hbcolorscheme.h"
#include "hbevent.h"

#include <QTextDocument>
#include <QStyle>
#include <QGraphicsSceneResizeEvent>
#include <QTextBlock>
#include <QTextLayout>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QWidget>
#include <QStyleOptionGraphicsItem>

const qreal KMinimumRichTextWidth = 6.0;

static const QString KDefaultColorThemeName = "qtc_view_normal";

HbRichTextItemPrivate::HbRichTextItemPrivate() :
        mTextOption(Qt::AlignLeft|Qt::AlignVCenter),
        mRtf(0)
{
}

HbRichTextItemPrivate::~HbRichTextItemPrivate()
{
}

/*
 * private constructor
 */
void HbRichTextItemPrivate::init()
{
    Q_Q(HbRichTextItem);

    q->setFlag(QGraphicsItem::ItemClipsToShape, true);
    q->setFlag(QGraphicsItem::ItemIsSelectable, false);
    q->setFlag(QGraphicsItem::ItemIsFocusable,  false);

    mRtf = new QTextDocument(q);
    mRtf->setDocumentMargin(0.0); // no margins needed

    mTextOption.setWrapMode(QTextOption::NoWrap);
    mRtf->setDefaultTextOption(mTextOption);

    mRtf->setDefaultFont(q->font());
}

void HbRichTextItemPrivate::clear()
{
}

void HbRichTextItemPrivate::setDocumentWidth(qreal newWidth)
{
    if (!qFuzzyCompare(mRtf->textWidth(), newWidth)) {
        mRtf->setTextWidth(newWidth);
        calculateOffset();
    }
}

// #define HB_RICH_TEXT_ITEM_ALWAS_SHOW_FIRST_LINE
void HbRichTextItemPrivate::calculateOffset()
{
    Q_Q(HbRichTextItem);

    qreal diff;
    Qt::Alignment align = mTextOption.alignment();
    if (align.testFlag(Qt::AlignTop)) {
        diff = 0.0;
    } else {
        diff = q->geometry().height() - mRtf->size().height();
        if (!align.testFlag(Qt::AlignBottom)) {
            // default align Qt::AlignVCenter if no flags are set
            diff*=0.5;
        }
    }
#ifdef HB_RICH_TEXT_ITEM_ALWAS_SHOW_FIRST_LINE
    diff = qMax(diff, (qreal)0.0);
#endif

    if (diff!=mOffset.y()) {
        mOffset.setY(diff);
        q->prepareGeometryChange();
    }
}

QSizeF HbRichTextItemPrivate::minimumSizeHint(const QSizeF &/*constraint*/) const
{
    QSizeF result(KMinimumRichTextWidth, 0);

    QTextBlock textBlock = mRtf->begin();
    if (textBlock.isValid() && textBlock.layout()->lineCount() > 0) {
        QTextLine line = textBlock.layout()->lineAt(0);
        result.setHeight(line.height());
    } else {
        QFontMetricsF metrics(mRtf->defaultFont());
        result.setHeight(metrics.height());
    }

    qreal doubleDocMargin = mRtf->documentMargin() * (qreal)2.0;
    result.rheight() += doubleDocMargin;
    result.rwidth() += doubleDocMargin;

    return result;
}

void HbRichTextItemPrivate::clearPrefSizeCache()
{
    mPrefSize.setWidth(-1);
    mMinWidthForAdjust = QWIDGETSIZE_MAX;
    mMaxWidthForAdjust = -1;
}

QSizeF HbRichTextItemPrivate::preferredSizeHint(const QSizeF &constraint) const
{
    if (mPrefSizeConstraint==constraint && mPrefSize.isValid()) {
        return mPrefSize;
    }
    mPrefSizeConstraint=constraint;

    QSizeF result;

    if (constraint.width()<=0) {
        mRtf->setTextWidth(QWIDGETSIZE_MAX);
    } else {
        QTextOption::WrapMode wrapMode = mTextOption.wrapMode();
        // optimization when there is no automatic wrap there is no reason
        // to setTextWidth with width constraint (width measure is not needed)
        if (wrapMode!=QTextOption::NoWrap
            && wrapMode!=QTextOption::ManualWrap) {
            mRtf->setTextWidth(constraint.width());
        }
    }
    result = mRtf->size();
    mMaxWidthForAdjust = result.width();
    result.setWidth(mRtf->idealWidth());
    mMinWidthForAdjust = result.width();
    mDefaultPrefHeight = result.height();
    mPrefSize = result;

    return result;
}

bool HbRichTextItemPrivate::isAdjustHeightNeeded(qreal newWidth,
                                                 qreal prefHeight,
                                                 qreal minHeight,
                                                 qreal maxHeight)
{
    // first check if wrapping of text is not active
    QTextOption::WrapMode wrapMode = mTextOption.wrapMode();
    if (wrapMode==QTextOption::NoWrap
        || wrapMode==QTextOption::ManualWrap) {
        return false;
    }

    // preferred height was set from outside of this class so there is mo reason to adjust it
    if (mPrefSizeConstraint.height()>0) {
        return false;
    }

    // check if adjusted size has been already calculated
    // new width is bigger than last estimated range of same height
    if ((mMaxWidthForAdjust>=newWidth
         // new width is smaller than last estimated range of same height
         && newWidth>=mMinWidthForAdjust)) {
        return false;
    }

    if (!mPrefSize.isValid()) {
        // this means that preferred size is set outside of class by setPreferredSize
        // so sizeHint(Qt::Preferredsize) was not called and size adjustment is useless
        return false;
    }

    // new text width was set in setGeometry here it is not needed
    Q_ASSERT_X(qFuzzyCompare(mRtf->textWidth(), newWidth),
               "HbRichTextItemPrivate::isAdjustHeightNeeded",
               QString("mRtf->textWidth()=%1, newWidth=%2")
                    .arg(mRtf->textWidth())
                    .arg(newWidth).toAscii().data());

    // if preconditions are met test if current height is enough
    QSizeF newAdjust = mRtf->size();

    if (qFuzzyCompare(newAdjust.height(), mPrefSize.height())) {
        // height is same as last time update range of same height
        mMaxWidthForAdjust = qMax(mMaxWidthForAdjust, newWidth);
        mMinWidthForAdjust = qMin(mMinWidthForAdjust, newWidth);
        return false;
    }

    // new height was calculated create new range for which
    // current mPrefSize.height is valid
    mMaxWidthForAdjust = newWidth;
    mMinWidthForAdjust = mRtf->idealWidth();

    // store new height, don't change width
    mPrefSize.setHeight(newAdjust.height());

    Q_ASSERT_X(mPrefSizeConstraint.width()>0 || mPrefSize.width()>=mMinWidthForAdjust,
               "HbRichTextItemPrivate::isAdjustHeightNeeded",
               QString("Fail for (%1<%2) string: \"%3\"")
               .arg(mPrefSize.width())
               .arg(mMinWidthForAdjust)
               .arg(mText).toAscii().data());

    if (qFuzzyCompare(qBound(minHeight, mPrefSize.height(), maxHeight),
                      prefHeight)) {
        // updateGeometry has no effect
        return false;
    }

    // all conditions for calling updateGeometry are meet
    return true;
}

/*
    returns true if updateGeometry is needed
 */
bool HbRichTextItemPrivate::restoreDefaultHeightHint()
{
    if (mPrefSizeConstraint.height()>0) {
        return false;
    }

    if (mPrefSizeConstraint.width()>0) {
        clearPrefSizeCache();
        return true;
    }

    if (mPrefSize.height()==mDefaultPrefHeight) {
        return true;
    }

    mPrefSize.setHeight(mDefaultPrefHeight);
    mMinWidthForAdjust = mPrefSize.width();
    mMaxWidthForAdjust = QWIDGETSIZE_MAX;
    return true;
}

/*!
  @proto
  @hbcore
 \class HbRichTextItem
 \brief HbRichTextItem is a item for showing formatted text.


 This is mainly used as a primitive in widgets.
 It derives from HbWidgetBase so it can be layouted.

 */

/*!
    Constructor for the class.
 */

HbRichTextItem::HbRichTextItem(QGraphicsItem *parent) :
        HbWidgetBase(*new HbRichTextItemPrivate, parent)
{
    Q_D(HbRichTextItem);
    d->init();
}

/*!
    Constructor which set content using \a html format.
 */
HbRichTextItem::HbRichTextItem(const QString &html, QGraphicsItem *parent) :
        HbWidgetBase(*new HbRichTextItemPrivate, parent)
{
    Q_D(HbRichTextItem);
    d->init();
    setText(html);
}

/*
    Constructor for internal use only
 */
HbRichTextItem::HbRichTextItem(HbRichTextItemPrivate &dd, QGraphicsItem *parent) :
        HbWidgetBase(dd, parent)
{
    Q_D(HbRichTextItem);
    d->init();
}

/*!
    Destructor for the class.
 */
HbRichTextItem::~HbRichTextItem()
{
    Q_D(HbRichTextItem);
    d->clear();
}

/*!
    Sets the \a text in html format.

    \sa HbRichTextItem::text()
 */
void HbRichTextItem::setText(const QString &text)
{
    Q_D(HbRichTextItem);
    if (d->mText != text) {
        d->mText = text;
        d->mRtf->setHtml(text);
        d->clearPrefSizeCache();
        update();
        updateGeometry();
    }
}

/*!
    Returns the text in html format.

    \sa HbRichTextItem::setText()
 */

QString HbRichTextItem::text() const
{
    Q_D(const HbRichTextItem);
    return d->mText;
}

/*!
    Sets \a alignment for the text from Qt::Alignment enumeration.

    \sa HbRichTextItem::alignment()
 */
void HbRichTextItem::setAlignment(Qt::Alignment alignment)
{
    Q_D(HbRichTextItem);
    d->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextAlign, true);
    alignment = d->combineAlignment(alignment, d->mTextOption.alignment());
    if (d->mTextOption.alignment()!=alignment) {
        prepareGeometryChange();
        d->mTextOption.setAlignment(alignment);
        d->mRtf->setDefaultTextOption(d->mTextOption);
        d->calculateOffset();
        update();
    }
}

/*!
    Returns alignment for the text from Qt::Alignment enumeration.

    \sa HbRichTextItem::setAlignment()
 */
Qt::Alignment HbRichTextItem::alignment() const
{
    Q_D(const HbRichTextItem);
    return d->mTextOption.alignment();
}

/*!
    \reimp
 */
void HbRichTextItem::paint(QPainter *painter, 
                           const QStyleOptionGraphicsItem *option,
                           QWidget *widget)
{
    Q_UNUSED(widget);

    Q_D(HbRichTextItem);

    if (option->exposedRect.isEmpty()) {
        // nothing to paint
        return;
    }

    painter->translate(d->mOffset);

    QAbstractTextDocumentLayout::PaintContext context;
    context.clip = option->exposedRect;
    // painter was translated so it should be compensated
    context.clip.translate(-d->mOffset);

    context.palette.setColor(QPalette::Text, textDefaultColor());

    d->mRtf->documentLayout()->draw(painter, context);

    // restore painter
    painter->translate(-d->mOffset);
}

/*!
    \reimp

    Sets new position and relayouts text according to new size.
 */
void HbRichTextItem::setGeometry(const QRectF & rect)
{
    Q_D(HbRichTextItem);

    HbWidgetBase::setGeometry(rect);

    // this call is needed since there is possible scenario
    // when size was not changed after updateGeometry and sizeHint calls
    d->setDocumentWidth(size().width());

    if (parentLayoutItem() && parentLayoutItem()->isLayout()) {
        // rect.size can't be used here since size can be limited inside of
        // called method HbWidgetBase::setGeometry(rect) so size is used which
        // holds current size
        if (d->isAdjustHeightNeeded(size().width(),
                                    preferredHeight(),
                                    minimumHeight(),
                                    maximumHeight())) {
            updateGeometry();
            return;
        }
    }
}

/*!
    \reimp
 */
QRectF HbRichTextItem::boundingRect () const
{
    Q_D(const HbRichTextItem);

    QRectF result(d->mRtf->documentLayout()->frameBoundingRect(d->mRtf->rootFrame()));
    result.translate(d->mOffset);

    if (flags().testFlag(QGraphicsItem::ItemClipsToShape)) {
        // clip
        result = result.intersect(contentsRect());
    }
    return result;
}

/*!
    \reimp
    Relayouts text according to new size.
 */
void HbRichTextItem::resizeEvent(QGraphicsSceneResizeEvent * /*event*/)
{
    Q_D(HbRichTextItem);

    d->setDocumentWidth(size().width());
    d->calculateOffset();
}

/*!
    \reimp
    This implementation detects layout direction changes, font changes and theme changes.
 */
void HbRichTextItem::changeEvent(QEvent *event)
{
    Q_D(HbRichTextItem);

    switch(event->type()) {
    case QEvent::LayoutDirectionChange: {
            prepareGeometryChange();
            d->mTextOption.setTextDirection(layoutDirection());
            d->mRtf->setDefaultTextOption(d->mTextOption);
            update();
        }
        break;

    case QEvent::FontChange: {
            d->mRtf->setDefaultFont(font());
            d->clearPrefSizeCache();
            updateGeometry();
        }
        break;

    default:
        // Listens theme changed event so that item size hint is
        // update when physical font is changed.
        if (event->type() == HbEvent::ThemeChanged) {
            Q_D(HbRichTextItem);
            d->mDefaultColor = QColor();
            if (!d->mColor.isValid()) {
                update();
            }
        }
    }
    HbWidgetBase::changeEvent(event);
}

/*!
    \reimp
    For which = PreferredSize returns reasonable size (QTextDocument::adjustSize()).
    For which = MinimumSize returns size of 4 first letters
    \a constraint width is taken into account.
 */
QSizeF HbRichTextItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_D(const HbRichTextItem);
    QSizeF result;
    switch(which) {
    case Qt::MinimumSize: {
            result = d->minimumSizeHint(constraint);
        }
        break;

    case Qt::PreferredSize: {
            result = d->preferredSizeHint(constraint);
        }
        break;

    default:
        result = HbWidgetBase::sizeHint(which, constraint);
    }
    return result;
}

/*!
 * @proto
 * Sets the text wrapping mode.
 *
 * \sa HbRichTextItem::textWrapping
 * \sa QTextOption::setWrapMode
 */
void HbRichTextItem::setTextWrapping(Hb::TextWrapping mode)
{
    Q_D(HbRichTextItem);
    d->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextWrapMode, true);
    QTextOption::WrapMode textWrapMode = static_cast<QTextOption::WrapMode>(mode);

    if (d->mTextOption.wrapMode()!=textWrapMode) {
        prepareGeometryChange();
        d->mTextOption.setWrapMode(textWrapMode);
        d->mRtf->setDefaultTextOption(d->mTextOption);
        d->calculateOffset();
        if (d->restoreDefaultHeightHint()) {
            updateGeometry();
        }
    }
}

/*!
 * @proto
 * Returns style of text wrapping.
 *
 * \sa HbRichTextItem::setTextWrapping
 * \sa QTextOption::wrapMode
 */
Hb::TextWrapping HbRichTextItem::textWrapping() const
{
    Q_D(const HbRichTextItem);

    return static_cast<Hb::TextWrapping>(d->mTextOption.wrapMode());
}

/*!
 * Returns color used as a default text color.
 * If invalid color was set color for text is fetch from parent widget.
 * If invalid color was set and no parent widget was set this will return
 * default foreground color.
 *
 * \sa setTextDefaultColor()
 */
QColor HbRichTextItem::textDefaultColor() const
{
    Q_D(const HbRichTextItem);

    if (d->mColor.isValid()) { // Means user has set text color
        return d->mColor;
    } 
    if (!d->mDefaultColor.isValid()) {
        d->mDefaultColor = HbColorScheme::color(KDefaultColorThemeName);
    }
    
    return d->mDefaultColor;
}

/*!
 * Sets color of text.
 * If invalid color was set color for text is fetch from parent widget.
 * If invalid color was set and no parent widget was set default foreground color
 * will be used
 *
 * \sa textDefaultColor()
 */
void HbRichTextItem::setTextDefaultColor(const QColor &color)
{
    Q_D(HbRichTextItem);

    d->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor, color.isValid());
    if (d->mColor != color) {
        d->mColor = color;

        if (!color.isValid()) {
            QGraphicsWidget* ccsHandler = parentWidget();
            // check if there is a widget which handles CSS
            if (ccsHandler!=NULL) {
                // this is needed to enforce color fetch from CSS
                HbEvent themeEvent(HbEvent::ThemeChanged);
                QApplication::sendEvent(ccsHandler, &themeEvent);
            }
        }

        if (!d->mText.isEmpty()) {
            update();
        }
    }
}

/*!
 * Shows (default) or hides text. Size hint remains unchanged (same as when text is visible).
 */
void HbRichTextItem::setTextVisible(bool isVisible)
{
    setVisible(isVisible);
}

/*!
 * Returns if text is visible.
 */
bool HbRichTextItem::isTextVisible() const
{
    return isVisible();
}

/*!
 * Enables (default) or disables text clipping when item geometry is too small.
 */
void HbRichTextItem::setTextClip(bool clipping)
{
    setFlag(QGraphicsItem::ItemClipsToShape, clipping);
}

/*!
 * Returns true if text is clipped when item geometry is too small.
 */
bool HbRichTextItem::isTextClip() const
{
    return flags().testFlag(ItemClipsToShape);
}

// end of file
