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

#ifndef HBFORMATDIALOG_P_H
#define HBFORMATDIALOG_P_H

#include "hbformatdialog.h"
#include <hbdialog_p.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

class HbAbstractEdit;
class HbComboBox;
class HbPushButton;
class HbDialog;

class QGraphicsGridLayout;

class HbFormatDialogPrivate : public HbDialogPrivate
{
    Q_DECLARE_PUBLIC(HbFormatDialog)

public:
    HbFormatDialogPrivate();
    virtual ~HbFormatDialogPrivate();
    void _q_setColor();
    void colorChanged(const QColor &);
    void setAlignment(Qt::Alignment);
    void _q_setAlignment(bool);
    HbAbstractEdit *editor;

    QGraphicsGridLayout* layout;
    HbComboBox* comboSize;
    HbPushButton* buttonColor;
    Qt::Alignment currentAlignment;
    HbPushButton* buttonAlignLeft;
    HbPushButton* buttonAlignCenter;
    HbPushButton* buttonAlignRight;
    HbPushButton* buttonAlignJustify;
    HbPushButton* buttonBold;
    HbPushButton* buttonItalic;
    HbPushButton* buttonUnderline;
    HbPushButton* buttonBullet;
};

#endif // HBFORMATDIALOG_P_H
