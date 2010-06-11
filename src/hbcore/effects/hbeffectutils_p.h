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
#ifndef HB_EFFECT_UTILS_P_H
#define HB_EFFECT_UTILS_P_H

#include <hbglobal.h>
#include <QEasingCurve>
#include <QRectF>

QT_FORWARD_DECLARE_CLASS(QGraphicsItem)
class HbEffectFxmlParamData;

class HB_AUTOTEST_EXPORT HbEffectUtils
{
public:

    enum valueType {
        // Returns defined size in item size units
        Size = 0,
        // Returns defined position in item pos coordinates (can be used with item->setPos())
        Position = 1,
        // Returns the defined center relative to the item's pos (used with rotate)
        Center = 2,
        // Returns the defined center mapped to the target rectangle (used with scale)
        CenterMappedToTargetRect = 3
    };

    static qreal resolveFxmlRef(
        const QString &ref,
        const QString &value,
        bool *ok,
        const QGraphicsItem *item,
        valueType type,
        const QRectF &extRect = QRectF());

    static qreal resolveFxmlRef(
        const HbEffectFxmlParamData &data,
        bool *ok,
        const QGraphicsItem *item,
        valueType type,
        const QRectF &extRect = QRectF());

    static void resolveFxmlCurveShape(QEasingCurve &curve, const HbEffectFxmlParamData &data);

    static void resolveFxmlDuration(int &duration, const HbEffectFxmlParamData &data);

    static inline bool fuzzyIsNull(double d) {
        return qAbs(d) <= 0.000000000001;
    }

    static inline bool fuzzyIsNull(float f) {
        return qAbs(f) <= 0.00001f;
    }

    static inline bool fuzzyIsOneOrGreater(double d) {
        return d >= 1.0 - 0.000000000001;
    }

    static inline bool fuzzyIsOneOrGreater(float f) {
        return f >= 1.0f - 0.00001f;
    }

};

#endif // HB_EFFECT_UTILS_P_H

