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

#include "hbiconitem.h"
#include "hbiconitem_p.h"
#include "hbiconanimator.h"
#include "hbicon_p.h"
#include "hboogmwatcher_p.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

/*!
  @stable
  @hbcore
  \class HbIconItem
  \brief HbIconItem displays an HbIcon instance.


  HbIconItem derives from HbWidgetBase and so can be added to a layout.
  Use HbIconItem when you need your icon's position and size to be managed by a layout.

  See HbIcon for a list of supported icon graphics file types.

  Example of how to create an HbIconItem and directly set its position and size
  (the HbIconItem is not added to a layout in this case):
  \code
  //Create new icon, set position and size for it
  HbIconItem *icon = new HbIconItem("qtg_large_phonebook", this);
  icon->setPos(10,150);
  icon->setSize(QSizeF(200,20));
  \endcode

  Example of how to create an HbIconItem and set its size to be same as its defaultSize.
  (the HbIconItem is not added to a layout in this case):
  \code
  //Create new icon, set position and size for it
  HbIconItem *icon = new HbIconItem("qtg_large_phonebook", this);
  icon->setPos(10,150);
  icon->setSize(icon->defaultSize());
  \endcode
  
  Example of how to add HbIconItem to a layout.
  \code
  HbButton *button = new HbButton("Button 1");
  HbIconItem *icon = new HbIconItem("qtg_large_phonebook");
  QGraphicsGridLayout *layout = new QGraphicsGridLayout();

  //No need of setting the position and size. In this case
  //it will be decided by layout.
  layout->addItem(icon, 0, 0);
  layout->addItem(button, 0, 1);
  .
  .
  setLayout(layout);
  \endcode
*/

/*!
    \var static const QIcon::Mode defaultMode
    Default mode of an icon item.
*/

/*!
    \var static const QIcon::State defaultState
    Default state of an icon item.
*/

/*!
    \var static const Qt::AspectRatioMode defaultAspectRatioMode
    Default aspect ratio mode of an icon item.
*/

/*!
    \var static const Qt::Alignment defaultAlignment
    Default alignment of an icon item.
*/

//Initialize the default values
const QIcon::Mode HbIconItem::defaultMode = QIcon::Normal;
const QIcon::State HbIconItem::defaultState = QIcon::Off;
const Qt::AspectRatioMode HbIconItem::defaultAspectRatioMode = Qt::KeepAspectRatio;
const Qt::Alignment HbIconItem::defaultAlignment = Qt::AlignCenter;

bool HbIconItemPrivate::outlinesEnabled = false;

HbIconItemPrivate::HbIconItemPrivate(const HbIcon &icon) :
        mIcon(icon),
        mAnimator(),
        mAlignment(HbIconItem::defaultAlignment),
        mAspectRatioMode(HbIconItem::defaultAspectRatioMode),
        mState(HbIconItem::defaultState),
        mMode(HbIconItem::defaultMode)
{
    q_ptr = 0;
}

HbIconItemPrivate::~HbIconItemPrivate ()
{
}

void HbIconItemPrivate::updateIconItem()
{
    Q_Q( HbIconItem );
    const QRectF boundingRect = q->rect();
    if (!boundingRect.size().isEmpty()) {
        mIconRect = boundingRect;
        mIcon.setSize(mIconRect.size());
        mAnimator.setIcon(mIcon);
        q->update();
    }
}

/*!
 Constructs a new HbIconItem with \a iconName and \a parent.
 \param iconName the name of the icon.
 See HbIcon for a description of how the icon name and the icon image filename are related.
 \param parent the graphics item parent of this icon.
 */
HbIconItem::HbIconItem(const QString &iconName, QGraphicsItem *parent) :
    HbWidgetBase(*new HbIconItemPrivate(iconName), parent)
{
    Q_D( HbIconItem );
    d->q_ptr = this;
    // Set this graphics item to be updated on icon animations
    d->mAnimator.setGraphicsItem(this);
    HbOogmWatcher::instance()->registerIconItem(this);
}

/*!
 Constructs a new HbIconItem from HbIcon.
 \param icon contains the icon object that this HbIconItem displays on screen.
 \param parent the graphics item parent of this icon.
 */
HbIconItem::HbIconItem(const HbIcon &icon, QGraphicsItem *parent) :
    HbWidgetBase(*new HbIconItemPrivate(icon), parent)
{
    Q_D( HbIconItem );
    d->q_ptr = this;
    // Set this graphics item to be updated on icon animations
    d->mAnimator.setGraphicsItem(this);
    HbOogmWatcher::instance()->registerIconItem(this);
}

