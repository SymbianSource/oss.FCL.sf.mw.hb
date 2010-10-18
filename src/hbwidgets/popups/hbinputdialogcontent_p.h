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

#ifndef HBINPUTDIALOGCONTENT_P_H
#define HBINPUTDIALOGCONTENT_P_H

#include "hbnamespace_p.h"
#include <hbwidget.h>

class HbLabel;
class HbLineEdit;
class HbInputDialogPrivate;

class HbInputDialogContentWidget : public HbWidget
{
    Q_OBJECT
public:
    explicit HbInputDialogContentWidget(HbInputDialogPrivate* priv,QGraphicsItem* parent =0);

    enum { Type = HbPrivate::ItemType_HbInputDialogContentWidget };
    int type() const { return Type; }

    void setAdditionalRowVisible(bool visible);
public slots:
    void emitTextChange(const QString &text);
    
signals:
    void textChanged(const QString &text);

public:
    HbInputDialogPrivate* d;
    QGraphicsObject *mLabel1;
    QGraphicsObject *mLabel2; 
    HbLineEdit *mEdit1;
    HbLineEdit *mEdit2;
    bool mAdditionalRowVisible;
};

#endif //HBINPUTDIALOGCONTENT_P_H

