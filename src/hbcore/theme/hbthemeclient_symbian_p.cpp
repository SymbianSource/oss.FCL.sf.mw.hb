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
#include "hbthemecommon_symbian_p.h"
#include "hbthemelistener_symbian_p.h"
#include "hbmemorymanager_p.h"
#include "hbeffectfxmldata_p.h"
#include "hbinstance.h"
#include "hbtheme_p.h"

#include <QFile>
#include <QDebug>
#include <QTime>

const TUint KDefaultMessageSlots = 4;

/**
* constructor
*/
HbThemeClientPrivate::HbThemeClientPrivate():
    clientConnected(false)
{
    if(THEME_SERVER_NAME != HbMemoryUtils::getCleanAppName()) {
        themelistener = new CHbThemeListenerPrivate(this);
    }


#ifdef HB_SGIMAGE_ICON
    sgDriverInit = false;
#endif
}

/**
* HbThemeClientPrivate::connectToServer()
*
* Connects to the server
*/
bool HbThemeClientPrivate::connectToServer()
{
    TInt error(KErrNone);
    for( int tries(0); tries < 100; tries++) {
        error = CreateSession(KThemeServerName, Version(), KDefaultMessageSlots);
        if(!error) {
            // connected to existing server - OK
            break;
        }
        if(error != KErrNotFound && error != KErrServerTerminated) {
            // problem other than server not here - propagate error
            break;
        }
        error = StartServer();
        if(!error || (error ==KErrAlreadyExists)) {
            // If server launched ok , try again to connect
            continue;
        }
        break; // server not launched : don't cycle round again.
    }
#ifdef HB_SGIMAGE_ICON
    if (!error && !sgDriverInit) {
        error = sgDriver.Open();
    	if (error == KErrNone) {
            sgDriverInit = true;
        }
    }
#endif
    return ( clientConnected = (KErrNone == error) );
}



/**
 * HbThemeClientPrivate::Version()
 *
 * Returns the version number
*/
TVersion HbThemeClientPrivate::Version(void) const
{
    return(TVersion(KThemeServerMajorVersionNumber,
                    KThemeServerMinorVersionNumber,
                    KThemeServerBuildVersionNumber));
}

QSizeF HbThemeClientPrivate::getSharedIconDefaultSize(const QString& iconPath)
{
    if (!clientConnected) {
        return QSizeF();
    }

    QSizeF defaultSize;

    TBuf<256> buffer(iconPath.utf16());
    TPckg<QSizeF> retPckg(defaultSize);

    TIconParams params;
    params.fileName = buffer;

    TPckg<TIconParams> paramPckg(params);
    TIpcArgs args(&paramPckg, &retPckg);

    TInt err = SendReceive(EIconDefaultSize, args);
    if (KErrNone != err) {
        defaultSize = QSizeF();
    }
    return defaultSize;
}

/**
 * HbThemeClientPrivate::getSharedIconInfo()
 *
 * Returns the shared icon information
*/
HbSharedIconInfo HbThemeClientPrivate::getSharedIconInfo(const QString& iconPath ,
                        const QSizeF &size,
                        Qt::AspectRatioMode aspectRatioMode,
                        QIcon::Mode mode,
                        bool mirrored,
                        HbIconLoader::IconLoaderOptions options,
                        const QColor &color,
                        HbRenderingMode renderMode)
{
    HbSharedIconInfo sharedIconInfo;
    sharedIconInfo.type = INVALID_FORMAT;

    if ( !clientConnected ) {
        return sharedIconInfo;
    }

    TBuf<256> buffer(iconPath.utf16());
    TPckg<HbSharedIconInfo> iconInfo(sharedIconInfo);

    TIconParams params;
    params.fileName = buffer;
    params.width = size.width();
    params.height = size.height();
    params.aspectRatioMode = (TUint8)aspectRatioMode;
    params.mode = (TUint8)mode;
    params.options = (TUint8)options;
    params.mirrored = (TBool)mirrored;
    params.rgba = (TUint32) color.rgba();
    params.colorflag = color.isValid();
    params.renderMode = renderMode;

    TPckg<TIconParams> paramPckg(params);

    TIpcArgs args(&paramPckg,&iconInfo);

    TInt err = SendReceive(EIconLookup, args);
    if (KErrNone != err) {
        sharedIconInfo.type  = INVALID_FORMAT;
    }
    return sharedIconInfo;
}

