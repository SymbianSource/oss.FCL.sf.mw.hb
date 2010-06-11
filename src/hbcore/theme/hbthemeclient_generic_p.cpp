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

#include "hbthemeclient_p_p.h"
#include <QLocalSocket>
#include <QProcess>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>

#include <hbinstance.h>
#include <hbtheme_p.h>
#include <hbtheme.h>
#include <hbevent.h>
#include "hbthemecommon_p.h"
#include "hbmemorymanager_p.h"

static const int WAIT_TIME_TO_CONNECT_TO_SERVER = 500;
static const int WAIT_TIME_TO_START_SERVER = 5000;
#if defined(Q_OS_SYMBIAN) || defined(Q_OS_WIN)
static const QString SERVERFILENAME = QLatin1String("hbthemeserver.exe");
#else
static const QString SERVERFILENAME = QLatin1String("hbthemeserver");
#endif
static const QString SERVERFILEPATH = QLatin1String(HB_BIN_DIR)
                                      + QDir::separator() + SERVERFILENAME;
static const QStringList SERVERARGUMENTS = QStringList() << QLatin1String("-start");

/**
 * Constructor
 */
HbThemeClientPrivate::HbThemeClientPrivate() :
        clientConnected(false),
        localSocket(new QLocalSocket())
{
#ifdef THEME_SERVER_TRACES
    qDebug() << Q_FUNC_INFO ;
#endif
}

/**
 * connectToServer
 */
bool HbThemeClientPrivate::connectToServer()
{
    localSocket->connectToServer(THEME_SERVER_NAME);

    // This logic needs to be improved
    bool success = localSocket->waitForConnected(WAIT_TIME_TO_CONNECT_TO_SERVER);

#ifdef THEME_SERVER_TRACES
    qDebug() << Q_FUNC_INFO << "Socket Connect status: " << success;
#endif

    if(!success) {
        QProcess *newProcess = new QProcess();
        if (QFile::exists(SERVERFILENAME)) {
            newProcess->start(SERVERFILENAME, SERVERARGUMENTS);
            success = newProcess->waitForStarted(WAIT_TIME_TO_START_SERVER);
        }
        if (!success) {
            newProcess->start(SERVERFILEPATH, SERVERARGUMENTS);
            success = newProcess->waitForStarted(WAIT_TIME_TO_START_SERVER);
        }
#ifdef THEME_SERVER_TRACES
        qDebug() << Q_FUNC_INFO << "Server Start Status: " << success
                 << "Error = " << newProcess->error();
#endif

        // If server started
        if (success) {
            // ToDo: This is to wait for server to start running. Logic needs to be improved.
            newProcess->waitForFinished(3000);
#ifdef THEME_SERVER_TRACES
            qDebug() << Q_FUNC_INFO << " Server Start Wait is over" ;
#endif
            localSocket->connectToServer(THEME_SERVER_NAME);
            success = localSocket->waitForConnected();
#ifdef THEME_SERVER_TRACES
            qDebug() << Q_FUNC_INFO << "socketconnected : " << success;
#endif
        }
    }
    if (success) {
        connect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    }

    return clientConnected = success;
}

/**
 * Destructor
 */
HbThemeClientPrivate::~HbThemeClientPrivate()
{
    localSocket->disconnectFromServer();
    delete localSocket;
}

/**
 * HbThemeClientPrivate::getSharedIconInfo()
 */
