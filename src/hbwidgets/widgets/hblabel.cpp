/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbWidgets module of the UI Extensions for Mobile.
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

#include <QPalette>
#include <QPainter>
#include <QStyle>
#include <QDebug>
#include <QTextDocument>
#include <QGraphicsSceneResizeEvent>

#include "hbiconitem.h"
#include "hbinstance.h"
#include "hbcolorscheme.h"
#include "hbwidget_p.h"
#include "hbstyleoptionlabel_p.h"
#include "hbwidgetbase.h"
#include "hblabel.h"
#include "hbstyle_p.h"
#include "hbstyletextprimitivedata.h"
#include "hbstylerichtextprimitivedata.h"
#include "hbstyleiconprimitivedata.h"

/*!
    @alpha
    @hbcore

    \class HbLabel
    \brief HbLabel is a label widget for showing a text or an icon.

    HbLabel supports the following content types
    \li plain text
    \li html (rich text)
    \li icon

    A label can show only one type of content at a time.

    The following is an example of how to create an icon and add it to a label using the HbLabel API.
    As the label is not managed by a layout, it must be positioned and sized.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,5}

    An example how to add labels into a layout. Explicit positioning and
    sizing of the HbLabel objects are not needed.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,6}

    The following is an example how to set the color of plain text label. This is a special case
    because of colors are usually set by layout system and there is no need to set colors explicitly.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,7}

    \sa HbIcon
 */

/*!
    \var HbLabel::PlainText

    Text in plain text format.
 */

/*!
    \var HbLabel::RichText

    Text in rich text format.
 */

/*!
    \var HbLabel::Icon

    Icon.
 */

class HbLabelPrivate: public HbWidgetPrivate {
    Q_DECLARE_PUBLIC(HbLabel)

public:
    HbLabelPrivate();
    ~HbLabelPrivate();

    void init();
    void clear();

    void clearAll();

    void setText(const QString &text, HbStyle::PrimitiveType primitiveId);
    void setIcon(const HbIcon &icon);

    void updatePrimitives ();
    void createPrimitives ();

    //shared between icon and text
    HbStyleValue<Qt::Alignment> mAlignment;

    // text section
    HbStyleValue<QString> mText;
    HbStyleValue<Qt::TextElideMode> mElideMode;
    HbStyleValue<Hb::TextWrapping> mTextWrapping;
    HbStyleValue<QColor> mColor;
    HbStyleValue<int> mMaxLines;

    // icon section
    HbStyleValue<HbIcon> mIcon;
    HbStyleValue<Qt::AspectRatioMode> mAspectRatioMode;

    // primitive handling
    QGraphicsObject *mPrimitiveItem;

    HbStyle::PrimitiveType mActivePrimitive;
};

HbLabelPrivate::HbLabelPrivate() :
        HbWidgetPrivate(),
        mPrimitiveItem(0),
        mActivePrimitive(HbStyle::PT_None)
{
}

void HbLabelPrivate::init()
{
    Q_Q(HbLabel);
    q->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
}

void HbLabelPrivate::clear()
{
    // no implementation needed
}


void HbLabelPrivate::clearAll()
{
    if (mPrimitiveItem) {
        delete mPrimitiveItem;
        mPrimitiveItem = 0;
        mActivePrimitive = HbStyle::PT_None;
    }

    mText.clear();
    mIcon.clear();
}

void HbLabelPrivate::setText(const QString &text, HbStyle::PrimitiveType primitiveId)
{
    Q_Q(HbLabel);

    if (text.isNull()) {
        clearAll();
        return;
    }

    if (mActivePrimitive != primitiveId) {
        clearAll();
    }

    if (mText!=text) {
        mText = text;
        if (mActivePrimitive != primitiveId) {
            mActivePrimitive = primitiveId;
            createPrimitives();
            q->repolish(); // reconecting new primitive to HbAnchorLayout so it is really needed!
        }
        q->updatePrimitives();
    }
}

void HbLabelPrivate::setIcon(const HbIcon &icon)
{
    Q_Q(HbLabel);

    if (icon.isNull()) {
        clearAll();
        return;
    }

    if (mActivePrimitive != HbStyle::PT_IconItem) {
        clearAll();
    }

    if (mIcon != icon) {
        mIcon = icon;

        if (mActivePrimitive != HbStyle::PT_IconItem) {
            mActivePrimitive = HbStyle::PT_IconItem;
            createPrimitives();
            q->repolish(); // reconecting new primitive to HbAnchorLayout so it is really needed!
        }
        q->updatePrimitives();
    }
}

HbLabelPrivate::~HbLabelPrivate()
{
}

