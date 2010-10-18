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

#ifndef HBTOOLBAREXTENSION_H
#define HBTOOLBAREXTENSION_H

#include <hbglobal.h>
#include <hbnamespace.h>
#include <hbdialog.h>

class HbToolBarExtensionPrivate;
class HbAction;

class HB_CORE_EXPORT HbToolBarExtension : public HbDialog
{
    Q_OBJECT

public:
    explicit HbToolBarExtension ( QGraphicsItem *parent = 0 );
    virtual ~HbToolBarExtension ();

    using HbWidget::addAction;
    HbAction *addAction ( const QString &text );
    HbAction *addAction ( const HbIcon &icon, const QString &text );
    HbAction *addAction ( const QString &text, const QObject *receiver, const char *member );
    HbAction *addAction ( const HbIcon &icon, const QString &text,
                          const QObject *receiver, const char *member );
    HbAction *extensionAction () const;

    enum {
        Type = Hb::ItemType_ToolBarExtension
    };
    int type () const { return Type; }

protected:
    HbToolBarExtension ( HbToolBarExtensionPrivate &dd, QGraphicsItem *parent );
    bool event ( QEvent *event );
    void polish( HbStyleParameters &params );
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive);

private:
    Q_DECLARE_PRIVATE_D( d_ptr, HbToolBarExtension )
    Q_DISABLE_COPY( HbToolBarExtension )

    Q_PRIVATE_SLOT( d_func(), void _q_orientationChanged() )
    Q_PRIVATE_SLOT( d_func(), void _q_toolbarOrientationChanged() )
    Q_PRIVATE_SLOT( d_func(), void _q_animateButtonClicked() )
};

#endif // HBTOOLBAREXTENSION_H
