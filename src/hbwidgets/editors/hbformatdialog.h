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
#ifndef HBFORMATDIALOG_H
#define HBFORMATDIALOG_H

#include <hbdialog.h>
#include <hbglobal.h>

#include <QTextCursor>
#include <QPointer>

class HbAbstractEdit;
class HbFormatDialogPrivate;
QT_FORWARD_DECLARE_CLASS(QTextCharFormat)

class HB_WIDGETS_EXPORT HbFormatDialog : public HbDialog
{
   Q_OBJECT

public:

    explicit HbFormatDialog(QGraphicsItem *parent = 0);
    virtual ~HbFormatDialog();

    void mergeCharFormat(const QTextCharFormat &format);

public slots:
    void launch(HbAbstractEdit *editor);

    void setFontPointSize(const QString &size);

    void setAlignmentLeft(bool toggled);
    void setAlignmentCenter(bool toggled);
    void setAlignmentRight(bool toggled);
    void setAlignmentJustify(bool toggled);

    void setStyleBold(bool toggled);
    void setStyleItalic(bool toggled);
    void setStyleUnderline(bool toggled);
    void setStyleBullet(bool toggled);

protected:
    QColor currentCharColor() const;
    virtual void setContent(QTextCursor cursor);
    virtual void polish(HbStyleParameters& params);

private:
    Q_DISABLE_COPY(HbFormatDialog)
    Q_DECLARE_PRIVATE_D(d_ptr, HbFormatDialog)
    Q_PRIVATE_SLOT(d_func(), void _q_setColor())
    Q_PRIVATE_SLOT(d_func(), void _q_setAlignment(bool))
    Q_PRIVATE_SLOT(d_func(), void _q_colorSelected(QColor))
};

typedef QPointer <HbFormatDialog> HbFormatDialogPointer;

#endif // HBFORMATDIALOG_H
