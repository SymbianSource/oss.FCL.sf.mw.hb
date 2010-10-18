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

#ifndef HBSTYLE_H
#define HBSTYLE_H

#include <hbglobal.h>
#include <QWindowsStyle>
#include <QGraphicsItem>
#include <QStyleOptionGraphicsItem>
#include <hbicon.h>
#include <hbdeviceprofile.h>

class HbStylePrivate;
class HbStyleParameters;
class HbStyleOption;
class HbWidgetBase;
class HbWidget;
class HbStylePrimitiveData;


class HB_CORE_EXPORT HbStyle:public QObject
{
    Q_OBJECT
public:
    HbStyle();
    ~HbStyle();


    // HbStyle::Primitive enums are DEPRECATED
    enum Primitive {
        P_CustomBase = 0x0f000000
    };

    enum PrimitiveType {
        PT_None = 0,
        PT_TextItem = 1,
        PT_RichTextItem = 2,
        PT_FrameItem = 3,
        PT_IconItem = 4,
        PT_MarqueeItem = 5,
        PT_TouchArea = 6,
        PT_ReservedValue = 0xffff
                          
    };                                   

    virtual QGraphicsObject *createPrimitive(HbStyle::PrimitiveType primitiveType, const QString &itemName, QGraphicsObject *parent = 0) const;
    virtual bool updatePrimitive(QGraphicsObject *primitive, const HbStylePrimitiveData *data, QGraphicsObject *parent = 0) const;

    static void setItemName( QGraphicsItem *item, const QString &name );
    static QString itemName( const QGraphicsItem *item );

    bool parameter(const QString &param, qreal &value, const HbDeviceProfile &profile = HbDeviceProfile()) const;
    void parameters(HbStyleParameters &params, const HbDeviceProfile &profile = HbDeviceProfile()) const;

    void widgetParameters(HbStyleParameters &params, HbWidget* widget) const;

protected:
    friend class HbWidget;
    friend class HbWidgetStyleLoader;
    friend class HbXmlLoaderBaseActions;

    virtual void polish(HbWidget *widget, HbStyleParameters &params);
    virtual void updateThemedParams(HbWidget *widget);
    virtual bool hasOrientationSpecificStyleRules(HbWidget *widget);
    
    HbStylePrivate * const d_ptr;
    HbStyle( HbStylePrivate &dd, QStyle *parent );

private slots:
    void widgetDestroyed(QObject* obj);

private:
    Q_DISABLE_COPY( HbStyle )
    Q_DECLARE_PRIVATE_D( d_ptr, HbStyle )
};


#endif // HBSTYLE_H

