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
#include "hbframedrawer.h"
#include "hbframedrawer_p.h"
#include "hbimagetraces_p.h"
#include "hbmaskableiconimpl_p.h"

#include <hbinstance.h>
#include <hbiconimpl_p.h>
#include <hbiconloader_p.h>
#include <hbthemeutils_p.h>

#include <QPainter>
#include <QIcon>
#include <QPixmap>
#include <QDebug>
#include <QBitmap>

/*!
    @stable
    @hbcore
    \class HbFrameDrawer
    \brief HbFrameDrawer draws frames graphics.

    This class is not intended to be derived from.

    Frame graphics mean graphics that consist of either 3 or 9 parts that are positioned
    and scaled followingly.

    A 9-piece frame consists of a 3x3 grid, where the corner parts have a fixed size and
    the other parts are scaled.

    A 3-piece frame consists of 3 parts tiled either horizontally or vertically. The side
    parts preserve their aspect ratio and the center part is scaled to fill the remaining space.

    For convenience, frame drawer can also draw the frame with a single graphic.

    The frame drawer implements the logic of scaling and positioning the frame parts and
    takes care that there are no visible gaps between the frame parts when the whole frame
    is painted to the display even if the frame bounding rectangle uses floating point accuracy.

    Filenames for 9-piece frame graphics are defined according to the following convention.

    - top left        <name>_tl.<ext>
    - top             <name>_t.<ext>
    - top right       <name>_tr.<ext>
    - left            <name>_l.<ext>
    - center          <name>_c.<ext>
    - right           <name>_r.<ext>
    - bottom left     <name>_bl.<ext>
    - bottom          <name>_b.<ext>
    - bottom right    <name>_br.<ext>

    Filenames for 3-piece horizontal frame graphics are defined according to the following convention.

    - left            <name>_l.<ext>
    - center          <name>_c.<ext>
    - right           <name>_r.<ext>

    Filenames for 3-piece vertical frame graphics are defined according to the following convention.

    - top             <name>_t.<ext>
    - center          <name>_c.<ext>
    - bottom          <name>_b.<ext>

    \note The suffixes (e.g. "_l") in the filenames can be redefined with method
    HbFrameDrawer::setFileNameSuffixList().

    \note If all of the frame pieces cannot be found, the frame drawer falls back to a single
    frame graphics piece with filename <name>.<ext> to draw the frame.
    If even that does not exist, nothing is drawn.

    See class HbIcon for information of supported icon formats and file locations.

    The frame drawer stores the entire generated frame graphics in a pixmap for
    improving the speed of the paint operations. Therefore, the frame drawer instance should not
    be re-created each time the frame needs to be rendered.

    Example of how to create a frame drawer and use it.

    \dontinclude ultimatecodesnippet/ultimatecodesnippet.cpp
    \skip nine-piece
    \until }

    \sa class HbFrameItem
*/

/*!
  \enum HbFrameDrawer::FrameType

  \b Undefined The frame type is undefined. It must be defined before being able to draw the frame.

  \b OnePiece The frame consist of a single graphic piece, which is scaled to the frame's bounding rectangle.

  \b ThreePiecesHorizontal The frame consists of three graphic pieces that are tiled horizontally.
            By default, the side parts preserve their aspect ratio and the center part is scaled to fill the remaining space.
            Filenames for 3-piece horizontal frame graphics are defined according to the following.

            - left            <name>_l.<ext>
            - center          <name>_c.<ext>
            - right           <name>_r.<ext>

  \b ThreePiecesVertical The frame consists of three graphic pieces that are tiled vertically.
            By default, the side parts preserve their aspect ratio and the center part is scaled to fill the remaining space.
            Filenames for 3-piece vertical frame graphics are defined according to the following.

            - top             <name>_t.<ext>
            - center          <name>_c.<ext>
            - bottom          <name>_b.<ext>

  \b NinePieces The frame consists of a 3x3 grid of graphic pieces, where by default the corner
            parts have a fixed size and the other parts are scaled.
            Filenames for 9-piece frame graphics are defined according to the following.

            - top left        <name>_tl.<ext>
            - top             <name>_t.<ext>
            - top right       <name>_tr.<ext>
            - left            <name>_l.<ext>
            - center          <name>_c.<ext>
            - right           <name>_r.<ext>
            - bottom left     <name>_bl.<ext>
            - bottom          <name>_b.<ext>
            - bottom right    <name>_br.<ext>
*/

#define DEFAULT_OPTIONS (HbIconLoader::ReturnUnknownIcon | HbIconLoader::BitmapIcons | HbIconLoader::VectorIcons)
// Same rounding function is used everywhere for consistency
static inline int realToInt(qreal r)
{
    return (int)(r + 0.5);
}

/*!
\internal
*/
HbFrameDrawerPrivate::HbFrameDrawerPrivate() :
    QSharedData(),
    type(HbFrameDrawer::Undefined),
    frameParts(0),
    mirroring(HbIcon::Default),
    defaultMirroring(Unknown),
    flags(0),
    layoutDirection(Qt::LeftToRight),
    graphicsItem(0),
    color(),
    icon(0),
    maskChanged(false),
    clipPath(QPainterPath())
{
    borderWidths[0] = 0.0;
    borderWidths[1] = 0.0;
    borderWidths[2] = 0.0;
    borderWidths[3] = 0.0;
    //Register the HbFrameDrawerPrivate Instance to HbIconLoader
    HbIconLoader::global()->storeFrameDrawerInfo(this);
}

/*!
\internal
*/
HbFrameDrawerPrivate::HbFrameDrawerPrivate(const QString &frameGraphicsName, HbFrameDrawer::FrameType type) :
    QSharedData(),
    frameGraphicsName(frameGraphicsName),
    type(type),
    mask(QPixmap()),
    frameParts(0),
    mirroring(HbIcon::Default),
    defaultMirroring(Unknown),
    flags(0),
    layoutDirection(Qt::LeftToRight),
    graphicsItem(0),
    color(),
    icon(0),
    maskChanged(false),
    clipPath(QPainterPath())
{
    borderWidths[0] = 0.0;
    borderWidths[1] = 0.0;
    borderWidths[2] = 0.0;
    borderWidths[3] = 0.0;

    if (HbThemeUtils::isLogicalName(frameGraphicsName)) {
        // Remove possible file extension
        int index = this->frameGraphicsName.lastIndexOf(QChar('.'));
        if (index>0) {
            this->frameGraphicsName.resize(index);
        }
    }

    //Register the HbFrameDrawerPrivate Instance to HbIconLoader
    HbIconLoader::global()->storeFrameDrawerInfo(this);
}

/*!
\internal
*/
HbFrameDrawerPrivate::HbFrameDrawerPrivate(const HbFrameDrawerPrivate &other) :
    QSharedData(other),
    frameGraphicsName(other.frameGraphicsName),
    type(other.type),
    rect(other.rect),
    mask(QPixmap()),
    frameParts(other.frameParts),
    mirroring(other.mirroring),
    defaultMirroring(other.defaultMirroring),
    flags(other.flags),
    layoutDirection(other.layoutDirection),
    graphicsItem(other.graphicsItem),
    color(other.color),
    icon(0),
    maskChanged(false),
    clipPath(QPainterPath())
{
    borderWidths[0] = other.borderWidths[0];
    borderWidths[1] = other.borderWidths[1];
    borderWidths[2] = other.borderWidths[2];
    borderWidths[3] = other.borderWidths[3];
    //Register the HbFrameDrawerPrivate Instance to HbIconLoader
    HbIconLoader::global()->storeFrameDrawerInfo(this);
}

