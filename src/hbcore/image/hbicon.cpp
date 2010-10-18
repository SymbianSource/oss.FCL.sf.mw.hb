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

#include "hbicon.h"
#include "hbicon_p.h"
#include "hbiconanimator.h"
#include "hbbadgeicon_p.h"
#include "hbiconloader_p.h"

#include <QObject>
#include <QPainter>
#include <QVariant>

/*!
    @stable
    @hbcore
    \class HbIcon
    \brief HbIcon is a paintable instance of an icon. The term icon means any piece of
    graphics that can be drawn. This class is not intended to be derived from.

    For more information on image and icon classes and general notes about
    graphics memory usage see \ref graph_mem_usage "this page".

    This class implements copy-on-write semantics, so copy constructor and assignment operator
    only perform shallow copy and are fast. A deep copy is done if the icon state is changed.

    Supported formats and file extensions are the following.

    - SVG-T (.svg)
    - Externalized QPicture (.qpic)
    - PNG (.png)
    - XPM (.xpm)
    - JPG (.jpg)
    - Frame animations (.axml)

    An icon is defined by its name. The filename should be same as the logical name of the icon,
    accompanied with the file extension.

    E.g. logical name of the icon is "frame". Assuming the icon file is in SVG-T format,
    the filename should be "frame.svg".

    The icon name can be either a logical icon name that is fetched from the theme
    or an absolute filename. Logical icon names should not include a filename extension.

    An absolute icon filename must include full path and filename extension.

    The icon can be resized with method HbIcon::setSize.
    When resizing, by default the aspect ratio of the icon is preserved.
    To change this, define the aspect ratio mode parameter in the method HbIcon::paint().

    An icon itself can be a combination of existing icons; in this case the main icon is
    "badged" with smaller icons to form a distinct icon. You can badge icons in this way
    with the HbIcon::addBadge method, specifying the location where the badge should be drawn
    and a second HbIcon to draw at the indicated location. To prevent possible recursion ,
    you can't badge an icon with a badge icon that is itself badged, however.

    Scaled instances of the icons are shared by the framework
    to decrease memory consumption.

    A brief example of how to create an icon and use it.

    \snippet{clock/clockworldview.cpp,1}

    This class provides built-in support for icon animations.
    Supported formats are the following.

    - GIF (.gif)
    - MNG (.mng)

    The formats mentioned above contain all the animation information inside a single file.
    They are pixel formats, so the quality of resizing the animations is not good.

    Icon animations can also be constructed from separate frame icons.
    The frames are defined in separate files, which can be in any format supported by HbIcon.
    The animation frame information for the icon is defined using class HbIconAnimationManager.
    The animated icon can be then constructed normally with its name.

    As default the icon animation is started automatically when the icon is painted for the first time.
    This can be disabled with flag HbIcon::NoAutoStartAnimation.

    An example of how to define a frame-by-frame animation and construct an icon using it.

    \dontinclude ultimatecodesnippet/ultimatecodesnippet.cpp
    \skip frame-by-frame
    \until }
*/