void HbLabelPrivate::createPrimitives()
{
    Q_Q(HbLabel);

    Q_ASSERT(mPrimitiveItem==0);

    switch (mActivePrimitive) {
    case HbStyle::PT_None:
        break;

    case HbStyle::PT_IconItem:
        mPrimitiveItem = q->style()->createPrimitive(mActivePrimitive, QString("icon"),q);
        break;

    case HbStyle::PT_TextItem: // no break
    case HbStyle::PT_RichTextItem:
        mPrimitiveItem = q->style()->createPrimitive(mActivePrimitive, QString("text"),q);
        break;

    default:
        Q_ASSERT(0);
    }
}

void HbLabelPrivate::updatePrimitives()
{
    Q_Q(HbLabel);

    if (mActivePrimitive != HbStyle::PT_None) {
        Q_ASSERT(mActivePrimitive == HbStyle::PT_IconItem
                 || mActivePrimitive == HbStyle::PT_RichTextItem
                 || mActivePrimitive == HbStyle::PT_TextItem);

        switch (mActivePrimitive) {
        case HbStyle::PT_IconItem: {
                HbStyleIconPrimitiveData data;
                // set common data:
                data.alignment = mAlignment;

                // set icon data:
                data.aspectRatioMode = mAspectRatioMode;
                data.icon = mIcon;

                q->style()->updatePrimitive(mPrimitiveItem,
                                            &data);
            }
            break;
        case HbStyle::PT_TextItem: {
                HbStyleTextPrimitiveData data;

                // set common data:
                data.alignment = mAlignment;

                // set text common data:
                data.text = mText;
                data.textColor = mColor;
                data.textWrapping = mTextWrapping;

                // plain text specyfic:
                data.elideMode = mElideMode;
                data.maximumLines = mMaxLines;

                q->style()->updatePrimitive(mPrimitiveItem,
                                            &data);
            }
            break;

        case HbStyle::PT_RichTextItem: {
                HbStyleRichTextPrimitiveData data;

                // set common data:
                data.alignment = mAlignment;

                // set text common data:
                data.text = mText;
                data.defaultColor = mColor;
                data.textWrappingMode = mTextWrapping;

                q->style()->updatePrimitive(mPrimitiveItem,
                                            &data);
            }
            break;

        case 0: {
            }
            break;

        default:
            Q_ASSERT(0);
        }
    }
}

/*!
    Constructs the label with a given \a parent.
    \param parent - the parent graphics item.
 */
HbLabel::HbLabel(QGraphicsItem *parent) :
        HbWidget(*new HbLabelPrivate, parent)
{
    Q_D(HbLabel);
    d->init();
}

/*!
    \internal
 */
HbLabel::HbLabel(HbLabelPrivate &dd, QGraphicsItem * parent) :
        HbWidget(dd, parent)
{
    Q_D(HbLabel);
    d->init();
}

/*!
    Constructs the label with a given \a text and \a parent. This constructor is a convenience for
    common use case to have a label with plain text content. Using this contructor you do not need
    to call setPlainText() separately in initialization.
    \param displayText - the plain text that is shown in the label.
    \param parent - the parent graphics item.
 */
HbLabel::HbLabel(const QString &displayText, QGraphicsItem *parent) :
        HbWidget(*new HbLabelPrivate, parent)
{
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setPlainText(displayText);
}

/*!
    Label destructor.
 */
HbLabel::~HbLabel ()
{
    Q_D(HbLabel);
    d->clear();
}

/*!
    Sets the label contents to plain text containing the textual
    representation of integer \a num. Any previous content is cleared.
    Does nothing if the integer's string representation is the same as
    the current contents of the label.
    \param num - the number that is shown in the label.

    \sa setPlainText()
 */
void HbLabel::setNumber(int number)
{
    QString str;
    str.setNum(number);
    setPlainText(str);
}

/*!
    \overload

    Sets the label contents to plain text containing the textual
    representation of double \a num. Any previous content is cleared.
    Does nothing if the double's string representation is the same as
    the current contents of the label.
    \param num - the number that is shown in the label.

    \sa setPlainText()
 */
void HbLabel::setNumber(qreal number)
{
    QString str;
    str.setNum(number);
    setPlainText(str);
}

/*!
    Sets the text elide mode to \a elideMode.
    The elide mode specifies where text tructation and ellipsis "..." are applied if the label text
    is too large to fit the label's dimensions.
    \param elideMode - the new elide mode.

    \sa elideMode
    \sa Qt::TextElideMode
 */
void HbLabel::setElideMode (Qt::TextElideMode elideMode)
{
    Q_D(HbLabel);
    if (elideMode != d->mElideMode) {
        d->mElideMode = elideMode;
        if (d->mText.isSet()) {
            updatePrimitives();
        }
    }
}

/*!
    By default this method returns invalid value to indicate that
    eliding is controlled by CSS (setElideMode was not used).
    In CSS elide by default is set to Qt::ElideRight.

    \return the elide mode of the text.

    \sa HbLabel::setElideMode()
 */