/*!
\internal
*/
HbFrameDrawerPrivate::~HbFrameDrawerPrivate()
{
    unLoadIcon();
    // Unregister the HbFrameDrawerPrivate Instance to HbIconLoader.
    // However it may already have been destroyed, if the app is shuting down
    // so be prepared for null ptr too.
    HbIconLoader *loader = HbIconLoader::global();
    if (loader) {
        loader->removeFrameDrawerInfo(this);
    }
}

/*!
\internal
*/
void HbFrameDrawerPrivate::prepareFrameIcon()
{
    if (!frameParts) {
        checkFrameParts();
    }
    if ((!icon) && (!fallbackMaskableIconList.count())) {
        createFrameIcon();
    }
}

/*!
* Returns whether this frame drawer's border widths have been set by API
*
* \sa HbFrameDrawerPrivate::setBorderApiProtectionFlag()
*/
bool HbFrameDrawerPrivate::testBorderApiProtectionFlag() const
{
    return flags & HbFrameDrawerPrivate::BorderWidthSetByApi;
}

/*!
* Clears the flag denoting that the border widths have been set by API.
* This will cause control of border widths to return to the CSS system.
*
* \sa HbFrameDrawerPrivate::testBorderApiProtectionFlag()
*/
void HbFrameDrawerPrivate::setBorderApiProtectionFlag(bool on)
{
    if (on) {
        flags |= HbFrameDrawerPrivate::BorderWidthSetByApi;
    } else {
        flags &= ~HbFrameDrawerPrivate::BorderWidthSetByApi;
    }
}

/*!
* Checks whether all the frame parts are available for the given frame type.
* Fallback is 1-part frame rendering.
* \internal
*/
void HbFrameDrawerPrivate::checkFrameParts()
{
    // fall back is 1 piece frame
    frameParts = 1;

    if (type == HbFrameDrawer::ThreePiecesHorizontal || type == HbFrameDrawer::ThreePiecesVertical) {
        if (HbIconLoader::iconsExist(frameGraphicsName, fileNameSuffixList())) {
            frameParts = 3;
        }
    } else if (type == HbFrameDrawer::NinePieces) {
        if (HbIconLoader::iconsExist(frameGraphicsName, fileNameSuffixList())) {
            frameParts = 9;
        }
    }
}

/*!
\internal
*/
QStringList HbFrameDrawerPrivate::fileNameSuffixList() const
{
    // If the suffix list has been set explicitly, return that if it contains enough suffixes.
    if (!suffixList.isEmpty()) {
        switch (type) {
        case HbFrameDrawer::ThreePiecesHorizontal: // fall through
        case HbFrameDrawer::ThreePiecesVertical:
            if (suffixList.count() >= 3) {
                return suffixList;
            }
            break;

        case HbFrameDrawer::NinePieces:
            if (suffixList.count() >= 9) {
                return suffixList;
            }
            break;

        default:
            return suffixList;
        }
    }

    // Otherwise, return default suffix list.
    QStringList list;

    switch (type) {
    case HbFrameDrawer::ThreePiecesHorizontal:
        list << "_l" << "_c" << "_r";
        break;
    case HbFrameDrawer::ThreePiecesVertical:
        list << "_t" << "_c" << "_b";
        break;
    case HbFrameDrawer::NinePieces:
        list << "_tl" << "_t" << "_tr" << "_l" << "_c" << "_r" << "_bl" << "_b" << "_br";
        break;
    default:
        break;
    }

    return list;
}

/*!
* Creates a consolidated HbIconImpl (Icon) with the available piece information
* If failed to create the same, then creates HbIconImpls for the pieces.
* \internal
*/
void HbFrameDrawerPrivate::createFrameIcon()
{
    // Divide rectangle area to the frame parts first.
    HbMultiPartSizeData data;
    QSize frameIconSize = divideSpace(data);
    data.multiPartIconId = multiPartIconId();
    multiPartSizeData = data;

    // Create the frame icon and add it in the icon cache
    HbIconLoader *loader = HbIconLoader::global();
    if (!loader) {
        return;
    }

    //If it's one-piece frame-item, it's loaded using HbIconLoader::loadIcon()
    if (frameParts == 1) {
        HbIconImpl *iconImpl =  loader->loadIcon(frameGraphicsName, HbIconLoader::AnyType,
                                HbIconLoader::AnyPurpose,
                                frameIconSize,
                                Qt::IgnoreAspectRatio,
                                QIcon::Normal,
                                iconLoaderOptions());
        if (iconImpl) {
            icon = new HbMaskableIconImpl(iconImpl);
        }

    } else {
        QStringList multiPieceFileNames = resolveMultiPieceFileNames();
        
        for (int i = 0; i < frameParts; i++) {
            if (data.pixmapSizes[i].isEmpty()) {
                data.pixmapSizes[i] = QSize(0, 0);
            }
        }

        QVector<HbIconImpl *> listOfIcons;

        //For multi-piece frame-items, HbIocnLoader::loadMultipieceIcon is used
        //This function returns the consolidate (stitched) icon created on the themeserver.
        //If the consolidated icon-creation on themeserver fails, then the consolidated
        //icon is created on the client-side (fallback)
        HbIconImpl *iconImpl = loader->loadMultiPieceIcon(multiPieceFileNames, data, frameIconSize,
                               Qt::IgnoreAspectRatio, QIcon::Normal, iconLoaderOptions(),
                               listOfIcons, color);

        if (iconImpl) {
            icon = new HbMaskableIconImpl(iconImpl);
        } else {
            int implCount = listOfIcons.count();
            for (int i = 0; i < implCount; i++) {
                fallbackMaskableIconList.append(new HbMaskableIconImpl(listOfIcons[i]));
            }
        }

    }
}

QStringList HbFrameDrawerPrivate::resolveMultiPieceFileNames()
{
    QStringList multiPieceFileNames;

    bool logicalName = HbThemeUtils::isLogicalName(frameGraphicsName);

    int suffixIndex = frameGraphicsName.length();
    if (!logicalName) {
        // If it is an absolute icon path, the suffix is inserted before the file extension
        int index = frameGraphicsName.lastIndexOf(QChar('.'));
        if (index > 0) {
            suffixIndex = index;
        }
    }

    QStringList list = fileNameSuffixList();
    int count = list.count();
    for (int i = 0; i < count; i++) {
        QString nameWithSuffix = frameGraphicsName;
        nameWithSuffix.insert(suffixIndex, list.at(i));
        multiPieceFileNames.append(nameWithSuffix);
    }

    return multiPieceFileNames;
}

