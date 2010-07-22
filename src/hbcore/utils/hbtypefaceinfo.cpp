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

#include "hbtypefaceinfo_p.h"
#include "hbtypefacexmlparser_p.h"
#include <QMap>
#include <QFontMetrics>
#include <QFontInfo>
#include <QRegExp>

#include "hbtypefaceinfodatabase_p.h"
#include "hbthemeclient_p.h"


#undef HBTYPEFACEINFO_DEBUG_ENABLE
//#define HBTYPEFACEINFO_DEBUG_ENABLE

class HbTypefaceInfoPrivate : public QObject
{
    Q_OBJECT

public:
    HbTypefaceInfoPrivate() : initialized(false) {}
    ~HbTypefaceInfoPrivate();
    void initialize();
    bool containsRole(HbFontSpec::Role role, int &index) const;
    bool containsFamily(const QString &typefaceFamily, int &index) const;

public: // data
    bool initialized;

    HbTypefaceInfoVector *mTypefaceVector;
};

HbTypefaceInfoPrivate::~HbTypefaceInfoPrivate()
{
}


void HbTypefaceInfoPrivate::initialize()
{

    mTypefaceVector = HbThemeClient::global()->typefaceInfo();

    if( !mTypefaceVector ) {
        HbTypefaceInfoDatabase *heapDatabase = HbTypefaceInfoDatabase::instance( HbMemoryManager::HeapMemory );
        mTypefaceVector = HbMemoryUtils::getAddress<HbTypefaceInfoVector>(HbMemoryManager::HeapMemory,
                                                heapDatabase->typefaceInfoVectorOffset());
        if( heapDatabase && mTypefaceVector ) {
#ifdef HBTYPEFACEINFO_DEBUG_ENABLE
            qDebug() << "Read " << mTypefaceVector->size() << " typefaces in heap mode";
#endif // HBTYPEFACEINFO_DEBUG_ENABLE
        }
    } else {
#ifdef HBTYPEFACEINFO_DEBUG_ENABLE
        qDebug() << "HbTypefaceInfoPrivate, server connection works! Read " << mTypefaceVector->size() << " typefaces";
#endif // HBTYPEFACEINFO_DEBUG_ENABLE
    }

    if( !mTypefaceVector ) {
        qWarning() << "HbTypefaceInfoPrivate, unable to allocate typeface vector";
        return;
    }
    initialized = true;
}




HbTypefaceInfo::HbTypefaceInfo( QObject *parent )
    : QObject( parent ),
    d( new HbTypefaceInfoPrivate )
{
	d->initialize();
}


HbTypefaceInfo::~HbTypefaceInfo()
{
    delete d;
}

bool HbTypefaceInfoPrivate::containsRole(HbFontSpec::Role role, int &index) const
{
    if( ! initialized ) {
        qWarning( "HbTypefaceInfoPrivate in not initialized, line %d", __LINE__ );
        return false;
    }
    for( int i = 0; i < mTypefaceVector->size(); i++ ) {
        if (mTypefaceVector->at(i).mRoleEnum == role && 
			mTypefaceVector->at(i).mIsAlias == false) {
            index = i;
            return true;
        }
    }
    return false;
}


