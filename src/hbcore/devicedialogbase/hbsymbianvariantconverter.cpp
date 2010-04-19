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
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QRect>

#include "hbsymbianvariantconverter_p.h"
#include "hbsymbianvariant.h"

void HbSymbianVariantConverter::toQtVariant(const CHbSymbianVariant* source, 
                                            QVariant& result )
{
    if (!source) {
        result.clear();
        return;
    }
    switch(source->Type()) {
        case CHbSymbianVariant::EInt:
        {
            result.setValue(*source->Value<TInt>());
        }
        break;
        case CHbSymbianVariant::EBool:
        {
            result.setValue((bool)*source->Value<TInt>());
        }
        break;
        case CHbSymbianVariant::EUint:
        {
            result.setValue(*source->Value<TUint>());
        }
        break;
        case CHbSymbianVariant::EReal:
        {
            result.setValue(*source->Value<TReal>());
        }
        break;
        case CHbSymbianVariant::ERect:
        {
            TRect* srcRect = source->Value<TRect>();
            QRect rect(srcRect->iTl.iX, srcRect->iTl.iY, srcRect->Width(),
                    srcRect->Height());
            result.setValue(rect);
        }
        break;
        case CHbSymbianVariant::EPoint:
        {
            TPoint* srcPoint = source->Value<TPoint>();
            QPoint point(srcPoint->iX, srcPoint->iY);
            result.setValue(point);
        }
        break;
        case CHbSymbianVariant::ESize:
        {
            TSize* srcSize = source->Value<TSize>();
            QSize size(srcSize->iWidth, srcSize->iHeight);
            result.setValue(size);
        }
        break;
        case CHbSymbianVariant::EChar:
        {
            TChar* srcChar = source->Value<TChar>();
            QChar ch(*srcChar);
            result.setValue(ch);
        }
        break;
        case CHbSymbianVariant::EDes:
        {
            TDesC* srcDes = source->Value<TDesC>();
            result.setValue(QString::fromUtf16(srcDes->Ptr(), srcDes->Length()));
        }
        break;
        case CHbSymbianVariant::EBinary:
        {
            TDesC8* srcBin = source->Value<TDesC8>();
            QByteArray byteArray((char*)srcBin->Ptr(), srcBin->Length());
            result.setValue(byteArray);
        }
        break;
        case CHbSymbianVariant::EDesArray:
        {
            CDesC16ArrayFlat* srcArray = source->Value<CDesC16ArrayFlat>();
            QStringList stringList;
            for(TInt i = 0; i < srcArray->MdcaCount(); i++) {
                stringList.append(QString::fromUtf16(srcArray->MdcaPoint(i).Ptr(), srcArray->MdcaPoint(i).Length()));
            }
            result.setValue(stringList);
        }
        break;
        case CHbSymbianVariant::EVariantMap: 
        {
            const CHbSymbianVariantMap* src = source->Value<CHbSymbianVariantMap>();
            QVariantMap map;
            toQtVariantMap(*src, map);
            result.setValue(map);
        }
        break;
        default:
        {
            User::Panic(_L("CHbDeviceDialog: Not supported type"), source->Type());
        }
        break;
    }
}

void HbSymbianVariantConverter::toQtVariantMap(const CHbSymbianVariantMap &aSymbianVariantMap, 
                                               QVariantMap& aQvMap)
{
    MDesCArray& keys = aSymbianVariantMap.Keys();
    for(TInt i = 0; i < keys.MdcaCount() ; i++) {
        TPtrC ptr = keys.MdcaPoint(i);
        QString key = QString::fromUtf16(ptr.Ptr(), ptr.Length());
        QVariant variant;
        toQtVariant(aSymbianVariantMap.Get(ptr), variant);
        aQvMap[key] = variant;
    }
}