/*!
  \enum HbIcon::Flag

  \b DoNotCache \b (0x01) Icon pixmap is not cached in the system wide icon pixmap cache.
         Setting this flag improves cache performance for other icons, if this icon does
         not need to be cached for being able to load it faster next time.

  \b ResolutionCorrected \b (0x02) This flag is useful for making icon sizes automatically
         adapt to different screen resolutions (DPI values) and zoom factors used in the user
         interface. The current display resolution (DPI value) and zoom factor are taken
         in consideration when defining the default size of the icon. This affects on what default
         size is reported for the icon and also in which size the icon is rendered if its size is
         not set explicitly. The default DPI value is 144 and the default zoom factor 1.0.
         When this flag is set, the corrected default size of icon is defined as:

         Corrected default size = original default size * current DPI value / default DPI
         value * current zoom factor

         \note Currently this flag has an effect on vector icons only.
         \note If the icon size is set explicitly this flag has no effect.

  \b NoAutoStartAnimation \b (0x04) This flag disables automatic starting of the icon animation
         when the icon is painted for the first time. The client can then control the animation with
         startAnimation() and stopAnimation() methods in class HbIconAnimator.
         If the icon is not animated this flag has no effect.

  \b Colorized \b (0x08) This flag enables the usage of the color that is set via
         setColor(). If this flag is not set then the color that is set with setColor() is
         ignored, unless the icon refers some theme graphics. For icons coming from the
         theme this flag can be left unset because they will still be colorized properly
         if the logical icon name indicates that the icon is a mono icon. Therefore this
         flag is only relevant for icons loaded from regular files.

  \b NonThemeable \b (0x10) This flag indicates that the icon is not themeable and thus
         there is no need to handle theme and layout direction changes for this icon.
         This is merely an optimization flag and must be set directly after creating the
         HbIcon instance (before the first paint of the icon) to have any performance
         benefits.

  \b KeepDefaultQIconSize \b (0x20) This flag disables all scaling for icons constructed
         from QIcon. It has no effect on regular, native HbIcons. It only affects the
         painting: The size of the HbIcon will remain whatever was set via setSize(), but
         only a part of that area will be painted. This is similar to
         HbIconItem::setIconScaling(false) but in that case the underlying icon's size
         would also be changed to match the default size, while here size() remains
         unaffected.

*/

/*!
  \enum HbIcon::MirroringMode

  \b Default This is the default mirroring mode. The icon is mirrored in right-to-left
         layout if its name is listed in the icon framework's configuration list of mirrored icons.

  \b Forced Forces horizontal mirroring of the icon. As default, the system
         automatically mirrors the icon pixmap based on a predefined configuration,
         when required for the purpose of a mirrored layout.
         This mirroring mode explicitly forces the icon pixmap to be mirrored,
         regardless of the layout direction.

  \b Prevented Prevents horizontal mirroring of the icon. As default, the system
         automatically mirrors the icon pixmap based on a predefined configuration,
         when required for the purpose of a mirrored layout.
         This mirroring mode explicitly prevents the icon pixmap from being mirrored,
         regardless of the layout direction.

  \b LayoutDirection This mirroring mode automatically mirrors the icon pixmap
         in a right-to-left layout.
*/

// Must be initialized dynamically because QIcon cannot be constructed
// when static variables are constructed.
static HbIconPrivate *shared_null = 0;

static const int iconMetaType = qRegisterMetaType<HbIcon>();

/*!
This constructor is used for shared_null instance only
\internal
*/
HbIconPrivate::HbIconPrivate() :
    engine(new HbIconEngine(QString())),
    qicon(engine),
    badgeInfo(0),
    mQIconFlags(0),
    mQIconPixmapMode(QIcon::Normal),
    mQIconPixmapState(QIcon::Off)
{
    ref.ref(); // Need to do extra ref so the shared null does not get destructed
}

/*!
\internal
*/
HbIconPrivate::HbIconPrivate(const QIcon &qicon) :
    engine(0),
    qicon(qicon),
    badgeInfo(0),
    mQIconFlags(0),
    mQIconPixmapMode(QIcon::Normal),
    mQIconPixmapState(QIcon::Off)
{
}

/*!
\internal
*/
HbIconPrivate::HbIconPrivate(const QString &iconName) :
    engine(new HbIconEngine(iconName)),
    qicon(engine),
    badgeInfo(0),
    mQIconFlags(0),
    mQIconPixmapMode(QIcon::Normal),
    mQIconPixmapState(QIcon::Off)
{
}