/*!
 Constructs a new HbIconItem from HbIcon.
 \param parent the graphics item parent of this HbIconItem.
 */
HbIconItem::HbIconItem(QGraphicsItem *parent) :
    HbWidgetBase(*new HbIconItemPrivate(QString()), parent)
{
    Q_D( HbIconItem );
    d->q_ptr = this;
    // Set this graphics item to be updated on icon animations
    d->mAnimator.setGraphicsItem(this);
    HbOogmWatcher::instance()->registerIconItem(this);
}


/*!
 \internal
 */
HbIconItem::HbIconItem(HbIconItemPrivate &dd, QGraphicsItem * parent) :
    HbWidgetBase(dd, parent)
{
    // Set this graphics item to be updated on icon animations
    dd.mAnimator.setGraphicsItem(this);
    HbOogmWatcher::instance()->registerIconItem(this);
}

/*!
 Destructor.
 */
HbIconItem::~HbIconItem()
{
    HbOogmWatcher::instance()->unregisterIconItem(this);
}

/*!
 \return the HbIcon associated with this HbIconItem instance.

 \sa setIcon
 */
HbIcon HbIconItem::icon() const
{
    Q_D(const HbIconItem );
    return d->mIcon;
}

/*!
 Sets the HbIcon instance associated with this HbIconItem.

 \param icon the HbIcon instance that this HbIconItem displays.

 When \a takeIconSettings is false, the following settings are not
 taken (ignored) from \a icon: flags, color, mirroring mode.  Instead,
 the previous values set via HbIconItem's setters are used.

 \sa icon
*/
void HbIconItem::setIcon(const HbIcon &icon, bool takeIconSettings)
{
    Q_D(HbIconItem );
    if (d->mIcon != icon) {
        if (takeIconSettings) {
            d->mIcon = icon;
        } else {
            // Must preserve settings like flags, colors, etc. In this case the
            // settings made previously through HbIconItem take precedence over the
            // newly set HbIcon's own settings.
            HbIcon::Flags prevFlags = d->mIcon.flags();
            QColor prevColor = d->mIcon.color();
            HbIcon::MirroringMode prevMirroringMode = d->mIcon.mirroringMode();
            d->mIcon = icon;
            d->mIcon.setFlags(prevFlags);
            d->mIcon.setColor(prevColor);
            d->mIcon.setMirroringMode(prevMirroringMode);
        }
        d->updateIconItem();
    }
}

/*!
 Sets the size for icon.
 This function needs to be called only when image item
 is not added in layout. If its added to layout then layout
 will decide the position and size which will then be set
 in setGeometry() function.

 This is a convenience function for setGeometry(QRectF(pos(), size))

 \sa HbIcon::setSize(), HbIconItem::defaultSize()
 */
void HbIconItem::setSize(const QSizeF &size)
{
    setGeometry(QRectF(pos(), size));
}

/*!
 Sets the alignment for the icon.

 If this method is not called, the icon uses its default alignment.
 The default alignment can be defined in the icon definition file, otherwise it defaults to Qt::AlignCenter.
 \param alignment the new icon alignment.gnCenter.

 \sa alignment
 */
void HbIconItem::setAlignment(Qt::Alignment alignment)
{
    Q_D(HbIconItem );
    if (d->mAlignment != alignment) {
        d->mAlignment = alignment;
        update();
    }
}

/*!
 Sets the aspect ratio mode for the icon.

 If this method is not called, the icon uses its default aspect ratio mode.
 The default mode can be defined in the icon definition file, otherwise it defaults to Qt::KeepAspectRatio.
 \param aspectRatio the new icon aspect ratio.

 \sa aspectRatioMode
 */
void HbIconItem::setAspectRatioMode(Qt::AspectRatioMode aspectRatioMode)
{
    Q_D(HbIconItem );
    d->setApiProtectionFlag(HbWidgetBasePrivate::AC_IconAspectRatioMode, true);
    if (d->mAspectRatioMode != aspectRatioMode) {
        d->mAspectRatioMode = aspectRatioMode;
        update();
    }
}

/*!
 Sets the QIcon mode for the icon. An icon can show different images for different modes.
 If this method is not called, the icon uses the default mode which is QIcon::Normal.

 \param mode the new icon mode.
 \warning Currently this method makes use of pixmap() routine in case of NVG icons. 
 pixmap() slows down the hardware accelerated rendering.
 \sa mode
 */
