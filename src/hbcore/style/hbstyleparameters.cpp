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

#include <hbstyleparameters.h>


class HbStyleParametersPrivate
{
public:
    HbStyleParametersPrivate();

    QList<QString> params;
    QList<QVariant> values;
};
HbStyleParametersPrivate::HbStyleParametersPrivate()
{
}


/*!
    @stable
    @hbcore
    \class HbStyleParameters
    \brief HbStyleParameters is a class that can be used to fetch style parameters with
    HbStyle::polish.
        
    \sa HbStyle::polish.
*/

/*!
    Default constructor.
*/
HbStyleParameters::HbStyleParameters() :
    d_ptr(new HbStyleParametersPrivate)
{
}

/*!
    Destructor.
*/
HbStyleParameters::~HbStyleParameters()
{
    delete d_ptr;
}

/*!
    Returns the number of the parameters.

    \return number of parameters
*/
int HbStyleParameters::count()
{
    return d_ptr->params.count();
}

/*!
    indexOf

    Returns the index of the parameter with given name.

    \param param name of the parameter
    \return index of given parameter
*/
int HbStyleParameters::indexOf( const QString& param )
{
    return d_ptr->params.indexOf(param);
}

/*!
    Returns the name of the parameter at given index.

    \param index position of the parameter
    \return name of the parameter
*/
QString HbStyleParameters::parameter( int index )
{
    return d_ptr->params.at(index);
}

/*!
    Returns the value of the parameter at given index.

    \param index position of the parameter
    \return value of the parameter
*/
QVariant HbStyleParameters::value( int index )
{
    return d_ptr->values.at(index);
}

/*!
    Returns the value of the parameter with given name.

    This is a convenience method. Equals to calling value(indexOf(param)).

    \param param name of the parameter
    \return value of the parameter
*/
QVariant HbStyleParameters::value( const QString &param )
{
    int index = indexOf(param);
    if ( index >= 0 && index < d_ptr->values.count() )
        return value(index);

    return QVariant();
}

/*!
    Sets the value of the parameter at given index.

    \param index position of the parameter
    \param value value to be set
*/
void HbStyleParameters::setValue( int index, const QVariant &value )
{
    d_ptr->values.replace(index, value);
}

/*!
    Sets the value of the parameter with given name.

    This is a convenience method. Equals to calling setValue(indexOf(param), value).

    \param param name of the parameter
    \param value value to be set
*/
void HbStyleParameters::setValue( const QString &param, const QVariant &value )
{
    setValue(indexOf(param), value);
}

/*!
    Add a new parameter.

    \param param name of the parameter to be added
    \param value default value
*/
void HbStyleParameters::addParameter( const QString &param, const QVariant &value )
{
    d_ptr->params.append(param);
    d_ptr->values.append(value);
}

/*!
    Remove a parameter from given index

    \param index position of the parameter
*/
void HbStyleParameters::removeAt( int index )
{
    d_ptr->params.removeAt(index);
    d_ptr->values.removeAt(index);
}

/*!
    \internal
*/
QList<QString> &HbStyleParameters::params()
{
    return d_ptr->params;
}

/*!
    \internal
*/
QList<QVariant> &HbStyleParameters::values()
{
    return d_ptr->values;
}