/*!
\internal
*/
HbIconPrivate::HbIconPrivate(const HbIconPrivate &other) :
    QSharedData(other),
    size(other.size),
    engine(0),
    qicon(),
    badgeInfo(0),
    mQIconFlags(other.mQIconFlags),
    mQIconPixmap(other.mQIconPixmap),
    mQIconPixmapMode(other.mQIconPixmapMode),
    mQIconPixmapState(other.mQIconPixmapState)
{
    if (other.engine) {
        engine = new HbIconEngine(*other.engine);
        // Have to instantiate a temporary QIcon because
        // QIcon's copy constructor shares the engine.
        QIcon temp(engine);
        qicon = temp;
    } else {
        // Copy constructed from qicon - so just copy the qicon.
        qicon = other.qicon;
        if (other.badgeInfo) {
            badgeInfo = new HbBadgeIcon(*other.badgeInfo);
        }
    }
}

/*!
\internal
*/
HbIconPrivate::~HbIconPrivate()
{
    delete badgeInfo;

    if (engine) {
        engine->setAnimator(0);
    }
    // engine is deleted by QIcon destructor.
}

/*!
\internal
*/
void HbIconPrivate::clear()
{
    if (badgeInfo) {
        badgeInfo->removeAllBadges();
    }
    if (engine) {
        engine->clear();
        size = QSizeF();
    } else {
        qicon = QIcon();
    }
}

/*!
\internal
*/
void HbIconPrivate::removeAllBadges()
{
    if (engine) {
        engine->removeAllBadges();
    } else if (badgeInfo) {
        badgeInfo->removeAllBadges();
    }
}

/*!
\internal
*/
bool HbIconPrivate::isBadged() const
{
    if (engine) {
        return engine->isBadged();
    } else if (badgeInfo) {
        return badgeInfo->isBadged();
    }
    return false;
}

/*!
\internal
*/
void HbIconPrivate::setThemedColor(const QColor &color)
{
    if (engine && color != engine->themedColor()) {
        engine->setThemedColor(color);
    }
}

/*!
\internal
*/
QColor HbIconPrivate::themedColor() const
{
    return engine ? engine->themedColor() : QColor();
}

/*!
\internal
*/
QDataStream &operator>>(QDataStream &stream, HbIconPrivate &icon)
{
    stream >> icon.size;
    qptrdiff enginePtr = 0;
    stream >> enginePtr;
    if (enginePtr) {
        icon.engine = new HbIconEngine(stream);
    } else {
        stream >> icon.qicon;
    }

    return stream;
}

/*!
\internal
*/
QDataStream &operator<<(QDataStream &stream, const HbIconPrivate &icon)
{
    stream << icon.size;
    // Put the engine pointer in stream to see if it exists
    stream << (qptrdiff)(icon.engine);
    // If the icon has engine, externalize it in the stream
    if (icon.engine) {
        icon.engine->externalize(stream);
    } else {
        stream << icon.qicon;
    }

    return stream;
}

/*!
\internal
*/
bool HbIconPrivate::addBadge(Qt::Alignment align,
                             const HbIcon &icon,
                             int z,
                             const QSizeF &sizeFactor,
                             Qt::AspectRatioMode aspectRatio)
{
    if (icon.isBadged()) {
        return false;
    } else if (engine) {
        engine->addBadge(align, icon, z, sizeFactor, aspectRatio);
    } else {
        if (!badgeInfo) {
            badgeInfo = new HbBadgeIcon();
        }
        badgeInfo->addBadge(align, icon, z, sizeFactor, aspectRatio);
    }
    return true;
}

/*!
\internal
 */
bool HbIconPrivate::removeBadge(const HbIcon &badge)
{
    bool result = false;

    if (engine) {
        result = engine->removeBadge(badge);
    } else if (badgeInfo) {
        result = badgeInfo->removeBadge(badge);
    }

    return result;
}

/*!
  \internal
*/
void HbIconPrivate::setAsync(bool async, HbIconEngine::AsyncCallback callback, void *param)
{
    if (engine) {
        engine->setAsync(async, callback, param);
    }
}

/*!
  \internal
*/
bool HbIconPrivate::async() const
{
    if (engine) {
        return engine->async();
    }
    return false;
}

/*! Default constructor. If this constructor is used, the icon name needs to be set
* by calling HbIcon::setIconName.
*/
HbIcon::HbIcon()
{
    // Construct shared_null if not done yet.
    if (!shared_null) {
        shared_null = new HbIconPrivate;
    }
    d = shared_null;
}