/**
 * getMultiPartIconInfo
 */

HbSharedIconInfo HbThemeClientPrivate::getMultiPartIconInfo(const QStringList &multiPartIconList,
                                                            const HbMultiPartSizeData  &multiPartIconData ,
                                                            const QSizeF &size,
                                                            Qt::AspectRatioMode aspectRatioMode,
                                                            QIcon::Mode mode,
                                                            bool mirrored,
                                                            HbIconLoader::IconLoaderOptions options,
                                                            const QColor &color,
                                                            HbRenderingMode renderMode)
{
    HbSharedIconInfo sharedIconInfo;
    sharedIconInfo.type = INVALID_FORMAT;

    if ( !clientConnected ) {
        return sharedIconInfo;
    }

    TPckg<HbSharedIconInfo> iconInfo(sharedIconInfo);
    TMultiIconSymbParams params;

    TBuf<256> iconId(multiPartIconData.multiPartIconId.utf16());
    params.multiPartIconId.Copy(iconId);
    for (int i = 0; i < multiPartIconList.length(); i++) {
        TBuf<256> pieceIconId(multiPartIconList[i].utf16());
        params.multiPartIconList[i].Copy(pieceIconId);
    }
     int noOfPieces = 1;
     if (multiPartIconData.multiPartIconId.contains("_3PV",Qt::CaseInsensitive)
           || multiPartIconData.multiPartIconId.contains("_3PH",Qt::CaseInsensitive)) {
        noOfPieces = 3;
    } else if (multiPartIconData.multiPartIconId.contains("_9P",Qt::CaseInsensitive)) {
        noOfPieces = 9;
    }

    for (int i = 0; i < noOfPieces; i++) {
        params.sources[i] = (TRect &)multiPartIconData.sources[i];
    }
    for (int i = 0; i < noOfPieces; i++) {
        params.targets[i] = (TRect &)multiPartIconData.targets[i];
    }
    for (int i = 0; i < noOfPieces; i++) {
        params.pixmapSizes[i] = (TSize &)multiPartIconData.pixmapSizes[i];
    }

    params.size = size;
    params.aspectRatioMode = (TUint8)aspectRatioMode;
    params.mode = (TUint8)mode;
    params.options = (TUint8)options;
    params.mirrored = (TBool)mirrored;
    params.rgba = (TUint32) color.rgba();
    params.colorflag = color.isValid();
    params.renderMode = (TUint8)renderMode;

    TPckg<TMultiIconSymbParams> paramPckg(params);
    TIpcArgs args(&paramPckg,&iconInfo);
    TInt err = SendReceive(EMultiPieceIcon, args);
    if (KErrNone != err) {
        sharedIconInfo.type  = INVALID_FORMAT;
    }
    return sharedIconInfo;
}



/**
 * HbThemeClientPrivate::getSharedStyleSheet()
 *
 * Returns the shared css(stylesheet) information
*/
HbCss::StyleSheet *HbThemeClientPrivate::getSharedStyleSheet(const QString &fileName, HbLayeredStyleLoader::LayerPriority priority)
{
    if ( !clientConnected ) {
        return 0;
    }
    HbCss::StyleSheet* styleSheet(0);

    TBuf<256> fileDes(fileName.utf16());
    TBuf<5> layerPriority;
    layerPriority.AppendNum((TInt)priority);

    HbSharedStyleSheetInfo stylesheetInfo;
    TPckg<HbSharedStyleSheetInfo> sharedInfo(stylesheetInfo);

    //TInt fileOffset = -1;
    TIpcArgs args(&fileDes, &layerPriority, &sharedInfo);

#ifdef THEME_SERVER_TRACES
    QTime time;
    time.start();
#endif
    TInt err = SendReceive(EStyleSheetLookup, args);
#ifdef THEME_SERVER_TRACES
    qDebug("Time elapsed in IPC is : %d ms", time.elapsed());
#endif

    if (KErrNone == err) {
        if (stylesheetInfo.offset >= 0) {
            styleSheet = HbMemoryUtils::getAddress<HbCss::StyleSheet>(
                HbMemoryManager::SharedMemory, stylesheetInfo.offset);
        }
    }
    return styleSheet;
}