HbSharedIconInfo HbThemeClientPrivate::getSharedIconInfo(const QString &iconPath,
                                                         const QSizeF &size,
                                                         Qt::AspectRatioMode aspectRatioMode,
                                                         QIcon::Mode mode,
                                                         bool mirrored,
                                                         HbIconLoader::IconLoaderOptions options,
                                                         const QColor &color,
                                                         HbRenderingMode renderMode)
{
#ifdef THEME_SERVER_TRACES
    qDebug() << Q_FUNC_INFO;
#endif
    HbSharedIconInfo iconInfo;
    iconInfo.type = INVALID_FORMAT;

    if ( !clientConnected ) {
        return iconInfo;
    }

    QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    HbThemeServerRequest requestType;
    requestType = EIconLookup;
    outputDataStream << (int)requestType;
    outputDataStream << iconPath;
    outputDataStream << size;
    outputDataStream << aspectRatioMode;
    outputDataStream << mode;
    outputDataStream << mirrored;
    outputDataStream << options;
    outputDataStream << color;
    outputDataStream << renderMode;

    //@to do  block the segment upto the connect
    // changeTheme() slot should not be called for pixmap lookup case. So disconnecting.
    disconnect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    localSocket->write(outputByteArray);
    localSocket->flush();
    localSocket->waitForReadyRead();
#ifdef THEME_SERVER_TRACES
    qDebug() << "image req : " << iconPath;
#endif
    QByteArray inputByteArray = localSocket->readAll();
    QDataStream inputDataStream(inputByteArray);
    HbThemeServerRequest request;
    int temp;
    inputDataStream >> temp;
    request = (HbThemeServerRequest)temp;

    // Need to handle the situation when both themechange
    // request and pixmap info comes at the same time
    // Just posting the ThemeChnaged event so that it can be handled
    // as next event and current pixmap load is not interrupted
    if (EIconLookup==request) {
        readIconInfo(inputDataStream, iconInfo);
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EThemeSelection==request) {
                QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
            }
        }
    } else if (EThemeSelection==request){
        // Asked for pixmap, got theme change request.. clean theme name
        QString themeName;
        inputDataStream >> themeName;
        QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EIconLookup==request) {
                readIconInfo(inputDataStream, iconInfo);
            }
        }
    }
    // connecting again to handle theme change request from server
    connect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    return iconInfo;
}

/*
 * HbThemeClientPrivate::getSharedLayoutDefs()
 *
 * Returns the layout definition for the given file name,layout name,section name
*/
HbWidgetLoader::LayoutDefinition *HbThemeClientPrivate::getSharedLayoutDefs(
        const QString &fileName, const QString &layout, const QString &section)
{
    if ( !clientConnected ) {
        return 0;
    }

    QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    HbThemeServerRequest requestType = EWidgetMLLookup;

    outputDataStream << int(requestType);
    outputDataStream << fileName;
    outputDataStream << layout;
    outputDataStream << section;

    disconnect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));

    localSocket->write(outputByteArray);
    localSocket->flush();
    localSocket->waitForReadyRead();

    QByteArray inputByteArray = localSocket->readAll();
    QDataStream inputDataStream(&inputByteArray, QIODevice::ReadOnly);
    HbThemeServerRequest request;
    int temp;
    int sharedMLOffset = -1;
    inputDataStream >> temp;
    request = static_cast<HbThemeServerRequest>(temp);

     if (EWidgetMLLookup == request) {
        inputDataStream >> sharedMLOffset;
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = static_cast<HbThemeServerRequest>(temp);
            if (EThemeSelection == request) {
                QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
            }
        }
    } else if (EThemeSelection == request){
        QString themeName;
        inputDataStream >> themeName;
        QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EWidgetMLLookup==request) {
                inputDataStream >> sharedMLOffset;
            }
        }
    }
    // connecting again to handle theme change request from server
    connect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));

    if (sharedMLOffset >= 0) {
        return HbMemoryUtils::getAddress<HbWidgetLoader::LayoutDefinition>(
                HbMemoryManager::SharedMemory, sharedMLOffset);
    } else {
        return 0;
    }
}

/**
 * HbThemeClientPrivate::getSharedStyleSheet()
 */
