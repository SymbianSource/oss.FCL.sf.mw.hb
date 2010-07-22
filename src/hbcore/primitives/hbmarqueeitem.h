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

#ifndef HBMARQUEEITEM_H
#define HBMARQUEEITEM_H


#include <hbnamespace.h>
#include <hbwidgetbase.h>

class HbMarqueeItemPrivate;

class HB_CORE_EXPORT HbMarqueeItem: public HbWidgetBase
{
    Q_OBJECT
public:
    explicit HbMarqueeItem(QGraphicsItem *parent = 0);
    explicit HbMarqueeItem(const QString &text, QGraphicsItem *parent = 0);
    virtual ~HbMarqueeItem();

    QString text() const;
    void setTextColor (const QColor &color);
    QColor textColor() const;
    bool isAnimating() const;
    int loopCount() const;
    void setLoopCount(int count);

    enum { Type = Hb::ItemType_MarqueeItem };
    int type() const { return Type; }

public slots:
    void setText(const QString &text);
    void startAnimation();
    void stopAnimation();

signals:
    void animationStarted();
    void animationStopped();

protected:
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
    void changeEvent(QEvent *event);
    void resizeEvent ( QGraphicsSceneResizeEvent * event );
    QVariant itemChange ( GraphicsItemChange change, const QVariant & value );
    bool event(QEvent *e);

    HbMarqueeItem(HbMarqueeItemPrivate &dd, QGraphicsItem *parent );

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbMarqueeItem)
    Q_PRIVATE_SLOT(d_func(), void _q_stateChanged())


};

#endif // HBMARQUEEITEM_H