/**
 * HbThemeClientPrivate::getSharedEffect()
 *
 * Returns the shared effect information
*/
HbEffectFxmlData *HbThemeClientPrivate::getSharedEffect(const QString &filePath)
{
#ifdef THEME_SERVER_TRACES
    qDebug() << "HbThemeClientPrivate::getSharedEffect" << filePath;
#endif
    if ( !clientConnected ) {
        return 0;
    }

    HbEffectFxmlData *fxmlData = 0;

    TBuf<256> fileDes(filePath.utf16());
    HbSharedEffectInfo effectInfo;

    TPckg<HbSharedEffectInfo> sharedInfo(effectInfo);

    TIpcArgs args(&fileDes, &sharedInfo);

#ifdef THEME_SERVER_TRACES
    QTime time;
    time.start();
#endif
    TInt err = SendReceive(EEffectLookupFilePath, args);
#ifdef THEME_SERVER_TRACES
    qDebug() << "Time elapsed in EEffectLookupFilePath IPC is : %d ms" << time.elapsed();
#endif

    if (KErrNone == err) {
#ifdef THEME_SERVER_TRACES
        qDebug() << "HbThemeClientPrivate::getSharedEffect effectInfo.offSet is:" <<  effectInfo.offset;
#endif
        if (effectInfo.offset >= 0) {
            fxmlData = HbMemoryUtils::getAddress<HbEffectFxmlData>(
                HbMemoryManager::SharedMemory, effectInfo.offset);
        } else {
            qWarning() << "get effect offset error!" << effectInfo.offset;
        }
    } else {
        qWarning() << "get effect sendreceive error!" << (int)err;
    }
    return fxmlData;
}

/**
 * HbThemeClientPrivate::addSharedEffect()
 *
 * Adds the shared effect information
*/
bool HbThemeClientPrivate::addSharedEffect(const QString& filePath)
{
#ifdef THEME_SERVER_TRACES
    qDebug() << "HbThemeClientPrivate::addSharedEffect" << filePath;
#endif
    if ( !clientConnected ) {
        return false;
    }

    TBuf<256> fileDes(filePath.utf16());
    TInt retVal = KErrGeneral;

    TPckg<TInt> sharedInfo(retVal);

    TIpcArgs args(&fileDes, &sharedInfo);

#ifdef THEME_SERVER_TRACES
    QTime time;
    time.start();
#endif
    TInt err = SendReceive(EEffectAdd, args);
#ifdef THEME_SERVER_TRACES
    qDebug("Time elapsed in EEffectAdd IPC is : %d ms", time.elapsed());
#endif

    if (KErrNone == err) {
        TInt result = sharedInfo();
#ifdef THEME_SERVER_TRACES
        qDebug() << "TInt result (offset):" << result;
#endif

        if (result >= 0) {
#ifdef THEME_SERVER_TRACES
            qDebug() << "add effect results returning TRUE";
#endif
            return true;
        }
        qWarning() << "add effect offset error!" << (int) result;
    } else {
        qWarning() << "add effect sendreceive error!" << (int) err;
    }
    return false;
}