/*!
\internal
*/
void HbFrameDrawerPrivate::paint(QPainter *painter)
{
    // createFrameIcon() must be called before calling this method.

    QRectF centeredRect(rect);
    QRect integerRect = rect.toRect();
    if (icon) {
        QSize size = icon->size();

        if (integerRect.width() != size.width()) {
            centeredRect.adjust((rect.width() - size.width()) / 2, 0, 0, 0);
            centeredRect.setWidth(size.width());
        }
        if (integerRect.height() != size.height()) {
            centeredRect.adjust(0, (rect.height() - size.height()) / 2, 0, 0);
            centeredRect.setHeight(size.height());
        }

        if (!mask.isNull() && maskChanged) {
            icon->setMask(mask);
        }
        //paint the stitched icon
        icon->paint(painter, centeredRect, Qt::AlignHCenter, clipPath);
        if (icon->isCreatedOnServer()) {
            iconType = icon->iconImpl()->iconData().type;
        }

#ifdef HB_FRAME_DRAWER_TRACES
        qDebug() << "FRAMENAME: " << frameGraphicsName << "Drawn at: "
                 << centeredRect.topLeft().x() << "," << centeredRect.topLeft().y() << " | "
                 << centeredRect.bottomRight().x() << "," << centeredRect.bottomRight().y();
#endif
    } else {
        for (int i = 0; i < fileNameSuffixList().count(); i++) {
            if (i < fallbackMaskableIconList.count() && fallbackMaskableIconList[i]) {
                QPoint position = multiPartSizeData.targets[i].topLeft();
                if (isMirrored()) {
                    int centerTopRightX = (int)(centeredRect.topLeft().x() + centeredRect.width());
                    int targetTopRightX = multiPartSizeData.targets[i].topLeft().x()
                                          + multiPartSizeData.targets[i].width();
                    position.setX((int)(centeredRect.topLeft().x() + (centerTopRightX - targetTopRightX)));
                } else {
                    position.setX((int)(centeredRect.topLeft().x() + position.x()));
                }
                position.setY((int)(centeredRect.topLeft().y() + position.y()));
                if ((!mask.isNull()) && maskChanged) {
                    QBitmap pieceMask ;
                    QRect maskRect(position.x(), position.y(), multiPartSizeData.targets[i].width(), multiPartSizeData.targets[i].height());
                    pieceMask = mask.copy(maskRect);
                    fallbackMaskableIconList[i]->setMask(pieceMask);
                }
                if (!multiPartSizeData.targets[i].isEmpty()) {
                    fallbackMaskableIconList[i]->paint(painter,
                                                       QRect(position, multiPartSizeData.pixmapSizes[i]),
                                                       Qt::AlignHCenter, clipPath);
                }
            }
        }
        if ( fallbackMaskableIconList.count() && fallbackMaskableIconList[0]->iconImpl()
             && fallbackMaskableIconList[0]->iconImpl()->isCreatedOnServer() ) {
            if ( fallbackMaskableIconList[0]->iconImpl()->iconData().type != INVALID_FORMAT ) {
                // store the icon type
                iconType = fallbackMaskableIconList[0]->iconImpl()->iconData().type;
            }
        }
    }
    maskChanged = false;
}

/*!
\internal
*/
bool HbFrameDrawerPrivate::fillWholeRect() const
{
    return flags & HbFrameDrawerPrivate::FillWholeRect;
}

