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


#ifndef HBPROGRESSDIALOG_H
#define HBPROGRESSDIALOG_H

#include <hbdialog.h>

class HbProgressDialogPrivate;

class HB_WIDGETS_EXPORT HbProgressDialog : public HbDialog
{
    Q_OBJECT
    Q_ENUMS(ProgressDialogType)
    Q_PROPERTY( int maximum READ maximum WRITE setMaximum )
    Q_PROPERTY( int minimum READ minimum WRITE setMinimum )
    Q_PROPERTY( int value READ progressValue WRITE setProgressValue )
    Q_PROPERTY( bool autoClose READ autoClose WRITE setAutoClose )
    Q_PROPERTY( ProgressDialogType progressDialogType READ progressDialogType WRITE setProgressDialogType)
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( HbIcon icon READ icon WRITE setIcon )
    Q_PROPERTY( Qt::Alignment textAlignment READ textAlignment WRITE setTextAlignment )
    Q_PROPERTY( Qt::Alignment iconAlignment READ iconAlignment WRITE setIconAlignment )

public:

    enum ProgressDialogType { ProgressDialog,WaitDialog };

    HbProgressDialog(ProgressDialogType type , QGraphicsItem *parent = 0);
    explicit HbProgressDialog(QGraphicsItem *parent = 0);

    enum { Type = Hb::ItemType_ProgressDialog };
    int type() const { return Type; }

    int maximum() const;
    int minimum() const;
   
	void setRange(int min,int max);
    int progressValue() const;
    
	bool autoClose () const ;
    void setAutoClose ( bool b ) ;

	void setText(const QString &text);
	QString text() const;
    
	void setIcon(const HbIcon &icon);
	HbIcon icon() const;
    
	void setTextAlignment(Qt::Alignment align);
	Qt::Alignment textAlignment() const;
    
	void setIconAlignment(Qt::Alignment align);
	Qt::Alignment iconAlignment() const;
    
    void setProgressDialogType(HbProgressDialog::ProgressDialogType type );
    HbProgressDialog::ProgressDialogType progressDialogType() const;
    
    QGraphicsItem* primitive(HbStyle::Primitive primitive) const;
    
public slots:
    void cancel();
    void setProgressValue(int progressValue);
    void setMinimum(int min);
	void setMaximum(int max);
    void delayedShow();

signals:
    void cancelled();

protected:
	void showEvent(QShowEvent *event);
	void initStyleOption(HbStyleOption *option) const;
    void closeEvent ( QCloseEvent * event );

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbProgressDialog)
    Q_DISABLE_COPY(HbProgressDialog)
    Q_PRIVATE_SLOT(d_func(), void _q_finished())
	Q_PRIVATE_SLOT(d_func(), void _q_progressValueChanged(int))
	Q_PRIVATE_SLOT(d_func(), void _q_userCancel())
};

#endif // HBPROGRESSDIALOG_H
