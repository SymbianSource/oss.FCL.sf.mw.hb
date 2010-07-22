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
            HbLabelPrivate ();
    ~HbLabelPrivate ();

    void clearAll();

    void setText(const QString &text, HbStyle::Primitive primitiveId);
    void setIcon(const HbIcon &icon);

    void updatePrimitives ();
    void createPrimitives ();

    //shared between icon and text
    Qt::Alignment mAlignment;

    // text section
    QString mText;
    Qt::TextElideMode mElideMode;
    Hb::TextWrapping mTextWrapping;
    QColor mColor;

    // icon section
    HbIcon mIcon;
    Qt::AspectRatioMode mAspectRatioMode;

    // primitive handling
    QGraphicsItem *mPrimitiveItem;
    HbStyle::Primitive mActivePrimitive;
};

HbLabelPrivate::HbLabelPrivate() :
        HbWidgetPrivate(),
        mAlignment(Qt::AlignLeft | Qt::AlignVCenter),
        mText(QString()),
        mElideMode(Qt::ElideRight),
        mTextWrapping(Hb::TextNoWrap),
        mAspectRatioMode(Qt::KeepAspectRatio),
        mPrimitiveItem(0),
        mActivePrimitive(HbStyle::P_None)
{
}

void HbLabelPrivate::clearAll()
{
    if (mPrimitiveItem) {
        delete mPrimitiveItem;
        mPrimitiveItem = 0;
        mActivePrimitive = HbStyle::P_None;
    }

    mText.clear();
    mIcon.clear();
}

void HbLabelPrivate::setText(const QString &text, HbStyle::Primitive primitiveId)
{
    Q_Q(HbLabel);

    if (text.isNull()) {
        clearAll();
        return;
    }

    if (mActivePrimitive != primitiveId) {
        clearAll();
    }

    if (mText != text || mText.isNull()) {
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

    if (mActivePrimitive != HbStyle::P_Label_icon) {
        clearAll();
    }

    if (mIcon != icon) {
        mIcon = icon;

        if (mActivePrimitive != HbStyle::P_Label_icon) {
            mActivePrimitive = HbStyle::P_Label_icon;
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

    if (mActivePrimitive != HbStyle::P_None) {
        mPrimitiveItem = q->style()->createPrimitive(mActivePrimitive, q);
    }
}

void HbLabelPrivate::updatePrimitives()
{
    Q_Q(HbLabel);

    if (mActivePrimitive != HbStyle::P_None) {
        Q_ASSERT(mActivePrimitive == HbStyle::P_Label_icon
                 || mActivePrimitive == HbStyle::P_Label_richtext
                 || mActivePrimitive == HbStyle::P_Label_text);

        HbStyleOptionLabel option;
        q->initStyleOption(&option);
        q->style()->updatePrimitive(mPrimitiveItem, mActivePrimitive, &option);
    }
}

/*!
    Constructs the label with a given \a parent.
    \param parent - the parent graphics item.
 */
HbLabel::HbLabel(QGraphicsItem *parent) :
        HbWidget(*new HbLabelPrivate, parent)
{
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
}

/*!
    \internal
 */
HbLabel::HbLabel(HbLabelPrivate &dd, QGraphicsItem * parent) :
        HbWidget(dd, parent)
{
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
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
        if (!d->mText.isNull()) {
            updatePrimitives();
        }
    }
}

/*!
    Default elide mode is Qt::ElideRight.
    \return the elide mode of the text.

    \sa HbLabel::setElideMode()
 */
Qt::TextElideMode HbLabel::elideMode() const
{
    Q_D(const HbLabel);
    return d->mElideMode;
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
        if (!d->mText.isNull()) {
            updatePrimitives();
        }
    }
}

/*!
    \return the label's current text wrapping mode.
    Default value is NoWrap.

    \sa setTextWrapping()
 */
Hb::TextWrapping HbLabel::textWrapping() const
{
    Q_D(const HbLabel);
    return d->mTextWrapping;
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
    return d->mIcon;
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
        if (!d->mIcon.isNull()) {
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
    return d->mAspectRatioMode;
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
    d->setText(text, HbStyle::P_Label_text);
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
    d->setText(text, HbStyle::P_Label_richtext);
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
        if (d->mActivePrimitive!=HbStyle::P_None) {
            updatePrimitives();
        }
    }
}

/*!
    \return the alignment. Default alignment is 'Qt::AlignLeft | Qt::AlignVCenter'

    \sa HbLabel::setAlignment()
 */
Qt::Alignment HbLabel::alignment() const
{
    Q_D(const HbLabel);
    return d->mAlignment;
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
    return d->mActivePrimitive == HbStyle::P_None;
}

/*!

    \deprecated HbLabel::primitive(HbStyle::Primitive)
        is deprecated.

    Returns a pointer to the QGraphicsItem primitive used by this label.
    \param primitive - the type of graphics primitive required.
    HbLabel supports HbStyle::P_Label_text and HbStyle::P_Label_icon.
    \return the QGraphicsItem used by the label. It is 0 if type \a primitive not currently in use.
    It is also 0 if the text or icon object is empty.

    \reimp

    \sa isEmpty()
 */
QGraphicsItem * HbLabel::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbLabel);
    if (primitive == d->mActivePrimitive) {
        return d->mPrimitiveItem;
    }
    return HbWidget::primitive(primitive);
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

    if (!d->mText.isNull()) {
        option->text = d->mText;
        option->elideMode = d->mElideMode;
        option->textWrapMode = d->mTextWrapping;
        option->color = d->mColor;
    }

    if (!d->mIcon.isNull()) {
        option->icon = d->mIcon;
        option->aspectRatioMode = d->mAspectRatioMode;
    }
}

QSizeF HbLabel::sizeHint ( Qt::SizeHint which, const QSizeF & constraint ) const
{
    if (isEmpty()) {
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
    if (d->mActivePrimitive == HbStyle::P_Label_text) {
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
    if (d->mActivePrimitive == HbStyle::P_Label_richtext) {
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
        if (!d->mText.isNull()) {
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
    return d->mColor;
}

#include "moc_hblabel.cpp"
