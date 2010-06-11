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

#include "hbiconengine_p.h"

#include "hbicon.h"
#include "hbiconloader_p.h"
#include "hbtheme.h"
#include "hbtheme_p.h"
#include "hblayoutdirectionnotifier_p.h"
#include "hbiconanimation_p.h"
#include "hbimagetraces_p.h"
#include "hbiconimpl_p.h"
#include "hbpixmapiconimpl_p.h"
#include "hbbadgeicon_p.h"

#include <hbinstance.h>
#include <hbiconanimator.h>
#include <hbiconanimator_p.h>


#include <QObject>
#include <QPainter>
#include <QDebug>

#define DEFAULT_OPTIONS (HbIconLoader::ReturnUnknownIcon | HbIconLoader::BitmapIcons | HbIconLoader::VectorIcons)

class HbIconEnginePrivate
{
public:
    HbIconEnginePrivate(const QString &iconName);
    HbIconEnginePrivate(const HbIconEnginePrivate &other);
    HbIconEnginePrivate(QDataStream &stream);
    ~HbIconEnginePrivate();

    void clear();

    bool isMirrored();
    HbIconLoader::IconLoaderOptions iconLoaderOptions();

    QString iconName(QIcon::Mode mode, QIcon::State state) const;
    void setIconName(const QString &iconName, QIcon::Mode mode, QIcon::State state);

    void externalize(QDataStream &stream);
    void appendLoadFail(QIcon::Mode mode, QIcon::State state);
    void removeLoadFail(QIcon::Mode mode, QIcon::State state);
    void unLoadIcon(bool unloadedByServer = false);

    void addBadge(Qt::Alignment alignment,
                  const HbIcon &badge,
                  int z = 0);
    bool removeBadge(const HbIcon &badge);
    void removeAllBadges();
    const QList<HbBadgeIconInfo> badges() const;
    bool isBadged() const;

    QColor colorToUse(const QString &iconName) const;

public:
    QSizeF size;

    struct IconName {
        QIcon::Mode mode;
        QIcon::State state;
        QString name;
    };

    // Logical icon names specified for different states and modes of the icon
    // The default one is with mode = QIcon::Normal and state = QIcon::Off.
    QList<IconName> iconNames;

    QPixmap pixmap; // last accessed pixmap is stored here for fast re-use, it has the size as defined by d->size.
    Qt::AspectRatioMode aspectRatioMode; // aspect ratio mode of the last accessed pixmap
    QIcon::Mode mode; // icon mode of the last accessed pixmap
    QIcon::State state; // icon state of last accessed pixmap

    QSizeF defaultSize; // default icon size returned by the loader
    bool defaultSizeFailed;

    // List of those mode and state combinations which have failed in loading the icon.
    // This list should be empty normally but in case there is a failure in loading an icon,
    // it allows quick check for not trying to reload it again.
    // Mode and state are stored in integer as (mode | state << 16)
    QVector<int> loadFailed;

    HbIcon::Flags flags;
    HbIcon::MirroringMode mirroringMode;

    enum DefaultMirroring {
        Unknown = 0,
        Enabled = 1,
        Disabled = 2
    };
    DefaultMirroring defaultMirroring;

    // Pointer to the icon animator interface that is using this HbIcon instance.
    // If this pointer is 0, animations are not used.
    HbIconAnimator *animator;

    // Variables introduced for handling colorizable icons
    QColor color;
    // The color coming from the color group associated with a widget in its css.
    QColor themedColor;

    //Icon Implementation interface which provides abstraction for the type of icon ( Pixmap, NVG, SgImage etc).
    // Each class derived from HbIconImpl will have to implement the paint(), pixmap() and defaultSize() API
    HbIconImpl *icon;

    // Icons decorating this engine
    HbBadgeIcon *badgeInfo;

    bool signalConnectionsSet;
    // Icon FormatType
    HbIconFormatType iconType;
};

// Class HbIconEnginePrivate

HbIconEnginePrivate::HbIconEnginePrivate(const QString &iconName) :
    aspectRatioMode(Qt::KeepAspectRatio),
    mode(QIcon::Normal),
    state(QIcon::Off),
    defaultSizeFailed(false),
    loadFailed(0),
    flags(0),
    mirroringMode(HbIcon::Default),
    defaultMirroring(Unknown),
    animator(0),
    icon(0),
    badgeInfo(0),
    signalConnectionsSet(false),
    iconType(INVALID_FORMAT)
{
    if (!iconName.isEmpty()) {
        HbIconEnginePrivate::IconName newName = {QIcon::Normal, QIcon::Off, iconName};
        iconNames.append(newName);
    }
}