bool HbTypefaceInfoPrivate::containsFamily(const QString &typefaceFamily, int &index) const
{
    if( ! initialized ) {
        qWarning( "HbTypefaceInfoPrivate in not initialized, line %d", __LINE__ );
        return false;
    }
    for( int i = 0; i < mTypefaceVector->size(); i++ ) {
        if (!typefaceFamily.compare(mTypefaceVector->at(i).mFamily, Qt::CaseInsensitive)) {
            index = i;
            return true;
        }
    }
    return false;
}
bool HbTypefaceInfo::containsFamily(const QString &typefaceFamily) const
{
	int dummy;
	return d->containsFamily(typefaceFamily, dummy);
}
bool HbTypefaceInfo::tryGetFamilyFromAliasName( 
	const QString &aliasFamily, 
	QString &typefaceFamily,
	int &weight) const
{
    for (int i = 0; i < d->mTypefaceVector->size(); i++) {
        if (d->mTypefaceVector->at(i).mIsAlias == true &&
            !aliasFamily.compare(d->mTypefaceVector->at(i).mAliasedFamily, Qt::CaseInsensitive)) {
            typefaceFamily = d->mTypefaceVector->at(i).mFamily ;
			weight = d->mTypefaceVector->at(i).mIsBold ? QFont::Bold : QFont::Normal;
			return true;
        }
    }
	return false;
}
int HbTypefaceInfo::getWeight(const QString &typefaceFamily) const
{
	int index, weight;
	if (d->containsFamily( typefaceFamily, index)) {
		weight = d->mTypefaceVector->at(index).mIsBold ? QFont::Bold : QFont::Normal;
	}
	else {
		weight = QFont::Normal;
	}
	return weight;
}
void HbTypefaceInfo::roleToTypeface(HbFontSpec::Role role, QString& typefaceFamily, int& weight) const
{
    int index;

	bool validRole = d->containsRole( role, index );
	if (!validRole) {
		role = HbFontSpec::Undefined;
		validRole = d->containsRole( role, index );
	if( !validRole ) {
		    qWarning( "HbTypefaceInfo: cannot find corresponding font role %d, line %d", role, __LINE__ );
	    return;
		}
	}

	typefaceFamily = d->mTypefaceVector->at( index ).mFamily;
	weight = d->mTypefaceVector->at( index ).mIsBold ? QFont::Bold : QFont::Normal;

	return;
}

int HbTypefaceInfo::textHeightToSizeInPixels(
	const QString& typefaceFamily,
	int weight,
	qreal textHeight) const
{
	Q_UNUSED(weight);
	int index;
	int heightIndex;

	if (! d->containsFamily( typefaceFamily, index )) {
		// need to generate on server size, not supported yet
	    qWarning( "HbTypefaceInfo:textHeightToSizeInPixels no font family %s, line %d", typefaceFamily.toLatin1().data(),  __LINE__ );
	    return 1;
	}

	HbTypefaceInfoItem *item = &( d->mTypefaceVector->at( index ) );

	int size(-1);

	int flooredPaneHeight = qRound(textHeight-0.5);
	if ( item->tableContainsHeight( flooredPaneHeight, heightIndex ) ) {
		size = item->mDownSizeTable.at( heightIndex ).pixelSize;
	} else if ( textHeight < item->mLowestExtent ) { // Assumed that downsize info goes quite low, below really usable fonts
		size = qRound( textHeight );
	} else if ( textHeight < item->mHighestExtent ) { // interpolate. May fail and leave size unset.
	    int indexLowerB = -1;
	    int indexUpperB = -1;
	    int lowerBValue = 0xffffff;
	    int upperBValue = 0xffffff;
	    for( int i = 0; i < item->mDownSizeTable.size(); i++ ) {
	        int current = item->mDownSizeTable.at( i ).textHeight;
	        if( ( current < lowerBValue ) && ( current >= flooredPaneHeight ) ) {
	            lowerBValue = current;
	            indexLowerB = i;
	        }
            if( ( current < upperBValue ) && ( current > flooredPaneHeight ) ) {
                upperBValue = current;
                indexUpperB = i;
            }
	    }

	    if( ( indexUpperB != -1 ) && ( indexLowerB != -1 ) ) {
            int k1 = lowerBValue;
            int v1 = item->mDownSizeTable.at( indexLowerB ).pixelSize;
            int k2 = upperBValue;
            int v2 = item->mDownSizeTable.at( indexUpperB ).pixelSize;
            int diff = k2 - k1;
            if (diff > 0) {
                size = qRound(v1 + (flooredPaneHeight - k1)*(v2 - v1)/diff);
            }
	    }
	}
	// Come here for text heights that are too large for the table, or for failure of interpolation
	if (size == -1) {
	    int highestExtentIndex;
	    if( ! item->tableContainsHeight( item->mHighestExtent, highestExtentIndex ) ) {
	        qWarning( "HbTypefaceInfo:textHeightToSizeInPixels something wrong with downsize table, line %d", __LINE__ );
	        return 1;
	    }

		size = qRound( textHeight
			* item->mDownSizeTable.at( highestExtentIndex ).pixelSize
			/ item->mHighestExtent);
	}

    // If the caller intends non-zero size, and zero would be returned by downsizing, then return 1.
    // This avoids QFont's special treatment of zero size.
    if(textHeight > 0 && size <= 0) {
        size = 1;
    }
	return size;
}

#include "hbtypefaceinfo.moc"