/*! Constructs a new icon with the icon name \a iconName.
*/
HbIcon::HbIcon(const QString &iconName) : d( new HbIconPrivate(iconName) )
{
}

/*!
 * Constructs a new icon to be a copy of the given QIcon.  Due to the
 * limitations listed below, this constructor should be used only for
 * compatibility reasons if a QIcon instance needs to be passed as a parameter
 * to a method taking a HbIcon parameter.
 *
 * \note If this constructor is used, the following limitations apply:
 *
 * - HbIcon::defaultSize() may return QSizeF().
 * - HbIcon::paint() ignores the parameter aspectRatioMode and converts the given QRectF to QRect.
 * - HbIcon::iconName() returns empty string by default.
 * - HbIcon::pixmap() returns null pixmap.
 * - Colorization and mirroring support are not available.
 *
 * This method should only be used if absolute necessary, as this is not ideal
 * for hardware accelerated environment and there may be huge differences in
 * painting performance when compared to a native HbIcon.  Some advanced resource
 * management features are not available for such icons either.
 */
HbIcon::HbIcon(const QIcon &icon) : d( new HbIconPrivate(icon) )
{
}

/*!
* Copy constructs a new icon using the \a other icon.
* Copy-on-write semantics is used, so this only does a shallow copy.
*/
HbIcon::HbIcon(const HbIcon &other) :
    d(other.d)
{
}

/*!
* Assigns the \a other icon to this icon and returns a reference to
* this icon. Copy-on-write semantics is used, so this only does a shallow copy.
*/
HbIcon &HbIcon::operator=(const HbIcon &other)
{
    if (&other != this) {
        d = other.d;
    }
    return *this;
}

/*!
* Destroys the icon.
*/
HbIcon::~HbIcon()
{
}

/*!
* Returns true if either
* - Icon was not copy constructed from QIcon and it does not have the any icon names set.
* - Icon was copy constructed from QIcon and QIcon::isNull returns true.
* \note Even a non-null icon might not be able to create valid pixmaps,
* e.g. if the file does not exist or cannot be read.
*/
bool HbIcon::isNull() const
{
    if (d->engine) {
        return d->engine->isNull();
    } else {
        return d->qicon.isNull();
    }
}

/*!
* Clears the icon. After this it is same as a default constructed HbIcon.
* Calling this method is faster than assigning a default constructed HbIcon to this icon.
*/
void HbIcon::clear()
{
    // A NULL icon is always cleared - save some time not detaching from it
    if (d.constData() != shared_null) {
        d.detach();
        d->clear();
    }
}

/*!
* Indicates whether or not the icon is badged.
* returns true if this icon is bearing a badge.
*/
bool HbIcon::isBadged() const
{
    return d->isBadged();
}

/*!
* Returns the the default icon pixmap in state QIcon::Normal and mode QIcon::Off.
*
* \note This method should be used only if it is necessary to get hold of the pixel data
* of the pixmap. It slows down hardware accelerated rendering. In normal use cases, this
* method is not needed. HbIcon::size() can be used to retrieve the size of the icon
* and HbIcon::paint() to paint the icon.
*/
QPixmap HbIcon::pixmap()
{
    if (d->engine) {
        return d->engine->pixmap(d->size.toSize(), QIcon::Normal, QIcon::Off);
    }

    return QPixmap();
}

/*!
* Sets the new color that is used to colorize mono icons.
*
* \param  color to be set
*
* This setting will be ignored for regular files when the HbIcon::Colorized flag
* is not set.  This does not apply to theme elements, for them the color is
* always taken into account when the logical graphics name indicates that it is
* a mono icon.
*
* Note that if a widget defines a color for its icon primitive (as most standard
* widgets do) then the style will take care of colorizing with the correct color
* from the theme whenever the theme changes. Therefore mono icons shown in such
* widgets will automatically be colorized with a theme-specific color if the
* icon is either a mono icon coming from the theme or the icon has the
* HbIcon::Colorized flag set.
*
* However it is possible to override this theme-specific color with a custom one
* by calling this function.
*
* \sa HbIcon::color(), HbIcon::Colorized
*/
void HbIcon::setColor(const QColor &color)
{
    if (d->engine) {
        if (color != d->engine->color()) {
            d.detach();
            d->engine->setColor(color);
        }
    }
}

