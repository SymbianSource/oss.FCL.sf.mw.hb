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

#include "hbtextutils_p.h"
#include <QChar>

// Based on \src\common\generic\graphics\gdi\sgdi\BidiCopy.cpp:
// Copyright (C) 2002 Symbian Ltd. All rights reserved.

/*
    \class HbTextUtils
    This class contains internal text utility methods.
    
    \proto
    \internal
*/    

/*
Find out if the implicit directionality of a piece of text is right to
left.

\param text Text to be examined, terminated by null character.
\param length Length of the text.
\param found If non-null, returns true if the directionality could be
determined from \a text. 
\return \c true if the directionality is right to left. If the directionality
cannot be determined solely from this text, \c false is returned. This is the 
implicit directionality of text that is composed entirely of neutrals.
*/
bool HbTextUtils::ImplicitDirectionalityIsRightToLeft(
    const ushort* text, int length, bool* found)
{
    if (found) {
        *found = true;
    }
    
    qint32 lastUtf16 = 0;
    const ushort* end = text + length;

    while (text != end && *text)
        {
        qint32 code = *text++;
        if ((code & 0xFFFFFC00) == 0xD800 && 
            (lastUtf16 & 0xFFFFFC00) == 0xDC00)
            {
            code = (code << 10) + (lastUtf16 & 0x3FF)
                + (0x10000 - 0xD800*0x400);
            }
        lastUtf16 = code;
        QChar c(code);
        switch(c.direction())
            {
        case QChar::DirL:
            return false;
        case QChar::DirR:
        case QChar::DirAL:
            return true;
        default:
            break;
            }
        }
    if (found) {
        *found = false;
    }
    return false;
}
