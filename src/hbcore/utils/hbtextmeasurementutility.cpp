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

#include "hbtextmeasurementutility_r.h"
#include "hbtextmeasurementutility_r_p.h"
#include "hbwidgetbase.h"
#include "hbfontspec.h"
#include "hbinstance.h"
#include "hbfeaturemanager_r.h"

#include <QGraphicsWidget>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QDate>
#include <QTimer>
#include <QFont>

#include <QDebug> // for qWarning

#include <qmath.h>

#ifndef Q_OS_SYMBIAN
#include <iostream>
#endif

const QChar KCSVSeparator(',');
const QString KUnknown("UNKNOWN");

class HbTextRecord
{
public:
    HbTextRecord();
    QString fontLogicalName() const;
    HbFontSpec::Role fontRole(const QString logicalName) const;
    bool operator<(const HbTextRecord &other)const;
    static bool recordLessThan(HbTextRecord *rc1, HbTextRecord *rc2);
    static bool recordFullCompare(HbTextRecord *rc1, HbTextRecord *rc2);
public:
    QString mRecordName;  // Record name
    int mWidth;           // Text item width (in pixels)
    HbFontSpec mFontSpec; // Font spec
    int mRowCount;
};


HbTextRecord::HbTextRecord()
{
}

QString HbTextRecord::fontLogicalName() const
{
    QString logicalName(KUnknown);
    switch(mFontSpec.role()){
        case HbFontSpec::Primary:
            logicalName = "qfn_primary";
            break;
        case HbFontSpec::Secondary:
            logicalName = "qfn_secondary";
            break;
        case HbFontSpec::Title:
            logicalName = "qfn_title";
            break;
        case HbFontSpec::PrimarySmall:
            logicalName = "qfn_primary_small";
            break;
        case HbFontSpec::Digital:
            logicalName = "qfn_digital";
            break;
        default:
            break;
    }
    return logicalName;
}

HbFontSpec::Role HbTextRecord::fontRole(const QString logicalName) const
{
    HbFontSpec::Role fontRole(HbFontSpec::Undefined);
    if ( logicalName == "qfn_primary" ) {
        fontRole = HbFontSpec::Primary;
    } else if ( logicalName == "qfn_secondary" ) {
        fontRole = HbFontSpec::Secondary;
    } else if ( logicalName == "qfn_title" ) {
        fontRole = HbFontSpec::Title;
    } else if ( logicalName == "qfn_primary_small" ) {
        fontRole = HbFontSpec::PrimarySmall;
    } else if ( logicalName == "qfn_digital" ) {
        fontRole = HbFontSpec::Digital;
    }
    return fontRole;
}

bool HbTextRecord::operator<(const HbTextRecord& other)const
{
    return this->mRecordName < other.mRecordName;
}

bool HbTextRecord::recordLessThan(HbTextRecord* rc1, HbTextRecord* rc2)
{
    return rc1->mRecordName < rc2->mRecordName;
}

bool HbTextRecord::recordFullCompare(HbTextRecord* rc1, HbTextRecord* rc2)
{
    if (!rc1->mRecordName.compare(rc2->mRecordName)) {
        if (rc1->mFontSpec == rc2->mFontSpec) {
            if (rc1->mRowCount == rc2->mRowCount) {
                if (rc1->mWidth == rc2->mWidth) {
                    return true;
                } else {
                    qDebug() << "HbTextMeasurementUtility::recordFullCompare: Sizes don't match";
                }
            } else {
                qDebug() << "HbTextMeasurementUtility::recordFullCompare: Row counts don't match";
            }
        } else {
            qDebug() << "HbTextMeasurementUtility::recordFullCompare: Fonts don't match";
        }
    } else {
        qDebug() << "HbTextMeasurementUtility::recordFullCompare: Names don't match";
    }
    qDebug() << "HbTextMeasurementUtility::recordFullCompare: -- record1:"
        << rc1->mRecordName << rc1->mFontSpec.role() << rc1->mFontSpec.textHeight() << rc1->mRowCount << rc1->mWidth;
    qDebug() << "HbTextMeasurementUtility::recordFullCompare: -- record2:"
        << rc2->mRecordName << rc2->mFontSpec.role() << rc2->mFontSpec.textHeight() << rc2->mRowCount << rc2->mWidth;
    return false;
}