HbCss::StyleSheet *HbThemeClientPrivate::getSharedStyleSheet(
        const QString &filePath, HbLayeredStyleLoader::LayerPriority priority)
{
#ifdef THEME_SERVER_TRACES
    qDebug() << Q_FUNC_INFO;
#endif

    if ( !clientConnected ) {
        return 0;
    }

    QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    HbThemeServerRequest requestType = EStyleSheetLookup;

    outputDataStream << int(requestType);
    outputDataStream << filePath;
    outputDataStream << priority;

    disconnect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    localSocket->write(outputByteArray);
    localSocket->flush();
    localSocket->waitForReadyRead();
#ifdef THEME_SERVER_TRACES
    qDebug() << "stylesheet req : " << fileName;
#endif
    QByteArray inputByteArray = localSocket->readAll();
    QDataStream inputDataStream(inputByteArray);
    HbThemeServerRequest request;
    int temp;

    //-1 represents invalid offset
    int cssOffset = -1;
    inputDataStream >> temp;
    request = static_cast<HbThemeServerRequest>(temp);

    // Need to handle the situation when both themechange
    // request and stylesheet lookup info comes at the same time
    // Just posting the ThemeChanged event so that it can be handled
    // as next event and current stylesheet load is not interrupted
    if (EStyleSheetLookup==request) {
        inputDataStream >> cssOffset;
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EThemeSelection==request) {
                QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
            }
        }
    } else if (EThemeSelection==request){
        // Asked for stylesheet, got theme change request.. clean theme name
        QString themeName;
        inputDataStream >> themeName;
        QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EStyleSheetLookup==request) {
                inputDataStream >> cssOffset;
            }
        }
    }

    // connecting again to handle theme change request from server
    connect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));

    //if a valid offset is obtained from the server return this offset
    if (cssOffset >= 0) {
        return HbMemoryUtils::getAddress<HbCss::StyleSheet>(
            HbMemoryManager::SharedMemory, cssOffset);
    } else {
        return 0;
    }
}

/**
 * HbThemeClientPrivate::deviceProfiles()
 */
HbDeviceProfileList *HbThemeClientPrivate::deviceProfiles()
{
#ifdef THEME_SERVER_TRACES
	qDebug() << Q_FUNC_INFO;
#endif

    if ( !clientConnected ) {
        return 0;
    }

    QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    HbThemeServerRequest requestType = EDeviceProfileOffset;
    outputDataStream << int(requestType);

    disconnect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    localSocket->write(outputByteArray);
    localSocket->flush();
    localSocket->waitForReadyRead();

    QByteArray inputByteArray = localSocket->readAll();
    QDataStream inputDataStream(inputByteArray);
    HbThemeServerRequest request;
    int temp;

    //-1 represents invalid offset
    int deviceProfileOffset = -1;
    inputDataStream >> temp;
    request = (HbThemeServerRequest)temp;

    if (EDeviceProfileOffset == request) {
        inputDataStream >> deviceProfileOffset;
    	if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EThemeSelection==request) {
                QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
            }
        }
    } else if (EThemeSelection==request){
        // Asked for DeviceProfiles Offset, got theme change request..
        // clean theme name
        QString themeName;
        inputDataStream >> themeName;
        QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EDeviceProfileOffset== request) {
                inputDataStream >> deviceProfileOffset;
            }
        }
    }

    // connecting again to handle theme change request from server
    connect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));

    //if a valid offset is obtained from the server return this offset
    if (deviceProfileOffset >= 0) {
        return HbMemoryUtils::getAddress<HbDeviceProfileList>(
            HbMemoryManager::SharedMemory, deviceProfileOffset);
    } else {
        return 0;
    }
}

/**
 * HbThemeClientPrivate::getSharedEffect()
 */
HbEffectFxmlData *HbThemeClientPrivate::getSharedEffect(const QString &filePath)
{
#ifdef THEME_SERVER_TRACES
    qDebug() << Q_FUNC_INFO;
#endif

    if ( !clientConnected ) {
        return 0;
    }

    QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    HbThemeServerRequest requestType = EEffectLookupFilePath;

    outputDataStream << int(requestType);
    outputDataStream << filePath;

    disconnect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    localSocket->write(outputByteArray);
    localSocket->flush();
    localSocket->waitForReadyRead();

#ifdef THEME_SERVER_TRACES
    qDebug() << "EEffectLookupFilePath req : " << filePath;
#endif

    QByteArray inputByteArray = localSocket->readAll();
    QDataStream inputDataStream(inputByteArray);
    HbThemeServerRequest request;
    int temp;

    //-1 represents invalid offset
    int effectOffset = -1;
    inputDataStream >> temp;
    request = (HbThemeServerRequest)temp;

    // Need to handle the situation when both themechange
    // request and effect lookup info comes at the same time
    // Just posting the ThemeChanged event so that it can be handled
    // as next event and current effect load is not interrupted
    if (request == EEffectLookupFilePath) {
        inputDataStream >> effectOffset;
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EThemeSelection == request) {
                QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
            }
        }
    } else if (EThemeSelection == request){
        // Asked for effect, got theme change request.. clean theme name
        QString themeName;
        inputDataStream >> themeName;
        QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EEffectLookupFilePath == request) {
                inputDataStream >> effectOffset;
            }
        }
    }

    // connecting again to handle theme change request from server
    connect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));

    //if a valid offset is obtained from the server return this offset
    if (effectOffset >= 0) {
        return HbMemoryUtils::getAddress<HbEffectFxmlData>(
            HbMemoryManager::SharedMemory, effectOffset);
    } else {
        return 0;
    }
}

