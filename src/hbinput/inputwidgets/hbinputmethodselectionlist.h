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

#ifndef HB_INPUT_METHOD_SELECTION_LIST_H
#define HB_INPUT_METHOD_SELECTION_LIST_H

#include <hbdialog.h>
#include <hbinputdef.h>

class HbInputMethodSelectionListPrivate;
class HbListWidgetItem;
class HbInputMethodDescriptor;

class HB_INPUT_EXPORT HbInputMethodSelectionList : public HbDialog
{
    Q_OBJECT

public:
    HbInputMethodSelectionList(QGraphicsWidget* parent = 0);
    virtual ~HbInputMethodSelectionList();

public slots:
    void activateSelectedMethod(HbListWidgetItem *item);

signals:
    void inputMethodSelected(const HbInputMethodDescriptor &descriptor, const QByteArray &customData);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputMethodSelectionList)
    Q_DISABLE_COPY(HbInputMethodSelectionList)   
};

#endif // HB_INPUT_METHOD_SELECTION_LIST_H

// End of file