HbIconEnginePrivate::HbIconEnginePrivate(const HbIconEnginePrivate &other) :
    size(other.size),
    iconNames(other.iconNames),
    pixmap(other.animator ? QPixmap() : other.pixmap), // Pixmap is cleared if other object uses animation.
    aspectRatioMode(other.aspectRatioMode),
    mode(other.mode),
    state(other.state),
    defaultSize(other.defaultSize),
    defaultSizeFailed(other.defaultSizeFailed),
    loadFailed(other.loadFailed),
    flags(other.flags),
    mirroringMode(other.mirroringMode),
    defaultMirroring(other.defaultMirroring),
    animator(0), // Animator is instance specific and not copied.
    color(other.color), // No copy for themedColor, that one is an internal, instance-specific setting.
    icon(0),  // HbIconImpl is instance specific, it is recreated when icon is painted.
    badgeInfo(0),
    signalConnectionsSet(false),
    iconType(INVALID_FORMAT)
{
    if (other.badgeInfo) {
        badgeInfo = new HbBadgeIcon();
        *badgeInfo = *other.badgeInfo;
    }
}

HbIconEnginePrivate::HbIconEnginePrivate(QDataStream &stream) :
    aspectRatioMode(Qt::KeepAspectRatio),
    mode(QIcon::Normal),
    state(QIcon::Off),
    defaultSizeFailed(false),
    loadFailed(0),
    flags(0),
    mirroringMode(HbIcon::Default),
    defaultMirroring(Unknown),
    animator(0),
    icon(0),
    badgeInfo(new HbBadgeIcon),
    signalConnectionsSet(false),
    iconType(INVALID_FORMAT)
{
    // Internalize the icon from the stream
    stream >> size;
    int count;
    stream >> count;

    qint32 temp; // workaround for compiler problem

    for (int i = 0; i < count; i++) {
        IconName name;
        stream >> temp;
        name.mode = (QIcon::Mode)temp;
        stream >> temp;
        name.state = (QIcon::State)temp;
        stream >> name.name;
        iconNames.append(name);
    }

    stream >> temp;
    flags = (HbIcon::Flags)temp;
    stream >> temp;
    mirroringMode = (HbIcon::MirroringMode)temp;
    stream >> temp;
    defaultMirroring = (HbIconEnginePrivate::DefaultMirroring)temp;
    stream >> color;
    // Do not store themedColor, see the copy ctor.

    badgeInfo->internalize(stream);
}

void HbIconEnginePrivate::externalize(QDataStream &stream)
{
    // Externalize parameters that are needed to reconstruct the icon
    stream << size;
    stream << iconNames.count();
    Q_FOREACH(const IconName & name, iconNames) {
        stream << (qint32)(name.mode);
        stream << (qint32)(name.state);
        stream << name.name;
    }
    stream << (qint32)flags;
    stream << (qint32)mirroringMode;
    stream << (qint32)defaultMirroring;
    stream << (QColor)color;

    if (!badgeInfo) {
        badgeInfo = new HbBadgeIcon();
    }
    badgeInfo->externalize(stream);
}

void HbIconEnginePrivate::appendLoadFail(QIcon::Mode mode, QIcon::State state)
{
    int modeAndState = mode | (state << 16);
    int index = loadFailed.indexOf(modeAndState);
    if (index < 0) {
        loadFailed.append(modeAndState);
    }
}

void HbIconEnginePrivate::removeLoadFail(QIcon::Mode mode, QIcon::State state)
{
    int index = loadFailed.indexOf(mode | (state << 16));
    if (index >= 0) {
        loadFailed.remove(index);
    }
}

HbIconEnginePrivate::~HbIconEnginePrivate()
{
    unLoadIcon();
    delete badgeInfo;
}
/*!
  \internal

  Initiates an IPC call to the ThemeServer to unload ( decrement ref count )
  the icon if created on server side.

 */
void HbIconEnginePrivate::unLoadIcon(bool unloadedByServer)
{
    if (icon) {
        HbIconLoader *loader = HbIconLoader::global();
        loader->unLoadIcon(icon, unloadedByServer);
        icon->dispose();
        icon = 0;
    }
}

void HbIconEnginePrivate::clear()
{
    size = QSizeF();
    iconNames.clear();
    pixmap = QPixmap();
    aspectRatioMode = Qt::KeepAspectRatio;
    mode = QIcon::Normal;
    state = QIcon::Off;
    defaultSize = QSizeF();
    defaultSizeFailed = false;
    loadFailed.clear();
    flags = 0;
    mirroringMode = HbIcon::Default;
    defaultMirroring = Unknown;
    animator = 0;
    color = QColor();
    themedColor = QColor();
    unLoadIcon();
    if (badgeInfo) {
        badgeInfo->removeAllBadges();
    }
}

