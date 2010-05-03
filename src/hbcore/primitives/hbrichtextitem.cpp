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

const int KMinimumLetersToShow = 4;

static const QString KDefaultColorThemeName = "qtc_view_normal";

HbRichTextItemPrivate::HbRichTextItemPrivate() :
    mAlignment(Qt::AlignLeft|Qt::AlignVCenter),
    mTextOption(mAlignment),
    mDontPrint(false),
    mDontClip(false),
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

    q->setFlag(QGraphicsItem::ItemClipsToShape, !mDontClip);
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
    delete mRtf;
}

int HbRichTextItemPrivate::textFlagsFromTextOption() const
{
    int flags = (int)mAlignment;

    switch(mTextOption.wrapMode()) {
    case QTextOption::NoWrap:
        flags |= Qt::TextSingleLine;
        break;
    case QTextOption::WordWrap:
        flags |=Qt::TextWordWrap;
        break;
    case QTextOption::ManualWrap:
        break;
    case QTextOption::WrapAnywhere:
        flags |=Qt::TextWrapAnywhere;
        break;
    case QTextOption::WrapAtWordBoundaryOrAnywhere:
        flags |=Qt::TextWordWrap | Qt::TextWrapAnywhere;
        break;
    }

    if(mDontClip)  flags |= Qt::TextDontClip;
    if(mDontPrint) flags |= Qt::TextDontPrint;

    return flags;
}

bool HbRichTextItemPrivate::setLayoutDirection(Qt::LayoutDirection newDirection)
{
    Qt::Alignment oldAlign = mTextOption.alignment();
    Qt::Alignment alignment = QStyle::visualAlignment(newDirection, mAlignment);
    mTextOption.setAlignment(alignment);
    mTextOption.setTextDirection(newDirection);
    mRtf->setDefaultTextOption(mTextOption);

    return alignment!=oldAlign;
}

void HbRichTextItemPrivate::setSize(const QSizeF &newSize)
{
    if(mRtf->size()!=newSize) {
        Q_Q(HbRichTextItem);
        mRtf->setTextWidth(newSize.width());
        calculateOffset();
        q->update();
    }
}

// #define HB_RICH_TEXT_ITEM_ALWAS_SHOW_FIRST_LINE
void HbRichTextItemPrivate::calculateOffset()
{
    Q_Q(HbRichTextItem);

    qreal diff;
    if(mAlignment.testFlag(Qt::AlignTop)) {
        diff = 0.0;
    } else {
        diff = q->geometry().height() - mRtf->size().height();
        if(!mAlignment.testFlag(Qt::AlignBottom)) {
            // default align Qt::AlignVCenter if no flags are set
            diff*=0.5;
        }
    }
#ifdef HB_RICH_TEXT_ITEM_ALWAS_SHOW_FIRST_LINE
    diff = qMax(diff, (qreal)0.0);
#endif

    if(diff!=mOffset.y()) {
        mOffset.setY(diff);
        q->prepareGeometryChange();
    }
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
        updateGeometry();
    }
}

/*!
    Returns the text in html format.

    \sa HbRichTextItem::setText()
 */

QString HbRichTextItem::text() const
{
    Q_D( const HbRichTextItem );
    return d->mText;
}

/*!
    Sets \a alignment for the text from Qt::Alignment enumeration.

    \sa HbRichTextItem::alignment()
 */
void HbRichTextItem::setAlignment(Qt::Alignment alignment)
{
    Q_D( HbRichTextItem );
	d->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextAlign, true);
    alignment &= Qt::AlignVertical_Mask | Qt::AlignHorizontal_Mask;
    if( d->mAlignment!=alignment ) {
        prepareGeometryChange();
        d->mAlignment = alignment;
        d->calculateOffset();
        if(d->setLayoutDirection(layoutDirection())) {
            update();
        }
    }
}

/*!
    Returns alignment for the text from Qt::Alignment enumeration.

    \sa HbRichTextItem::setAlignment()
 */
Qt::Alignment HbRichTextItem::alignment() const
{
    Q_D( const HbRichTextItem );
    return d->mAlignment;
}

/*!
    \reimp
 */