/*!
* brief  Returns the color of the Icon.
* \sa HbIcon::setColor()
*/
QColor HbIcon::color() const
{
    if (d->engine) {
        return d->engine->color();
    } else {
        return QColor();
    }
}

/*!
* Returns the name of the icon.
* This icon name is used if there is no name set separately for the specified icon mode and state.
* \sa HbIcon::setIconName()
*/
QString HbIcon::iconName() const
{
    if (d->engine) {
        return d->engine->iconName();
    } else {
        return QString();
    }
}

/*!
* Sets the name of the icon. It can be either a logical icon name that is fetched from the theme
* or an absolute filename. Logical icon names should not include a filename extension.
* This icon name is used if there is no name set separately for the specified icon mode and state.
* \sa HbIcon::iconName()
*/
void HbIcon::setIconName(const QString &iconName)
{
    if (d->engine && d->engine->iconName() != iconName) {
        // Icon was normal HbIcon and the name is changed.
        d.detach();
        d->engine->setIconName(iconName);
    } else if (!d->engine) {
        // Icon was earlier copy constructed from QIcon, but now its name is set,
        // so it becomes a 'real' HbIcon.
        d.detach();
        d->engine = new HbIconEngine(iconName);
        d->engine->setSize(d->size);
        // Have to instantiate a temporary QIcon because
        // QIcon's assignment operator shares the engine.
        QIcon temp(d->engine);
        d->qicon = temp;
        d->mQIconPixmap = QPixmap();
    }
    // Otherwise icon was normal HbIcon and the name is same as before, so do nothing.
}

/*!
* Returns the name of the icon in the specified \a mode and \a state.
* If there is no icon name set for the specified \a mode and \a state,
* the icon name set without these parameters is returned.
* \sa HbIcon::setIconName()
*/
QString HbIcon::iconName(QIcon::Mode mode, QIcon::State state) const
{
    QString ret;

    if (d->engine) {
        ret = d->engine->iconName(mode, state);
        if (ret.isEmpty()) {
            ret = d->engine->iconName();
        }
    }

    return ret;
}

/*!
* Sets the name of the icon in the specified \a mode and \a state.
* \note Passing an empty \a iconName removes the icon name definition specific to
* the given mode and state.
* \sa HbIcon::iconName()
*/
void HbIcon::setIconName(const QString &iconName, QIcon::Mode mode, QIcon::State state)
{
    if (d->engine && d->engine->iconName(mode, state) != iconName) {
        d.detach();
        d->engine->setIconName(iconName, mode, state);
    }
}