bool HbIconEnginePrivate::isMirrored()
{
    // If mirroring mode is default, check automatic mirroring from loader.
    if (mirroringMode == HbIcon::Default && defaultMirroring == Unknown) {
        HbIconLoader *loader = HbIconLoader::global();
        defaultMirroring = loader->isAutomaticallyMirrored(
                               iconName(QIcon::Normal, QIcon::Off)) ? Enabled : Disabled;
    }

    bool basedOnLayoutDir = false;
    // Handle the different mirroring modes

    // Default
    if (mirroringMode == HbIcon::Default) {
        if (defaultMirroring == Enabled) {
            basedOnLayoutDir = true;
        } else if (defaultMirroring == Disabled) {
            return false;
        }
        // Forced
    } else if (mirroringMode == HbIcon::Forced) {
        return true;
        // Prevented
    } else if (mirroringMode == HbIcon::Prevented) {
        return false;
        // LayoutDirection
    } else if (mirroringMode == HbIcon::LayoutDirection) {
        basedOnLayoutDir = true;
    }

    if (basedOnLayoutDir) {
        // e.g. some unit tests do not have primary window at this point so need to do NULL checks here.
        if (hbInstance) {
            QList<HbMainWindow *> allWindows = hbInstance->allMainWindows();

            if (!allWindows.isEmpty() &&
                    allWindows.value(0) &&
                    allWindows.value(0)->layoutDirection() == Qt::RightToLeft) {
                return true;
            }
            return false;
        }
    }

#ifdef HB_ICON_TRACES
    qDebug() << "HbIconEnginePrivate, invalid mirroring mode";
#endif
    return false;
}


HbIconLoader::IconLoaderOptions HbIconEnginePrivate::iconLoaderOptions()
{
    HbIconLoader::IconLoaderOptions options = DEFAULT_OPTIONS;
    if (flags.testFlag(HbIcon::DoNotCache)) {
        options |= HbIconLoader::DoNotCache;
    }
    if (isMirrored()) {
        options |= HbIconLoader::HorizontallyMirrored;
    }
    if (flags.testFlag(HbIcon::ResolutionCorrected)) {
        options |= HbIconLoader::ResolutionCorrected;
    }
    if (flags.testFlag(HbIcon::NoAutoStartAnimation)) {
        options |= HbIconLoader::NoAutoStartAnimation;
    }

    return options;
}

QString HbIconEnginePrivate::iconName(QIcon::Mode mode, QIcon::State state) const
{
    QString ret;

    Q_FOREACH(const HbIconEnginePrivate::IconName & name, iconNames) {
        if (name.mode == mode && name.state == state) {
            ret = name.name;
            break;
        }
    }

    return ret;
}

void HbIconEnginePrivate::setIconName(const QString &iconName, QIcon::Mode mode, QIcon::State state)
{
    int count = iconNames.count();
    for (int i = 0; i < count; i++) {
        HbIconEnginePrivate::IconName &name = iconNames[i];
        if (name.mode == mode && name.state == state) {
            // If given icon name is empty, remove the corresponding definition
            if (iconName.isEmpty()) {
                iconNames.removeAt(i);
            } else { // Otherwise, update the icon name
                name.name = iconName;
                // Icon name changed so remove information of failed loading
                removeLoadFail(mode, state);
            }
            return;
        }
    }
    // Not found, insert new item, if the given icon name was not empty
    if (!iconName.isEmpty()) {
        HbIconEnginePrivate::IconName newName = {mode, state, iconName};
        iconNames.append(newName);
    }
}

inline bool isMonoIcon(const QString &name)
{
    // Even when the Colorized flag is not set certain icons from the theme must be
    // colorized. These mono icons are recognized from their name. The check should not be
    // done for normal files, only for logical theme graphics names.
    return name.startsWith(QLatin1String("qtg_mono_")) && !name.contains('.');
}

QColor HbIconEnginePrivate::colorToUse(const QString &iconName) const
{
    if (flags.testFlag(HbIcon::Colorized) || isMonoIcon(iconName)) {
        return color.isValid() ? color : themedColor;
    } else {
        return QColor();
    }
}

void HbIconEnginePrivate::addBadge(Qt::Alignment align,
                                   const HbIcon &icon,
                                   int z)
{
    if (!badgeInfo) {
        badgeInfo = new HbBadgeIcon();
    }
    badgeInfo->addBadge(align, icon, z);
}

bool HbIconEnginePrivate::removeBadge(const HbIcon &badge)
{
    return badgeInfo ? badgeInfo->removeBadge(badge) : false;
}

void HbIconEnginePrivate::removeAllBadges()
{
    if (badgeInfo) {
        badgeInfo->removeAllBadges();
    }
}