Qt::TextElideMode HbLabel::elideMode() const
{
    Q_D(const HbLabel);
    if (d->mElideMode.isSet()) {
        return d->mElideMode;
    } else {
        return (Qt::TextElideMode)-1;
    }
}

/*!
    Sets the text wrapping mode to \a mode.
    \param mode - wrapping mode

    \sa Hb::TextWrapping
 */
void HbLabel::setTextWrapping(Hb::TextWrapping mode)
{
    Q_D(HbLabel);
    if (d->mTextWrapping != mode) {
        d->mTextWrapping = mode;
        if (d->mText.isSet()) {
            updatePrimitives();
        }
    }
}

/*!
    \return the label's current text wrapping mode.
    Default value is Hb::TextNoWrap.

    \sa setTextWrapping()
 */
Hb::TextWrapping HbLabel::textWrapping() const
{
    Q_D(const HbLabel);

    if (d->mTextWrapping.isSet()) {
        return d->mTextWrapping;
    } else {
        return Hb::TextNoWrap;
    }
}

/*!
    Sets the icon displayed by this label.
    Removes any existing text from the label.
    \param icon - the icon that this label displays.

    \sa icon
 */
void HbLabel::setIcon(const HbIcon &icon)
{
    Q_D(HbLabel);

    d->setIcon(icon);
}

/*!
    \return the icon displayed by this label.

    \sa setIcon
 */
HbIcon HbLabel::icon() const
{
    Q_D(const HbLabel);

    if (d->mIcon.isSet()) {
        return d->mIcon;
    } else {
        return HbIcon();
    }
}

/*!
    Clears the content of the label. After this the label is empty.

    \sa isEmpty()
 */
void HbLabel::clear()
{
    Q_D(HbLabel);
    d->clearAll();
}

/*!
    Sets the aspect ratio mode for the icon. The default aspect ratio is Qt::KeepAspectRatio.
    \param aspectRatioMode - the new aspect ration mode.

    \sa aspectRatioMode()
 */
void HbLabel::setAspectRatioMode(Qt::AspectRatioMode aspectRatioMode)
{
    Q_D(HbLabel);
    if (d->mAspectRatioMode != aspectRatioMode) {
        d->mAspectRatioMode = aspectRatioMode;
        if (d->mIcon.isSet()) {
            updatePrimitives();
        }
    }
}

/*!
    \return the aspect ratio set for the icon. The default aspect ratio is
    Qt::KeepAspectRatio.

    \sa setAspectRatio()
 */
Qt::AspectRatioMode HbLabel::aspectRatioMode() const
{
    Q_D(const HbLabel);

    if (d->mAspectRatioMode.isSet()) {
        return d->mAspectRatioMode;
    } else {
        return Qt::KeepAspectRatio;
    }
}

/*!
    Sets the label contents to plain text containing \a text. Any previous content is cleared.
    Does nothing if the string representation is the same as the current contents of the label.
    \param text - the plain text that is shown in the label.

    \sa setHtml()
    \sa setIcon()
 */
void HbLabel::setPlainText(const QString &text)
{
    Q_D(HbLabel);
    d->setText(text, HbStyle::PT_TextItem);
}

/*!
    Sets the label contents to html text containing \a text. Any previous content is cleared.
    Does nothing if the string representation is the same as the current contents of the label.
    \param text - the html text that is shown in the label.

    \sa setPlainText()
    \sa setIcon()
 */
void HbLabel::setHtml(const QString &text)
{
    Q_D(HbLabel);
    d->setText(text, HbStyle::PT_RichTextItem);
}

/*!
    Sets the \a alignment of the label.
    \param alignment - the new alignment.

    \sa alignment
 */
void HbLabel::setAlignment(Qt::Alignment alignment)
{
    Q_D(HbLabel);
    if (d->mAlignment != alignment) {
        d->mAlignment = alignment;
        if (alignment == 0) {
            d->mAlignment.clear();
        }
        if (d->mActivePrimitive!=HbStyle::PT_None) {
            updatePrimitives();
        }
    }
}

/*!
    \return the alignment. Default alignment is '0' indicating that nothing was
    set (so CSS cotrols alignment).

    \sa HbLabel::setAlignment()
 */
Qt::Alignment HbLabel::alignment() const
{
    Q_D(const HbLabel);
    if (d->mAlignment.isSet()) {
        return d->mAlignment;
    } else {
        return 0;
    }
}

/*!
    \return true if both text and icon are empty; otherwise returns false.


    An empty icon is initialised by HbIcon() and empty text with QString().
    An icon initialised with HbIcon("") is empty. A string initialised with
    QString("") is not empty.

    \sa clear()
 */
bool HbLabel::isEmpty() const
{
    Q_D(const HbLabel);
    return d->mActivePrimitive == HbStyle::PT_None;
}