/*!
* Calculates frame part rectangles and icon sizes based on the following information.
* - frame bounding rectangle
* - frame inner rectangle (optional)
* - default sizes of the frame corner (9-piece) or side (3-piece) graphics.
*
* Note! Integer pixel sizes are used in this function to avoid one pixel gaps between the frame parts.
* \internal
*/
QSize HbFrameDrawerPrivate::divideSpace(HbMultiPartSizeData &data)
{
    // Do not need to divide space if there is only one frame part.
    if (frameParts == 1) {
        QSizeF size = rect.size();
        // If the whole rect is not filled, use default graphics size to define one of the dimensions.
        if (type == HbFrameDrawer::ThreePiecesHorizontal && !fillWholeRect()) {
            qreal defaultHeight = defaultSize("").height();
            if (defaultHeight < size.height()) {
                size.setHeight(defaultHeight);
            }
        } else if (type == HbFrameDrawer::ThreePiecesVertical && !fillWholeRect()) {
            qreal defaultWidth = defaultSize("").width();
            if (defaultWidth < size.width()) {
                size.setWidth(defaultWidth);
            }
        }

        // Using same rounding as in other code below
        return QSize(realToInt(size.width()), realToInt(size.height()));
    }

    // Frame part filename suffix list is needed.
    QStringList suffixList = fileNameSuffixList();

    int fullWidth = realToInt(rect.width());
    int fullHeight = realToInt(rect.height());

    if (type == HbFrameDrawer::ThreePiecesHorizontal) {
        int leftWidth;
        int rightWidth;

        if (borderWidths[0] == 0 && borderWidths[2] == 0) {
            QSizeF leftSize = defaultSize(suffixList.at(0)); // index 0 is leftmost frame part
            QSizeF rightSize = defaultSize(suffixList.at(2)); // index 2 is rightmost frame part

            if (fillWholeRect()) {
                leftSize.scale(rect.size(), Qt::KeepAspectRatio);
                rightSize.scale(rect.size(), Qt::KeepAspectRatio);
            } else {
                int leftHeight = leftSize.toSize().height();
                if (leftHeight < fullHeight) {
                    fullHeight = leftHeight;
                }
            }

            leftWidth = leftSize.toSize().width();
            rightWidth = rightSize.toSize().width();

        } else {
            // QSizeF::toSize rounds to nearest integer so do the same here
            leftWidth = realToInt(borderWidths[0]);
            rightWidth = realToInt(borderWidths[2]);

            // If whole rect is not filled, take height from default graphics height
            if (!fillWholeRect()) {
                QSizeF leftSize = defaultSize(suffixList.at(0)); // index 0 is leftmost frame part

                int leftHeight = leftSize.toSize().height();
                if (leftHeight < fullHeight) {
                    fullHeight = leftHeight;
                }
            }
        }

        int centerWidth = fullWidth - leftWidth - rightWidth;

        // Do all 3 pieces fit?
        if (fullWidth >= leftWidth + rightWidth) {
            data.targets[0] = QRect(0, 0, leftWidth, fullHeight);
            data.targets[1] = QRect(leftWidth, 0, centerWidth, fullHeight);
            data.targets[2] = QRect(fullWidth - rightWidth, 0, rightWidth, fullHeight);

            data.pixmapSizes[0] = QSize(leftWidth, fullHeight);
            data.pixmapSizes[1] = QSize(centerWidth, fullHeight);
            data.pixmapSizes[2] = QSize(rightWidth, fullHeight);

            data.sources[0] = QRect(QPoint(0, 0), data.pixmapSizes[0]);
            data.sources[1] = QRect(QPoint(0, 0), data.pixmapSizes[1]);
            data.sources[2] = QRect(QPoint(0, 0), data.pixmapSizes[2]);
        } else {
            // Only sides fit
            int totalSideWidth = leftWidth + rightWidth;
            data.targets[0] = QRect(0, 0, leftWidth * fullWidth / totalSideWidth, fullHeight);
            data.targets[1] = QRect();
            data.targets[2] = QRect(data.targets[0].width(), 0, fullWidth - data.targets[0].width(), fullHeight);

            data.pixmapSizes[0] = QSize(data.targets[0].width(), fullHeight);
            data.pixmapSizes[1] = QSize();
            data.pixmapSizes[2] = QSize(data.targets[2].width(), fullHeight);

            data.sources[0] = data.targets[0];
            data.sources[1] = QRect();
            data.sources[2] = data.targets[2];
            data.sources[2].moveLeft(data.pixmapSizes[2].width() - data.targets[2].width());
        }
    } else if (type == HbFrameDrawer::ThreePiecesVertical) {

        int topHeight;
        int bottomHeight;

        if (borderWidths[1] == 0 && borderWidths[3] == 0) {
            QSizeF topSize = defaultSize(suffixList.at(0)); // index 0 is top frame part
            QSizeF bottomSize = defaultSize(suffixList.at(2)); // index 2 is bottom frame part

            if (fillWholeRect()) {
                topSize.scale(rect.size(), Qt::KeepAspectRatio);
                bottomSize.scale(rect.size(), Qt::KeepAspectRatio);
            } else {
                int topWidth = topSize.toSize().width();
                if (topWidth < fullWidth) {
                    fullWidth = topWidth;
                }
            }

            topHeight = topSize.toSize().height();
            bottomHeight = bottomSize.toSize().height();
        } else {
            // QSizeF::toSize rounds to nearest integer so do the same here
            topHeight = realToInt(borderWidths[1]);
            bottomHeight = realToInt(borderWidths[3]);

            // If whole rect is not filled, take width from default graphics height
            if (!fillWholeRect()) {
                QSizeF topSize = defaultSize(suffixList.at(0)); // index 0 is top frame part

                int topWidth = topSize.toSize().width();
                if (topWidth < fullWidth) {
                    fullWidth = topWidth;
                }
            }
        }

        int centerHeight = fullHeight - topHeight - bottomHeight;

        // Do all 3 pieces fit?
        if (fullHeight >= topHeight + bottomHeight) {
            data.targets[0] = QRect(0, 0, fullWidth, topHeight);
            data.targets[1] = QRect(0, topHeight, fullWidth, centerHeight);
            data.targets[2] = QRect(0, fullHeight - bottomHeight, fullWidth, bottomHeight);

            data.pixmapSizes[0] = QSize(fullWidth, topHeight);
            data.pixmapSizes[1] = QSize(fullWidth, centerHeight);
            data.pixmapSizes[2] = QSize(fullWidth, bottomHeight);

            data.sources[0] = QRect(QPoint(0, 0), data.pixmapSizes[0]);
            data.sources[1] = QRect(QPoint(0, 0), data.pixmapSizes[1]);
            data.sources[2] = QRect(QPoint(0, 0), data.pixmapSizes[2]);
        } else {
            // Only sides fit
            int totalSideHeight = topHeight + bottomHeight;
            data.targets[0] = QRect(0, 0, fullWidth, topHeight * fullHeight / totalSideHeight);
            data.targets[1] = QRect();
            data.targets[2] = QRect(0, data.targets[0].height(), fullWidth, fullHeight - data.targets[0].height());

            data.pixmapSizes[0] = QSize(fullWidth, data.targets[0].height());
            data.pixmapSizes[1] = QSize();
            data.pixmapSizes[2] = QSize(fullWidth, data.targets[2].height());

            data.sources[0] = data.targets[0];
            data.sources[1] = QRect();
            data.sources[2] = data.targets[2];
            data.sources[2].moveTop(data.pixmapSizes[2].height() - data.targets[2].height());
        }

    } else if (type == HbFrameDrawer::NinePieces) {
        QSize tlSize;
        QSize brSize;

        if (!hasBorderWidths()) {
            tlSize = defaultSize(suffixList.at(0)).toSize(); // index 0 is top left
            brSize = defaultSize(suffixList.at(8)).toSize(); // index 8 is bottom right
        } else {
            tlSize = QSizeF(borderWidths[0], borderWidths[1]).toSize();
            brSize = QSizeF(borderWidths[2], borderWidths[3]).toSize();
        }

        // Do all 9 pieces fit?
        if (fullWidth >= tlSize.width() + brSize.width() &&
                fullHeight >= tlSize.height() + brSize.height()) {

            int centerWidth = fullWidth - tlSize.width() - brSize.width();
            int centerHeight = fullHeight - tlSize.height() - brSize.height();

            data.targets[0] = QRect(0, 0, tlSize.width(), tlSize.height());
            data.targets[1] = QRect(tlSize.width(), 0, centerWidth, tlSize.height());
            data.targets[2] = QRect(tlSize.width() + centerWidth, 0, brSize.width(), tlSize.height());
            data.targets[3] = QRect(0, tlSize.height(), tlSize.width(), centerHeight);
            data.targets[4] = QRect(tlSize.width(), tlSize.height(), centerWidth, centerHeight);
            data.targets[5] = QRect(tlSize.width() + centerWidth, tlSize.height(), brSize.width(), centerHeight);
            data.targets[6] = QRect(0, tlSize.height() + centerHeight, tlSize.width(), brSize.height());
            data.targets[7] = QRect(tlSize.width(), tlSize.height() + centerHeight, centerWidth, brSize.height());
            data.targets[8] = QRect(tlSize.width() + centerWidth, tlSize.height() + centerHeight, brSize.width(), brSize.height());

            data.pixmapSizes[0] = data.targets[0].size();
            data.pixmapSizes[1] = data.targets[1].size();
            data.pixmapSizes[2] = data.targets[2].size();
            data.pixmapSizes[3] = data.targets[3].size();
            data.pixmapSizes[4] = data.targets[4].size();
            data.pixmapSizes[5] = data.targets[5].size();
            data.pixmapSizes[6] = data.targets[6].size();
            data.pixmapSizes[7] = data.targets[7].size();
            data.pixmapSizes[8] = data.targets[8].size();

            data.sources[0] = QRect(QPoint(0, 0), data.pixmapSizes[0]);
            data.sources[1] = QRect(QPoint(0, 0), data.pixmapSizes[1]);
            data.sources[2] = QRect(QPoint(0, 0), data.pixmapSizes[2]);
            data.sources[3] = QRect(QPoint(0, 0), data.pixmapSizes[3]);
            data.sources[4] = QRect(QPoint(0, 0), data.pixmapSizes[4]);
            data.sources[5] = QRect(QPoint(0, 0), data.pixmapSizes[5]);
            data.sources[6] = QRect(QPoint(0, 0), data.pixmapSizes[6]);
            data.sources[7] = QRect(QPoint(0, 0), data.pixmapSizes[7]);
            data.sources[8] = QRect(QPoint(0, 0), data.pixmapSizes[8]);

        } else {
            // All 9 pieces do not fit.
            // Corners are drawn always but it is possible that either left&right or top&bottom parts get drawn also.
            int totalCornerWidth = tlSize.width() + brSize.width();
            int totalCornerHeight = tlSize.height() + brSize.height();

            int splitPointX = 0;
            int splitPointY = 0;

            // If corners do not fit fully horizontally or vertically, define split point
            if (totalCornerWidth > fullWidth) {
                splitPointX = tlSize.width() * fullWidth / totalCornerWidth;
            }

            if (totalCornerHeight > fullHeight) {
                splitPointY = tlSize.height() * fullHeight / totalCornerHeight;
            }

            calculateShrinkedNinePieceCorners(
                data, tlSize, brSize,
                QSize(fullWidth, fullHeight), QPoint(splitPointX, splitPointY));

            // Left and right get drawn if corners height does not cover the total height
            if (!splitPointY) {
                data.targets[3] = QRect(0, data.targets[0].height(), data.targets[0].width(), fullHeight - tlSize.height() - brSize.height());
                data.targets[5] = QRect(splitPointX, data.targets[0].height(), data.targets[2].width(), data.targets[3].height());
            } else {
                data.targets[3] = QRect();
                data.targets[5] = QRect();
            }

            // Top and bottom get drawn if corners width does not cover the total width
            if (!splitPointX) {
                data.targets[1] = QRect(data.targets[0].width(), 0, fullWidth - tlSize.width() - brSize.width(), data.targets[0].height());
                data.targets[7] = QRect(data.targets[0].width(), splitPointY, data.targets[1].width(), data.targets[8].height());
            } else {
                data.targets[1] = QRect();
                data.targets[7] = QRect();
            }

            // Center is always empty
            data.targets[4] = QRect();

            data.pixmapSizes[0] = data.targets[0].size();
            data.pixmapSizes[1] = data.targets[1].size();
            data.pixmapSizes[2] = data.targets[2].size();
            data.pixmapSizes[3] = data.targets[3].size();
            data.pixmapSizes[4] = QSize(); // Center is always empty
            data.pixmapSizes[5] = data.targets[5].size();
            data.pixmapSizes[6] = data.targets[6].size();
            data.pixmapSizes[7] = data.targets[7].size();
            data.pixmapSizes[8] = data.targets[8].size();

            data.sources[0] = data.targets[0];
            data.sources[1] = QRect(QPoint(0, 0), data.pixmapSizes[1]);
            data.sources[2] = QRect(QPoint(0, 0), data.pixmapSizes[2]);
            data.sources[2].setBottomLeft(QPoint(data.pixmapSizes[2].width() - data.targets[2].width(), data.targets[2].height() - 1));
            data.sources[3] = QRect(QPoint(0, 0), data.pixmapSizes[3]);
            data.sources[4] = QRect(); // center is always empty
            data.sources[5] = QRect(QPoint(0, 0), data.pixmapSizes[5]);
            data.sources[6] = QRect(QPoint(0, 0), data.pixmapSizes[6]);
            data.sources[6].setTopRight(QPoint(data.targets[6].width() - 1, data.pixmapSizes[6].height() - data.targets[6].height()));
            data.sources[7] = QRect(QPoint(0, 0), data.pixmapSizes[7]);
            data.sources[8] = QRect(QPoint(0, 0), data.pixmapSizes[8]);
            data.sources[8].setTopLeft(QPoint(data.pixmapSizes[8].width() - data.targets[8].width(), data.pixmapSizes[8].height() - data.targets[8].height()));
        }
    }

#ifdef HB_FRAME_DRAWER_TRACES
    qDebug() << "FRAMENAME: " << frameGraphicsName << "Size: " << fullWidth << ", " << fullHeight;
#endif

    return QSize(fullWidth, fullHeight);
}


