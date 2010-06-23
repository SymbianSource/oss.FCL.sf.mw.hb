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

#include "hbsearchpanel_p.h"

#include <hblineedit.h>
#include <hbpushbutton.h>

const QString cancelIcon = QString("qtg_mono_exit");
const QString searchOptionsIcon = QString("qtg_mono_options_menu");
const QString progressIcon = QString("qtg_mono_go");

HbSearchPanelPrivate::HbSearchPanelPrivate( ):
    HbWidgetPrivate (  ),
    mProgressive ( true ),
    mSearchOptions ( false ),
    mCancelEnabled(true),
    mSearchOptionsButton(0),
    mLineEdit(0),
    mProgressButton(0),
    mCancelButton(0)
{
}

HbSearchPanelPrivate::~HbSearchPanelPrivate( )
{
}

void HbSearchPanelPrivate::init( )
{
    Q_Q( HbSearchPanel );
    constructUi();

    //set default values
    q->setAttribute(Hb::InputMethodNeutral);
    q->setFlag(QGraphicsItem::ItemIsFocusable);

    // connect initial state signals
    QObject::connect(mLineEdit, SIGNAL(textChanged(QString)), q, SIGNAL(criteriaChanged(QString)));

}

void HbSearchPanelPrivate::addSearchOptionsButton()
{
    Q_Q( HbSearchPanel );
    mSearchOptions = true;
    mSearchOptionsButton = new HbPushButton(q);
    mSearchOptionsButton->setObjectName("optionsbutton");

    mSearchOptionsButton->setIcon(HbIcon(searchOptionsIcon));
    QObject::connect(mSearchOptionsButton, SIGNAL(clicked()), q, SLOT(_q_searchOptionsClicked()));

    HbStyle::setItemName( mSearchOptionsButton, "searchoptionsbutton" );
    q->repolish();
}

void HbSearchPanelPrivate::removeSearchOptionsButton()
{
    Q_Q( HbSearchPanel );
    mSearchOptions = false;
    HbStyle::setItemName( mSearchOptionsButton, "" );
    QObject::disconnect(mSearchOptionsButton, SIGNAL(clicked()), q, SLOT(_q_searchOptionsClicked()));
    mSearchOptionsButton->deleteLater();
    mSearchOptionsButton = 0;
    q->repolish();
}

void HbSearchPanelPrivate::addProgressButton()
{
    Q_Q( HbSearchPanel );
    mProgressive = false;
    mProgressButton = new HbPushButton(q);
    mProgressButton->setObjectName("progressbutton");

    mProgressButton->setIcon(HbIcon(progressIcon));
    QObject::connect(mProgressButton, SIGNAL(clicked()), q, SLOT(_q_criteriaChanged()));
    QObject::disconnect(mLineEdit,SIGNAL(textChanged(QString)),q, SIGNAL(criteriaChanged(QString)));

    HbStyle::setItemName( mProgressButton, "progressbutton" );
    q->repolish();
}

void HbSearchPanelPrivate::removeProgressButton()
{
    Q_Q( HbSearchPanel );
    mProgressive = true;
    HbStyle::setItemName( mProgressButton, "" );
    QObject::disconnect(mProgressButton, SIGNAL(clicked()), q, SLOT(_q_criteriaChanged()));
    mProgressButton->deleteLater();
    mProgressButton = 0;

    // start to send events always when text is updated
    QObject::connect(mLineEdit, SIGNAL(textChanged(QString)), q, SIGNAL(criteriaChanged(QString)));
    q->repolish();
}

void HbSearchPanelPrivate::addCancelButton()
{
    Q_Q( HbSearchPanel );
    mCancelEnabled = true;
    mCancelButton = new HbPushButton(q);
    mCancelButton->setObjectName("cancelbutton");
    mCancelButton->setIcon(HbIcon(cancelIcon));
    mCancelButton->setFlag(QGraphicsItem::ItemIsPanel, true);
    mCancelButton->setActive(false);
    HbStyle::setItemName( mCancelButton, "cancelbutton" );
    QObject::connect(mCancelButton, SIGNAL(clicked()), q, SLOT(_q_hideClicked()));
    q->repolish();
}

void HbSearchPanelPrivate::removeCancelButton()
{
    Q_Q( HbSearchPanel );
    mCancelEnabled = false;
    HbStyle::setItemName( mCancelButton, "" );
    mCancelButton->deleteLater();
    mCancelButton = 0;
    q->repolish();
}


void HbSearchPanelPrivate::constructUi()
{
    Q_Q( HbSearchPanel );

    //construct default ui.
    mLineEdit = new HbLineEdit(q);
    mLineEdit->setObjectName("lineedit");
    HbStyle::setItemName( mLineEdit, "lineedit" );
    if(mCancelEnabled) {
        addCancelButton();
    }
    q->setFocusProxy(mLineEdit);
}

void HbSearchPanelPrivate::_q_hideClicked()
{
    Q_Q( HbSearchPanel );
    emit q->exitClicked();
}

void HbSearchPanelPrivate::_q_criteriaChanged()
{
    Q_Q( HbSearchPanel );
    emit q->criteriaChanged(mLineEdit->text());
}

void HbSearchPanelPrivate::_q_searchOptionsClicked( )
{
    Q_Q( HbSearchPanel );
    emit q->searchOptionsClicked();
}

#include "moc_hbsearchpanel.cpp"
