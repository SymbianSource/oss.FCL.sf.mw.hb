/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbWidgets module of the UI Extensions for Mobile.
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
#ifndef HBGRIDVIEWITEM_P_H
#define HBGRIDVIEWITEM_P_H

#include "hbgridviewitem.h"

#include <hbabstractviewitem_p.h>
#include <hbicon.h>

class HbGridViewItem;
class HbGridViewPrivate;

class QGraphicsItem;

class HbGridViewItemPrivate: public HbAbstractViewItemPrivate
{
    Q_DECLARE_PUBLIC( HbGridViewItem )

public:
    explicit HbGridViewItemPrivate(HbAbstractViewItem *prototype);
    ~HbGridViewItemPrivate();

    void init();

    // below variables store items that are inside GridViewItem
    QGraphicsItem *mIconItem;
    QGraphicsItem *mTextItem;

    bool isTextValid(const QVariant& text) const;
    bool isIconValid(const QVariant& icon) const;

    QString text(const QVariant& text) const;
    HbIcon icon(const QVariant& icon) const;

    void updateTextItem(const HbGridViewPrivate &viewPrivate);
    void updateIconItem(const HbGridViewPrivate &viewPrivate);

    // used to differentiate null QString() from invalid content in display role. The previous case creates text item.
    bool    mTextValid;
    QString mText;

    // used to differentiate null HbIcon() from invalid content in decoration role. The previous case creates icon item.
    bool    mIconValid;
    HbIcon  mIcon;
};

#endif /*HBGRIDVIEWITEM_P_H*/