void HbTextMeasurementUtilityPrivate::readEntries(QTextStream &csvReader)
{
    // Read the file header.
    QString line = csvReader.readLine();
    
    while ( !csvReader.atEnd() ) {

        line = csvReader.readLine();
        QStringList list = line.split(KCSVSeparator);
        if ( list.count() != 5 ) {
            qDebug() << "HbTextMeasurementUtilityPrivate::readEntries: Invalid csv file row read";
            continue;
        }
        HbTextRecord *record = new HbTextRecord();        
        record->mRecordName = list.at(0);
        record->mFontSpec.setRole(record->fontRole(list.at(1)));
        record->mFontSpec.setTextHeight(list.at(2).toInt());
        record->mWidth = list.at(3).toInt();
        record->mRowCount = list.at(4).toInt();
        records.append(record);
    }
}


/*!
    Write a report headers to csv file
    \internal
*/
void HbTextMeasurementUtilityPrivate::writeHeaders(QTextStream &csvWriter)
{
    csvWriter << "Layout";
    csvWriter << KCSVSeparator;
    csvWriter << "Font";
    csvWriter << KCSVSeparator;
    csvWriter << "Row height";
    csvWriter << KCSVSeparator;
    csvWriter << "Row width";
    csvWriter << KCSVSeparator;
    csvWriter << "Max rows";
    csvWriter << "\n";
}

/*!
    Write a text item record to csv file
    \internal
*/
void HbTextMeasurementUtilityPrivate::writeEntry(
    QTextStream &csvWriter,
    const HbTextRecord *record)
{
    // "Layout"
    csvWriter << record->mRecordName;
    csvWriter << KCSVSeparator;
    // "Font"
    csvWriter << record->fontLogicalName();
    csvWriter << KCSVSeparator;
    // "Row height"
    csvWriter << qRound(record->mFontSpec.textHeight()-0.5); // Floor.
    csvWriter << KCSVSeparator;
    // "Row width"
    csvWriter << record->mWidth;
    csvWriter << KCSVSeparator;
    // "Max rows"
    csvWriter << record->mRowCount;
    csvWriter << '\n';
}

/*!
    Validate records by removing duplicate items.
    \internal
*/
bool HbTextMeasurementUtilityPrivate::validateRecords(HbDeviceProfile &profile)
{
    if (records.isEmpty()) {
        qDebug() << "HbTextMeasurementUtility::validateRecords: No result entries";
        return false;
    }
    QList<HbTextRecord*> temp;
    qSort(records.begin(), records.end(), &HbTextRecord::recordLessThan);
    bool ret = true;

    foreach (HbTextRecord *record, records) {

        bool validRecord = true;

        for(int i=0; i<temp.count();i++) {
            if (!temp[i]->mRecordName.compare(record->mRecordName)) {
                // duplicate with same data.
                if (HbTextRecord::recordFullCompare(temp[i], record)) {
                    qDebug() << "HbTextMeasurementUtility::validateRecords: Duplicate removed";
                    validRecord = false;
                    break;
                } else { // duplicates id with unequal data.
                    qDebug() << "HbTextMeasurementUtility::validateRecords: Duplicate text id found";
                    ret = false;
                    continue;  // search still duplicates with same data.
                }
            }
        }
        if (validRecord) {
            temp.append(record);
        } else {
            delete record;
        }
    }

    records = temp;
    foreach (const HbTextRecord *record, records) {
        if ( !record->fontLogicalName().compare(KUnknown) ) {
            qDebug() << "HbTextMeasurementUtility::validateRecords: Result item" << record->mRecordName << "Fontspec is null";
            ret = false;
        }

        if ( record->mWidth > profile.logicalSize().width() ) {
            qDebug() << "HbTextMeasurementUtility::validateRecords: Result item" << record->mRecordName << "width is too wide";
            qDebug() << "HbTextMeasurementUtility::validateRecords: Profile width: " << profile.logicalSize().width();
            qDebug() << "HbTextMeasurementUtility::validateRecords: Record width:" << record->mWidth;
            ret = false;
        }
    }
    return ret;
}