const QList<HbBadgeIconInfo> HbIconEnginePrivate::badges() const
{
    return badgeInfo ? badgeInfo->badges() : QList<HbBadgeIconInfo>();
}

bool HbIconEnginePrivate::isBadged() const
{
    return badgeInfo ? badgeInfo->isBadged() : false;
}


// Class HbIconEngine

HbIconEngine::HbIconEngine() :
    QObject(),
    QIconEngineV2(),
    d(new HbIconEnginePrivate(QString()))
{
    // Register the HbIconEngine Instance to HbIconLoader
    HbIconLoader *loader = HbIconLoader::global();
    loader->storeIconEngineInfo(this);
}

HbIconEngine::HbIconEngine(const QString &iconName) :
    QObject(),
    QIconEngineV2(),
    d(new HbIconEnginePrivate(iconName))
{
    // Register the HbIconEngine Instance to HbIconLoader
    HbIconLoader *loader = HbIconLoader::global();
    loader->storeIconEngineInfo(this);
}

HbIconEngine::HbIconEngine(const HbIconEngine &other) :
    QObject(),
    QIconEngineV2(other),
    d(new HbIconEnginePrivate(*other.d))
{
    HbIcon::Flags newFlags = d->flags;
    d->flags = 0;
    setFlags(newFlags);
    // Register the HbIconEngine Instance to HbIconLoader
    HbIconLoader *loader = HbIconLoader::global();
    loader->storeIconEngineInfo(this);
}

HbIconEngine::HbIconEngine(QDataStream &stream) :
    d(new HbIconEnginePrivate(stream))
{
    HbIcon::Flags newFlags = d->flags;
    d->flags = 0;
    setFlags(newFlags);
    // Register the HbIconEngine Instance to HbIconLoader
    HbIconLoader *loader = HbIconLoader::global();
    loader->storeIconEngineInfo(this);
}

HbIconEngine::~HbIconEngine()
{
    // Unregister the HbIconEngine Instance to HbIconLoader
    HbIconLoader *loader = HbIconLoader::global();
    loader->removeIconEngineInfo(this);
    delete d;
}

bool HbIconEngine::loadFailed(QIcon::Mode mode, QIcon::State state) const
{
    return d->loadFailed.indexOf(mode | (state << 16)) >= 0;
}

HbIconAnimation *HbIconEngine::animation() const
{
    return d->animator ? d->animator->d->animation : 0;
}

bool HbIconEngine::isMirrored() const
{
    return d->isMirrored();
}

void HbIconEngine::externalize(QDataStream &stream)
{
    d->externalize(stream);
}

QString HbIconEngine::iconName() const
{
    return d->iconName(QIcon::Normal, QIcon::Off);
}

void HbIconEngine::setIconName(const QString &iconName)
{
    // Clear previously stored information of the icon, as name changes.
    clearStoredIconContent();

    d->setIconName(iconName, QIcon::Normal, QIcon::Off);
}

QString HbIconEngine::iconName(QIcon::Mode mode, QIcon::State state) const
{
    return d->iconName(mode, state);
}

void HbIconEngine::setIconName(const QString &iconName, QIcon::Mode mode, QIcon::State state)
{
    // Clear previously stored information of the icon, as name changes.
    clearStoredIconContent();

    d->setIconName(iconName, mode, state);
}

bool HbIconEngine::isNull() const
{
    // Return true if no icon names have been defined.
    return !d->iconNames.count();
}

void HbIconEngine::clear()
{
    d->clear();
}

HbIcon::Flags HbIconEngine::flags() const
{
    return d->flags;
}

void HbIconEngine::setFlags(HbIcon::Flags flags)
{
    bool needToReset = false;

    if (flags.testFlag(HbIcon::ResolutionCorrected) && !d->flags.testFlag(HbIcon::ResolutionCorrected)) {
        // Connect to default size adjustment change signal if resolution correction flag was activated
        connect(
            HbIconLoader::global(), SIGNAL(defaultSizeAdjustmentChanged()),
            this, SLOT(handleDefaultSizeAdjustmentChanged()));
        needToReset = true;
    } else if (!flags.testFlag(HbIcon::ResolutionCorrected) && d->flags.testFlag(HbIcon::ResolutionCorrected)) {
        // Disconnect if the flag was deactivated
        disconnect(
            HbIconLoader::global(), SIGNAL(defaultSizeAdjustmentChanged()),
            this, SLOT(handleDefaultSizeAdjustmentChanged()));
        needToReset = true;
    } else if (flags.testFlag(HbIcon::Colorized) != d->flags.testFlag(HbIcon::Colorized)) {
        needToReset = true;
    }

    if (needToReset) {
        clearStoredIconContent();
    }

    d->flags = flags;
}

