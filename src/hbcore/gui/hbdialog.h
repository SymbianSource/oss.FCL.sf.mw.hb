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

#ifndef HBDIALOG_H
#define HBDIALOG_H

#include <hbglobal.h>
#include <hbpopup.h>

class HbAction;
class HbDialogPrivate;

class HB_CORE_EXPORT HbDialog : public HbPopup
{
    Q_OBJECT

public:
    enum DialogCode { Rejected, Accepted };

    explicit HbDialog( QGraphicsItem *parent = 0 );
    virtual  ~HbDialog();

    QGraphicsWidget *headingWidget() const;
    void setHeadingWidget( QGraphicsWidget *headingWidget );

    QGraphicsWidget *contentWidget() const;
    void setContentWidget( QGraphicsWidget *contentWidget );

    HbAction *primaryAction() const;
    void setPrimaryAction( HbAction *action );

    HbAction *secondaryAction() const;
    void setSecondaryAction( HbAction *action );

    void open( QObject* receiver, const char* member );

    enum { Type = Hb::ItemType_Dialog }; // TODO: Hb::ItemType_Dialog
    int type() const { return Type; }

public slots:
    void open();
    virtual void done(int code);
    virtual void accept();
    virtual void reject();   

signals:
    void finished(HbAction*); 
    void finished(int);
    void accepted();
    void rejected();

protected:
    HbDialog( HbDialogPrivate &dd, QGraphicsItem *parent );

    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
    void focusInEvent( QFocusEvent *event );
    void closeEvent( QCloseEvent * event );
    void changeEvent(QEvent *event );
    bool event( QEvent *event );

private:
    Q_DECLARE_PRIVATE_D( d_ptr, HbDialog )
    Q_DISABLE_COPY( HbDialog )
};

#endif // HBDIALOG_H