/*!
    \internal
*/
void HbTextMeasurementUtilityPrivate::doMeasureItems()
{
#ifndef HB_TEXT_MEASUREMENT_UTILITY
    return;
#else
    // Process all pending events.
    for (int i = 0; i < 3; ++i) {
        QCoreApplication::sendPostedEvents();
        QCoreApplication::processEvents();
    }

    QList<HbMainWindow*> mainWindows;
    if (mWindow) {
        mainWindows.append(mWindow);
    } else {
        mainWindows = hbInstance->allMainWindows();
    }
    foreach (HbMainWindow* mainWindow, mainWindows ) {
        QGraphicsScene* scene = mainWindow->scene(); //krazy:exclude=qclasses
        QList<QGraphicsItem*> sceneItems = scene->items();
        foreach (QGraphicsItem* sceneItem, sceneItems ) {
            if ( sceneItem->isWidget() ) {
                HbWidgetBase* widget = qobject_cast<HbWidgetBase*>(static_cast<QGraphicsWidget*>(sceneItem));
                QVariant textId = widget
                    ? widget->property( HbTextMeasurementUtilityNameSpace::textIdPropertyName )
                    : QVariant();
                if( widget && widget->isVisible() && ( textId != QVariant::Invalid ) 
                    && ( !textId.toString().isEmpty() ) ) {
                    HbTextRecord *record = new HbTextRecord();
                    record->mRecordName = textId.toString();
                    record->mWidth = qRound(widget->size().width() - 0.5); // Floor
                    record->mFontSpec = widget->effectiveFontSpec();
                    if ( record->mFontSpec != widget->fontSpec() ) {
                        qDebug() << "HbTextMeasurementUtility::measureItems: fontSpec and effectiveFontSpec do not match for item"
                                 << record->mRecordName;
                        qDebug() << "- fontSpec, role:" << widget->fontSpec().role() << "textHeight:" << widget->fontSpec().textHeight();
                        qDebug() << "- effectiveFontSpec, role:" << record->mFontSpec.role() << "textHeight:" << record->mFontSpec.textHeight();
                    }
                    record->mFontSpec.setTextHeight(qRound(record->mFontSpec.textHeight() - 0.5)); // Floor
                    QVariant rowCount = widget->property( HbTextMeasurementUtilityNameSpace::textMaxLines );
                    record->mRowCount = rowCount.toInt();
                    if (record->mRowCount <= 0) {
                        record->mRowCount = -1;
                    }
                    records.append(record);
                }
            }
        }
    }
#endif
}

QString HbTextMeasurementUtilityPrivate::reportFilePath(HbDeviceProfile &profile, const QString &domainName) const
{
#ifdef Q_OS_SYMBIAN
    const QString KDriveF("F:\\");
    const QString KDriveC("C:\\");
    const QString KDirectory("data\\log\\qtestcase\\loc\\"); 

    QString filePath;
    if (QFile::exists(KDriveF)) {
        filePath = KDriveF + KDirectory;
    } else {
        filePath = KDriveC + KDirectory;
    }
#else
    QString filePath(QDir::tempPath());
    filePath.append(QDir::separator());
    filePath.append("loc");
    filePath.append(QDir::separator());
    filePath.append(profile.name());
    filePath.append(QDir::separator());
#endif
    filePath = QDir::toNativeSeparators(filePath);

    QDir dir(filePath);
    if (!dir.exists()) {
        dir.mkpath(filePath);
    }

    // Make sure there are no illegal characters in "domainName"
    QString tempName = domainName;
    tempName.remove(QRegExp("[^a-zA-Z0-9]"));
    if (tempName.isEmpty()) {
        tempName = "unknown";
    }

    filePath.append(tempName);
    filePath.append('_');
    filePath.append(profile.name());
    filePath.append('_');
    filePath.append(QString::number(QDate::currentDate().year()));
    filePath.append("wk");
    filePath.append(QString::number(QDate::currentDate().weekNumber()));
    filePath.append(".csv");
    return filePath;
}


