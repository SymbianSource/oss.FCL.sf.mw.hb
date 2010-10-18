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

#ifndef HBRICHTEXTITEM_P_H
#define HBRICHTEXTITEM_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Hb API.  It exists purely as an
// implementation detail.  This file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "hbwidgetbase_p.h"
#include "hbrichtextitem.h"

#include <QTextOption>

class QTextDocument;
class HB_CORE_PRIVATE_EXPORT HbRichTextItemPrivate: public HbWidgetBasePrivate {
    Q_DECLARE_PUBLIC(HbRichTextItem)

public:

    HbRichTextItemPrivate();
    ~HbRichTextItemPrivate();

    void init();
    void clear();

    void setDocumentWidth(qreal newWidth);

    void calculateOffset();

    QSizeF minimumSizeHint(const QSizeF &constraint) const;
    void clearPrefSizeCache();
    QSizeF preferredSizeHint(const QSizeF &constraint) const;
    bool   isAdjustHeightNeeded(qreal newWidth,
                                qreal prefHeight,
                                qreal minHeight,
                                qreal maxHeight);

    bool restoreDefaultHeightHint();

    QString mText;
    QTextOption mTextOption; // used for alignment

    QTextDocument  *mRtf;
    QPointF mOffset;

    mutable QSizeF mPrefSize;
    mutable QSizeF mPrefSizeConstraint;
    mutable qreal  mMinWidthForAdjust;
    mutable qreal  mMaxWidthForAdjust;
    mutable qreal  mDefaultPrefHeight;

    QColor mColor;
    mutable QColor mDefaultColor;
};

#endif // HBRICHTEXTITEM_P_H
