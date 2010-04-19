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

#include "hbformatdialog_p.h"
#include "hbformatdialog.h"

#include <hbabstractedit.h>
#include <hbpushbutton.h>
#include <hbcolordialog.h>

#include <QSharedData>
#include <QTextCursor>
#include <QPainter>

HbFormatDialogPrivate::HbFormatDialogPrivate()
{}

HbFormatDialogPrivate::~HbFormatDialogPrivate()
{}

void HbFormatDialogPrivate::setAlignment(Qt::Alignment alignment)
{
    currentAlignment = alignment;
    QTextCursor cursor = editor->textCursor();
    QTextBlockFormat fmt = cursor.blockFormat();
    if (fmt.alignment() != alignment) {
        fmt.setAlignment(alignment);
        cursor.mergeBlockFormat(fmt);
    }
}

void HbFormatDialogPrivate::_q_setAlignment(bool toggled)
{
    Q_UNUSED(toggled)
    buttonAlignLeft->setChecked(currentAlignment == Qt::AlignLeft);
    buttonAlignCenter->setChecked(currentAlignment == Qt::AlignCenter);
    buttonAlignRight->setChecked(currentAlignment == Qt::AlignRight);
    buttonAlignJustify->setChecked(currentAlignment == Qt::AlignJustify);
}

void HbFormatDialogPrivate::_q_setColor()
{
    Q_Q(HbFormatDialog);
    QTextCursor cursor = editor->textCursor();
    QTextCharFormat fmt = cursor.charFormat();
    QColor color = fmt.foreground().color();
    HbColorDialog dialog;
    dialog.setInitialColor(color);
    color = dialog.getColor();
    if (!color.isValid())
        return;
    fmt.setForeground(color);
    q->mergeCharFormat(fmt);
    colorChanged(color);
}

void HbFormatDialogPrivate::colorChanged(const QColor &color)
{
    HbIcon icon("qtg_graf_colorpicker_filled");
    HbIcon maskIcon("qtg_graf_colorpicker_mask");
    QPixmap maskPixmap = maskIcon.pixmap();
    QPainter mp(&maskPixmap);
    mp.setCompositionMode(QPainter::CompositionMode_SourceIn);
    mp.fillRect(maskPixmap.rect(),color);
    mp.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    icon.paint(&mp,maskPixmap.rect());
    mp.end();
    buttonColor->setIcon(HbIcon(maskPixmap));
}

#include "moc_hbformatdialog.cpp"
