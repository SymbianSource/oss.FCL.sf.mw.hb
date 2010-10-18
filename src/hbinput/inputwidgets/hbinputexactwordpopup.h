/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbInput module of the UI Extensions for Mobile.
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

#ifndef HB_INPUT_EXACT_WORD_POPUP_H
#define HB_INPUT_EXACT_WORD_POPUP_H

#include <hbinputpopupbase.h>
#include <hbinputdef.h>

class HbExactWordPopupPrivate;

class HB_INPUT_EXPORT HbExactWordPopup : public HbInputPopupBase
{
    Q_OBJECT

public:
    enum HbExactWordPopupIndicator {
        HbNoIndicatorArrow,
        HbIndicatorArrow
    };

    static HbExactWordPopup *instance(HbExactWordPopupIndicator indicatorArrow = HbNoIndicatorArrow);

    QString text();
    int type() const {
        return Type;
    }

public slots:
    void setText(const QString &text);
    void showText(QPointF pos);
    void hideText();
    void setIndicatorArrow(HbExactWordPopupIndicator indicatorArrow);

protected:
    HbExactWordPopup(QGraphicsWidget *parent = 0, HbExactWordPopupIndicator indicatorArrow = HbNoIndicatorArrow);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

signals:
    void exactWordSelected();

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbExactWordPopup)
    Q_DISABLE_COPY(HbExactWordPopup)
};

#endif // HB_INPUT_EXACT_WORD_POPUP_H

