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

#ifndef HBTOOLBUTTON_P_H
#define HBTOOLBUTTON_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <hbicon.h>
#include <hbabstractbutton_p.h>
#include <QPointer>
#include <hbstyleframeprimitivedata.h>
#include <hbstyleiconprimitivedata.h>
#include <hbstyletextprimitivedata.h>

class HbToolButton;
class HbStyle;
class HbTextItem;

class HbToolButtonPrivate : public HbAbstractButtonPrivate
{
    Q_DECLARE_PUBLIC(HbToolButton)

public:
    HbToolButtonPrivate();
    virtual ~HbToolButtonPrivate();

    enum ToolButtonPosition
    {
        TB_None,
        TB_OnlyOne,
        TB_Beginning,
        TB_Middle,
        TB_End
    };

    void createPrimitives();
    void setOrientation( Qt::Orientation orientation );
    void setToolBarPosition( ToolButtonPosition position );
    void setBackgroundVisible( bool visible );
    void setExtensionBackgroundVisible( bool visible );
    void setLayoutProperty(const char *name, bool value);

    bool useTransparentGraphics() const;
    bool isToolBarExtension() const;

    void framePrimitiveData(HbStyleFramePrimitiveData *data);
    void iconPrimitiveData(HbStyleIconPrimitiveData *data);
    void textPrimitiveData(HbStyleTextPrimitiveData *data);

    QSizeF getMinimumSize();

    QPointer<QAction> action;

    QGraphicsObject *textItem;
    QGraphicsObject *iconItem;
    QGraphicsObject *frameItem;

    HbIcon customBackground;
    bool backgroundVisible;
    ToolButtonPosition toolBarPosition;
    Qt::Orientation orientation;


    bool mDialogToolBar;
    bool toolbarExtensionFrame;

    void _q_actionTriggered();
    void _q_actionChanged();

    void showToolTip();

    QSizeF mButtonSize;

private:
    // Provided for HbToolBar who have to access
    // HbToolButtonPrivate in order to hide the background
    // and/or set the toolbar position of the tool button.
    // NOTE: Still kept as private to track dependencies...
    static HbToolButtonPrivate *d_ptr( HbToolButton *button ) {
        Q_ASSERT(button);
        return button->d_func();
    }
    friend class HbToolBar;
    friend class HbToolBarPrivate; 
    friend class HbToolBarExtensionPrivate;
};

#endif // HBTOOLBUTTON_P_H
