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

#include "hbtextmeasurementutility_p.h"
#include "hbwidgetbase.h"
#include "hbfontspec.h"
#include "hbinstance.h"
#include "hbfeaturemanager_p.h"

#include <QGraphicsWidget>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QDate>
#include <QTimer>
#include <QFont>
#include <hbstyle.h>

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
    bool operator<(const HbTextRecord& other)const;
    static bool recordLessThan(HbTextRecord* rc1, HbTextRecord* rc2);
    static bool recordFullCompare(HbTextRecord* rc1, HbTextRecord* rc2);
public:
    QString mRecordName;  // Record name
    QSizeF mSize;         // Text item size (in pixels)
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
                if (rc1->mSize == rc2->mSize) {
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
        << rc1->mRecordName << rc1->mFontSpec.role() << rc1->mFontSpec.textHeight() << rc1->mRowCount << rc1->mSize;
    qDebug() << "HbTextMeasurementUtility::recordFullCompare: -- record2:"
        << rc2->mRecordName << rc2->mFontSpec.role() << rc2->mFontSpec.textHeight() << rc2->mRowCount << rc2->mSize;
    return false;
}


class HbTextMeasurementUtilityPrivate
{
public:
    void writeHeaders(QTextStream &csvWriter);
    void writeEntry(QTextStream &csvWriter, const HbTextRecord *record);
    bool validateRecords(HbDeviceProfile &profile);

public:
    QList<HbTextRecord*> records;
};


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
    csvWriter << qRound(record->mSize.width()-0.5); // Floor.
    csvWriter << KCSVSeparator;
    // "Max rows"
    csvWriter << record->mRowCount;
    csvWriter << '\n';
}

/*!
    Validate records. Try to remove duplicate items.
    \internal
*/
bool HbTextMeasurementUtilityPrivate::validateRecords(HbDeviceProfile &profile)
{
    if (records.isEmpty()){
        qDebug() << "HbTextMeasurementUtility::validateRecords: No result entries";
        return false;
    }
    QList<HbTextRecord*> temp;
    qSort(records.begin(), records.end(), &HbTextRecord::recordLessThan);
    bool ret = true;

    foreach (HbTextRecord *record, records) {

        bool notFound = true;

        for(int i=0; i<temp.count();i++) {
            if (!temp[i]->mRecordName.compare(record->mRecordName)) {
                // duplicate with same data
                if (HbTextRecord::recordFullCompare(temp[i], record)) {
                    qDebug() << "HbTextMeasurementUtility::validateRecords: Duplicate removed";
                    notFound = false;

                // duplicate with same id and correct data
                } else if ( !temp[i]->fontLogicalName().compare(KUnknown) &&
                record->fontLogicalName().compare(KUnknown)) {
                    qDebug() << "HbTextMeasurementUtility::validateRecords: Duplicate overwritten";
                    notFound = false;
                    temp[i] = record;
                // duplicates
                } else {
                    qDebug() << "HbTextMeasurementUtility::validateRecords: Duplicate items found";
                    ret = false;
                }
                break;
            }
        }
        if (notFound) {
            temp.append(record);
        }
    }

    records = temp;
    foreach (const HbTextRecord *record, records) {
        if ( !record->fontLogicalName().compare(KUnknown) ) {
            qDebug() << "HbTextMeasurementUtility::validateRecords: Result item" << record->mRecordName << "Fontspec is null";
            ret = false;
        }

        if ( record->mSize.width() > profile.logicalSize().width() ) {
            qDebug() << "HbTextMeasurementUtility::validateRecords: Result item" << record->mRecordName << "width is too wide";
            qDebug() << "HbTextMeasurementUtility::validateRecords: Profile width: " << profile.logicalSize().width();
            qDebug() << "HbTextMeasurementUtility::validateRecords: Record width:" << record->mSize.width();
            ret = false;
        }
    }
    return ret;
}


/*!
    \class HbTextMeasurementUtility
    \internal
    \proto
*/

HbTextMeasurementUtility::HbTextMeasurementUtility()
{
    d = new HbTextMeasurementUtilityPrivate;
}

HbTextMeasurementUtility::~HbTextMeasurementUtility()
{
    qDeleteAll(d->records);
    delete d;
}

