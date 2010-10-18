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
    if (THEME_SERVER_NAME != HbMemoryUtils::getCleanAppName()) {
        themelistener = new CHbThemeListenerPrivate(this);
    }

#ifdef HB_SGIMAGE_ICON
    sgDriverInit = false;
#endif

    queueCheckInvoker = CIdle::New(CActive::EPriorityLow); // important to set the proper prio here
}

/**
* HbThemeClientPrivate::connectToServer()
*
* Connects to the server
*/
bool HbThemeClientPrivate::connectToServer()
{
    TInt error(KErrNone);
    for(int tries = 0;;++tries) {
        error = CreateSession(KThemeServerName, Version(), KDefaultMessageSlots);
        if(!error || tries >= 3) {
            //session created ok or give up trying.
            break;
        }
        if(error != KErrNotFound && error != KErrServerTerminated) {
            // problem other than server not here.
            break;
        }
        error = StartServer();
        if (error != KErrNone && error != KErrAlreadyExists) {
            // unexpected error occurred.
            break;
        }
    }
#ifdef HB_SGIMAGE_ICON
    if (!error && !sgDriverInit) {
        error = sgDriver.Open();
        if (error == KErrNone) {
            sgDriverInit = true;
        }
    }
#endif
    return (clientConnected = (KErrNone == error));
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

class QueueEntry : public CActive
{
public:
    QueueEntry(HbThemeClientPrivate *tc);
    ~QueueEntry();

    void DoCancel();
    void RunL();

    void init(const HbThemeClient::IconReqInfo &reqInfo,
              HbAsyncIconInfoCallback callback,
              void *callbackParam);

    void issueRequest();

    HbThemeClientPrivate *mTc;
    TPckg<HbSharedIconInfo> *mInfoPckg;
    TPckg<TIconParams> *mParamsPckg;
    void *mCallbackParam;
    HbAsyncIconInfoCallback mCallback;
    HbSharedIconInfo mInfo;
    TIconParams mParams;
    TFileName mIconPathBuf;
};

// Never call this directly, use scheduleQueueCheck() to have it
// invoked asynchronously when there is nothing better to do.
static TInt checkQueue(TAny *param)
{
    HbThemeClientPrivate *tc = static_cast<HbThemeClientPrivate *>(param);
    // Only one request can be sent a time. The themeserver is single-threaded
    // and has no special async support so it blocks anyway.
    if (tc->reqQueue.isEmpty()) {
        return 0;
    }
    foreach (QueueEntry *e, tc->reqQueue) {
        if (e->IsActive()) {
            return 0;
        }
    }
    tc->reqQueue.head()->issueRequest();
    return 0;
}

QueueEntry::QueueEntry(HbThemeClientPrivate *tc)
    : CActive(EPriorityStandard),
      mTc(tc), mInfoPckg(0), mParamsPckg(0), mCallbackParam(0)
{
    CActiveScheduler::Add(this);
}

QueueEntry::~QueueEntry()
{
    Cancel();
    delete mParamsPckg;
    delete mInfoPckg;
}

void QueueEntry::DoCancel()
{
    // No cancel support on server-side, so complete here. This also means that
    // Cancel() can only be called from the dtor. See cancelGetSharedIconInfo().
    TRequestStatus *rs = &iStatus;
    User::RequestComplete(rs, KErrCancel);
}

void QueueEntry::RunL()
{
    if (iStatus != KErrCancel) {
        mTc->reqQueue.removeOne(this);
        if (iStatus != KErrNone) {
            mInfo.type = INVALID_FORMAT;
        }
        if (!mCallback || !mCallback(mInfo, mCallbackParam)) {
            // Requestor is not interested, may not even exist anymore, so unload.
            HbThemeClient::IconReqInfo reqInfo;
            reqInfo.iconPath = QString::fromUtf16(mParams.fileName.Ptr(), mParams.fileName.Length());
            reqInfo.size = QSizeF(mParams.width, mParams.height);
            reqInfo.aspectRatioMode = (Qt::AspectRatioMode) mParams.aspectRatioMode;
            reqInfo.mode = (QIcon::Mode) mParams.mode;
            reqInfo.mirrored = (bool) mParams.mirrored;
            reqInfo.color = mParams.colorflag ? QColor(mParams.rgba) : QColor();
            reqInfo.renderMode = (HbRenderingMode) mParams.renderMode;
            mTc->unloadIcon(reqInfo);
        }
        mTc->scheduleQueueCheck();
        delete this;
    }
}

void QueueEntry::init(const HbThemeClient::IconReqInfo &reqInfo,
                      HbAsyncIconInfoCallback callback,
                      void *callbackParam)
{
    mCallback = callback;
    mCallbackParam = callbackParam;
    mInfo.type = INVALID_FORMAT;
    mIconPathBuf = TFileName(reqInfo.iconPath.utf16());
    mParams.fileName = mIconPathBuf;
    mParams.width = reqInfo.size.width();
    mParams.height = reqInfo.size.height();
    mParams.aspectRatioMode = (TUint8) reqInfo.aspectRatioMode;
    mParams.mode = (TUint8) reqInfo.mode;
    mParams.options = (TUint8) reqInfo.options;
    mParams.mirrored = (TBool) reqInfo.mirrored;
    mParams.rgba = (TUint32) reqInfo.color.rgba();
    mParams.colorflag = reqInfo.color.isValid();
    mParams.renderMode = reqInfo.renderMode;
    mInfoPckg = new TPckg<HbSharedIconInfo>(mInfo);
    mParamsPckg = new TPckg<TIconParams>(mParams);
}

void QueueEntry::issueRequest()
{
    TIpcArgs args(mParamsPckg, mInfoPckg);
    mTc->SendReceive(EIconLookup, args, iStatus);
    SetActive();
}

/**
 * HbThemeClientPrivate::getSharedIconInfo()
 *
 * Returns the shared icon information, asynchronous version.
*/
void HbThemeClientPrivate::getSharedIconInfo(const HbThemeClient::IconReqInfo &reqInfo,
                                             HbAsyncIconInfoCallback callback,
                                             void *callbackParam)
{
    if (!clientConnected) {
        HbSharedIconInfo info;
        info.type = INVALID_FORMAT;
        callback(info, callbackParam);
        return;
    }
    QueueEntry *e = new QueueEntry(this);
    e->init(reqInfo, callback, callbackParam);
    reqQueue.enqueue(e);
    scheduleQueueCheck();
}

void HbThemeClientPrivate::scheduleQueueCheck()
{
    if (queueCheckInvoker && !queueCheckInvoker->IsActive()) {
        queueCheckInvoker->Start(TCallBack(checkQueue, this));
    }
}

/**
 * HbThemeClientPrivate::cancelGetSharedIconInfo
 *
 * Cancels a previous async getSharedIconInfo request.
 * If callbackParam is 0 then it is ignored and only \a callback is used in the matching.
 * Otherwise both \a callback and \a callbackParam must match.
*/
void HbThemeClientPrivate::cancelGetSharedIconInfo(HbAsyncIconInfoCallback callback,
                                                   void *callbackParam)
{
    for (int i = 0; i < reqQueue.count(); ++i) {
        QueueEntry *e = reqQueue.at(i);
        if (e->mCallback == callback && (!callbackParam || callbackParam == e->mCallbackParam)) {
            if (e->IsActive()) {
                // There is no real cancelation support, the themeserver is busy
                // and is blocked at this point, so just let it go and ignore
                // the future results. Calling Cancel() would potentially result
                // in a stray signal.
                e->mCallback = 0;
            } else {
                delete e;
                reqQueue.removeAt(i--);
            }
        }
    }
}

inline TIconParams reqInfoToParams(const HbThemeClient::IconReqInfo &reqInfo)
{
    TIconParams params;
    params.fileName.Copy(TPtrC(static_cast<const TUint16 *>(reqInfo.iconPath.utf16()),
                               reqInfo.iconPath.length()));
    params.width = reqInfo.size.width();
    params.height = reqInfo.size.height();
    params.aspectRatioMode = (TUint8)  reqInfo.aspectRatioMode;
    params.mode = (TUint8) reqInfo.mode;
    params.options = (TUint8) reqInfo.options;
    params.mirrored = (TBool) reqInfo.mirrored;
    params.rgba = (TUint32) reqInfo.color.rgba();
    params.colorflag = reqInfo.color.isValid();
    params.renderMode = reqInfo.renderMode;
    return params;
}

/**
 * HbThemeClientPrivate::getSharedIconInfo()
 *
 * Returns the shared icon information, synchronous version.
*/
HbSharedIconInfo HbThemeClientPrivate::getSharedIconInfo(const HbThemeClient::IconReqInfo &reqInfo)
{
    HbSharedIconInfo sharedIconInfo;
    sharedIconInfo.type = INVALID_FORMAT;

    if (!clientConnected) {
        return sharedIconInfo;
    }

    TPckg<HbSharedIconInfo> iconInfo(sharedIconInfo);
    TIconParams params = reqInfoToParams(reqInfo);

    TPckg<TIconParams> paramPckg(params);
    TIpcArgs args(&paramPckg, &iconInfo);

    TInt err = SendReceive(EIconLookup, args);
    if (KErrNone != err) {
        sharedIconInfo.type = INVALID_FORMAT;
    }
    return sharedIconInfo;
}

/**
 * getMultiPartIconInfo
 */
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
    HbSharedIconInfo sharedIconInfo;
    sharedIconInfo.type = INVALID_FORMAT;

    if ( !clientConnected ) {
        return sharedIconInfo;
    }

    TPckg<HbSharedIconInfo> iconInfo(sharedIconInfo);
    TMultiIconSymbParams params;

    TFileName iconId(multiPartIconData.multiPartIconId.utf16());
    params.multiPartIconId.Copy(iconId);
    for (int i = 0; i < multiPartIconList.length(); i++) {
        TFileName pieceIconId(multiPartIconList[i].utf16());
        params.multiPartIconList[i].Copy(pieceIconId);
    }
     int noOfPieces = 1;
     if (multiPartIconData.multiPartIconId.contains("_3PV", Qt::CaseInsensitive)
           || multiPartIconData.multiPartIconId.contains("_3PH", Qt::CaseInsensitive)) {
        noOfPieces = 3;
    } else if (multiPartIconData.multiPartIconId.contains("_9P", Qt::CaseInsensitive)) {
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
HbCss::StyleSheet *HbThemeClientPrivate::getSharedStyleSheet(const QString &fileName, 
            HbLayeredStyleLoader::LayerPriority priority, bool &fileExists)
{
    if ( !clientConnected ) {
        return 0;
    }
    HbCss::StyleSheet *styleSheet(0);

    TFileName fileDes(fileName.utf16());
    TBuf<5> layerPriority;
    layerPriority.AppendNum((TInt)priority);

    HbSharedStyleSheetInfo stylesheetInfo;
    TPckg<HbSharedStyleSheetInfo> sharedInfo(stylesheetInfo);

    TIpcArgs args(&fileDes, &layerPriority, &sharedInfo);

#ifdef THEME_SERVER_TRACES
    QTime time;
    time.start();
#endif
    TInt err = SendReceive(EStyleSheetLookup, args);
#ifdef THEME_SERVER_TRACES
    THEME_GENERIC_DEBUG() << "Time elapsed in IPC:" << time.elapsed() << "ms";
#endif    

    if (KErrNone == err) {
        if (stylesheetInfo.offset >= 0) {
            styleSheet = HbMemoryUtils::getAddress<HbCss::StyleSheet>(
                HbMemoryManager::SharedMemory, stylesheetInfo.offset);
        }
        fileExists = stylesheetInfo.fileExists;
    }
    return styleSheet;
}

/**
 * HbThemeClientPrivate::getSharedMissedHbCss()
 *
 * Returns a pointer to the list in shared memory of CSS files for classes
 * starting with 'hb' which the theme server attempted to load and found
 * the file does not exist
 */
HbVector<uint> *HbThemeClientPrivate::getSharedMissedHbCss()
{
    if (!clientConnected) {
        return 0;
    }

    HbVector<uint> *list(0);

    HbSharedMissedHbCssInfo missedListInfo;
    TPckg<HbSharedMissedHbCssInfo> listInfo(missedListInfo);
    TIpcArgs args(&listInfo);
    TInt err = SendReceive(EMissedHbCssLookup, args);

    if (KErrNone == err) {
        if (missedListInfo.offset >= 0) {
            list = HbMemoryUtils::getAddress<HbVector<uint> >(
                HbMemoryManager::SharedMemory, missedListInfo.offset);
        }
    }
    return list;
}

/**
 * HbThemeClientPrivate::getSharedEffect()
 *
 * Returns the shared effect information
*/
HbEffectFxmlData *HbThemeClientPrivate::getSharedEffect(const QString &filePath)
{
    THEME_GENERIC_DEBUG() << "HbThemeClientPrivate::getSharedEffect" << filePath;
    if ( !clientConnected ) {
        return 0;
    }

    HbEffectFxmlData *fxmlData = 0;

    TFileName fileDes(filePath.utf16());
    HbSharedEffectInfo effectInfo;

    TPckg<HbSharedEffectInfo> sharedInfo(effectInfo);

    TIpcArgs args(&fileDes, &sharedInfo);

#ifdef THEME_SERVER_TRACES
    QTime time;
    time.start();
#endif
    TInt err = SendReceive(EEffectLookupFilePath, args);
#ifdef THEME_SERVER_TRACES    
    THEME_GENERIC_DEBUG() << "Time elapsed in EEffectLookupFilePath IPC:" << time.elapsed() << "ms";
#endif    

    if (KErrNone == err) {
        THEME_GENERIC_DEBUG() << "HbThemeClientPrivate::getSharedEffect effectInfo.offSet is:"
                 <<  effectInfo.offset;
        if (effectInfo.offset >= 0) {
            fxmlData = HbMemoryUtils::getAddress<HbEffectFxmlData>(
                HbMemoryManager::SharedMemory, effectInfo.offset);
        } else {
            THEME_GENERIC_DEBUG() << "effect offset invalid: " << effectInfo.offset;
        }
    } else {
        THEME_GENERIC_DEBUG() << "effect sendreceive error:" << err;
    }
    return fxmlData;
}

/**
 * HbThemeClientPrivate::addSharedEffect()
 *
 * Adds the shared effect information
*/
bool HbThemeClientPrivate::addSharedEffect(const QString &filePath)
{
    THEME_GENERIC_DEBUG() << Q_FUNC_INFO << "with filePath:" << filePath;
    if ( !clientConnected ) {
        return false;
    }

    TFileName fileDes(filePath.utf16());
    TInt retVal = KErrGeneral;

    TPckg<TInt> sharedInfo(retVal);

    TIpcArgs args(&fileDes, &sharedInfo);

#ifdef THEME_SERVER_TRACES
    QTime time;
    time.start();
#endif
    TInt err = SendReceive(EEffectAdd, args);
#ifdef THEME_SERVER_TRACES    
    THEME_GENERIC_DEBUG() << "Time elapsed in EEffectAdd IPC:" << time.elapsed() << "ms";
#endif    

    if (KErrNone == err) {
        TInt result = sharedInfo();
        THEME_GENERIC_DEBUG() << "TInt result (offset):" << result;

        if (result >= 0) {
            THEME_GENERIC_DEBUG() << "add effect results returning TRUE";
            return true;
        }
        THEME_GENERIC_DEBUG() << "effect offset error:" << result;
    } else {
        THEME_GENERIC_DEBUG() << "effect sendreceive error:" << (int) err;
    }
    return false;
}

/**
 * HbThemeClientPrivate::unloadIcon()
*/
void HbThemeClientPrivate::unloadIcon(const HbThemeClient::IconReqInfo &reqInfo)
{
    if (!clientConnected) {
        return;
    }

    TIconParams params = reqInfoToParams(reqInfo);

    TPckg<TIconParams> paramPckg(params);
    TIpcArgs args(&paramPckg);
    SendReceive(EUnloadIcon, args);
}

/**
 * HbThemeClientPrivate::batchUnloadIcon()
*/
void HbThemeClientPrivate::batchUnloadIcon(const QVector<HbThemeClient::IconReqInfo> &reqInfos)
{
    if (!clientConnected) {
        return;
    }
    int idx = 0;
    typedef TIconParams Params[BATCH_SIZE_LIMIT];
    Params paramList;
    for (int i = 0, ie = reqInfos.count(); i != ie; ++i) {
        paramList[idx++] = reqInfoToParams(reqInfos.at(i));
        if (idx == BATCH_SIZE_LIMIT || i == ie - 1) {
            // There may be unused entries in the last batch.
            for (int j = idx; j < BATCH_SIZE_LIMIT; ++j) {
                paramList[j].fileName.Zero();
            }
            idx = 0;
            TPckg<Params> paramsPckg(paramList);
            TIpcArgs args(&paramsPckg);
            SendReceive(EBatchUnloadIcon, args);
        }
    }
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
HbWidgetLoader::LayoutDefinition *HbThemeClientPrivate::getSharedLayoutDefs(
        const QString &fileName, const QString &layout, const QString &section, bool &fileExists)
{
    if ( !clientConnected ) {
        return 0;
    }

    HbWidgetLoader::LayoutDefinition *layoutDef(0);

    TFileName fileDes(fileName.utf16());
    TFileName layoutDes(layout.utf16());
    TFileName sectionDes(section.utf16());

    HbSharedWMLInfo widgetmlInfo;
    TPckg<HbSharedWMLInfo> wmlInfo(widgetmlInfo);

    TIpcArgs args(&fileDes,&layoutDes,&sectionDes,&wmlInfo);

    TInt err = SendReceive(EWidgetMLLookup, args);

    if (KErrNone == err) {
        if( widgetmlInfo.offset >=0){
            layoutDef = HbMemoryUtils::getAddress<HbWidgetLoader::LayoutDefinition>(
                    HbMemoryManager::SharedMemory, widgetmlInfo.offset);
        }
        fileExists = widgetmlInfo.fileExists;
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
            THEME_GENERIC_DEBUG() << Q_FUNC_INFO << "connect to theme server failed";
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
    THEME_GENERIC_DEBUG() << Q_FUNC_INFO;
    hbInstance->theme()->d_ptr->handleThemeChange(themeName);
}

/**
 * Destructor
 */
HbThemeClientPrivate::~HbThemeClientPrivate()
{
    // Make a copy and destroy the elements after emptying the real queue so
    // checkQueue() can safely be called during the destruction of the elements.
    QueueType qc = reqQueue;
    reqQueue.clear();
    foreach (QueueEntry *e, qc) {
        delete e;
    }
    delete queueCheckInvoker; // destroy only when no QueueEntries are alive

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
    const TUid KServerUid2 = {0x100039CE};
    const TUidType serverUid(KNullUid, KServerUid2, KServerUid3);
    RProcess server;
    _LIT(KThemeServerExe,"hbthemeserver.exe");

    result = server.Create(KThemeServerExe, KNullDesC, EOwnerProcess);

    if (KErrNone != result) {
        return  result;
    }
    TRequestStatus status;
    server.Rendezvous(status);
    if (status != KRequestPending) {
        server.Kill(0); // abort startup
    } else {
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
 * HbThemeClientPrivate::setTheme()
 */
void HbThemeClientPrivate::setTheme(const QString &theme)
{
    // In Symbian P&S is used to change active theme
    Q_UNUSED(theme);
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
        THEME_GENERIC_DEBUG() << Q_FUNC_INFO << "connect to theme server failed.";
        return freeSharedMem;
    }

    TPckg<int> freeInfo(freeSharedMem);
    TIpcArgs args(0, &freeInfo);
    TInt err = SendReceive(EFreeSharedMem, args);
    return freeSharedMem;
}

/**
 * HbThemeClientPrivate::allocatedSharedMemory()
 */
int HbThemeClientPrivate::allocatedSharedMemory()
{
    int allocatedSharedMem = -1;
    if ( !clientConnected ) {
        THEME_GENERIC_DEBUG() << Q_FUNC_INFO << "connect to theme server failed.";
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
        THEME_GENERIC_DEBUG() << Q_FUNC_INFO << "connect to theme server failed.";
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
            hbWarning() << "Theme client unable to connect to server in HbThemeClientPrivate::typefaceInfo";
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
