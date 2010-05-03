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
#include "hbinputfilter.h"

/*!
@alpha
@hbcore
\class HbInputFilter
\brief Abstract base class for all editor filters.

An input filter knows how to answer one question: "Is given character legal?". It does not do postion based
validation in same way as QValidator but is much simpler position independent filter.

Input filter can be asigned to an editor through editor interface. Input method implementations should always
honor installed filter and never commit characters that do not pass active filter (all the methods in
HbInputFocusObject API do that automatically).

\sa HbEditorInterface
*/

/*!
\fn virtual bool filter(QChar character) = 0

Returns true if given character is valid.
*/

/*!
Performs filtering operation for string "in". Filtered string "out" is a copy of "in" without
invalid characters.
*/
void HbInputFilter::filterString(const QString& in, QString& out)
{
    for (int i = 0; i < in.length(); i++) {
        if (filter(in[i])) {
            out.append(in[i]);
        }
    }
}

// End of file
