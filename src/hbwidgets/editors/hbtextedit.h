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

#ifndef HBTEXTEDIT_H
#define HBTEXTEDIT_H

#include <hbabstractedit.h>
#include <hbglobal.h>
#include <hbnamespace.h>

class HbTextEditPrivate;

class HB_WIDGETS_EXPORT HbTextEdit: public HbAbstractEdit
{
    Q_OBJECT

    Q_PROPERTY(QString plainText READ toPlainText WRITE setPlainText USER true)
    Q_PROPERTY(bool lined READ isLined WRITE setLined)

public:

    explicit HbTextEdit (QGraphicsItem *parent = 0);
    explicit HbTextEdit (const QString &text, QGraphicsItem *parent = 0);
    virtual ~HbTextEdit ();

    enum {
        Type = Hb::ItemType_TextEdit
    };
    int type () const;

    void setDocument(QTextDocument *document);
    QTextDocument *document() const;

    void setTextCursor(const QTextCursor &cursor);
    QTextCursor textCursor() const;

    QString toPlainText() const;
    QString toHtml() const;

    void setLined(bool visible=true);
    bool isLined() const;

public slots:
    void setPlainText(const QString &text);
    void setHtml(const QString &text);

protected:
    HbTextEdit (HbTextEditPrivate &dd, QGraphicsItem *parent);
    void resizeEvent(QGraphicsSceneResizeEvent *event);
    void polish( HbStyleParameters& params );

private:
    Q_DISABLE_COPY(HbTextEdit)
    Q_DECLARE_PRIVATE_D(d_ptr, HbTextEdit)
};

#endif // HBTEXTEDIT_H
