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

#ifndef HBLABEL_H
#define HBLABEL_H

#include <hbwidget.h>

class HbLabelPrivate;
class HbStyleOptionLabel;
QT_BEGIN_NAMESPACE
class QGraphicsItem;
QT_END_NAMESPACE

class HB_WIDGETS_EXPORT HbLabel: public HbWidget
{
    Q_OBJECT

    Q_PROPERTY(QString plainText READ plainText WRITE setPlainText)
    Q_PROPERTY(QString html READ html WRITE setHtml)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(Qt::TextElideMode elideMode READ elideMode WRITE setElideMode)
    Q_PROPERTY(Hb::TextWrapping textWrapping READ textWrapping WRITE setTextWrapping)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
    Q_PROPERTY(HbIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(Qt::AspectRatioMode aspectRatioMode READ aspectRatioMode WRITE setAspectRatioMode)

public:

    explicit HbLabel (QGraphicsItem *parent = 0);
    explicit HbLabel (const QString &displayText, QGraphicsItem *parent = 0);
    virtual ~HbLabel ();

    QString plainText() const;
    QString html() const;

    void setElideMode(Qt::TextElideMode elideMode);
    Qt::TextElideMode elideMode() const;

    void setTextWrapping(Hb::TextWrapping mode);
    Hb::TextWrapping textWrapping() const;

    void setIcon(const HbIcon &icon);
    HbIcon icon() const;

    void setAspectRatioMode(Qt::AspectRatioMode mode);
    Qt::AspectRatioMode aspectRatioMode() const;

    void setAlignment(Qt::Alignment alignment);
    Qt::Alignment alignment() const;

    void setTextColor( const QColor &textColor );
    QColor textColor() const;

    bool isEmpty() const;

    virtual QGraphicsItem *primitive(HbStyle::Primitive primitive) const;

    enum {Type = Hb::ItemType_Label};
    int type() const;

public slots:
    void setPlainText(const QString &text);
    void setHtml(const QString &text);
    void setNumber(int number);
    void setNumber(qreal number);
    virtual void updatePrimitives();
    void clear();

protected:
    HbLabel(HbLabelPrivate &dd, QGraphicsItem *parent);

    void initStyleOption(HbStyleOptionLabel *option) const;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbLabel)
    Q_DISABLE_COPY(HbLabel)
};

#endif // HBLABEL_H
