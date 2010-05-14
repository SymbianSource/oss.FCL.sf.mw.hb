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

#ifndef HBPROGRESSDIALOGPRIVATE_H
#define HBPROGRESSDIALOGPRIVATE_H

#include <hbprogressbar.h>
#include <hbprogressdialog.h>
#include <hbdialog_p.h>
#include <hblabel.h>

#include <QFlags>
#include <QTimeLine>

class HbProgressDialogContentWidget;

class HbProgressDialogPrivate :public HbDialogPrivate
{
    Q_DECLARE_PUBLIC(HbProgressDialog)
public:
    HbProgressDialogPrivate();
    ~HbProgressDialogPrivate();

    void init(HbProgressDialog::ProgressDialogType noteType);
    void createPrimitives();
    void _q_finished();
    void _q_progressValueChanged(int);
    void _q_userCancel();

    enum StateFlag {
        Autoreset = 0x1,
        Autoclose = 0x2,
        Showtimer = 0x4,
        Closetimer = 0x8,
        Closepending = 0x10
     };

     Q_DECLARE_FLAGS(StateFlags, StateFlag)
     StateFlags flags;

    static HbProgressDialogPrivate *d_ptr(HbProgressDialog *note) {
        Q_ASSERT(note);
        return note->d_func();
    }
    QTimeLine *mTimer;
    HbIcon mIcon;
    HbProgressDialogContentWidget* mContentWidget;
    Qt::Alignment mIconAlignment;
    int mMinDuration;
    int mDelayTime;
    HbProgressDialog::ProgressDialogType mNoteType;
    QString mTextString;
    Qt::Alignment mAlign;
};

#endif