HbIcon::MirroringMode HbIconEngine::mirroringMode() const
{
    return d->mirroringMode;
}

void HbIconEngine::setMirroringMode(HbIcon::MirroringMode mode)
{
    if (d->mirroringMode != mode) {
        // Mirroring changed, clear stored icon content
        clearStoredIconContent();
    }

    d->mirroringMode = mode;
}

QSize HbIconEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(mode)
    Q_UNUSED(state)

    // This is called by QIcon's aligned painting routine and it needs to return
    // the icon size.
    QSizeF defSize = defaultSize();
    defSize.scale(size, Qt::KeepAspectRatio);
    return defSize.toSize();
}

QPixmap HbIconEngine::pixmap(const QSize &pixelSize, QIcon::Mode mode, QIcon::State state)
{
    // If load failed earlier, do not retry forever.
    if (loadFailed(mode, state)) {
        return QPixmap();
    }

    QSize s = pixelSize;

    // If size has been defined but it is empty, just return NULL pixmap.
    if (s.isValid() && s.isEmpty()) {
        return QPixmap();
    }

    // If size has not been defined, use icon's default size.
    if (!s.isValid()) {
        // With size (0,0), loader uses the icon's default size.
        s = QSize(0, 0);
    }

    QIcon::Mode modeForLoader = mode;

    QString name = iconName(mode, state);
    if (name.isEmpty()) {
        // Icon name not defined for this mode and state, use default icon name
        name = iconName(QIcon::Normal, QIcon::Off);
    } else {
        // Icon name was defined for this mode and state, do not apply mode in loader
        modeForLoader = QIcon::Normal;
    }

    if (!name.isEmpty()) {
        HbIconLoader *loader = HbIconLoader::global();

        // If requested size is not the same as set in the engine, get a new pixmap from loader
        if (pixelSize != size().toSize()) {
            // Clear the old icon first
            d->unLoadIcon();

            d->icon = loader->loadIcon(
                          name,
                          HbIconLoader::AnyType,
                          HbIconLoader::AnyPurpose,
                          s,
                          Qt::KeepAspectRatio,
                          modeForLoader,
                          d->iconLoaderOptions(),
                          0,
                          d->colorToUse(name));

            if (d->icon) {
                // Draw badges on this pixmap
                QPainter painter(&d->pixmap);
                if (d->badgeInfo) {
                    d->badgeInfo->paint(&painter, QRectF(QPointF(0, 0), pixelSize), mode, state, d->isMirrored());
                }
                d->pixmap = d->icon->pixmap();
                return d->pixmap;
            }

        }
        // Requested size was same as set in engine, use the pixmap stored in engine
        else {
            if (!d->icon || Qt::KeepAspectRatio != d->aspectRatioMode || mode != d->mode || state != d->state) {
                d->aspectRatioMode = Qt::KeepAspectRatio;
                d->mode = mode;
                d->state = state;
                // Clear the old icon first
                d->unLoadIcon();
                d->icon  = loader->loadIcon(
                               name,
                               HbIconLoader::AnyType,
                               HbIconLoader::AnyPurpose,
                               s,
                               Qt::KeepAspectRatio,
                               modeForLoader,
                               d->iconLoaderOptions(),
                               0,
                               d->colorToUse(name));

                // If loading failed, store information so it is not retried.
                if (!d->icon) {
                    d->appendLoadFail(mode, state);
                }
            }
            if (d->icon) {
                d->pixmap = d->icon->pixmap();
                // Draw badges on this pixmap
                QPainter painter(&d->pixmap);
                if (d->badgeInfo) {
                    d->badgeInfo->paint(&painter, QRectF(QPointF(0, 0), pixelSize), mode, state, d->isMirrored());
                }
                return d->pixmap;
            }
        }
    }

    return QPixmap();
}

void HbIconEngine::setColor(const QColor &color)
{
    // Store the color. Note that we won't pass it later to the loader
    // if the flags do not contain HbIcon::Colorized or the icon name
    // does not qualify as a mono icon.
    if (d->color != color) {
        clearStoredIconContent();
        d->color = color;
    }
}

QColor HbIconEngine::color() const
{
    return d->color;
}

void HbIconEngine::setThemedColor(const QColor &color)
{
    if (d->themedColor != color) {
        clearStoredIconContent();
        d->themedColor = color;
    }
}

QColor HbIconEngine::themedColor() const
{
    return d->themedColor;
}