/**
 * HbThemeClientPrivate::addSharedEffect()
 */
bool HbThemeClientPrivate::addSharedEffect(const QString &filePath)
{
#ifdef THEME_SERVER_TRACES
    qDebug() << Q_FUNC_INFO;
#endif
    if ( !clientConnected ) {
        return false;
    }

    QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    HbThemeServerRequest requestType = EEffectAdd;

    outputDataStream << int(requestType);
    outputDataStream << filePath;

    disconnect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    localSocket->write(outputByteArray);
    localSocket->flush();
    localSocket->waitForReadyRead();

#ifdef THEME_SERVER_TRACES
    qDebug() << "effectAdd req : " << filePath;
#endif
    QByteArray inputByteArray = localSocket->readAll();
    QDataStream inputDataStream(inputByteArray);
    HbThemeServerRequest request;
    int temp;
    //-1 represents an error adding file to server
    int effectAddReply = -1;
    inputDataStream >> temp;
    request = static_cast<HbThemeServerRequest>(temp);


    //TODO how to really handle situation when adding an effect when theme changes??

    // Need to handle the situation when both themechange
    // request and effect lookup info comes at the same time
    // Just posting the ThemeChanged event so that it can be handled
    // as next event and current effect load is not interrupted
    if (request == EEffectAdd) {
        inputDataStream >> effectAddReply;
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EThemeSelection == request) {
                QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
            }
        }
    }
    else if (EThemeSelection == request){
        // Asked for effect, got theme change request.. clean theme name
        QString themeName;
        inputDataStream >> themeName;
        QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EEffectAdd == request) {
                inputDataStream >> effectAddReply;
            }
        }
    }

    // connecting again to handle theme change request from server
    connect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));

    return (effectAddReply >= 0);
}

/**
 * HbThemeClientPrivate::changeTheme()
 */
void HbThemeClientPrivate::changeTheme()
{
#ifdef THEME_SERVER_TRACES
    qDebug() << Q_FUNC_INFO;
#endif

    QByteArray inputByteArray = localSocket->readAll();
    QDataStream inputDataStream(inputByteArray);
    int request;
    inputDataStream >> request;

#ifdef THEME_SERVER_TRACES
    qDebug() << Q_FUNC_INFO << "recognizer: " << request;
#endif

    if(EThemeSelection == request) {
        QString themeName;
        inputDataStream >> themeName;
#ifdef THEME_SERVER_TRACES
        qDebug() << Q_FUNC_INFO << "themeName is : " << themeName;
#endif
        handleThemeChange(themeName);
    }
    if(EThemeContentUpdate == request) {
        QStringList updatedFiles;
        inputDataStream >> updatedFiles;
        hbInstance->theme()->d_ptr->updateTheme(updatedFiles);
    }
}


/**
 * HbThemeClientPrivate::readIconInfo()
 */
void HbThemeClientPrivate::readIconInfo(QDataStream &dataStream, HbSharedIconInfo &iconInfo)
{
    int temp;
    int tempType;
    dataStream >> tempType;
    iconInfo.type = (HbIconFormatType)tempType;

    if( iconInfo.type == OTHER_SUPPORTED_FORMATS || iconInfo.type == SVG  ){
        dataStream >> iconInfo.pixmapData.offset;
        dataStream >> iconInfo.pixmapData.width;
        dataStream >> iconInfo.pixmapData.height;
        dataStream >> iconInfo.pixmapData.defaultWidth;
        dataStream >> iconInfo.pixmapData.defaultHeight;
        dataStream >> temp;
        iconInfo.pixmapData.format= (QImage::Format)temp;
    }
    else if (/*iconInfo.type == SVG ||*/ iconInfo.type == PIC ){
        dataStream >> iconInfo.picData.offset;
        dataStream >> iconInfo.picData.dataSize;
        dataStream >> iconInfo.picData.defaultWidth;
        dataStream >> iconInfo.picData.defaultHeight;

    }
    else if(iconInfo.type == NVG ){
        dataStream >> iconInfo.nvgData.offset;
        dataStream >> iconInfo.nvgData.dataSize;
    }
    else if (iconInfo.type == BLOB) {
        dataStream >> iconInfo.blobData.offset;
        dataStream >> iconInfo.blobData.dataSize;
    }
}