/*! Paints the icon in the given \a painter with the specified drawing parameters.
* \note If the constructor HbIcon::HbIcon(const QIcon &icon) is used, the parameter
* \a aspectRatioMode is ignored and Qt::KeepAspectRatio is used. Also in that case the icon
* is not scaled exactly to the given size but the best size match returned by the QIcon
* instance is used.
*/
void HbIcon::paint(QPainter *painter,
                   const QRectF &rect,
                   Qt::AspectRatioMode aspectRatioMode,
                   Qt::Alignment alignment,
                   QIcon::Mode mode,
                   QIcon::State state) const
{
    if (!rect.isEmpty() && d.constData() != shared_null) {
        if (d->engine) {
            d->engine->paint(painter, rect, aspectRatioMode, alignment, mode, state);
        } else {
            // This HbIcon was copy constructed from QIcon and
            // we cannot use HbIconEngine for painting.

            // Find out the size: It can be the size set via setSize(), or, in case of
            // KeepDefaultQIconSize, the first available size, or, if none are set, the
            // target area's size.
            QSizeF size = this->size();
            if (flags().testFlag(KeepDefaultQIconSize)) {
                QList<QSize> sizes = d->qicon.availableSizes();
                if (!sizes.isEmpty()) {
                    size = sizes.at(0);
                }
            } else {
                if (!size.isValid()) {
                    size = rect.size();
                }
            }

            // Get the pixmap with the needed size.
            QPixmap pixmap;
            QSize intSize = size.toSize();
            bool usingCached = false;
            if (d->mQIconPixmap.size() == intSize
                && mode == d->mQIconPixmapMode
                && state == d->mQIconPixmapState)
            {
                pixmap = d->mQIconPixmap;
                usingCached = true;
            } else {
                pixmap = d->qicon.pixmap(intSize, mode, state);
                d->mQIconPixmap = pixmap;
                d->mQIconPixmapMode = mode;
                d->mQIconPixmapState = state;
            }
            QSizeF pixmapSize = pixmap.size();

            // QIcon::pixmap() will not do upscaling, do it here if needed.
            if (!pixmap.isNull() && !usingCached && !flags().testFlag(KeepDefaultQIconSize)
                && (pixmapSize.width() < size.width() || pixmapSize.height() < size.height()))
            {
                // Native HbIcons are scaled using SmoothTransformation so use the same.
                pixmap = pixmap.scaled(size.toSize(), aspectRatioMode, Qt::SmoothTransformation);
                pixmapSize = pixmap.size();
            }

            // Adjust the alignment.
            QPointF topLeft = rect.topLeft();

            if (alignment & Qt::AlignRight) {
                topLeft.setX(rect.right() - pixmapSize.width());
            } else if (alignment & Qt::AlignHCenter) {
                topLeft.setX(topLeft.x() + (rect.width() - pixmapSize.width()) / 2);
            }

            if (alignment & Qt::AlignBottom) {
                topLeft.setY(rect.bottom() - pixmapSize.height());
            } else if (alignment & Qt::AlignVCenter) {
                topLeft.setY(topLeft.y() + (rect.height() - pixmapSize.height()) / 2);
            }

            // Draw.
            painter->drawPixmap(topLeft, pixmap, pixmap.rect());

            // Draw the badges on this icon.
            if (d->badgeInfo) {
                d->badgeInfo->paint(painter, rect, mode, state, false);
            }
        }
    }
}

/*! Returns the current size of the icon.
* \sa HbIcon::setSize(), HbIcon::defaultSize()
*/
QSizeF HbIcon::size() const
{
    if (flags().testFlag(HbIcon::ResolutionCorrected)) {
        if (d->engine) {
            d->size = d->engine->size();
        }
        if (d->size.isValid()) {
            return d->size;
        } else {
            QSizeF defSize(defaultSize());
            return defSize;
        }
    } else if (d->size.isValid()) {
        return d->size;
    }
    return defaultSize();
}

/*! Returns the default size of the icon.
*/
QSizeF HbIcon::defaultSize() const
{
    // Default constructed icon?
    if (d.constData() == shared_null) {
        return QSizeF();
    }

    // Do not cache default size in this class,
    // because it may change when icon definitions change.
    else if (d->engine) {
        return d->engine->defaultSize();
    }
    // Constructed from QIcon
    else {
        QList<QSize> sizes = d->qicon.availableSizes();
        if (sizes.count()) {
            return QSizeF(sizes.at(0));
        } else {
            return QSizeF();
        }
    }
}

/*! Sets the \a height of the icon. Its width is computed using the aspect ratio of the icon.
*/
void HbIcon::setHeight(qreal height)
{
    QSizeF size = defaultSize();
    if (size.height() > 0) {
        qreal ar = size.width() / size.height();
        setSize(QSizeF(ar * height , height));
    }
}

/*! Sets the \a width of the icon. Its height is computed using the aspect ratio of the icon.
*/
void HbIcon::setWidth(qreal width)
{
    QSizeF size = defaultSize();
    if (size.width() > 0) {
        qreal ar = size.height() / size.width();
        setSize(QSizeF(width, ar * width));
    }
}

