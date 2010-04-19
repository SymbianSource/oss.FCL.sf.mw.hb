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

#ifndef HBLISTWIDGETITEM_H
#define HBLISTWIDGETITEM_H

#include <hbglobal.h>
#include <hbnamespace.h>
#include <hbicon.h>

class HbIcon;
class HbListWidgetItemPrivate;
class HbListModel;

class HB_WIDGETS_EXPORT HbListWidgetItem
{    
public:

    HbListWidgetItem(int type = Hb::StandardItem);
    virtual ~HbListWidgetItem();

    QVariant data(int role = Hb::UserRole) const;
    void setData(const QVariant &value, int role = Hb::UserRole);

    inline QVariant background() const
        {return data(Qt::BackgroundRole);}
    inline void setBackground(const QVariant &background)
        {setData(background, Qt::BackgroundRole);}

    QString text() const;
    void setText(const QString &text);

    QString secondaryText() const;
    void setSecondaryText(const QString &text);

    HbIcon icon() const;
    void setIcon(const HbIcon &icon);

    HbIcon secondaryIcon() const;
    void setSecondaryIcon(const HbIcon &icon);
    
    void setEnabled(bool enabled);
    bool isEnabled() const;

    int type() const;

private:
    Q_DISABLE_COPY(HbListWidgetItem)
    HbListWidgetItemPrivate *d;
    friend class HbListModel; 
    friend class HbListWidgetItemPrivate;
};

#endif // HBLISTWIDGETITEM_H