/**
 * HbThemeClientPrivate::handleThemeChange()
 */
void HbThemeClientPrivate::handleThemeChange(const QString &themeName)
{
#ifdef THEME_SERVER_TRACES
    qDebug() << Q_FUNC_INFO << "themeChanged(): called";
#endif
    hbInstance->theme()->d_ptr->handleThemeChange(themeName);
}

/**
 * HbThemeClientPrivate::event()
 */
bool HbThemeClientPrivate::event(QEvent *e)
{
    if (e->type() == HbEvent::ThemeChanged) {
        hbInstance->theme()->d_ptr->handleThemeChange();
        return true;
    }
    return QObject::event(e);
}

/**
 * HbThemeClientPrivate::unloadIcon()
 */
void HbThemeClientPrivate::unloadIcon(const QString &iconPath,
                                      const QSizeF &size,
                                      Qt::AspectRatioMode aspectRatioMode,
                                      QIcon::Mode mode,
                                      bool mirrored,
                                      const QColor &color,
                                      HbRenderingMode renderMode)
{
    if ( !clientConnected ) {
        return;
    }

    QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    HbThemeServerRequest requestType;
    requestType = EUnloadIcon;
    outputDataStream << int(requestType);
    outputDataStream << iconPath;
    outputDataStream << size;
    outputDataStream << aspectRatioMode;
    outputDataStream << mode;
    outputDataStream << mirrored;
    //outputDataStream << options;
    outputDataStream << color;
    outputDataStream << renderMode;

    //@to do  block the segment upto the connect
    // changeTheme() slot should not be called for pixmap lookup case. So disconnecting.
    disconnect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    localSocket->write(outputByteArray);
    localSocket->flush();
    localSocket->waitForReadyRead();

    QByteArray inputByteArray = localSocket->readAll();
    QDataStream inputDataStream(inputByteArray);
    HbThemeServerRequest request;
    int temp;
    inputDataStream >> temp;
    request = (HbThemeServerRequest)temp;

    // Need to handle the situation when both themechange
    // request and Reference count decrement comes at the same time
    // Just posting the ThemeChnaged event so that it can be handled
    // as next event and current pixmap load is not interrupted
    if (EUnloadIcon == request) {

        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EThemeSelection == request) {
                QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
            }
        }
    }
    else if (EThemeSelection == request){
        // Asked for reference count decrement got theme change request.. clean theme name
        QString themeName;
        inputDataStream >> themeName;
        QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EUnloadIcon == request) {

            }
        }
    }

    // connecting again to handle theme change request from server
    connect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));

}

/**
 * HbThemeClientPrivate::unLoadMultiIcon()
 *
 * unload multiple icons
*/
void HbThemeClientPrivate::unLoadMultiIcon(const QStringList &iconPathList,
                const QVector<QSizeF> &sizeList,
                Qt::AspectRatioMode aspectRatioMode,
                QIcon::Mode mode,
                bool mirrored,
                const QColor &color,
                HbRenderingMode renderMode)
{
    // TODO: this request is currently not implemented in server side.

    if ( !clientConnected ) {
        return;
    }

	QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    HbThemeServerRequest requestType;
    requestType = EUnloadMultiIcon;
    outputDataStream << (int)requestType;
	for(int i = 0; i < iconPathList.count(); i++) {
        outputDataStream << iconPathList[i];
		outputDataStream << sizeList[i];
	}

    outputDataStream << aspectRatioMode;
    outputDataStream << mode;
    outputDataStream << mirrored;
    //outputDataStream << options;
    outputDataStream << color;
    outputDataStream << renderMode;

    //@to do  block the segment upto the connect
    // changeTheme() slot should not be called for pixmap lookup case. So disconnecting.
    disconnect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    localSocket->write(outputByteArray);
    localSocket->flush();
    localSocket->waitForReadyRead();

    QByteArray inputByteArray = localSocket->readAll();
    QDataStream inputDataStream(inputByteArray);
    HbThemeServerRequest request;
    int temp;
    inputDataStream >> temp;
    request = (HbThemeServerRequest)temp;

    // Need to handle the situation when both themechange
    // request and Reference count decrement comes at the same time
    // Just posting the ThemeChnaged event so that it can be handled
    // as next event and current pixmap load is not interrupted
    if (EUnloadMultiIcon == request) {
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = static_cast<HbThemeServerRequest>(temp);
            if (EThemeSelection == request) {
                QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
            }
        }
    } else if (EThemeSelection == request){
        // Asked for reference count decrement got theme change request.. clean theme name
        QString themeName;
        inputDataStream >> themeName;
        QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EUnloadIcon == request) {

            }
        }
    }
    // connecting again to handle theme change request from server
    connect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
}