/*!
\internal
*/
void HbFrameDrawerPrivate::calculateShrinkedNinePieceCorners(
    HbMultiPartSizeData &data,
    const QSize &tlSize, const QSize &brSize,
    const QSize &fullSize, const QPoint &splitPoint)
{
    // Make sure that corner sizes don't exceed frames full size

    // Top-left corner
    int topLeftWidth = splitPoint.x() ? splitPoint.x() : tlSize.width();
    int topLeftHeight = splitPoint.y() ? splitPoint.y() : tlSize.height();
    data.targets[0] = QRect(
                          0,
                          0,
                          topLeftWidth < fullSize.width() ? topLeftWidth : fullSize.width(),
                          topLeftHeight < fullSize.height() ? topLeftHeight : fullSize.height());

    // Top-right corner
    int topRightX = splitPoint.x() ? splitPoint.x() : fullSize.width() - brSize.width();
    int topRightWidth = splitPoint.x() ? fullSize.width() - splitPoint.x() : brSize.width();
    if (topRightWidth > fullSize.width()) {
        topRightWidth = fullSize.width();
        topRightX = 0;
    }
    int topRightHeight = splitPoint.y() ? splitPoint.y() : tlSize.height();
    if (topRightHeight > fullSize.height()) {
        topRightHeight = fullSize.height();
    }
    data.targets[2] = QRect(topRightX, 0, topRightWidth, topRightHeight);

    // Bottom-left corner
    int bottomLeftY = splitPoint.y() ? splitPoint.y() : fullSize.height() - brSize.height();
    int bottomLeftWidth = splitPoint.x() ? splitPoint.x() : tlSize.width();
    if (bottomLeftWidth > fullSize.width()) {
        bottomLeftWidth = fullSize.width();
    }
    int bottomLeftHeight = splitPoint.y() ? fullSize.height() - splitPoint.y() : brSize.height();
    if (bottomLeftHeight > fullSize.height()) {
        bottomLeftHeight = fullSize.height();
        bottomLeftY = 0;
    }
    data.targets[6] = QRect(0, bottomLeftY, bottomLeftWidth, bottomLeftHeight);

    // Bottom-right corner
    int bottomRightX = splitPoint.x() ? splitPoint.x() : fullSize.width() - brSize.width();
    int bottomRightY = splitPoint.y() ? splitPoint.y() : fullSize.height() - brSize.height();
    int bottomRightWidth = splitPoint.x() ? fullSize.width() - splitPoint.x() : brSize.width();
    if (bottomRightWidth > fullSize.width()) {
        bottomRightWidth = fullSize.width();
        bottomRightX = 0;
    }
    int bottomRightHeight = splitPoint.y() ? fullSize.height() - splitPoint.y() : brSize.height();
    if (bottomRightHeight > fullSize.height()) {
        bottomRightHeight = fullSize.height();
        bottomRightY = 0;
    }
    data.targets[8] = QRect(bottomRightX, bottomRightY, bottomRightWidth, bottomRightHeight);
}

/*!
\internal
*/
QSizeF HbFrameDrawerPrivate::defaultSize(const QString &framePartSuffix)
{
    HbIconLoader *loader = HbIconLoader::global();
    
    QString nameWithSuffix = frameGraphicsName;
    if (HbThemeUtils::isLogicalName(frameGraphicsName)) {
        // Logical icon name, append suffix in the end
        nameWithSuffix.append(framePartSuffix);
    } else {
        // Absolute icon path, insert suffix before file extension
        int index = this->frameGraphicsName.lastIndexOf(QChar('.'));
        if (index > 0) {
            nameWithSuffix.insert(index, framePartSuffix);
        }
    }

    return loader->defaultSize(nameWithSuffix).toSize();
}

bool HbFrameDrawerPrivate::isMirrored()
{
    // If mirroring mode is default, check automatic mirroring from loader.
    if (mirroring == HbIcon::Default && defaultMirroring == Unknown) {
        HbIconLoader *loader = HbIconLoader::global();
        defaultMirroring = loader->isAutomaticallyMirrored(frameGraphicsName) ? Enabled : Disabled;
    }

    bool basedOnLayoutDir = false;
    // Handle the different mirroring modes

    // Default
    if (mirroring == HbIcon::Default) {
        if (defaultMirroring == Enabled) {
            basedOnLayoutDir = true;
        } else if (defaultMirroring == Disabled) {
            return false;
        }
        // Forced
    } else if (mirroring == HbIcon::Forced) {
        return true;
        // Prevented
    } else if (mirroring == HbIcon::Prevented) {
        return false;
        // LayoutDirection
    } else if (mirroring == HbIcon::LayoutDirection) {
        basedOnLayoutDir = true;
    }

    if (basedOnLayoutDir) {
        Qt::LayoutDirection usedDirection = Qt::LeftToRight; // default;
        if (flags & HbFrameDrawerPrivate::LayoutDirectionSet) {
            usedDirection = layoutDirection;
        } else {
            usedDirection = QApplication::layoutDirection();
        }

        return usedDirection == Qt::LeftToRight ? false : true;
    }

#ifdef HB_FRAME_DRAWER_TRACES
    qDebug() << "HbFrameDrawerPrivate, invalid mirroring mode";
#endif
    return false;
}