CHbSymbianVariant* HbSymbianVariantConverter::fromQtVariantL(QVariant& aVariant)
{
    CHbSymbianVariant* return_value = 0;

    switch(aVariant.type()) {
        case QVariant::Bool:
        {
            TBool boolean = aVariant.value<bool>();
            return_value = CHbSymbianVariant::NewL(&boolean, CHbSymbianVariant::EBool);
        }
        break;
        case QVariant::ByteArray:
        {
            QByteArray array = aVariant.value<QByteArray>();
            TPtrC8 bufPtr((uchar*)array.data(), array.size());
            return_value = CHbSymbianVariant::NewL(&bufPtr, CHbSymbianVariant::EBinary);
        }
        break;
        case QVariant::Char:
        {
            TChar sCh(aVariant.value<QChar>().unicode());
            return_value = CHbSymbianVariant::NewL(&sCh, CHbSymbianVariant::EChar);
        }
            break;
        case QVariant::Double:
        {
            double tempDouble = aVariant.value<double>();
            return_value = CHbSymbianVariant::NewL(&tempDouble, CHbSymbianVariant::EReal);
        }
        break;
        case QVariant::Int:
        {
            int tempInt = aVariant.value<int>();
            return_value = CHbSymbianVariant::NewL(&tempInt, CHbSymbianVariant::EInt);
        }
        break;
        case QVariant::Point:
        {
            QPoint tempPoint = aVariant.value<QPoint>();
            TPoint point(tempPoint.x(), tempPoint.y());
            return_value = CHbSymbianVariant::NewL(&point, CHbSymbianVariant::EPoint);
        }
        break;
        case QVariant::Rect:
        {
            QRect tempRect = aVariant.value<QRect>();
            TRect rect(tempRect.left(), tempRect.top(), tempRect.right(),
                    tempRect.bottom());
            return_value = CHbSymbianVariant::NewL(&rect, CHbSymbianVariant::ERect);
        }
        break;
        case QVariant::Size:
        {
            QSize tempSize = aVariant.value<QSize>();
            TSize size(tempSize.width(), tempSize.height());
            return_value = CHbSymbianVariant::NewL(&size, CHbSymbianVariant::ESize);
        }
        break;
        case QVariant::String:
        {
            QString tempString = aVariant.value<QString>();
            TPtrC descriptor(static_cast<const TUint16*>(tempString.utf16()),
                    tempString.length());
            return_value = CHbSymbianVariant::NewL(&descriptor, CHbSymbianVariant::EDes);
        }
        break;
        case QVariant::StringList:
        {
            QStringList stringList = aVariant.value<QStringList>();
            QStringList::iterator i;
            CPtrCArray* descriptorArray = 0;
            for (i = stringList.begin(); i != stringList.end(); ++i) {
            descriptorArray = new CPtrCArray(stringList.count());
                TPtrC listItem(static_cast<const TUint16*>((*i).utf16()),
                        (*i).length());
                descriptorArray->AppendL(listItem);
            }
            MDesCArray* arrayInterface = static_cast<MDesCArray*>(descriptorArray);
            return_value = CHbSymbianVariant::NewL(arrayInterface, CHbSymbianVariant::EDesArray);
        }
        break;
        case QVariant::UInt:
        {
            uint tempUint = aVariant.value<uint>();
            return_value = CHbSymbianVariant::NewL(&tempUint, CHbSymbianVariant::EUint);
        }
        break;
        case QVariant::Map: 
        {
            QVariantMap map = aVariant.value<QVariantMap>();
            return_value = CHbSymbianVariant::NewL(fromQVariantMapL(map), CHbSymbianVariant::EVariantMap);
        }
        break;
        default:
        {
            User::Panic(_L("CHbDeviceDialog: Not supported type"), aVariant.type());
        }
        break;
    }
    return return_value;
}

CHbSymbianVariantMap *HbSymbianVariantConverter::fromQVariantMapL(QVariantMap& aQvMap)
{
    CHbSymbianVariantMap* map = CHbSymbianVariantMap::NewL();
    QVariantMap::const_iterator i;
    for (i = aQvMap.constBegin(); i != aQvMap.constEnd(); ++i){
      QString keyString = i.key();
      TPtrC descriptor(static_cast<const TUint16*>(keyString.utf16()),
              i.key().length());
      QVariant value = i.value();
      CHbSymbianVariant* symbianVariant = fromQtVariantL(value);
      map->Add(descriptor, symbianVariant);
    }
    return map;
}

