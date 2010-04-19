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

#ifndef HBLINEEDIT_P_H
#define HBLINEEDIT_P_H

#include "hbglobal.h"
#include "hbabstractedit_p.h"
#include "hblineedit.h"

class QTextDocument;

class HbLineEditPrivate: public HbAbstractEditPrivate {
    Q_DECLARE_PUBLIC(HbLineEdit)

public:
    HbLineEditPrivate ();
    virtual ~HbLineEditPrivate ();

    void init();
    void updatePaletteFromTheme();

    bool forwardKeyEvent (QKeyEvent *event);
    void updateEditingSize();
    bool canCopy() const;
    bool isCursorVisible() const;
    void prepDocForPaste();
    void setCapitalization ( QFont::Capitalization caps );
    QFont::Capitalization capitalization() const;
    bool isPasswordMode() const;
    void setEchoMode(HbLineEdit::EchoMode echoMode);
    QString passwordString(const QString &text) const;
    QString echoString(const QString &text);
    void updateWrappingMode();
    void validateMaxLength();
    virtual void _q_textChanged();
    virtual void _q_textChange(int position, int charsRemoved,int charsAdded);

    int maxLength;
    bool expandable;
    int minimumRows;
    int maximumRows;
    HbLineEdit::EchoMode echoMode;
    QString passwordText;
    bool clearOnEdit;
    bool emitTextChanged;
    QTextOption::WrapMode defaultWrapMode;
};

#endif // HBLINEEDIT_P_H