/*!
    @alpha
    @hbcore
    \class HbTextMeasurementUtility
    \brief HbTextMeasurementUtility is used for measuring available space for localized texts.
    
    This class collects metrics from localized text items and writes a report to 
    a CSV file. The report is intended to be used when lenghts of text strings are decided
    and texts are localized to different language variants.

    The report contains the following data for each measured text item:
    - Layout: The logical name for the measuser text item (typically the logical text id).
    - Font: The logical fong (e.g. qfn_primary, qfn_secondary, etc.).
    - Row height: Height of one text row in pixels. This measure corresponds to HbFontSpec::textHeight().
    - Row width: Width of one text row in pixels.
    - Max rows: Maximum row count for the measured text item.

    The text strings (set to text items) must have been allocated with hbTrId function.
    It stores the logical text id's to text items and this uses that information when
    collecting the metrics.
    
    Also the localization test mode (see: locTestMode) mush have been enabled
    before any hbTrId calls. In practice it's best to enable it before launching the
    application, or before creating the HbMainWindow, at latest.    
*/


/*!
    \enum HbTextMeasurementUtility::LocTestMode
    The LocTestMode enum identifies the possible localization test modes.
*/
/*!
    \var HbTextMeasurementUtility::Disabled
    Localization tests disabled.
*/
/*!
    \var HbTextMeasurementUtility::Manual
    Manual localization test mode. The client needs to call readReport, measureItems
    and writeReport functions explicitly.
*/
/*!
    \var HbTextMeasurementUtility::Automatic
    Automatic localization test mode. The UI framework calls readReport, measureItems 
    and writeReport automatically when needed.
    
    Notice that for performance reasons there's (at least) half a second timeout
    between measureItems calls. You should wait a while before moving between views
    in order to guarantee that the metrics will be harvested properly.
*/


/*!
    Default constructor.
*/
HbTextMeasurementUtility::HbTextMeasurementUtility()
{
    d = new HbTextMeasurementUtilityPrivate;    
#ifndef HB_TEXT_MEASUREMENT_UTILITY
    d->mLocTestMode_cached = HbTextMeasurementUtility::Disabled;
#else
    d->mLocTestMode_cached = HbFeatureManager::instance()->featureStatus( HbFeatureManager::TextMeasurement );
#endif
}

/*!
    Destructor.
*/
HbTextMeasurementUtility::~HbTextMeasurementUtility()
{
    qDeleteAll(d->records);
    delete d;
}

/*!
    Returns singleton instance.
*/
HbTextMeasurementUtility *HbTextMeasurementUtility::instance()
{
    static HbTextMeasurementUtility theUtility;
    return &theUtility;
}

/*!
    Sets the localization test mode to \a mode.
*/
void HbTextMeasurementUtility::setLocTestMode( int mode )
{
#ifndef HB_TEXT_MEASUREMENT_UTILITY
    Q_UNUSED( mode );
    return;
#else
    HbFeatureManager::instance()->setFeatureStatus( HbFeatureManager::TextMeasurement, mode );
    d->mLocTestMode_cached = mode;
#endif
}

/*!
    Returns the current localization test mode.
*/
int HbTextMeasurementUtility::locTestMode() const
{
    return d->mLocTestMode_cached;
}

/*!
    Measures all currently visible text items.
    This method is asynchronous if time interval \a after (in milliseconds) is larger than zero,
    and synchronous otherwise. This method will process any pending events in both cases, so 
    the synchronous call should be sufficient in most of the cases.
    
    It's possible to give \a window parameter if the measurement is only made for text items
    of specific HbMainWindow. The measurement is made for all text items in all HbMainWindows
    if the parameter is omitted.
*/
void HbTextMeasurementUtility::measureItems(int after, HbMainWindow *window)
{
#ifndef HB_TEXT_MEASUREMENT_UTILITY
    Q_UNUSED( after );
    Q_UNUSED( window );
    return;
#else
    // Store the window pointer, because it would not survive the singleShot timer call.
    d->mWindow = window;

    if (after > 0) {
        // Asynchronous
        QTimer::singleShot(after, d, SLOT(doMeasureItems()));
    } else {
        // Synchronous
        d->doMeasureItems();
    }
#endif
}