/*! Returns the width of the icon.
*/
qreal HbIcon::width() const
{
    return size().width();
}

/*! Returns the height of the icon.
*/
qreal HbIcon::height() const
{
    return size().height();
}

/*! Returns the mirroring mode set for the icon.
* \sa HbIcon::setMirroringMode()
*/
HbIcon::MirroringMode HbIcon::mirroringMode() const
{
    if (d->engine) {
        return d->engine->mirroringMode();
    } else {
        return HbIcon::Default;
    }
}

/*! Sets the mirroring \a mode for the icon.
* \sa HbIcon::mirroringMode()
*/
void HbIcon::setMirroringMode(HbIcon::MirroringMode mode)
{
    if (d->engine) {
        if (mode != d->engine->mirroringMode()) {
            d.detach();
            d->engine->setMirroringMode(mode);
        }
    }
}


/*! Returns the flags set for the icon.
*/
HbIcon::Flags HbIcon::flags() const
{
    if (d->engine) {
        return d->engine->flags();
    } else {
        return d->mQIconFlags;
    }
}

/*! Sets the flags for the icon.
*/
void HbIcon::setFlags(HbIcon::Flags flags)
{
    if (d->engine) {
        if (flags != d->engine->flags()) {
            d.detach();
            d->engine->setFlags(flags);
        }
    } else if (flags != d->mQIconFlags) {
        d.detach();
        d->mQIconFlags = flags;
    }
}

/*! Sets the size for the icon. Without calling this method, the icon uses its default size.
* \sa HbIcon::size(), HbIcon::defaultSize()
*/
void HbIcon::setSize(const QSizeF &size)
{
    if (size != d->size) {
        d.detach();

        d->size = size;
        if (d->engine) {
            d->engine->setSize(size);
        }
    }
}

/*!
* Returns the icon as a QVariant.
*/
HbIcon::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
* Returns a reference to a QIcon instance representing this icon.
* \note The returned reference is valid only for the life time of this HbIcon instance.
*/
QIcon &HbIcon::qicon() const
{
    return d->qicon;
}

/*!
* Equality operator. It compares the icon names for all the state and mode combinations.
* It also compares the badges, the color and the mirroring mode of the icon. The sizes
* set for the icons are not used for the comparison.
*/
bool HbIcon::operator==(const HbIcon &other) const
{
    return !(*this != other);
}

/*!
* Inequality operator. It compares the icon names for all the state and mode combinations.
* It also compares the badges, the color and the mirroring mode of the icon. The sizes
* set for the icons are not used for the comparison.
*/
bool HbIcon::operator!=(const HbIcon &other) const
{
    // NULL icons are equal
    if (isNull() && other.isNull()) {
        if (d->badgeInfo && other.d->badgeInfo) {
            if (d->badgeInfo->badges() != other.d->badgeInfo->badges()) {
                return true;
            }
        }
        return false;
    }

    const HbIconEngine *engine1 = d->engine;
    const HbIconEngine *engine2 = other.d->engine;

    // If both icons do not have engines, they are unequal.
    // An icon does not have an engine if it is constructed with a QIcon.
    if (!engine1 || !engine2) {
        return true;
    }

    if (engine1->iconName() != engine2->iconName() ||
            engine1->iconName(QIcon::Normal, QIcon::Off)    != engine2->iconName(QIcon::Normal, QIcon::Off) ||
            engine1->iconName(QIcon::Normal, QIcon::On)     != engine2->iconName(QIcon::Normal, QIcon::On) ||
            engine1->iconName(QIcon::Disabled, QIcon::Off)  != engine2->iconName(QIcon::Disabled, QIcon::Off) ||
            engine1->iconName(QIcon::Disabled, QIcon::On)   != engine2->iconName(QIcon::Disabled, QIcon::On) ||
            engine1->iconName(QIcon::Active, QIcon::Off)    != engine2->iconName(QIcon::Active, QIcon::Off) ||
            engine1->iconName(QIcon::Active, QIcon::On)     != engine2->iconName(QIcon::Active, QIcon::On) ||
            engine1->iconName(QIcon::Selected, QIcon::Off)  != engine2->iconName(QIcon::Selected, QIcon::Off) ||
            engine1->iconName(QIcon::Selected, QIcon::On)   != engine2->iconName(QIcon::Selected, QIcon::On)) {

        return true;
    }

    // If they have different badges, they are unequal
    if (engine1->badges() != engine2->badges()) {
        return true;
    }

    if (engine1->color() != engine2->color()) {
        return true;
    }

    // two icons are considered different if their mirroring modes are different
    if (engine1->mirroringMode() != engine2->mirroringMode()) {
        return true;
    }

    return false;
}

