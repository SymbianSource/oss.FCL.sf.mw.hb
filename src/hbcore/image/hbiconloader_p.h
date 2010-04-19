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

#ifndef HBICONLOADER_P_H
#define HBICONLOADER_P_H

#include <hbglobal.h>
#include <hbnamespace.h>
#include <hbthemecommon_p.h>
#include <QStringList>
#include <QIcon> //krazy:exclude=qclasses
#include <QFlags>

class HbIconLoaderPrivate;
class HbIconAnimator;
class HbIconImpl;
struct HbIconLoadingParams;
class HbIconAnimationDefinition;
class HbIconSource;

class HB_CORE_PRIVATE_EXPORT HbIconLoader : public QObject
{
 Q_OBJECT

public:
    enum Purpose {
        AnyPurpose = -1,
        Lists = 0,
        Desktop,
        Title,
        Generic,
        Style
    };

    enum IconDataType {
        AnyType,
        RasterData,
        VectorData
    };

    enum IconLoaderOption {
        NoOptions               = 0x00,
        ReturnUnknownIcon       = 0x01,
        VectorIcons             = 0x02,
        BitmapIcons             = 0x04,
        DoNotCache              = 0x08,
        HorizontallyMirrored    = 0x10,
        ResolutionCorrected     = 0x20,
        NoAutoStartAnimation    = 0x40
    };

    Q_DECLARE_FLAGS (IconLoaderOptions, 
                    IconLoaderOption)
 
    explicit HbIconLoader(const QString &appName = QString(), QObject *parent = 0);

    ~HbIconLoader();

    static HbIconLoader *global();
    static QString formatFromPath(const QString &iconPath);

    QPixmap loadIcon(
        const QString &iconName,
        HbIconLoader::Purpose = AnyPurpose,
        const QSizeF &size = QSizeF(0.0, 0.0),
        Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio,
        QIcon::Mode = QIcon::Normal,
        IconLoaderOptions options = IconLoaderOptions(ReturnUnknownIcon | BitmapIcons | VectorIcons),
        HbIconAnimator *animator = 0,
        const QColor &color = QColor());

    HbIconImpl* loadIcon(
        const QString &iconName,
        HbIconLoader::IconDataType type,
        HbIconLoader::Purpose = AnyPurpose,
        const QSizeF &size = QSizeF(0.0, 0.0),
        Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio,
        QIcon::Mode = QIcon::Normal,
        IconLoaderOptions options = IconLoaderOptions(ReturnUnknownIcon | BitmapIcons | VectorIcons),
        HbIconAnimator *animator = 0,
        const QColor &color = QColor());

    void unLoadIcon(HbIconImpl * icon);
    void unLoadMultiIcon(QVector<HbIconImpl *> &multiPieceImpls);
        
    HbIconImpl* loadMultiPieceIcon(const QStringList &listOfIcons,
                    HbMultiPartSizeData &multiPartIconData,
                    const QSizeF &size,
                    Qt::AspectRatioMode aspectRatioMode,
                    QIcon::Mode mode,
                    IconLoaderOptions options,
                    QVector<HbIconImpl *> &multiPieceImpls,
                    const QColor &color);

    QSizeF defaultSize(
        const QString &iconName,
        const QString &appName = QString(),
        IconLoaderOptions options = IconLoaderOptions(ReturnUnknownIcon | BitmapIcons | VectorIcons ));

    QSizeF size(HbIconLoader::Purpose) const;

    static bool iconsExist(const QString &iconName, const QStringList &suffixList);
    HbIconSource *getIconSource(const QString &filename, const QString &format);
    bool isAutomaticallyMirrored(const QString &iconName);

    void setSourceResolution( int resolution );
    int sourceResolution() const;

    void setResolution( int resolution );
    int resolution() const;

    void applyResolutionCorrection(QSizeF &size);

    QString findSharedResource(const QString &name,
                               Hb::ResourceType resType = Hb::IconResource);

signals:
    void defaultSizeAdjustmentChanged();

private slots:
    void destroy();
    void updateLayoutDirection();

private:
    void resolveCleanIconName(HbIconLoadingParams &params) const;
    QSizeF getAnimationDefaultSize(HbIconAnimationDefinition &def, HbIconLoadingParams &params);
    void loadAnimation(HbIconAnimationDefinition &def, HbIconLoadingParams &params);
    QString resolveIconFileName(HbIconLoadingParams &params);
    HbIconImpl *getIconFromServer(HbIconLoadingParams &params);
    void getMultiIconImplFromServer(QStringList &multiPartIconList,                                                             
                                    QVector<QSizeF>  &sizeList,
                                    Qt::AspectRatioMode aspectRatioMode,
                                    QIcon::Mode mode,
                                    bool mirrored,
                                    HbIconLoader::IconLoaderOptions options,
                                    const QColor &color,
                                    HbIconLoader::IconDataType type,
                                    HbIconLoader::Purpose,
                                    QVector<HbIconImpl *> & iconImplList);
   
    void loadSvgIcon(HbIconLoadingParams &params);
    void loadPictureIcon(HbIconLoadingParams &params);
    void loadAnimatedIcon(HbIconLoadingParams &params, const QString &format);
    void loadPixmapIcon(HbIconLoadingParams &params, const QString &format);

    friend class HbApplication;
    friend class HbIconLoaderPrivate;

private:
    Q_DISABLE_COPY (HbIconLoader)
    HbIconLoaderPrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(HbIconLoader::IconLoaderOptions)

#endif // HBICONLOADER_P_H