/**
 * HbThemeClientPrivate::unloadIcon()
 *
 * unload icon
*/
void HbThemeClientPrivate::unloadIcon(const QString& iconPath ,
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

    TBuf<256> buffer(iconPath.utf16());

    TIconParams params;
    params.fileName = buffer;
    params.width = size.width();
    params.height = size.height();
    params.aspectRatioMode = (TUint8)aspectRatioMode;
    params.mode = (TUint8)mode;
    params.options = (TUint8)0;
    params.mirrored = (TBool)mirrored;
    params.rgba = (TUint32) color.rgba();
    params.colorflag = color.isValid();
    params.renderMode = (TUint8)renderMode;

    TPckg<TIconParams> paramPckg(params);
    TIpcArgs args(&paramPckg,0);
    SendReceive(EUnloadIcon, args);
}

/**
 * HbThemeClientPrivate::unLoadMultiIcon()
 *
 * unload multiple icons
*/
void HbThemeClientPrivate::unLoadMultiIcon(const QStringList& iconPathList,
                const QVector<QSizeF> &sizeList,
                Qt::AspectRatioMode aspectRatioMode,
                QIcon::Mode mode,
                bool mirrored,
                const QColor &color,
                HbRenderingMode renderMode)
{
    if ( !clientConnected ) {
        return;
    }

    TIconListParams params;
    int noOfPieces = iconPathList.length();

    for (int i = 0; i < noOfPieces; i++) {

        TFileName pieceIconId(iconPathList[i].utf16());
        params.iconList[i].Copy(pieceIconId);
        params.sizeList[i] = sizeList[i];
    }
    params.aspectRatioMode = (TUint8)aspectRatioMode;
    params.mode = (TUint8)mode;
    params.mirrored = (TBool)mirrored;
    params.rgba = (TUint32) color.rgba();
    params.colorflag = color.isValid();
    params.renderMode = (TUint8)renderMode;
    params.iconCount = noOfPieces;

    TPckg<TIconListParams> paramPckg(params);
    TIpcArgs args(&paramPckg);
    TInt err = SendReceive(EUnloadMultiIcon, args);
}

/* HbThemeClientPrivate::getSharedLayoutDefs()
 *
 * Returns the layout definition for the given file name,layout name,section name
*/
HbWidgetLoader::LayoutDefinition* HbThemeClientPrivate::getSharedLayoutDefs(const QString &fileName,const QString &layout,const QString &section)
{
    if ( !clientConnected ) {
        return 0;
    }

    HbWidgetLoader::LayoutDefinition* layoutDef(0);

    TBuf<256> fileDes(fileName.utf16());
    TBuf<256> layoutDes(layout.utf16());
    TBuf<256> sectionDes(section.utf16());

    HbSharedWMLInfo widgetmlInfo;
    TPckg<HbSharedWMLInfo> wmlInfo(widgetmlInfo);

    TIpcArgs args(&fileDes,&layoutDes,&sectionDes,&wmlInfo);

    TInt err = SendReceive(EWidgetMLLookup, args);

    if (KErrNone == err) {
        if( widgetmlInfo.offset >=0){
            layoutDef = HbMemoryUtils::getAddress<HbWidgetLoader::LayoutDefinition>(
                    HbMemoryManager::SharedMemory, widgetmlInfo.offset);
        }
    }
    return layoutDef;
}

/*
Returns the list of Device Profiles.
*/
HbDeviceProfileList *HbThemeClientPrivate::deviceProfiles()
{
    if ( !clientConnected ) {
		if(!connectToServer()) {
        	qWarning()<<"Theme client unable to connect to server in HbThemeClientPrivate::deviceProfiles";
	    	return 0;
        }
    }

    HbDeviceProfileList *deviceProfiles(0);
    HbDeviceProfileInfo deviceProfileInfo;
    TPckg<HbDeviceProfileInfo> profileInfo(deviceProfileInfo);
    TIpcArgs args(&profileInfo);
    TInt err = SendReceive(EDeviceProfileOffset, args);
    if (KErrNone == err) {
        if( deviceProfileInfo.offset >= 0) {
            deviceProfiles = HbMemoryUtils::getAddress<HbDeviceProfileList>(
                    HbMemoryManager::SharedMemory, deviceProfileInfo.offset);
        }
    }
    return deviceProfiles;
}