HbSharedIconInfo HbThemeClientPrivate::getMultiPartIconInfo(
        const QStringList &multiPartIconList,
        const HbMultiPartSizeData &multiPartIconData,
        const QSizeF &size,
        Qt::AspectRatioMode aspectRatioMode,
        QIcon::Mode mode,
        bool mirrored,
        HbIconLoader::IconLoaderOptions options,
        const QColor &color,
        HbRenderingMode renderMode)
{

#ifdef THEME_SERVER_TRACES
    qDebug() << Q_FUNC_INFO;
#endif
    HbSharedIconInfo iconInfo;
    iconInfo.type = INVALID_FORMAT;
    if ( !clientConnected ) {
        return iconInfo;
    }

    int noOfPieces = 1;
    if (multiPartIconData.multiPartIconId.contains("_3PV", Qt::CaseInsensitive)
          || multiPartIconData.multiPartIconId.contains("_3PH", Qt::CaseInsensitive)) {
        noOfPieces = 3;
    } else if (multiPartIconData.multiPartIconId.contains("_9P", Qt::CaseInsensitive)) {
        noOfPieces = 9;
    }

    QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    HbThemeServerRequest requestType;
    requestType = EMultiPieceIcon;
    outputDataStream << int(requestType);
    outputDataStream << multiPartIconList;
    outputDataStream << multiPartIconData.multiPartIconId;

    for (int i = 0; i< noOfPieces; i++) {
        outputDataStream << multiPartIconData.sources[i];
    }
    for (int i = 0; i < noOfPieces; i++) {
        outputDataStream << multiPartIconData.targets[i];
    }
    for (int i = 0; i < noOfPieces; i++) {
        outputDataStream << multiPartIconData.pixmapSizes[i];
    }
    outputDataStream << size;
    outputDataStream << aspectRatioMode;
    outputDataStream << mode;
    outputDataStream << mirrored;
    outputDataStream << options;
    outputDataStream << color;
    outputDataStream << renderMode;
    disconnect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    localSocket->write(outputByteArray);
    localSocket->flush();
    localSocket->waitForReadyRead();
#ifdef THEME_SERVER_TRACES
    qDebug() <<"image req : " <<multiPartIconList;
#endif
    QByteArray inputByteArray = localSocket->readAll();
    QDataStream inputDataStream(inputByteArray);
    HbThemeServerRequest request;
    int temp;
    inputDataStream >> temp;
    request = (HbThemeServerRequest)temp;

    // Need to handle the situation when both themechange
    // request and pixmap info comes at the same time
    // Just posting the ThemeChnaged event so that it can be handled
    // as next event and current pixmap load is not interrupted
    if (EMultiPieceIcon == request) {
        inputDataStream >> temp; // Read the EIconLookup request identifier
        readIconInfo(inputDataStream, iconInfo);
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EThemeSelection == request) {
                QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
            }
        }
    } else if (EThemeSelection == request) {
        // Asked for pixmap, got theme change request.. clean theme name
        QString themeName;
        inputDataStream >> themeName;
        QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EMultiPieceIcon == request) {
                inputDataStream >> temp; // Read the EIconLookup request identifier
                readIconInfo(inputDataStream, iconInfo);
            }
        }
    }

    // connecting again to handle theme change request from server
    connect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    return iconInfo;
}

/**
 * getMultiIconInfo  function returns a list of HbSharedIconInfo
 * for the given list of frameitems.
 */