HbTextMeasurementUtility *HbTextMeasurementUtility::instance()
{
    static HbTextMeasurementUtility theUtility;
    return &theUtility;
}

/*!
    Reset the report
*/
void HbTextMeasurementUtility::reset()
{
    d->records.clear();
}


/*!
    Measures all currently visible text items.
    Method is asynchronous if \a after is larger than zero.
    Method is synchronous otherwise.
*/
void HbTextMeasurementUtility::measureItems(int after)
{
    if (after > 0) {
        // Asynchronous
        QTimer::singleShot(after, this, SLOT(doMeasureItems()));
    } else {
        // Synchronous
        QCoreApplication::processEvents();
        QCoreApplication::processEvents();
        QCoreApplication::processEvents();
        doMeasureItems();
    }
}

/*!
    \internal
*/
void HbTextMeasurementUtility::doMeasureItems()
{
#ifndef HB_TEXT_MEASUREMENT_UTILITY
    return;
#else
    QList<HbMainWindow*> mainWindows = hbInstance->allMainWindows();
    foreach (HbMainWindow* mainWindow, mainWindows ) {
        QGraphicsScene* scene = mainWindow->scene();
        QList<QGraphicsItem*> sceneItems = scene->items();
        foreach (QGraphicsItem* sceneItem, sceneItems ) {
            if ( sceneItem->isWidget() ) {
                HbWidgetBase* widget = qobject_cast<HbWidgetBase*>(static_cast<QGraphicsWidget*>(sceneItem));
                QVariant textId = widget
                    ? widget->property( HbTextMeasurementUtilityNameSpace::textIdPropertyName )
                    : QVariant();
                if( ( textId != QVariant::Invalid ) && ( !textId.toString().isEmpty() ) ) {
                    HbTextRecord *record = new HbTextRecord();
                    record->mRecordName = textId.toString();
                    record->mSize = widget->size();
                    record->mFontSpec = widget->effectiveFontSpec();
                    if ( record->mFontSpec != widget->fontSpec() ) {
                        qDebug() << "HbTextMeasurementUtility::measureItems: fontSpec and effectiveFontSpec do not match for item"
                                 << record->mRecordName;
                    }
                    QVariant rowCount = widget->property( HbTextMeasurementUtilityNameSpace::textMaxLines );
                    record->mRowCount = rowCount.toInt();
                    if (record->mRowCount <= 0) {
                        record->mRowCount = -1;
                    }
                    d->records.append(record);
                }
            }
        }
    }
#endif
}

/*!
    Writes a layout metric report into a file, by default C: drive is used.
    Report contains metrics data for each text item at the time of last call to the
    measureItems() method.

    Report is written to a csv (Comma Separated Values) file.

    Report is intended to be used when text strings lengths are decided and localized to
    different language variants.

    Report contains data for each measured text item such as:

    - Logical name (see the documentation of recordName method)
    - Font type (qfn_primary, qfn_secondary...)
    - Font size (The pixel size of the font)
    - Text width (Text width in pixels)
*/
bool HbTextMeasurementUtility::writeReport(HbDeviceProfile &profile, const QString &domainName)
{
#ifndef HB_TEXT_MEASUREMENT_UTILITY
    Q_UNUSED( profile );
    Q_UNUSED( domainName );
    return false;
#else

    qDebug() << "HbTextMeasurementUtility::writeReport: Using profile" << profile.name();

#ifdef Q_OS_SYMBIAN
    QString filePath("C:\\data\\log\\qtestcase\\loc\\");
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

void HbTextMeasurementUtility::setLocTestMode( bool enabled )
{
#ifndef HB_TEXT_MEASUREMENT_UTILITY
    Q_UNUSED( enabled );
    return;
#else
    HbFeatureManager::instance()->setFeatureStatus( HbFeatureManager::TextMeasurement, ( int )enabled );
#endif
}

bool HbTextMeasurementUtility::locTestMode() const
{
#ifndef HB_TEXT_MEASUREMENT_UTILITY
    return false;
#else
    return HbFeatureManager::instance()->featureStatus( HbFeatureManager::TextMeasurement );
#endif

}

