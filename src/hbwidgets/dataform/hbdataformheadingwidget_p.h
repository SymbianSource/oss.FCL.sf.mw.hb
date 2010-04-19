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

#ifndef HBDATAFORMHEADINGWIDGET_P_H
#define HBDATAFORMHEADINGWIDGET_P_H

#include <hbwidget.h>
#include "hbnamespace_p.h"

class QGraphicsItem;
class HbComboBox;
class HbStyleOptionDataForm;

class HbDataFormHeadingWidget : public HbWidget
{
    Q_OBJECT

public:
    explicit HbDataFormHeadingWidget(QGraphicsItem *parent = 0);

    ~HbDataFormHeadingWidget() {
    }

    void createPrimitives();
    void updatePrimitives();
    void initStyleOption(HbStyleOptionDataForm *option);

    enum { Type = HbPrivate::ItemType_DataFormHeadingWidget};
    int type() const { return Type; }

    void callPolish();

    int pageIndex(const QString& page) const;
    void updatePageName(int index, const QString& page);

public:
    QGraphicsItem *mHeadingItem;
    QGraphicsItem *mDescriptionItem;
    QGraphicsItem *mBackgroundItem;
    HbComboBox *mPageCombo;
    int mActivePage;
    QString mHeading;
    QString mDescription;
};

#endif // HBDATAFORM_PRIVATE_H