/*!

    \deprecated HbLabel::primitive(HbStyle::Primitive)
        is deprecated.

    Returns a pointer to the QGraphicsItem primitive used by this label.
    \param primitive - the type of graphics primitive required.
    HbLabel supports HbStyle::PT_TextItem and HbStyle::PT_IconItem.
    \return the QGraphicsItem used by the label. It is 0 if type \a primitive not currently in use.
    It is also 0 if the text or icon object is empty.

    \reimp

    \sa isEmpty()
 */
QGraphicsItem * HbLabel::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbLabel);
    switch ((HbStylePrivate::Primitive)primitive) {
    case HbStylePrivate::P_Label_icon:
        if (d->mActivePrimitive != HbStyle::PT_IconItem) {
            break;
        }
        return d->mPrimitiveItem;

    case HbStylePrivate::P_Label_text:
        if (d->mActivePrimitive != HbStyle::PT_TextItem) {
            break;
        }
        return d->mPrimitiveItem;

    case HbStylePrivate::P_Label_richtext:
        if (d->mActivePrimitive != HbStyle::PT_RichTextItem) {
            break;
        }
        return d->mPrimitiveItem;

    default:
        return HbWidget::primitive(primitive);
    }
    return 0;
}

/*!
    Initializes \a option with the values from this HbLabel.
    HbStyleOptionLabel is used by HbStyle to perform changes in appearance
    of this label.
    \param option - the object in which the label's style options are set.
 */
void HbLabel::initStyleOption(HbStyleOptionLabel *option) const
{
    Q_D(const HbLabel);

    HbWidget::initStyleOption(option);

    option->alignment = d->mAlignment;

    if (d->mText.isSet()) {
        option->text = d->mText;
        option->elideMode = d->mElideMode;
        option->textWrapMode = d->mTextWrapping;
        option->color = d->mColor;
        option->maximumLines = d->mMaxLines;
    }

    if (d->mIcon.isSet()) {
        option->icon = d->mIcon;
        option->aspectRatioMode = d->mAspectRatioMode;
    }
}

QSizeF HbLabel::sizeHint ( Qt::SizeHint which, const QSizeF & constraint ) const
{
    if (isEmpty() && which!=Qt::MaximumSize) {
        return QSizeF(0,0);
    }
    return HbWidget::sizeHint(which,constraint);
}

/*!
    Slot to be called when the style primitives need to be updated.
    This function does not initiate redrawing this widget.

    \reimp
 */
void HbLabel::updatePrimitives()
{
    Q_D(HbLabel);
    d->updatePrimitives();
    HbWidget::updatePrimitives();
}

int HbLabel::type() const
{
    return HbLabel::Type;
}

/*!
    Plain text accessor. Returns empty string if not set.
 */
QString HbLabel::plainText() const
{
    Q_D(const HbLabel);
    if (d->mActivePrimitive == HbStyle::PT_TextItem) {
        return d->mText;
    }
    return QString();
}

/*!
    Rich text text accessor. Returns empty string if not set.
 */
QString HbLabel::html() const
{
    Q_D(const HbLabel);
    if (d->mActivePrimitive == HbStyle::PT_RichTextItem) {
        return d->mText;
    }
    return QString();
}

/*!
    Set color of text. If color is set to invalid value theme color is used.
 */
void HbLabel::setTextColor( const QColor &textColor )
{
    Q_D(HbLabel);
    if (d->mColor!=textColor) {
        d->mColor=textColor;
        if (d->mText.isSet()) {
            updatePrimitives();
        }
    }
}

/*!
    Returns color of text or invalid value if theme color is used.
 */
QColor HbLabel::textColor() const
{
    Q_D(const HbLabel);

    if (d->mColor.isSet()) {
        return d->mColor;
    } else {
        return QColor();
    }
}

/*!
    If plain text is used (\sa setPlainText) this will set maximum number of lines
    to be visible in label.
    Zero or negative value disables the feature.

    \sa maximumLines()
 */
void HbLabel::setMaximumLines(int maxLines)
{
    Q_D(HbLabel);

    maxLines = qMax(maxLines, 0);
    if (d->mMaxLines != maxLines) {
        d->mMaxLines = maxLines;
        if (d->mActivePrimitive == HbStyle::PT_TextItem) {
            updatePrimitives();
        }
    }
}

/*!
    Returns maximum number of lines which can be visible in label when
    plain text is used (\sa setPlainText).
    Zero value means that there is no limitation.

    \sa setMaximumLines(int)
 */
int HbLabel::maximumLines() const
{
    Q_D(const HbLabel);

    if (d->mMaxLines.isSet()) {
        return d->mMaxLines;
    } else {
        return 0;
    }
}

#include "moc_hblabel.cpp"
