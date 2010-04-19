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

#ifndef HBRICHTEXTITEM_H
#define HBRICHTEXTITEM_H

#include <hbnamespace.h>
#include <hbwidgetbase.h>

class QFont;
class HbRichTextItemPrivate;

class HB_CORE_EXPORT HbRichTextItem: public HbWidgetBase
{
    Q_OBJECT

public:
    explicit HbRichTextItem(QGraphicsItem *parent = 0);
    explicit HbRichTextItem(const QString &html, QGraphicsItem *parent = 0);
    virtual ~HbRichTextItem();

    void setText(const QString &text);
    QString text() const;

    QColor textDefaultColor() const;
    void setTextDefaultColor(const QColor &color );
    
    void setAlignment(Qt::Alignment);
    Qt::Alignment alignment() const;

    void setTextWrapping(Hb::TextWrapping mode);
    Hb::TextWrapping textWrapping() const;

    void setTextVisible(bool isVisible=true);
    bool isTextVisible() const;

    void setTextClip(bool cliping=true);
    bool isTextClip() const;

    enum { Type = Hb::ItemType_RichTextItem };
    int type() const { return Type; }

    void setGeometry( const QRectF & rect );
    QRectF boundingRect () const;

protected:
    HbRichTextItem(HbRichTextItemPrivate &dd, QGraphicsItem *parent );

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
    void resizeEvent ( QGraphicsSceneResizeEvent * event );
    void changeEvent( QEvent *event) ;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbRichTextItem)
};

#endif // HBRICHTEXTITEM_H