QSizeF HbIconEngine::defaultSize() const
{
    QString name = iconName();
    if (!d->defaultSizeFailed && !d->defaultSize.isValid() && !name.isEmpty()) {
        if (d->icon) {
            d->defaultSize = d->icon->defaultSize();
        } else {
            HbIconLoader *loader = HbIconLoader::global();
            d->defaultSize = loader->defaultSize(name, QString(), d->iconLoaderOptions());
        }
        // If default size could not get retrieved,
        // store information about that so it does not need to be checked again.
        if (!d->defaultSize.isValid()) {
            d->defaultSizeFailed = true;
        }
    }
    return d->defaultSize;
}

QSizeF HbIconEngine::size() const
{
    return d->size;
}

void HbIconEngine::setSize(const QSizeF &size)
{
    if (size != d->size) {
        d->size = size;
        // Update default size if size is set before painting
        // to obtain the actual default size of the image
        if (!d->icon && !d->defaultSize.isValid()) {
            defaultSize();
        }
        // Size changed, invalidate pixmap stored in this object.
        clearStoredIconContent(KeepDefaultSize);
    }
}

void HbIconEngine::paint(QPainter *painter,
                         const QRect &rect,
                         QIcon::Mode mode,
                         QIcon::State state)
{
    // This method is called by QIcon and it should paint the icon with the size defined by 'rect'.
    HbIconImpl *icon = 0;

    // update the rendering mode
    QPaintEngine *paintEngine = painter->paintEngine();
    if (paintEngine) {
        HbIconLoader::global()->updateRenderingMode(paintEngine->type());
    }

    icon = paintHelper(rect.size(), Qt::KeepAspectRatio, mode, state);
    if (icon) {
        icon->paint(painter, rect, Qt::AlignCenter);
        HbIconLoader::global()->unLoadIcon(icon);
        icon->dispose();

        // Now paint any necessary badges.
        if (d->badgeInfo) {
            d->badgeInfo->paint(painter, rect, mode, state, icon->isMirrored());
        }
    }
}

void HbIconEngine::paint(QPainter *painter,
                         const QRectF &rect,
                         Qt::AspectRatioMode aspectRatioMode,
                         Qt::Alignment alignment,
                         QIcon::Mode mode,
                         QIcon::State state)
{
    // If loading the pixmap has failed, do not retry forever
    if (loadFailed(mode, state)) {
        return;
    }

    QSizeF s = size();

    // If size has been defined but it is empty, just return.
    if (s.isValid() && s.isEmpty()) {
        return;
    }

    // FrameSet animations need to be reloaded if size related parameters have changed,
    // so delete the current animation in that case.
    HbIconAnimation *anim = animation();
    if (anim && anim->type() == HbIconAnimation::FrameSet) {
        if (((!s.isEmpty() || !anim->size().isEmpty()) && s != anim->size()) ||
                aspectRatioMode != anim->aspectRatioMode()) {
#ifdef HB_ICON_TRACES
            qDebug("HbIconEngine: deleting anim");
#endif
            delete anim;
            d->animator->d->animation = 0;
        }
    }

    // If size has not been defined, use icon's default size.
    if (!s.isValid()) {
        // With size (0,0), loader uses the icon's default size.
        s = QSizeF(0, 0);
    }

    // If pixmap has not been loaded yet or parameters affecting to pixmap have changed,
    // load the pixmap using paintHelper.
    if ((!d->icon) || aspectRatioMode != d->aspectRatioMode || mode != d->mode || state != d->state) {
        d->aspectRatioMode = aspectRatioMode;
        d->mode = mode;
        d->state = state;
        // If icon parameters changed unload the icon first, and get the new icon
        d->unLoadIcon();
        // Update the rendering mode
        QPaintEngine *paintEngine = painter->paintEngine();
        if (paintEngine) {
            HbIconLoader::global()->updateRenderingMode(paintEngine->type());
        }
        d->icon = paintHelper(s, aspectRatioMode, mode, state);
        if (d->icon && d->icon->isCreatedOnServer()) {
            d->iconType = d->icon->iconData().type;
        }
    }

    // Adjust the alignment and draw the icon.
    if (d->icon) {
        d->icon->paint(painter, rect, alignment);
    }

    // Paint the badge icons
    if (d->badgeInfo && d->icon) {
        d->badgeInfo->paint(painter, rect, mode, state, d->isMirrored());
    }
}

HbIconAnimator *HbIconEngine::animator() const
{
    return d->animator;
}

void HbIconEngine::setAnimator(HbIconAnimator *animator)
{
    d->animator = animator;
    // Reconnect animation signals
    HbIconAnimation *anim = animation();

    if (anim) {
        disconnect(anim); // To make sure we don't get multiple signals
        connect(anim, SIGNAL(animationUpdated()), this, SLOT(handleAnimationUpdated()));
        connect(anim, SIGNAL(animationStarted()), this, SLOT(handleAnimationStarted()));
        connect(anim, SIGNAL(animationStopped()), this, SLOT(handleAnimationStopped()));
        connect(anim, SIGNAL(animationFinished()), this, SLOT(handleAnimationFinished()));
    }
}

