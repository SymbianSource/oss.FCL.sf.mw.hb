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

#include "hbsearchpanel.h"
#include "hbsearchpanel_p.h"
#include "hblineedit.h"


/*!
 @beta
 @hbwidgets
 \class HbSearchPanel

 \brief The HbSearchPanel widget provides a common look and feel for inputting a search term.

 The searchPanel widget contains the following controls by default:
 \li search term input field
 \li cancel button

 The searchPanel may contain the following optional controls depending on the properties:
 \li ok button
 \li searchOptions button

 The ok button exists when progressive property is false. The ok button emits criteriaChanged()
 when the button is clicked. By default progressive property is true and the ok button is not
 available.

 The searchOptions button exists when searchOptions property is true. The searchOptions button
 emits searchOptionsClicked() when the button is clicked. By default searchOptions property is
 false.

 An example how to create HbSearchPanel with non-progressive mode and searchOptions button
 available.
 \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,44}

 */

/*!
    \fn void HbSearchPanel::criteriaChanged(const QString &criteria)
    This signal is emitted whenever the new search should be progressed.
    It depends on the progressive property when this signal is sent.
    \a criteria contains new search criteria.
*/

/*!
    \fn void HbSearchPanel::exitClicked()
    This signal is emitted whenever the exit button is clicked.
*/

/*!
    \fn void HbSearchPanel::searchOptionsClicked()
    This signal is emitted whenever the search options button is clicked.
    The availability of search options button depends on searchOptions property.
*/


/*!
    Constructs a new HbSearchPanel with \a parent.
 */
HbSearchPanel::HbSearchPanel(QGraphicsItem *parent)
    : HbWidget( *new HbSearchPanelPrivate, parent )
{
    Q_D( HbSearchPanel );
    d->q_ptr = this;
    d->init( );
}


/*!
    \internal
 */
HbSearchPanel::HbSearchPanel (HbSearchPanelPrivate &dd, QGraphicsItem *parent) :
    HbWidget(dd, parent)
{
    Q_D(HbSearchPanel);
    d->init();
}

/*!
    Destructor.
 */
HbSearchPanel::~HbSearchPanel( )
{
}

/*!
    \property HbSearchPanel::progressive
    True, Progressive mode where criteriaChanged() signal is emitted every time a content of input
    field is modified. False, Non progressive mode where criteriaChanged() signal is emitted
    whenever OK button is clicked.

    \sa setProgressive
 */
bool HbSearchPanel::isProgressive( ) const
{
    Q_D( const HbSearchPanel );
    return d->mProgressive;
}

/*!
    Set progressive the property of the search panel.
    Parameter \a progressive is the new value of the property.

    \sa isProgressive
 */
void HbSearchPanel::setProgressive( bool progressive )
{
    Q_D( HbSearchPanel );

    if( progressive != isProgressive( ) ) {
        if(progressive) {
            d->removeProgressButton();
        } else {
            d->addProgressButton();
        }
    }
}

/*!
    \property HbSearchPanel::cancelEnabled
    True, there is a cancel button in the search panel.
    False, there is not a cancel button in the search panel.

    \sa setCancelEnabled
 */
bool HbSearchPanel::isCancelEnabled( ) const
{
    Q_D( const HbSearchPanel );
    return d->mCancelEnabled;
}

/*!
    Set cancelEnabled property of the search panel. Parameter \a enabled is the new value of the
    property.

    \sa isCancelEnabled
 */
void HbSearchPanel::setCancelEnabled( bool enabled )
{
    Q_D( HbSearchPanel );
    if( enabled != isCancelEnabled( ) ) {
        if(enabled) {
            d->addCancelButton();
        } else {
            d->removeCancelButton();
        }
    }
}


/*!
    \property HbSearchPanel::searchOptions
    True, there is a search options button in the search panel.
    False, there is not a search options button in the search panel.

    \sa setSearchOptionsEnabled
 */
bool HbSearchPanel::isSearchOptionsEnabled( ) const
{
    Q_D( const HbSearchPanel );
    return d->mSearchOptions;
}

/*!
    Set searchOptions property of the search panel. Parameter \a enabled is the new value of the
    property.

    \sa isSearchOptionsEnabled
 */
void HbSearchPanel::setSearchOptionsEnabled( bool enabled )
{
    Q_D( HbSearchPanel );
    if( enabled != isSearchOptionsEnabled( ) ) {
        if(enabled) {
            d->addSearchOptionsButton();
        } else {
            d->removeSearchOptionsButton();
        }
    }
}


/*!
    \property HbSearchPanel::placeholderText
    \brief the search panel's placeholder text

    Setting this property makes the editor in search panel display a grayed-out
    placeholder text as long as the criteria() is empty.
    By default, this property contains an empty string.

    \sa criteria()
*/

QString HbSearchPanel::placeholderText() const
{
    Q_D(const HbSearchPanel);
    return d->mLineEdit->placeholderText();
}

/*!
    \sa placeholderText()
*/
void HbSearchPanel::setPlaceholderText(const QString &text)
{
    Q_D(HbSearchPanel);
    d->mLineEdit->setPlaceholderText(text);
}

/*!
    Returns the search criteria.

    \sa setCriteria()
*/
QString HbSearchPanel::criteria() const
{
    Q_D(const HbSearchPanel);
    return d->mLineEdit->text();
}

/*!
    Sets the search criteria to the search panel.

    \sa criteria()
*/
void HbSearchPanel::setCriteria(const QString &text)
{
    Q_D(HbSearchPanel);
    d->mLineEdit->setText(text);
}