void HbIconItem::setMode(QIcon::Mode mode)
{
    Q_D(HbIconItem );
    if (d->mMode != mode) {
        d->mMode = mode;
        update();
    }
}

/*!
 Sets the QIcon state for the icon Item. An icon can show different images for different modes.
 If this method is not called, the icon uses its default state which is QIcon::Off.

 \param state the new icon state.

 \sa state
 */
void HbIconItem::setState(QIcon::State state)
{
    Q_D(HbIconItem );
    if (d->mState != state) {
        d->mState = state;
        update();
    }
}

/*!
 Sets the new icon name for HbIconItem. If iconName is already set in
 HbIconItem then it will be replaced with this new iconName.
 \param iconName the new icon name.

 \sa HbIcon::setIconName()
 \sa iconName
 */
void HbIconItem::setIconName(const QString &iconName)
{
    Q_D(HbIconItem );
    if (d->mIcon.iconName() != iconName) {
        d->mIcon.setIconName(iconName);
        d->updateIconItem();
    }
}

/*!
 Sets the icon \a flags for the icon item.
 \param flags new icon flags.

 \sa HbIcon::setFlags()
 \sa flags
 */
void HbIconItem::setFlags(HbIcon::Flags flags)
{
    Q_D(HbIconItem);
    if (d->mIcon.flags() != flags) {
        d->mIcon.setFlags(flags);
        if (d->mIconRect.isValid())
            d->mAnimator.setIcon(d->mIcon);
        update();
    }
}

/*!
 Sets the mirroring \a mode for the icon item.
 Mirroring mode controls how to icon is displayed in right to left layouts.
 \param mode the new mirroring mode.

 \sa HbIcon::setMirroringMode()
 \sa mirroringMode
 */
void HbIconItem::setMirroringMode(HbIcon::MirroringMode mode)
{
    Q_D(HbIconItem);
    if (d->mIcon.mirroringMode() != mode) {
        d->mIcon.setMirroringMode(mode);
        if (d->mIconRect.isValid())
            d->mAnimator.setIcon(d->mIcon);
        update();
    }
}

/*!
 Sets the \a brush for the icon item.
 The brush defines a fill pattern that is used to paint the icon.
 \param brush the brush to be used to paint this icon.
 \sa brush
*/
void HbIconItem::setBrush(const QBrush &brush)
{
    Q_D(HbIconItem);
    d->setApiProtectionFlag(HbWidgetBasePrivate::AC_IconBrush, true);
    if (d->mBrush != brush) {
        d->mBrush = brush;
        update();
    }
}

/*!
 Sets the new icon name for this HbIconItem for \a mode and \a state. If iconName is already set in
 HbIconItem then it will be replaced with this new iconName.
 \param  iconName the icon name.
 \param  mode  mode of the  icon.
 \param  state state of the icon.


 \sa HbIcon::setIconName()
 \sa iconName
 */
void HbIconItem::setIconName(const QString &iconName, QIcon::Mode mode, QIcon::State state)
{
    Q_D( HbIconItem );
    d->mIcon.setIconName(iconName, mode, state);
    d->updateIconItem();
}

/*!
  Sets the new icon color for the HbIconItem. Note that the color is just
  stored but not actually used if the HbIcon::Colorized flag is not set and the
  icon is not a mono icon from the theme.

 \param  color to be set.
 \sa HbIconItem::color(), HbIcon::setColor()
*/
void HbIconItem::setColor(const QColor &color)
{
    Q_D( HbIconItem );
    if (d->mIcon.color() != color) {
        d->mIcon.setColor(color);
        if (d->mIconRect.isValid())
            d->mAnimator.setIcon(d->mIcon);
        update();
    }
}

/*!
* Returns the color of the HbIconItem.
* \sa HbIconItem::setColor()
*/
QColor HbIconItem::color() const
{
    Q_D(const  HbIconItem );
    return d->mIcon.color();
}

/*!
 Returns the default size of the icon.
 
 For raster images this is the original size of the image.

 \sa HbIcon::defaultSize()
 */
QSizeF HbIconItem::defaultSize() const
{
    Q_D(const HbIconItem );
    return d->mIcon.defaultSize();
}

/*!
 \return the size of this HbIconItem
 \sa HbIcon::size()
 \sa HbIconItem::defaultSize()
 */
QSizeF HbIconItem::iconItemSize() const
{
    return boundingRect().size();
}