HbSharedIconInfoList HbThemeClientPrivate::getMultiIconInfo(
        const QStringList &multiPartIconList,
        const QVector<QSizeF> &sizeList,
        Qt::AspectRatioMode aspectRatioMode,
        QIcon::Mode mode,
        bool mirrored,
        HbIconLoader::IconLoaderOptions options,
        const QColor &color,
        HbRenderingMode renderMode)
{
    HbSharedIconInfoList sharedIconInfoList;

#ifdef THEME_SERVER_TRACES
    qDebug() << Q_FUNC_INFO;
#endif
    if ( !clientConnected ) {
        return sharedIconInfoList;
    }

    int noOfPieces = multiPartIconList.count();

    QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    HbThemeServerRequest requestType;
    requestType = EMultiIcon;
    outputDataStream << (int)requestType;
    outputDataStream << multiPartIconList;
    outputDataStream << sizeList;
    outputDataStream << aspectRatioMode;
    outputDataStream << mode;
    outputDataStream << mirrored;
    outputDataStream << options;
    outputDataStream << color;
    outputDataStream << renderMode;

    disconnect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    localSocket->write(outputByteArray);
    localSocket->flush();
    localSocket->waitForReadyRead();
#ifdef THEME_SERVER_TRACES
    qDebug() <<"image req : " <<multiPartIconList;
#endif
    QByteArray inputByteArray = localSocket->readAll();
    QDataStream inputDataStream(inputByteArray);
    HbThemeServerRequest request;
    int temp;
    inputDataStream >> temp;
    request = (HbThemeServerRequest)temp;

    // Need to handle the situation when both themechange
    // request and pixmap info comes at the same time
    // Just posting the ThemeChanged event so that it can be handled
    // as next event and current pixmap load is not interrupted
    if (EMultiIcon == request) {
        for (int i = 0; i< noOfPieces; i++) {
            inputDataStream >> temp; // Read the EIconLookup request identifier
            readIconInfo(inputDataStream, sharedIconInfoList.icon[i]);
        }
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EThemeSelection == request) {
                QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
            }
        }
    } else if (EThemeSelection == request) {
        // Asked for pixmap, got theme change request.. clean theme name
        QString themeName;
        inputDataStream >> themeName;
        QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EMultiIcon == request) {
                for (int i = 0; i < noOfPieces; i++) {
                    inputDataStream >> temp; // Read the EIconLookup request identifier
                    readIconInfo(inputDataStream, sharedIconInfoList.icon[i]);
                }
            }
        }
    }

    // connecting again to handle theme change request from server
    connect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    return sharedIconInfoList;
}

/**
 * HbThemeClientPrivate::setTheme() 
 */
void HbThemeClientPrivate::setTheme(const QString &theme)
{
    if ( !clientConnected ) {
        return;
    }
    
    QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    HbThemeServerRequest requestType = EThemeSelection;
    outputDataStream << (int)requestType;
    outputDataStream << theme;
    localSocket->write(outputByteArray);
    localSocket->flush();
}

/**
 * HbThemeClientPrivate::notifyForegroundLostToServer()
 */
void HbThemeClientPrivate::notifyForegroundLostToServer()
{
}

/**
 * HbThemeClientPrivate::freeSharedMemory()
 */
int HbThemeClientPrivate::freeSharedMemory()
{
#ifdef THEME_SERVER_TRACES
        qDebug() << Q_FUNC_INFO;
#endif
    QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    HbThemeServerRequest requestType = EFreeSharedMem;

    outputDataStream << (int)requestType;

    disconnect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    localSocket->write(outputByteArray);
    localSocket->flush();
    localSocket->waitForReadyRead();

    QByteArray inputByteArray = localSocket->readAll();
    QDataStream inputDataStream(inputByteArray);
    HbThemeServerRequest request;
    int temp;
    int freeSharedMem = 0;

    inputDataStream >> temp;
    request = (HbThemeServerRequest)temp;

    if (EFreeSharedMem == request) {
        inputDataStream >> freeSharedMem;
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EThemeSelection == request) {
                QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
            }
        }
    } else if (EThemeSelection == request){
        QString themeName;
        inputDataStream >> themeName;
        QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EFreeSharedMem == request) {
                inputDataStream >> freeSharedMem;
            }
        }
    }
    // connecting again to handle theme change request from server
    connect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    return freeSharedMem;
}

