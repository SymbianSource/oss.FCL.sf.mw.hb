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

#ifndef HBWIDGETBASE_H
#define HBWIDGETBASE_H

#include <hbglobal.h>
#include <hbnamespace.h>
#include <hbfontspec.h>

#include <QGraphicsItem>
#include <QGraphicsWidget>

class HbStyleOption;
class HbWidgetBasePrivate;

class HB_CORE_EXPORT HbWidgetBase: public QGraphicsWidget
{
    Q_OBJECT
    
    Q_PROPERTY(HbFontSpec fontSpec READ fontSpec WRITE setFontSpec)

public:
    explicit HbWidgetBase(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~HbWidgetBase();
    
    bool event(QEvent *e);

    HbFontSpec fontSpec() const;
    void setFontSpec(const HbFontSpec &fontSpec);
    HbFontSpec effectiveFontSpec() const;

    void setAttribute(Qt::WidgetAttribute att, bool on = true);
    bool testAttribute(Qt::WidgetAttribute att) const;

    void setAttribute(Hb::WidgetAttribute att, bool on = true);
    bool testAttribute(Hb::WidgetAttribute att) const;

protected:
    HbWidgetBasePrivate * const d_ptr;
    HbWidgetBase(HbWidgetBasePrivate &dd, QGraphicsItem *parent, Qt::WindowFlags wFlags = 0);

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void initStyleOption(HbStyleOption *option) const;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbWidgetBase)
    Q_DISABLE_COPY(HbWidgetBase)
};

#endif // HBWIDGETBASE_H