void HbRichTextItem::paint(QPainter *painter, 
                            const QStyleOptionGraphicsItem *option, 
                            QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    Q_D(HbRichTextItem);

    // Save painter's state
    QRegion oldClipRegion = painter->clipRegion();
    QTransform oldTransform = painter->transform();

    if(!d->mDontPrint) {
        if(!d->mDontClip) {
            painter->setClipRect(contentsRect(), Qt::IntersectClip);
        }
        painter->translate(d->mOffset);
        QAbstractTextDocumentLayout::PaintContext context;
        context.palette.setColor(QPalette::Text, textDefaultColor());
        d->mRtf->documentLayout()->draw(painter, context);
    }

    // Restore painter's state
    painter->setClipRegion(oldClipRegion);
    painter->setTransform(oldTransform);
}

/*!
    \reimp

    Sets new position and relayouts text according to new size.
 */
void HbRichTextItem::setGeometry(const QRectF & rect)
{
    Q_D(HbRichTextItem);

    HbWidgetBase::setGeometry(rect);

    if(rect.isValid()) {
        d->setSize(rect.size());
    }
}

/*!
    \reimp
 */
QRectF HbRichTextItem::boundingRect () const
{
    Q_D(const HbRichTextItem);

    QRectF result(d->mOffset, d->mRtf->size());

    if(!d->mDontClip) {
        // clip
        result = result.intersect(QRectF(QPointF(),
                                          size()));
    }
    return result;
}

/*!
    \reimp
    Relayouts text according to new size.
 */
void HbRichTextItem::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_D(HbRichTextItem);

    d->setSize(event->newSize());
}

/*!
    \reimp
    This impelementation detects layout direction changes, font changes and theme changes.
 */
void HbRichTextItem::changeEvent(QEvent *event)
{
    Q_D(HbRichTextItem);

    switch(event->type()) {
    case QEvent::LayoutDirectionChange: {
            prepareGeometryChange();
            d->setLayoutDirection(layoutDirection());
            update();
        }
        break;

    case QEvent::FontChange: {
            d->mRtf->setDefaultFont(font());
            updateGeometry();
        }
        break;

    default:
        // Listens theme changed event so that item size hint is
        // update when physical font is changed.
        if (event->type() == HbEvent::ThemeChanged) {
            Q_D(HbRichTextItem);
            d->mDefaultColor = QColor();
            if(!d->mColor.isValid()) { 
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
            QTextBlock textBlock = d->mRtf->begin();
            if(textBlock.isValid() && textBlock.layout()->lineCount() > 0) {
                QTextLine line = textBlock.layout()->lineAt(0);
                result.setHeight(line.height());
                int cursorPos(KMinimumLetersToShow);
                result.setWidth( line.cursorToX(&cursorPos) );

                qreal doubleDocMargin = d->mRtf->documentMargin() * (qreal)2.0;
                result.rheight() += doubleDocMargin;
                result.rwidth() += doubleDocMargin;
            } else {
                result = HbWidgetBase::sizeHint(which, constraint);
            }
        }
        break;

    case Qt::PreferredSize: {
            if(constraint.width()<=0) {
                d->mRtf->adjustSize();
            } else {
                d->mRtf->setTextWidth(constraint.width());
            }
            result = d->mRtf->size();
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

    if(d->mTextOption.wrapMode()!=textWrapMode) {
        prepareGeometryChange();
        d->mTextOption.setWrapMode(textWrapMode);
        d->mRtf->setDefaultTextOption(d->mTextOption);
        updateGeometry();
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
    Q_D( const HbRichTextItem );

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
    Q_D(HbRichTextItem);
    if( d->mDontPrint == isVisible ) {
        d->mDontPrint = !isVisible;
        update();
    }
}

/*!
 * Returns if text is visible.
 */
bool HbRichTextItem::isTextVisible() const
{
    Q_D(const HbRichTextItem);
    return !d->mDontPrint;
}

/*!
 * Enables (default) or disables text clipping when item geometry is too small.
 */
void HbRichTextItem::setTextClip(bool cliping)
{
    Q_D(HbRichTextItem);
    if( d->mDontClip == cliping ) {
        prepareGeometryChange();
        d->mDontClip = !cliping;
        setFlag(QGraphicsItem::ItemClipsToShape, cliping);
        update();
    }
}

/*!
 * Returns true if text is clipped when item geometry is too small.
 */
bool HbRichTextItem::isTextClip() const
{
    Q_D(const HbRichTextItem);
    return !d->mDontClip;
}

// end of file