/*!
\internal
*/
bool HbFrameDrawerPrivate::hasBorderWidths() const
{
    return borderWidths[1] > 0
           || borderWidths[2] > 0
           || borderWidths[2] > 0
           || borderWidths[3] > 0;
}

/*!
\internal
*/
void HbFrameDrawerPrivate::reset(bool resetFrameCount, bool unloadedByServer)
{
    unLoadIcon(unloadedByServer);
    if (resetFrameCount) {
        frameParts = 0;
    }
}

/*!
 * Drops the underlying icon(s).  No need to send an unload req to server because the
 * server drops the icon by itself in this case.
 */
void HbFrameDrawerPrivate::resetMaskableIcon()
{
    HbIconLoader *loader = HbIconLoader::global();
    if (icon) {
        //consolidated icon case
        icon->decrementRefCount();
        if (icon->refCount() == 0) {
            // remove the item from cache and delete the icon
            if (loader) {
                loader->removeItemInCache(icon->iconImpl());
            }
            icon->dispose();
        }
        icon = 0;
    } else {
        int count = fallbackMaskableIconList.count();
        if (count) {
            // for each item in fallbackMaskableIconList - decrement the reference count and
            // remove the item in cache, dispose if needed.
            foreach(HbMaskableIconImpl * impl, fallbackMaskableIconList) {
                impl->decrementRefCount();
                if (impl->refCount() == 0) {
                    if (loader) {
                        loader->removeItemInCache(impl->iconImpl());
                    }
                    impl->dispose();
                }
            }
            fallbackMaskableIconList.clear();
        }
    }
    frameParts = 0;
}

HbIconLoader::IconLoaderOptions HbFrameDrawerPrivate::iconLoaderOptions()
{
    HbIconLoader::IconLoaderOptions options = DEFAULT_OPTIONS;
    if (flags & HbFrameDrawerPrivate::DoNotCache) {
        options |= HbIconLoader::DoNotCache;
    }
    if (isMirrored()) {
        options |= HbIconLoader::HorizontallyMirrored;
    }
    if (flags & HbFrameDrawerPrivate::ResolutionCorrected) {
        options |= HbIconLoader::ResolutionCorrected;
    }
    if (flags & HbFrameDrawerPrivate::NoAutoStartAnimation) {
        options |= HbIconLoader::NoAutoStartAnimation;
    }
    return options;
}



QString HbFrameDrawerPrivate::multiPartIconId() const
{
    QString id;
    id = frameGraphicsName;
    if (type == HbFrameDrawer::ThreePiecesHorizontal) {
        id.append("_3PH");
    } else if (type == HbFrameDrawer::ThreePiecesVertical) {
        id.append("_3PV");
    } else if (type == HbFrameDrawer::NinePieces) {
        id.append("_9P");
    } else {
        id.append("_1P");
    }

    if (type != HbFrameDrawer::OnePiece) {
        if (fillWholeRect()) {
            id.append("_1");
        } else {
            id.append("_0");
        }

        for (int i = 0; i < 4; i++) {
            QString boundary;
            id.append("_");
            id.append(boundary.setNum(borderWidths[i]));
        }
    }

    for (int i = 0; i < suffixList.count(); i++) {
        id.append(suffixList[i]);
    }

    return id;
}

void HbFrameDrawerPrivate::unLoadIcon(bool unloadedByServer)
{
    HbIconLoader *loader = HbIconLoader::global();
    if (icon) {
        //If a consolidated (stitched) icon was created on the themeserver, then
        //HbIconLoader::unloadIcon() is used to unload it.
        if (loader) {
            loader->unLoadIcon(icon->iconImpl(), unloadedByServer);
        }
        icon->dispose();
        icon = 0;
    }

    // get a vector of QVector<HbIconImpl*> from QVector<HbMaskableIconImpl*> for unloading.
    QVector<HbIconImpl *> fallbackIconList;
    int count = fallbackMaskableIconList.count();
    for (int i = 0; i < count ; i++) {
        if (fallbackMaskableIconList.at(i)) {
            fallbackIconList.append(fallbackMaskableIconList.at(i)->iconImpl());
        }
    }
    count = fallbackIconList.count();
    if (count > 0) {
        //If a consolidated (stitched) icon-creation on themeserver fails, unload-request for all individual
        //frame-items are batched together in a single IPC, which is initiated in HbIconLoader::unLoadMultiIcon().
        loader->unLoadMultiIcon(fallbackIconList);
        for (int i = 0; i < count ; i++) {
            if (fallbackMaskableIconList.at(i)) {
                fallbackMaskableIconList.at(i)->dispose();
            }
        }
    }
    fallbackIconList.clear();  // vector of HbIconImpl*
    fallbackMaskableIconList.clear(); // vector of HbMaskableIconImpl*
}

// This is called from HbIconLoader when building for tools.
// It is not in use in normal builds.
void HbFrameDrawerPrivate::themeChange(const QStringList &updatedFiles)
{
    bool unloadIcons = false;
    if (updatedFiles.count() == 0 || (icon && updatedFiles.contains(icon->iconFileName()))) {
        unloadIcons = true;
    } else {
        HbMaskableIconImpl *fallbackIcon;
        foreach(fallbackIcon, fallbackMaskableIconList) {
            if (fallbackIcon && updatedFiles.contains(fallbackIcon->iconFileName())) {
                unloadIcons = true;
                break;
            }
        }
    }

    if (unloadIcons) {
        unLoadIcon(true);
    }
}

/*! Constructs a new frame drawer item with the cacheFlag enabled  by default.
*/
HbFrameDrawer::HbFrameDrawer(bool cacheFlag)
{
    d = new HbFrameDrawerPrivate();
    if (!cacheFlag) {
        d->flags &= HbFrameDrawerPrivate::DoNotCache;
    }
}

/*! Constructs a new frame drawer with the given \a frameGraphicsName and \a type  and cacheFlag is enabled.
*/
HbFrameDrawer::HbFrameDrawer(const QString &frameGraphicsName, FrameType type, bool cacheFlag)
{
    d = new HbFrameDrawerPrivate(frameGraphicsName, type);
    if (!cacheFlag) {
        d->flags &= HbFrameDrawerPrivate::DoNotCache;
    }
}

/*!
* Copy constructs a new frame drawer using the \a other frame drawer.
* This is very fast.
* Copy-on-write semantics is used, so this only does a shallow copy.
*/
HbFrameDrawer::HbFrameDrawer(const HbFrameDrawer &other) :
    d(other.d)
{
}

/*!
* Assigns the \a other frame drawer to this frame drawer and returns a reference to
* this frame drawer. Copy-on-write semantics is used, so this only does a shallow copy.
*/
HbFrameDrawer &HbFrameDrawer::operator=(const HbFrameDrawer &other)
{
    if (&other != this) {
        d = other.d;
    }
    return *this;
}

/*!
* Destroys the frame drawer.
*/
HbFrameDrawer::~HbFrameDrawer()
{
    delete d;
}

