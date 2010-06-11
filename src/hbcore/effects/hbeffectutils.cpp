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

#include "hbeffectutils_p.h"
#include "hbeffectdef_p.h"
#include "hbeffectfxmldata_p.h"
#include "hbdeviceprofile.h"
#include <QGraphicsItem>
#include <QEasingCurve>

// ===========================================================================

/*
    \class HbEffectUtils

    \brief HbEffectUtils is an internal helper class used by effect framework.
....HbEffectUtils is an internal class used by effect framework to get common functionalities.

    \warning This class is a part of internal library implementation and may
    be removed or modified!

    \internal
*/

qreal HbEffectUtils::resolveFxmlRef(
    const QString &ref,
    const QString &value,
    bool *ok,
    const QGraphicsItem *item,
    HbEffectUtils::valueType type,
    const QRectF &extRect)
{
    bool valueOk = false;
    bool refOk = false;

    // Resolve value parameter
    qreal ret = value.toFloat(&valueOk);
    qreal refValue = 0;

    bool mapToItemCoordinates = false;
    bool addToPos = false;
    QRectF srcRect;
    bool x = false;
    bool y = false;
    QRectF rect = item->boundingRect();

    if (!ref.isEmpty()) {
        refOk = true; // changed to false if no match
        // Visual values
        if (ref == FXML_KEYWORD_VISUAL_LEFT) {
            refValue = 0;
            addToPos = true;
            x = true;
        } else if (ref == FXML_KEYWORD_VISUAL_RIGHT) {
            refValue = rect.width();
            addToPos = true;
            x = true;
        } else if (ref == FXML_KEYWORD_VISUAL_TOP) {
            refValue = 0;
            addToPos = true;
            y = true;
        } else if (ref == FXML_KEYWORD_VISUAL_BOTTOM) {
            refValue = rect.height();
            addToPos = true;
            y = true;
        } else if (ref == FXML_KEYWORD_VISUAL_WIDTH) {
            refValue = rect.width();
            addToPos = true;
            x = true;
        } else if (ref == FXML_KEYWORD_VISUAL_HEIGHT) {
            refValue = rect.height();
            addToPos = true;
            y = true;
        } else {
            // Reference is something else than visual, so need to map to item's coordinates
            mapToItemCoordinates = true;
            srcRect = extRect;

            // Extrect values
            if (ref == FXML_KEYWORD_EXTRECT_LEFT) {
                refValue = extRect.left();
                x = true;
            } else if (ref == FXML_KEYWORD_EXTRECT_RIGHT) {
                refValue = extRect.right();
                x = true;
            } else if (ref == FXML_KEYWORD_EXTRECT_TOP) {
                refValue = extRect.top();
                y = true;
            } else if (ref == FXML_KEYWORD_EXTRECT_BOTTOM) {
                refValue = extRect.bottom();
                y = true;
            } else if (ref == FXML_KEYWORD_EXTRECT_WIDTH) {
                refValue = extRect.width();
                x = true;
            } else if (ref == FXML_KEYWORD_EXTRECT_HEIGHT) {
                refValue = extRect.height();
                y = true;
            } else {
                // Screen values
                QSize screenSize = HbDeviceProfile::profile(item).logicalSize();
                srcRect = QRectF(QPointF(0, 0), screenSize);

                if (ref == FXML_KEYWORD_SCREEN_LEFT) {
                    refValue = 0;
                    x = true;
                } else if (ref == FXML_KEYWORD_SCREEN_RIGHT) {
                    refValue = screenSize.width();
                    x = true;
                } else if (ref == FXML_KEYWORD_SCREEN_TOP) {
                    refValue = 0;
                    y = true;
                } else if (ref == FXML_KEYWORD_SCREEN_BOTTOM) {
                    refValue = screenSize.height();
                    y = true;
                } else if (ref == FXML_KEYWORD_SCREEN_WIDTH) {
                    refValue = screenSize.width();
                    x = true;
                } else if (ref == FXML_KEYWORD_SCREEN_HEIGHT) {
                    refValue = screenSize.height();
                    y = true;
                }
                // If nothing matched, set ok to false
                else {
                    refOk = false;
                }
            }
        }
    }

    if (refOk) {
        // If value is ok, multiply reference value with it (e.g. 0.5 x screen.height)
        if (valueOk) {
            refValue *= ret;
        }

        // Add calculated value to item position if the reference was visual's width or height
        if (addToPos && type == HbEffectUtils::Position) {
            if (x) {
                refValue += item->pos().x();
            } else {
                refValue += item->pos().y();
            }
        }

        if (type == HbEffectUtils::Position || type == HbEffectUtils::Center) {
            // Map to parent coordinates if needed
            if (mapToItemCoordinates && item->parentItem()) {
                if (x) {
                    refValue = item->mapToParent(item->mapFromScene(refValue, 0)).x();
                } else if (y) {
                    refValue = item->mapToParent(item->mapFromScene(0, refValue)).y();
                }
            }
            // Calculate distance from item origin if needed
            if (type == HbEffectUtils::Center && mapToItemCoordinates) {
                if (x) {
                    refValue -= item->pos().x();
                } else if (y) {
                    refValue -= item->pos().y();
                }
            }
        }

        // Convert to item size units if needed
        if (type == HbEffectUtils::Size) {
            if (x) {
                if (!fuzzyIsNull(rect.width())) {
                    refValue /= rect.width();
                } else {
                    refValue = 0;
                }
            } else if (y) {
                if (!fuzzyIsNull(rect.height())) {
                    refValue /= rect.height();
                } else {
                    refValue = 0;
                }
            }
        }

        // This operation is needed e.g. when scale is done from some other rect to target rect
        if (type == HbEffectUtils::CenterMappedToTargetRect && mapToItemCoordinates) {
            QPointF scenePos = item->scenePos();
            // If the item is already translated, have to subtract that to get correct result
            QTransform trans = item->transform();
            qreal dx = trans.dx();
            qreal dy = trans.dy();
            scenePos -= QPointF(dx, dy);

            if (x) {
                qreal divider = srcRect.width() - rect.width();
                qreal mapped = !fuzzyIsNull(divider) ? (scenePos.x() - srcRect.left()) / divider : 0;
                mapped *= rect.width();
                // TODO: how should refvalue affect here?
                refValue = mapped;
            } else {
                qreal divider = srcRect.height() - rect.height();
                qreal mapped = !fuzzyIsNull(divider) ? (scenePos.y() - srcRect.top()) / divider : 0;
                mapped *= rect.height();
                // TODO: how should refvalue affect here?
                refValue = mapped;
            }
        }

        ret = refValue;
    }

    *ok = valueOk || refOk;
    return ret;
}