/**
 * HbThemeClientPrivate::handleThemeChange()
 */
void HbThemeClientPrivate::handleThemeChange(const QString &themeName)
{
#ifdef THEME_SERVER_TRACES
        qDebug() << Q_FUNC_INFO <<"themeChanged(): called";
#endif
        hbInstance->theme()->d_ptr->handleThemeChange(themeName);
}

/**
 * Destructor
 */
HbThemeClientPrivate::~HbThemeClientPrivate()
{
    RSessionBase::Close();
#ifdef HB_SGIMAGE_ICON
    if (sgDriverInit) {
        sgDriver.Close();
        sgDriverInit = false;
    }
#endif
    delete themelistener;
}

/**
 * StartServer
 */
TInt HbThemeClientPrivate::StartServer()
{
    TFindServer findThemeServer(KThemeServerName);
    TFullName name;
    if ( KErrNone == findThemeServer.Next( name ) ) {
        // Server already running
        return KErrNone;
    }
    return CreateServerProcess();
}

/**
 * CreateServerProcess
 */
TInt HbThemeClientPrivate::CreateServerProcess()
{
    TInt result;
    const TUid KServerUid2={0x100039CE};
    const TUidType serverUid( KNullUid, KServerUid2, KServerUid3 );
    RProcess server;
    _LIT(KThemeServerExe,"hbthemeserver.exe");

    result = server.Create( KThemeServerExe, KNullDesC, EOwnerProcess );

    if (KErrNone != result) {
        return  result;
    }
    TRequestStatus status;
    server.Rendezvous(status);
    if (status!=KRequestPending) {
        server.Kill(0); // abort startup
    }
    else {
        server.Resume(); // logon OK - start the server
    }
    User::WaitForRequest(status);// wait for start or death
    server.Close();
    return status.Int(); // return the error
}

/**
 * getMultiIconInfo  function returns a list of HbSharedIconInfo
 * for the given list of frameitems.
 */
HbSharedIconInfoList HbThemeClientPrivate::getMultiIconInfo(const QStringList &multiPartIconList,
                                                            const QVector<QSizeF>  &sizeList ,
                                                            Qt::AspectRatioMode aspectRatioMode,
                                                            QIcon::Mode mode,
                                                            bool mirrored,
                                                            HbIconLoader::IconLoaderOptions options,
                                                            const QColor &color,
                                                            HbRenderingMode renderMode)
{
    Q_UNUSED(options)

    HbSharedIconInfoList sharedIconInfoList;

    if ( !clientConnected ) {
        return sharedIconInfoList;
    }

    TIconListParams params;
    params.iconCount = multiPartIconList.length();
    for (int i = 0; i < params.iconCount; i++) {
        TFileName pieceIconId(multiPartIconList[i].utf16());
        params.iconList[i].Copy(pieceIconId);
        params.sizeList[i] = sizeList[i];
    }
    params.aspectRatioMode = (TUint8)aspectRatioMode;
    params.mode = (TUint8)mode;
    params.mirrored = (TBool)mirrored;
    params.rgba = (TUint32) color.rgba();
    params.colorflag = color.isValid();
    params.renderMode = (TUint8)renderMode;

    TPckg<TIconListParams> paramPckg(params);
    TPckg<HbSharedIconInfoList> iconInfoList(sharedIconInfoList);
    TIpcArgs args(&paramPckg,&iconInfoList);
    TInt err = SendReceive(EMultiIcon, args);

    return sharedIconInfoList;
}

/**
 *  Notifies the server about the foreground lost event.
 */
void HbThemeClientPrivate::notifyForegroundLostToServer()
{
#if defined(HB_SGIMAGE_ICON) || defined(HB_NVG_CS_ICON)
    TInt err = SendReceive(ENotifyForegroundLost);
    if (KErrNone == err) {
        //ForegroundLostToServer Event is sent to server Successfully.
    }
#endif
}

