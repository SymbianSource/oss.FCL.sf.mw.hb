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

#ifndef HBNOTIFICATIONDIALOGCONTENT_H
#define HBNOTIFICATIONDIALOGCONTENT_H

#include <hbwidget.h>

class QGraphicsItem;
class HbIcon;
class HbStyleOptionNotificationDialog;

class HbNotificationDialogContent : public HbWidget
{ 
    Q_OBJECT

public:
    HbNotificationDialogContent(QGraphicsWidget *parent = 0);
    virtual ~HbNotificationDialogContent();
    void setText(const QString &text);
    void setTitle(const QString &title);
    void setIcon(const HbIcon &icon);
    void setTitleTextWrapping(Hb::TextWrapping wrapping);
    Hb::TextWrapping titleTextWrapping() const;
    
    QString text() const { return mText; }
    QString title() const { return mTitle; }
    HbIcon icon() const { return mIcon; }
    
    enum { Type = Hb::ItemType_NotificationDialog };
    int type() const { return Type; }
    void enableTouchActivation(bool enabled);
    bool isTouchActivating() const;
    QGraphicsItem *primitive(const QString &itemName) const;

public slots:
    virtual void updatePrimitives();
    
protected:
    void polish( HbStyleParameters& params );
    void initStyleOption(HbStyleOptionNotificationDialog *option) const;
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;
private:    
    Hb::TextWrapping mTitleTextWrapping;
    QString mText;
    QString mTitle;
    HbIcon mIcon;
    bool mIsTouchActivating;
  
    QGraphicsItem *mTextItem;
    QGraphicsItem *mTitleItem;
    QGraphicsItem *mIconItem;
};

#endif // HBNOTIFICATIONDIALOGCONTENT
