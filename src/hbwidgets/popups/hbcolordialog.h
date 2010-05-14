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

#ifndef HBCOLORDIALOG_H
#define HBCOLORDIALOG_H

#include <hbdialog.h>

class HbColorDialogPrivate;

class HB_WIDGETS_EXPORT HbColorDialog : public HbDialog
{
    Q_OBJECT

    Q_PROPERTY(QColor initialcolor READ initialColor WRITE setInitialColor)
    Q_PROPERTY(bool noneBlockVisible READ isNoneBlockVisible WRITE setNoneBlockVisible)

public:
    explicit HbColorDialog(QGraphicsItem *parent = 0);

    void setInitialColor(const QColor &initialColor);
    QColor initialColor() const;

    QColor currentColor() const;

    void addColor(const QColor &color);
    void addColors(const QList<QColor> &colorList);

    void setNoneBlockVisible(bool visible);
    bool isNoneBlockVisible() const;

    bool isNoneSelected() const;

    static void getColor( const QList<QColor> &colorList, 
                          const QColor &initialColor, 
                          QObject* receiver,
                          const char* member,
                          bool isNoneBlockVisible = false, 
                          QGraphicsScene *scene = 0, 
                          QGraphicsItem *parent = 0 );

    enum { Type = Hb::ItemType_ColorDialog};
    int type() const { return Type; }

signals:
    void colorSelected(const QColor &color);
    void noneSelected();
    void finished(QColor,bool);

protected:
    HbColorDialog(HbColorDialogPrivate &dd, QGraphicsItem *parent = 0);
    void showEvent(QShowEvent *event);
    void polish(HbStyleParameters &params);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbColorDialog)
    Q_DISABLE_COPY(HbColorDialog)
    Q_PRIVATE_SLOT(d_func(), void _q_setCurrentSelectedColor(const QModelIndex &index))
};

#endif // HBCOLORDIALOG_H