/**
 * HbThemeClientPrivate::freeSharedMemory()
 */
int HbThemeClientPrivate::freeSharedMemory()
{
    int freeSharedMem = -1;
    if ( !clientConnected ) {
        qWarning()<<"Theme client unable to connect to server in HbThemeClientPrivate::freeSharedMemory";
        return freeSharedMem;
    }

    TPckg<int> freeInfo(freeSharedMem);
    TIpcArgs args(0, &freeInfo);
    TInt err = SendReceive(EFreeSharedMem, args);
    qDebug() << "HbThemeClientPrivate::freeSharedMemory end";
    return freeSharedMem;
}

/**
 * HbThemeClientPrivate::allocatedSharedMemory()
 */
int HbThemeClientPrivate::allocatedSharedMemory()
{
    int allocatedSharedMem = -1;
    if ( !clientConnected ) {
        qWarning()<<"Theme client unable to connect to server in HbThemeClientPrivate::allocatedSharedMemory";
        return allocatedSharedMem;
    }

    TPckg<int> allocatedInfo(allocatedSharedMem);
    TIpcArgs args(0, &allocatedInfo);
    TInt err = SendReceive(EAllocatedSharedMem, args);
    return allocatedSharedMem;
}

/**
 * HbThemeClientPrivate::allocatedHeapMemory()
 */
int HbThemeClientPrivate::allocatedHeapMemory()
{
    int allocatedHeapMem = -1;
    if ( !clientConnected ) {
        qWarning()<<"Theme client unable to connect to server in HbThemeClientPrivate::allocatedHeapMemory";
        return allocatedHeapMem;
    }

    TPckg<int> allocatedInfo(allocatedHeapMem);
    TIpcArgs args(0, &allocatedInfo);
    TInt err = SendReceive(EAllocatedHeapMem, args);
    return allocatedHeapMem;
}

/**
 * HbThemeClientPrivate::switchRenderingMode()
 */
bool HbThemeClientPrivate::switchRenderingMode(HbRenderingMode renderMode)
{
    TInt mode = (TInt)renderMode;

#ifdef HB_SGIMAGE_ICON
    if(renderMode == EHWRendering) {
        TInt sgDriverOpenErrorCode = sgDriver.Open();        
        if(sgDriverOpenErrorCode != KErrNone) {
            return false;
        }        
    } else {
        sgDriver.Close();        
    }
#endif
    TPckg<TInt> paramPckg(mode);
    TIpcArgs args(&paramPckg);    
    TInt err = SendReceive(ERenderModeSwitch, args);
    
    if(err == KErrNone) {    
        return true;
    }
    return false;
}


#ifdef HB_THEME_SERVER_MEMORY_REPORT
void HbThemeClientPrivate::createMemoryReport() const
{
    if ( !clientConnected ) {
        return;
    }

    TIpcArgs args(0, 0);
    TInt err = SendReceive(ECreateMemoryReport, args);
}
#endif

/*
Returns the list of Typaface info.
*/
HbTypefaceInfoVector *HbThemeClientPrivate::typefaceInfo()
{
    if ( !clientConnected ) {
        if(!connectToServer()) {
            qWarning()<<"Theme client unable to connect to server in HbThemeClientPrivate::typefaceInfo";
            return 0;
        }
    }

    HbTypefaceInfoVector *typefaceVector(0);
    HbTypefaceDataInfo typefaceDataInfo;
    TPckg<HbTypefaceDataInfo> theTypefaceInfo(typefaceDataInfo);
    TIpcArgs args(&theTypefaceInfo);
    TInt err = SendReceive(ETypefaceOffset, args);
    if (KErrNone == err) {
        if( typefaceDataInfo.offset >= 0) {
            typefaceVector = HbMemoryUtils::getAddress<HbTypefaceInfoVector>(
                    HbMemoryManager::SharedMemory, typefaceDataInfo.offset);
        }
    }
    return typefaceVector;
}
