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

#ifndef HBSEARCHPANEL_H
#define HBSEARCHPANEL_H

#include <hbwidget.h>

class HbSearchPanelPrivate;

class HB_WIDGETS_EXPORT HbSearchPanel : public HbWidget
{
    Q_OBJECT

    Q_PROPERTY( bool progressive READ isProgressive WRITE setProgressive )
    Q_PROPERTY( bool searchOptions READ isSearchOptionsEnabled WRITE setSearchOptionsEnabled )
    Q_PROPERTY( bool cancel READ isCancelEnabled WRITE setCancelEnabled )
    Q_PROPERTY(QString criteria READ criteria WRITE setCriteria USER true)
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText)

public:
    explicit HbSearchPanel(QGraphicsItem *parent = 0);
    virtual ~HbSearchPanel();

    void setProgressive(bool progressive);
    bool isProgressive() const;

    void setSearchOptionsEnabled(bool enabled);
    bool isSearchOptionsEnabled() const;

    void setCancelEnabled(bool enabled);
    bool isCancelEnabled() const;

    QString placeholderText() const;
    void setPlaceholderText(const QString &text);

    QString criteria() const;
    void setCriteria(const QString &text);


signals:
    void searchOptionsClicked();
    void criteriaChanged(const QString &criteria);
    void exitClicked();

protected:
    HbSearchPanel(HbSearchPanelPrivate &dd, QGraphicsItem *parent = 0);

private:
    Q_DECLARE_PRIVATE_D( d_ptr, HbSearchPanel )
    Q_DISABLE_COPY(HbSearchPanel)
    Q_PRIVATE_SLOT( d_func(), void _q_hideClicked() )
    Q_PRIVATE_SLOT( d_func(), void _q_criteriaChanged() )
    Q_PRIVATE_SLOT( d_func(), void _q_searchOptionsClicked() )

};


#endif // HBSEARCHPANEL_H