/*!
    Reads existing layout metric report file, see writeReport for the file locations.
    
    By using this method it is possible to update existing data instead of overwriting
    data that is measurered earlier.

    This method should be called before first call of measureItems() and after reset().
    If there are some metrics data in memory, the file is not read for optimization reasons.
*/
bool HbTextMeasurementUtility::readReport( HbDeviceProfile &profile, const QString &domainName )
{
#ifndef HB_TEXT_MEASUREMENT_UTILITY
    Q_UNUSED( profile );
    Q_UNUSED( domainName );
    return false;
#else
    if (!d->records.isEmpty()) {
        return false;
    }
    QString filePath = d->reportFilePath(profile, domainName);
    QFile file;
    file.setFileName(filePath);
    if (!file.exists()) {
        return false;
    }
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
#ifndef Q_OS_SYMBIAN
        std::cerr << "Error: Cannot read file ";
        std::cerr << qPrintable(filePath);
        std::cerr << qPrintable(file.errorString()) << std::endl;
#endif
        return false;
    }
    QTextStream csvReader(&file);
    d->readEntries(csvReader);
    file.close();

    return true;
#endif
}


/*!
    Writes a layout metric report into a file. 
    
    The report contains metrics data for each text item at the time of last
    call to the measureItems() method.
        
    Report is written to a csv (Comma Separated Values) file.
    
    On Symbian environment the report is written to memory directory
    /data/log/qtestcase/loc/. Memory card is used if it exists (in F: drive).
    Otherwise C: drive is used.
    
    On other environments the report is written to system temp path + "loc".
    
    A subdirectory will be created for each orientation and/or resolution, 
    e.g. "NHD-3.2-inch_portrait".
    
    The CSV file name will be of format <domainName>_<device profile name>_<week number>.csv,
    e.g. "myapp_NHD-3.2-inch_portrait_2010wk38.csv".
*/
bool HbTextMeasurementUtility::writeReport(HbDeviceProfile &profile, const QString &domainName)
{
#ifndef HB_TEXT_MEASUREMENT_UTILITY
    Q_UNUSED( profile );
    Q_UNUSED( domainName );
    return false;
#else
    qDebug() << "HbTextMeasurementUtility::writeReport: Using profile" << profile.name();

    QString filePath = d->reportFilePath(profile, domainName);
    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
#ifndef Q_OS_SYMBIAN
        std::cerr << "Error: Cannot write file ";
        std::cerr << qPrintable(filePath);
        std::cerr << qPrintable(file.errorString()) << std::endl;
#endif
        return false;
    }
    bool ret = writeReport(profile, &file);

    file.close();
#ifndef Q_OS_SYMBIAN
    if (file.error()) {
        std::cerr << "Error: Cannot write file ";
        std::cerr << qPrintable(filePath);
        std::cerr << qPrintable(file.errorString()) << std::endl;
    }
#endif
   return ret;
#endif
}


/*!
    Overloaded function provided for convenience. Here you can manually specify QIODevice where to write report.
*/
bool HbTextMeasurementUtility::writeReport(HbDeviceProfile &profile, QIODevice *device)
{
#ifndef HB_TEXT_MEASUREMENT_UTILITY
    Q_UNUSED( device );
    return false;
#else

    if( device == 0 ) {
        return false;
    }

    bool succeed = d->validateRecords(profile);
    if (succeed) {
        qDebug() << "HbTextMeasurementUtility::writeReport: Measurements OK";
    } else {
        qDebug() << "HbTextMeasurementUtility::writeReport: Measurements NOT OK";
    }

    QTextStream csvWriter(device);
    d->writeHeaders(csvWriter);
    foreach (const HbTextRecord *record, d->records) {
        d->writeEntry(csvWriter, record);
    }
    return succeed;
#endif
}

/*!
    Reset layout metrics data from memory.
*/
void HbTextMeasurementUtility::reset()
{
    d->records.clear();
}




