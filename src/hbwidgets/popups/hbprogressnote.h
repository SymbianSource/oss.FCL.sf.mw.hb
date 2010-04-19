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


#ifndef HBPROGRESSNOTE_H
#define HBPROGRESSNOTE_H

#include <hbdialog.h>

class HbProgressNotePrivate;

class HB_WIDGETS_EXPORT HbProgressNote : public HbDialog
{
    Q_OBJECT
    Q_ENUMS(ProgressNoteType)
    Q_PROPERTY( int maximum READ maximum WRITE setMaximum )
    Q_PROPERTY( int minimum READ minimum WRITE setMinimum )
    Q_PROPERTY( int value READ progressValue WRITE setProgressValue )
    Q_PROPERTY( bool autoClose READ autoClose WRITE setAutoClose )
    Q_PROPERTY( ProgressNoteType progressNoteType READ progressNoteType WRITE setProgressNoteType)
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( HbIcon icon READ icon WRITE setIcon )
    Q_PROPERTY( Qt::Alignment textAlignment READ textAlignment WRITE setTextAlignment )
    Q_PROPERTY( Qt::Alignment iconAlignment READ iconAlignment WRITE setIconAlignment )
    Q_PROPERTY( bool textWrapping READ textWrapping WRITE setTextWrapping )
public:

    enum ProgressNoteType { ProgressNote,WaitNote };

    HbProgressNote(ProgressNoteType type , QGraphicsItem *parent = 0);
    explicit HbProgressNote(QGraphicsItem *parent = 0);

    enum { Type = Hb::ItemType_ProgressNote };
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
    
	void setTextWrapping(bool wrap);
    bool textWrapping() const;
    
    void setProgressNoteType(HbProgressNote::ProgressNoteType type );
    HbProgressNote::ProgressNoteType progressNoteType() const;
    
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
    Q_DECLARE_PRIVATE_D(d_ptr, HbProgressNote)
    Q_DISABLE_COPY(HbProgressNote)
    Q_PRIVATE_SLOT(d_func(), void _q_finished())
	Q_PRIVATE_SLOT(d_func(), void _q_progressValueChanged(int))
	Q_PRIVATE_SLOT(d_func(), void _q_userCancel())
};

#endif // HBPROGRESSNOTE_H