HbIconImpl *HbIconEngine::paintHelper(
    const QSizeF &size,
    Qt::AspectRatioMode aspectRatioMode,
    QIcon::Mode mode,
    QIcon::State state)
{
    // Set up signal connections if not yet done. This cannot be done during construction
    // because it would then consume time unnecessarily in case the NonThemeable flag is
    // set afterwards (after construction but before the first paint).
    ensureSignalConnections();

    QString name = iconName(mode, state);
    QIcon::Mode modeForLoader = mode;
    HbIconImpl *icon = 0;
    if (name.isEmpty()) {
        // Icon name not defined for this mode and state, use default icon name
        name = iconName(QIcon::Normal, QIcon::Off);
    } else {
        // Icon name was defined for this mode and state, do not apply mode in loader
        modeForLoader = QIcon::Normal;
    }

    HbIconAnimation *anim = animation();

    if (anim && anim->iconName() != name) {
        // Clear the current animation because there is different icon specified for the new mode/state.
        // This should be optimized so that there is list of animations so the existing ones are not deleted,
        // so there is no need to reload the animation every time mode/state is changed back and forth;
        delete anim;
        anim = 0;
        d->animator->d->animation = 0;
    }

    // If there is valid animation object, get pixmap of the current animation frame from it
    if (anim) {
        icon = new HbPixmapIconImpl(getPixmapFromAnimation());
    }
    // Otherwise use icon loader to load pixmap
    else {
        if (!name.isEmpty()) {
            HbIconLoader *loader = HbIconLoader::global();

            icon = loader->loadIcon(
                       name,
                       HbIconLoader::AnyType,
                       HbIconLoader::AnyPurpose,
                       size,
                       aspectRatioMode,
                       modeForLoader,
                       d->iconLoaderOptions(),
                       d->animator,
                       d->colorToUse(name));

            // If loading failed, store information so it is not retried in every repaint.
            if (!icon) {
                d->appendLoadFail(mode, state);
            }

            // If animation was returned, set it in the animator and connect to it
            HbIconAnimation *anim = animation();
            if (anim) {
                connect(anim, SIGNAL(animationUpdated()), this, SLOT(handleAnimationUpdated()));
                connect(anim, SIGNAL(animationStarted()), this, SLOT(handleAnimationStarted()));
                connect(anim, SIGNAL(animationStopped()), this, SLOT(handleAnimationStopped()));
                connect(anim, SIGNAL(animationFinished()), this, SLOT(handleAnimationFinished()));
                // If animation start has been forced with HbIconAnimator::startAnimation,
                // start the animation here if auto-start is not enabled.
                if (d->animator->d->started && d->flags.testFlag(HbIcon::NoAutoStartAnimation)) {
                    d->animator->startAnimation();
                }
            }
        }
    }

    return icon;
}

/*!
    Clears the icon data (e.g. QPixmaps that are stored internally)
    but does not touch the settings, e.g. the size, state, flags, unlike clear().

    This function is called when some parameters change so that the content has
    to be reloaded, and in low-graphics-memory situations to get rid of all
    cached image data for the icon.

    The goal in the OOGM case is to destroy all unnecessary QPixmaps which will in turn
    lead to freeing graphics memory (in case we are running on the OpenVG paint engine).
    The data will be reloaded (well, at least tried to be reloaded) when the icon is
    painted the next time.
 */
void HbIconEngine::clearStoredIconContent(ClearingFlags flags)
{
#ifdef HB_ICON_TRACES
    qDebug("HbIconEngine %x: clearStoredIconContent", (int) this);
#endif

    d->pixmap = QPixmap();
    d->unLoadIcon(flags.testFlag(UnloadedByServer));
    if (!(flags.testFlag(KeepDefaultSize))) {
        d->defaultSize = QSizeF();
    }
    if (flags.testFlag(ResetIconSize)) {
        d->size = QSizeF();
    }
    d->defaultSizeFailed = false;
    d->loadFailed.clear();

    // Lazy loading is used, new pixmap will be rasterized when it's required by paint method.
    // Clear animation also - it will be restarted.
    HbIconAnimation *anim = animation();
    if (anim) {
        delete anim;
        d->animator->d->animation = 0;
    }
}

/*!  Similar to clearStoredIconContent() but has no effect for animated icons.
  This is useful in cases where animations must not be stopped (in contrast,
  clearStoredIconContent() stops all icon animations and they are restarted from
  the beginning during the next paint).
 */
void HbIconEngine::clearStoredNonAnimIconContent()
{
    if (!d->animator) {
        clearStoredIconContent();
    }
}