/*!
 \return the icon's aspect ratio.

 \sa setAspectRatioMode
 */
Qt::AspectRatioMode HbIconItem::aspectRatioMode() const
{
    Q_D(const HbIconItem );
    return d->mAspectRatioMode;
}

/*!
 \return the icon's alignment.

 \sa setAlignment
 */
Qt::Alignment HbIconItem::alignment() const
{
    Q_D(const HbIconItem );
    return d->mAlignment;
}

/*!
 \return the icon's mode.

 \sa setMode
 */
QIcon::Mode HbIconItem::mode() const
{
    Q_D(const HbIconItem );
    return d->mMode;
}

/*!
 \return the icon's state.

 \sa setState
 */
QIcon::State HbIconItem::state() const
{
    Q_D(const HbIconItem );
    return d->mState;
}

/*!
 \return the icon's name.

 \sa HbIcon::iconName()
 \sa setIconName
 */
QString HbIconItem::iconName() const
{
    Q_D(const HbIconItem );
    return d->mIcon.iconName();
}

/*!
 \return the icon name for a specific mode and state.
 \param mode the mode of the icon.
 \param state the state of the icon.

 \sa HbIcon::iconName()
 \sa setIconName
 */
QString HbIconItem::iconName(QIcon::Mode mode, QIcon::State state) const
{
    Q_D(const HbIconItem );
    return d->mIcon.iconName(mode, state);
}

/*!
 \return the HbIcon flags for the icon displayed by this HbIconItem.

 \sa HbIcon::flags()
 \sa setFlags
 */
HbIcon::Flags HbIconItem::flags() const
{
    Q_D(const HbIconItem);
    return d->mIcon.flags();
}

/*!
 \return the icon's mirroring mode.

 \sa HbIcon::mirroringMode()
 \sa setMirroringMode
 */
HbIcon::MirroringMode HbIconItem::mirroringMode() const
{
    Q_D(const HbIconItem);
    return d->mIcon.mirroringMode();
}

/*!
 \return the brush for this HbIconItem, or an empty brush if no brush has been set.

 \sa setBrush
 */
QBrush HbIconItem::brush() const
{
    Q_D(const HbIconItem);
    return d->mBrush;
}

/*!
 * Refer HbIcon::isNull()
 */
bool HbIconItem::isNull() const
{
    Q_D(const HbIconItem );
    return d->mIcon.isNull();
}

/*!
 Paints the graphics item.
 \note Parameters \a option and \a widget are unused.
 \reimp
 */
void HbIconItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)
    Q_D(HbIconItem);
    const QRectF rect(boundingRect());
    if(!rect.isEmpty()){
        if (d->mIconRect != rect) {
            d->mIconRect = rect;
            d->mIcon.setSize(d->mIconRect.size());
            d->mAnimator.setIcon(d->mIcon);
        }
        painter->fillRect(rect, d->mBrush);
        d->mAnimator.paint(painter, rect,
                           d->mAspectRatioMode, d->mAlignment,
                           d->mMode, d->mState);
    }
    if (HbIconItemPrivate::outlinesEnabled) {
        painter->setBrush(QBrush(QColor(0, 255, 0, 50)));
        painter->drawRect(contentsRect()); 
    }
}

/*!
    \reimp
 */
void HbIconItem::changeEvent(QEvent *event)
{
    switch (event->type()) {
        // suppress events (optimize)
    case QEvent::FontChange:
    case QEvent::PaletteChange:
    case QEvent::ParentChange:
        break;
    case QEvent::StyleChange: // flow through
    default:
        HbWidgetBase::changeEvent(event);
        break;
    }
}

 /*!
    \reimp
 */
QVariant HbIconItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    Q_D(HbIconItem);
    if (QGraphicsItem::ItemEnabledHasChanged==change) {
        d->mMode = value.toBool() ? QIcon::Normal : QIcon::Disabled;
    }

    return HbWidgetBase::itemChange(change, value);
}

/*!
 Returns the icon animator object of the icon item.

 If the icon is not animated, the animator does not do anything.
 For more information about icon animations, see also class HbIcon.
*/
HbIconAnimator &HbIconItem::animator()
{
    Q_D(HbIconItem);
    return d->mAnimator;
}

/*!
  Requests clearStoredIconContent() for the currently set icon.
 */
void HbIconItemPrivate::clearStoredIconContent()
{
    HbIconPrivate::d_ptr(&mIcon)->clearStoredIconContent();
}

// end of file