/*!
* Returns true if the frame drawer has been constructed with the default constructor
* and the frame graphics name has not been set.
* \note Even a non-null frame drawer might not be able to draw anything, e.g. if the graphics file do not exist or cannot be read.
*/
bool HbFrameDrawer::isNull() const
{
    return d->frameGraphicsName.isNull();
}

/*!
* Returns the frame graphics name.
* \sa HbFrameDrawer::setFrameGraphicsName()
*/
QString HbFrameDrawer::frameGraphicsName() const
{
    return d->frameGraphicsName;
}

/*!
* Sets the frame graphics name. See the class description for the file name convention for different frame parts.
* \sa HbFrameDrawer::frameGraphicsName()
* \note Logical frame names that define a themable frame should not include filename extension, whereas absolute filenames
* must include full path and filename extension.
*/
void HbFrameDrawer::setFrameGraphicsName(const QString &frameGraphicsName)
{
    QString nameWithoutExt = frameGraphicsName;

    if (HbThemeUtils::isLogicalName(frameGraphicsName)) {
        // Remove possible file extension
        int index = nameWithoutExt.lastIndexOf(QChar('.'));
        if (index>0) {
            nameWithoutExt.resize(index);
        }
    }

    if (d->frameGraphicsName != nameWithoutExt) {
        d->frameGraphicsName = nameWithoutExt;
        // Frame graphics changed, clear frame icon
        d->reset();
        // Frame graphics changed, clear default frame mirroring information based on the automatic mirroring list
        d->defaultMirroring = HbFrameDrawerPrivate::Unknown;

        // if graphicsItem is set, request redraw
        if (d->graphicsItem) {
            d->graphicsItem->update();
        }
    }
}

/*!
* Returns the frame type.
* \sa HbFrameDrawer::setFrameType()
*/
HbFrameDrawer::FrameType HbFrameDrawer::frameType() const
{
    return d->type;
}

/*!
* Sets the frame type.
* \sa HbFrameDrawer::frameType()
*/
void HbFrameDrawer::setFrameType(HbFrameDrawer::FrameType type)
{
    if (d->type != type) {
        d->type = type;
        // Frame type changed, clear frame icon
        d->reset();
    }
}

/*!
* Returns the border width values. If the border widths has not been set by calling setBorderWidths(),
* this method returns zero for all values.
*
* \sa HbFrameDrawer::setBorderWidths()
*/
void HbFrameDrawer::borderWidths(qreal &left, qreal &top, qreal &right, qreal &bottom) const
{
    left = d->borderWidths[0];
    top = d->borderWidths[1];
    right = d->borderWidths[2];
    bottom = d->borderWidths[3];
}

/*!
* Sets the frame border widths ("frame thickness") explicitly for the 3 or 9 parts of the frame.
*
* \note If this method is not called, the bounding rectangle of the frame is divided as follows.
*
* Frame type \b ThreePiecesHorizontal:
*
* - Left part width = (default size of the left part graphics scaled to the bounding rectangle).width
* - Right part width = (default size of the right part graphics scaled to the bounding rectangle).width
* - Center part width = remaining width
*
* Frame type \b ThreePiecesVertical:
*
* - Top part height = (default size of the top part graphics scaled to the bounding rectangle).height
* - Bottom part height = (default size of the bottom part graphics scaled to the bounding rectangle).height
* - Center part height = remaining height
*
* Frame type \b NinePieces:
*
* - Default sizes of the top left and bottom right part graphics are used.
* - Other sizes are calculated from them and the bounding rectangle.
*
* \sa HbFrameDrawer::borderWidths()
*/
void HbFrameDrawer::setBorderWidths(const qreal left, const qreal top, const qreal right, const qreal bottom)
{
    d->setBorderApiProtectionFlag(true);
    if (left != d->borderWidths[0] || top != d->borderWidths[1] ||
            right != d->borderWidths[2] || bottom != d->borderWidths[3]) {
        // Negative values are converted to zero.
        d->borderWidths[0] = qMax((qreal)0.0, left);
        d->borderWidths[1] = qMax((qreal)0.0, top);
        d->borderWidths[2] = qMax((qreal)0.0, right);
        d->borderWidths[3] = qMax((qreal)0.0, bottom);
        // borderWidths changed, clear frame icon
        d->reset(false);
    }
}

/*!
* This is a convenience method. Equals to:
*
* setBorderWidths( horizontal, vertical, horizontal, vertical );
*
* \sa HbFrameDrawer::borderWidths()
*/
void HbFrameDrawer::setBorderWidths(const qreal horizontal, const qreal vertical)
{
    setBorderWidths(horizontal, vertical, horizontal, vertical);
}

/*!
* This is a convenience method. Equals to:
*
* setBorderWidths( width, width, width, width );
*
* \sa HbFrameDrawer::borderWidths()
*/
void HbFrameDrawer::setBorderWidth(const qreal width)
{
    setBorderWidths(width, width, width, width);
}


/*!
* Returns the bounding rectangle of the frame.
* \sa HbFrameDrawer::setRect()
*/
QRectF HbFrameDrawer::rect() const
{
    return d->rect;
}

/*!
* Sets the bounding rectangle for the frame.
* \sa HbFrameDrawer::rect()
*/
void HbFrameDrawer::setRect(const QRectF &rect)
{
    if (rect != d->rect) {
        d->rect = rect;
        // Rect changed, clear frame icon
        d->reset(false);
    }
}

/*!
* Paints the frame with the given \a painter.
*/
void HbFrameDrawer::paint(QPainter *painter, const QRectF &rect) const
{
    const_cast<HbFrameDrawer *>(this)->setRect(rect);

    HbIconLoader *loader = HbIconLoader::global();
    if (d->frameGraphicsName.isEmpty() || d->type == HbFrameDrawer::Undefined || d->rect.isEmpty() || !loader) {
        return;
    }

    // Lazy graphics rasterizing is used.
    // Rasterize the frame parts now if that has not been done yet.
    if (d->icon && (rect.toRect().size() !=  d->prevRect.size())) {
        d->reset();
    }

    // update the rendering mode
    QPaintEngine *paintEngine = painter->paintEngine();
    if (paintEngine) {
        loader->updateRenderingMode(paintEngine->type());
    }
    d->prepareFrameIcon();
    d->prevRect = rect.toRect();
    // Paint the frame
    d->paint(painter);
}

/*!
* Returns whether the whole bounding rectangle is always filled by the frame graphics.
* By default, this property is false and in that case, the 3-piece horizontal frame height
* and 3-piece vertical frame width are defined by the default sizes of the frame part graphics.
* If the whole bounding rectangle is not filled, the frame area is centered
* in the bounding rectangle.
*
* \note 9-piece frames always fill the whole bounding rectangle.
*
* \sa HbFrameDrawer::setFillWholeRect()
*/
bool HbFrameDrawer::fillWholeRect() const
{
    return d->fillWholeRect();
}