qreal HbEffectUtils::resolveFxmlRef(
    const HbEffectFxmlParamData &data, bool *ok, const QGraphicsItem *item, HbEffectUtils::valueType type, const QRectF &extRect)
{
    return HbEffectUtils::resolveFxmlRef(data.getAttribute(FXML_PARAM_REF), data.getValue(), ok, item, type, extRect);
}

void HbEffectUtils::resolveFxmlCurveShape(QEasingCurve &curve, const HbEffectFxmlParamData &data)
{
    QString style = data.getAttribute(FXML_STYLE);

    if (!style.isEmpty()) {
        if (style == FXML_KEYWORD_STYLE_LINEAR) {
            curve = QEasingCurve::Linear;
        } else if (style == FXML_KEYWORD_STYLE_INQUAD) {
            curve = QEasingCurve::InQuad;
        } else if (style == FXML_KEYWORD_STYLE_OUTQUAD) {
            curve = QEasingCurve::OutQuad;
        } else if (style == FXML_KEYWORD_STYLE_INOUTQUAD) {
            curve = QEasingCurve::InOutQuad;
        } else if (style == FXML_KEYWORD_STYLE_OUTINQUAD) {
            curve = QEasingCurve::OutInQuad;
        } else if (style == FXML_KEYWORD_STYLE_INBACK) {
            curve = QEasingCurve::InBack;
        } else if (style == FXML_KEYWORD_STYLE_OUTBACK) {
            curve = QEasingCurve::OutBack;
        } else if (style == FXML_KEYWORD_STYLE_INOUTBACK) {
            curve = QEasingCurve::InOutBack;
        } else if (style == FXML_KEYWORD_STYLE_OUTINBACK) {
            curve = QEasingCurve::OutInBack;
        }
    }
}

void HbEffectUtils::resolveFxmlDuration(int &duration, const HbEffectFxmlParamData &data)
{
    QString string = data.duration();

    if (!string.isEmpty()) {
        bool ok = false;
        qreal value = string.toFloat(&ok);
        if (ok) {
            duration = (int)(value * 1000); // convert to milliseconds
        }
    }
}
