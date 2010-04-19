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

#ifndef HBSEARCHPANEL_P_H
#define HBSEARCHPANEL_P_H


#include "hbwidget_p.h"
#include <hbsearchpanel.h>

class HbPushButton;
class HbLineEdit;

class HbSearchPanelPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC( HbSearchPanel )

public:
    HbSearchPanelPrivate( );
    ~HbSearchPanelPrivate( );
    void init( );
    void _q_hideClicked();
    void _q_criteriaChanged();
    void _q_searchOptionsClicked();

    void addSearchOptionsButton();
    void removeSearchOptionsButton();

    void addProgressButton();
    void removeProgressButton();

    void constructUi();

public:
    bool mProgressive;
    bool mSearchOptions;
    HbPushButton *mSearchOptionsButton;
    HbLineEdit *mLineEdit;
    HbPushButton *mProgressButton;
    HbPushButton *mCancelButton;
};

#endif // HBSEARCHPANEL_P_H