/*!
* Sets the property whether the whole bounding rectangle is always filled by the frame graphics.
* By default, this property is false and in that case, the 3-piece horizontal frame height
* and 3-piece vertical frame width are defined by the default sizes of the frame part graphics.
* If the whole bounding rectangle is not filled, the frame area is centered
* in the bounding rectangle.
*
* \note 9-piece frames always fill the whole bounding rectangle.
*
* \sa HbFrameDrawer::fillWholeRect()
*/
void HbFrameDrawer::setFillWholeRect(bool fill)
{
    if ((fill && !d->fillWholeRect()) || (!fill && d->fillWholeRect())) {
        if (fill) {
            d->flags |= HbFrameDrawerPrivate::FillWholeRect;
        } else {
            d->flags &= ~HbFrameDrawerPrivate::FillWholeRect;
        }

        // Fill mode changed, clear frame Icon
        d->reset(false);
    }
}

/*! Returns the mirroring mode set for the frame drawer.
* \sa HbFrameDrawer::setMirroringMode()
*/
HbIcon::MirroringMode HbFrameDrawer::mirroringMode() const
{
    return d->mirroring;
}

/*! Sets the mirroring \a mode for the frame drawer.
* \sa HbFrameDrawer::mirroringMode()
*/
void HbFrameDrawer::setMirroringMode(HbIcon::MirroringMode mode)
{
    if (mode != d->mirroring) {
        d->mirroring = mode;
        // Mirroring changed, clear frame Icon
        d->reset(false);
    }
}

/*! Returns the filename suffix list.
* See the class level documentation for how frame part filenames are determined
* using the these filename suffixes. If the suffix list has been set explicitly with
* method HbFrameDrawer::setFileNameSuffixList, that list is returned.
* Otherwise, a list of default suffixes (see class level documentation) is returned.
*
* \note If the explicitly given suffix list does not have enough entries for the
* defined frame type, the default suffix list is returned.
*
* \sa HbFrameDrawer::setFileNameSuffixList()
*/
QStringList HbFrameDrawer::fileNameSuffixList() const
{
    return d->fileNameSuffixList();
}

/*! Sets the filename suffix list.
* If the frame type is ThreePiecesHorizontal or ThreePiecesVertical, the list must contain three entries.
* If the frame type is NinePieces, the list must contain nine entries.
* The order of the list entries is the following.
*
* Frame type \b ThreePiecesHorizontal:
*
* - [0] left
* - [1] center
* - [2] right
*
* Frame type \b ThreePiecesVertical:
*
* - [0] top
* - [1] center
* - [2] bottom
*
* Frame type \b NinePieces:
*
* - [0] top left
* - [1] top
* - [2] top right
* - [3] left
* - [4] center
* - [5] right
* - [6] bottom left
* - [7] bottom
* - [8] bottom right
*
* Example:
*
* A three piece frame has the following frame part filenames:
* ":/myapp/frame_a.svg", ":/myapp/frame_b.svg", ":/myapp/frame_c.svg".
* The following code sets the frame graphics name and the filename suffixes.
*
* \dontinclude ultimatecodesnippet/ultimatecodesnippet.cpp
* \skip Start of snippet 20
* \until End of snippet 20
*
* \sa HbFrameDrawer::fileNameSuffixList()
*/
void HbFrameDrawer::setFileNameSuffixList(const QStringList &list)
{
    if (list != d->suffixList) {
        d->suffixList = list;
        // Frame part filename suffix list changed, clear frame parts count info and generated frame icon.
        d->reset();
    }
}

/*!
* Sets the mask to be applied with the entire frame icon.
* If the mask is also a frame item, use another frame drawer to draw it.
* Mask should be of the same size as returned by frameSize().
* To unset the mask, set it to a null pixmap.
*
* \warning Currently this method makes use of pixmap() routine in case of NVG icons.
* pixmap() slows down the hardware accelerated rendering.
* \sa frameSize(), mask()
*/
void HbFrameDrawer::setMask(const QPixmap &mask)
{
    d->mask = mask;
    d->maskChanged = true;
}

/*!
* Sets the \a clipPath to be applied with the entire frame icon.
*
*/
void HbFrameDrawer::setClipPath(const QPainterPath &clipPath)
{
    d->clipPath = clipPath;
}

/*!
* Returns the clippath set on the frame drawer.
* By default, returns a empty QPainterPath.
* \sa setClipPath()
*/

QPainterPath HbFrameDrawer::clipPath() const
{
    return d->clipPath;
}

/*!
* Sets the mask to be applied with the entire frame icon.
* If the mask is also a frame item, use another frame drawer to draw it.
* Mask should be of the same size as returned by frameSize().
* To unset the mask, set it to a null bitmap.
*
 \warning Currently this method makes use of pixmap() routine in case of NVG icons.
* pixmap() slows down the hardware accelerated rendering.
*
*\sa frameSize(), mask()
*/
void HbFrameDrawer::setMask(const QBitmap &mask)
{
    d->mask = mask;
    d->maskChanged = true;
}

/*!
* Returns the mask set on the frame drawer.
* By default, returns a null pixmap.
* \sa setMask()
*/

QPixmap HbFrameDrawer::mask() const
{
    return d->mask;
}


/*!
* Returns the mask set on the frame drawer.
* By default, returns a null bitmap.
* \sa setMask()
*/
QBitmap HbFrameDrawer::maskBitmap() const
{
    return d->mask;
}
/*!
 * Returns the size of the entire frame icon.
 * Use this method to retrieve the correct size for a mask pixmap that can be set with method setMask().
 * If the frame graphics name, bounding rectangle or frame type have not been set, this method returns
 * an empty size.
 */
QSize HbFrameDrawer::frameSize() const
{
    if (d->frameGraphicsName.isEmpty() || d->type == HbFrameDrawer::Undefined || d->rect.isEmpty()) {
        return QSize(0, 0);
    }

    d->prepareFrameIcon();

    if (d->icon) {
        return d->icon->keySize().toSize();
    } else if (d->fallbackMaskableIconList.count()) {
        QSize sz;
        for (int i = 0, ie = d->fallbackMaskableIconList.count(); i != ie; ++i) {
            if (d->fallbackMaskableIconList[i]) {
                sz += d->multiPartSizeData.targets[i].size();
            }
        }
        return sz;
    }

    return QSize(0, 0);
}

/*!
* This function should be called when the theme changes. This clears the used frame
* graphics. This is typically handled by HbFrameItem so there is no need to call this function
* manually when using the framedrawer with a HbFrameItem.
*/
void HbFrameDrawer::themeChanged()
{
#ifndef HB_TOOL_INTERFACE
    // This needs to be disabled to prevent full theme updates when using partial updates with tools.
    d->reset(true, true);
#endif
}

/*!
* Sets the layout direction of the frame drawer.
*
* Valid only when the mirroring mode is HbIcon::LayoutDirection.
*/
void HbFrameDrawer::setLayoutDirection(Qt::LayoutDirection direction)
{
    d->flags |= HbFrameDrawerPrivate::LayoutDirectionSet;

    if (d->layoutDirection != direction) {
        d->layoutDirection = direction;

        if ((d->mirroring == HbIcon::Default
                && d->defaultMirroring == HbFrameDrawerPrivate::Enabled) ||
                d->mirroring == HbIcon::LayoutDirection) {

            d->reset(false);
        }
    }
}

/*!
* Sets the \a item which is needs to be redrawn when the frame drawer
* needs re-paint.
*/
void HbFrameDrawer::setGraphicsItem(QGraphicsItem *item)
{
    d->graphicsItem = item;
}

/*!
* Returns the iconType
*/
HbIconFormatType HbFrameDrawerPrivate::iconFormatType() const
{
    return iconType;
}
// End of File