/**
 * HbThemeClientPrivate::allocatedSharedMemory()
 */
int HbThemeClientPrivate::allocatedSharedMemory()
{
#ifdef THEME_SERVER_TRACES
        qDebug() << Q_FUNC_INFO;
#endif

    QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    HbThemeServerRequest requestType = EAllocatedSharedMem;

    outputDataStream << int(requestType);

    disconnect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    localSocket->write(outputByteArray);
    localSocket->flush();
    localSocket->waitForReadyRead();

    QByteArray inputByteArray = localSocket->readAll();
    QDataStream inputDataStream(inputByteArray);
    HbThemeServerRequest request;
    int temp;
    int allocatedSharedMem = 0;

    inputDataStream >> temp;
    request = (HbThemeServerRequest)temp;

    if (EAllocatedSharedMem == request) {
        inputDataStream >> allocatedSharedMem;
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EThemeSelection == request) {
                QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
            }
        }
    }else if (EThemeSelection == request){
        QString themeName;
        inputDataStream >> themeName;
        QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EAllocatedSharedMem == request) {
                inputDataStream >> allocatedSharedMem;
            }
        }
    }
    // connecting again to handle theme change request from server
    connect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    return allocatedSharedMem;
}

/**
 * HbThemeClientPrivate::allocatedHeapMemory()
 */
int HbThemeClientPrivate::allocatedHeapMemory()
{
#ifdef THEME_SERVER_TRACES
        qDebug() << Q_FUNC_INFO;
#endif
    // currently only supported in Symbian
    return -1;
}

/**
 * HbThemeClientPrivate::switchRenderingMode()
 */
bool HbThemeClientPrivate::switchRenderingMode(HbRenderingMode renderMode)
{
    Q_UNUSED(renderMode);
    return true;
}

#ifdef HB_THEME_SERVER_MEMORY_REPORT
void HbThemeClientPrivate::createMemoryReport() const
{
    if ( !clientConnected ) {
        return;
    }

    QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    HbThemeServerRequest requestType = ECreateMemoryReport;

    outputDataStream << int(requestType);

    disconnect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    localSocket->write(outputByteArray);
    localSocket->flush();
}
#endif

/**
 * HbThemeClientPrivate::typefaceInfo()
 */
HbTypefaceInfoVector *HbThemeClientPrivate::typefaceInfo()
{
#ifdef THEME_SERVER_TRACES
    qDebug() << Q_FUNC_INFO;
#endif
    if ( !clientConnected ) {
        return 0;
    }

    QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    HbThemeServerRequest requestType = ETypefaceOffset;

    outputDataStream << int(requestType);

    disconnect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));
    localSocket->write(outputByteArray);
    localSocket->flush();
    localSocket->waitForReadyRead();

    QByteArray inputByteArray = localSocket->readAll();
    QDataStream inputDataStream(inputByteArray);
    HbThemeServerRequest request;
    int temp;

    //-1 represents invalid offset
    int typefaceOffset = -1;

    inputDataStream >> temp;
    request = (HbThemeServerRequest)temp;

    if (ETypefaceOffset == request) {
        inputDataStream >> typefaceOffset;
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (EThemeSelection == request) {
                QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
            }
        }
    }else if (EThemeSelection == request){
        // Asked for Typeface Offset, got theme change request..
        // clean theme name
        QString themeName;
        inputDataStream >> themeName;
        QCoreApplication::postEvent(this, new HbEvent(HbEvent::ThemeChanged));
        if (!inputDataStream.atEnd()) {
            inputDataStream >> temp;
            request = (HbThemeServerRequest)temp;
            if (ETypefaceOffset == request) {
                inputDataStream >> typefaceOffset;
            }
        }
    }

    // connecting again to handle theme change request from server
    connect(localSocket, SIGNAL(readyRead()), this, SLOT(changeTheme()));

    //if a valid offset is obtained from the server return this offset
    if (typefaceOffset >= 0) {
        return HbMemoryUtils::getAddress<HbTypefaceInfoVector>(
            HbMemoryManager::SharedMemory, typefaceOffset);
    } else {
        return 0;
    }
}
