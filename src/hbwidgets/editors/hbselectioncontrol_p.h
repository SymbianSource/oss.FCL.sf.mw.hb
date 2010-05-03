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

#ifndef HBSELECTIONCONTROL_P_H
#define HBSELECTIONCONTROL_P_H

#include "hbwidget.h"

class HbSelectionControlPrivate;
class HbAbstractEdit;
QT_FORWARD_DECLARE_CLASS(QGraphicsSceneMouseEvent)

class HbSelectionControl : public HbWidget
{

    Q_OBJECT

public:

    enum HandleType {
        DummyHandle,
        SelectionStartHandle,
        SelectionEndHandle
    };

    static HbSelectionControl* attachEditor(HbAbstractEdit *edit);
    void detachEditor();
    void hideHandles();
    void showHandles();
    bool event(QEvent *event);

public slots:
    void scrollStarted();
    void scrollFinished();
    void updatePrimitives();

protected:
    
    void timerEvent (QTimerEvent *event);
    void polish( HbStyleParameters& params );
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void gestureEvent(QGestureEvent* event);

private:
    HbSelectionControl();
    Q_DECLARE_PRIVATE_D(d_ptr, HbSelectionControl)
    Q_DISABLE_COPY(HbSelectionControl)
    Q_PRIVATE_SLOT(d_func(), void _q_aboutToChangeView())
};

#endif // HBSELECTIONCONTROL_P_H