void HbIconEngine::ensureSignalConnections()
{
    if (!d->signalConnectionsSet) {
        d->signalConnectionsSet = true;
        if (!d->flags.testFlag(HbIcon::NonThemeable)) {
            connect(&hbInstance->theme()->d_ptr->iconTheme, SIGNAL(iconsUpdated(const QStringList &)), this, SLOT(themeChange(const QStringList &)));
            connect(HbLayoutDirectionNotifier::instance(), SIGNAL(layoutDirectionChanged()),
                    this, SLOT(handleLayoutDirectionChanged()));
            // No need to worry about missed notifications because if the icon was not
            // painted before then there is nothing to clear.
        }
    }
}

void HbIconEngine::themeChange(const QStringList &updatedFiles)
{
    // Theme has changed, clear stored icon content
    // Server side icon cache is already cleared when theme is changed
    if (updatedFiles.count() == 0 || (d->icon && updatedFiles.contains(d->icon->iconFileName()))) {
        clearStoredIconContent(UnloadedByServer);
    }
}

void HbIconEngine::handleLayoutDirectionChanged()
{
    if ((d->mirroringMode == HbIcon::Default && d->defaultMirroring == HbIconEnginePrivate::Enabled) ||
            d->mirroringMode == HbIcon::LayoutDirection) {
        // This icon is automatically mirrored based on layout direction.

        // Clear the stored icon content and its default size because the layout direction just changed.
        // Lazy loading is used, new pixmap will be rasterized when it's required by paint method.
        clearStoredIconContent();
    }
}

void HbIconEngine::handleDefaultSizeAdjustmentChanged()
{
    if (d->flags.testFlag(HbIcon::ResolutionCorrected)) {
        // Icon content not valid any more - clear it.
        clearStoredIconContent(ResetIconSize);
    }
}

void HbIconEngine::handleAnimationUpdated()
{
    // Clear the pixmap. It gets updated with the next animation frame when the icon is painted.
    d->pixmap = QPixmap();
    d->unLoadIcon();
    // If there is an animator then emit the 'progressed' signal on it.
    if (d->animator) {
        // Update the graphics item if set in the animator
        if (d->animator->d->item) {
            d->animator->d->item->update();
        }
        // Also emit the progress signal
        emit d->animator->animationProgressed();
    }
}

void HbIconEngine::handleAnimationStarted()
{
    // Just emit the similar signal on the animator.
    if (d->animator) {
        emit d->animator->animationStarted();
    }
}

void HbIconEngine::handleAnimationStopped()
{
    // Just emit the similar signal on the animator.
    if (d->animator) {
        d->animator->d->started = false;
        emit d->animator->animationStopped();
    }
}

void HbIconEngine::handleAnimationFinished()
{
    // Just emit the similar signal on the animator.
    if (d->animator) {
        d->animator->d->started = false;
        emit d->animator->animationFinished();
    }
}

QPixmap HbIconEngine::getPixmapFromAnimation() const
{
    HbIconAnimation *anim = animation();

    if (anim) {
        // Update parameters in animation
        anim->setSize(size());
        anim->setAspectRatioMode(d->aspectRatioMode);
        anim->setMode(d->mode);
        // Get the current frame as pixmap from the animation
        return anim->currentFrame();
    } else {
        return QPixmap();
    }
}

/*!
 * Removes the item in the cache if the ref. count is 0 and does delete on
 * HbIconImpl and Resets the IconImpl.
 */
void HbIconEngine::resetIconImpl() const
{
#if defined(HB_SGIMAGE_ICON) || defined(HB_NVG_CS_ICON)
    if ((d->iconType == SGIMAGE) || (d->iconType == NVG)) {
        if (d->icon) {
            d->icon->decrementRefCount();
            if (d->icon->refCount() == 0 && d->icon->isCreatedOnServer()) {
                HbIconLoader *loader = HbIconLoader::global();
                loader->removeItemInCache(d->icon);
                d->icon->dispose();
            }
            d->icon = 0;
        }
    }
#endif
}

void HbIconEngine::addBadge(Qt::Alignment align,
                            const HbIcon &icon,
                            int z)
{
    d->addBadge(align, icon, z);
}

bool HbIconEngine::removeBadge(const HbIcon &badge)
{
    return d->removeBadge(badge);
}

void HbIconEngine::removeAllBadges()
{
    d->removeAllBadges();
}

const QList<HbBadgeIconInfo> HbIconEngine::badges() const
{
    return d->badges();
}

bool HbIconEngine::isBadged() const
{
    return d->isBadged();
}

/*!
 * Returns the IconFormat Type
 */
HbIconFormatType HbIconEngine::iconFormatType() const
{
    return d->iconType;
}

// End of File