/*!
 * Adds a badge icon to the existing icon. The badge icons
 * are drawn relative to the alignment you specify with the
 * z-order you provide.
 *
 * By default the badge icon will use its default size.  If this is
 * not suitable (which is typical in case of vector graphics) then
 * call setSize() explicitly on \a badge before passing it to this
 * function.
 *
 * The aspect ratio for the badge is kept by default, but this can be
 * changed in the \a aspectRatio argument.
 *
 * \sa addProportionalBadge()
 */
bool HbIcon::addBadge(Qt::Alignment alignment,
                      const HbIcon &badge,
                      int z,
                      Qt::AspectRatioMode aspectRatio)
{
    d.detach();
    return d->addBadge(alignment, badge, z, QSizeF(), aspectRatio);
}

/*!
 * Adds a badge icon to the existing icon. The badge icons
 * are drawn relative to the alignment you specify with the
 * z-order you provide.
 *
 * With this function the size of the badge icon will be proportional
 * to the size of the badged icon.  If the size needs to be explicitly
 * specified then use addBadge() instead.
 *
 * \a sizeFactor's x and y values must be between 0 and 1. They
 * specify how much space the badge takes. For example (0.25, 0.25)
 * causes both the width and height to be 1/4 of the full icon width
 * and height.
 *
 * The aspect ratio for the badge is kept by default, but this can be
 * changed in the \a aspectRatio argument.
 *
 * \sa addBadge()
 */
bool HbIcon::addProportionalBadge(Qt::Alignment alignment,
                                  const HbIcon &badge,
                                  const QSizeF &sizeFactor,
                                  int z,
                                  Qt::AspectRatioMode aspectRatio)
{
    d.detach();
    return d->addBadge(alignment, badge, z, sizeFactor, aspectRatio);
}

/*!
 * Removes badge icon(s) from the icon.
 */
bool HbIcon::removeBadge(const HbIcon &badge)
{
    d.detach();
    return d->removeBadge(badge);
}

/*!
  Remove all badge icons from this icon
*/
void HbIcon::removeAllBadges()
{
    d.detach();
    d->removeAllBadges();
}

/*!
* Externalizes the \a icon information in a data \a stream. It can be transferred to other process and
* the icon can be reconstructed from the stream there.
*/
QDataStream &operator<<(QDataStream &stream, const HbIcon &icon)
{
    return stream << *icon.d.data();
}

/*!
* Internalizes the \a icon information from a data \a stream and reconstructs the icon.
* The streamed icon data can be transferred to another process and used from there.
*/
QDataStream &operator>>(QDataStream &stream, HbIcon &icon)
{
    icon.d.detach();
    return stream >> *icon.d.data();
}

/*!
    Clears the icon data (e.g. QPixmaps that are stored internally) but does not
    touch the settings, e.g. the size, state, flags, unlike clear().

    \internal
*/
void HbIconPrivate::clearStoredIconContent()
{
    // No need to detach here, usually the more icon's data we drop, the better. There are
    // no visual results anyway, icon data is reloaded next time the icon is painted.
    if (engine) {
        engine->clearStoredIconContent();
    }
    mQIconPixmap = QPixmap();
}
