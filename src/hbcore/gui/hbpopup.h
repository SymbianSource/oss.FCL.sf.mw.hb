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

#ifndef HBPOPUP_H
#define HBPOPUP_H

//
//  N O T E
//  -------
//
// This class is to be called HbPopup, the base class of all popups.
// It's merely a popup frame whose stacking order is handled by the
// popup manager. It doesn't have heading/content widgets nor actions.
// This functionality will be provided by the upcoming HbDialog class.
//

#include <hbglobal.h>
#include <hbwidget.h>

class HbPopupPrivate;

class HB_CORE_EXPORT HbPopup : public HbWidget
{
    Q_OBJECT
    Q_ENUMS( DefaultTimeout DismissPolicy FrameType )
    Q_PROPERTY( int timeout READ timeout WRITE setTimeout )
    Q_PROPERTY( bool modal READ isModal WRITE setModal )
    Q_PROPERTY( bool backgroundFaded 
                READ isBackgroundFaded WRITE setBackgroundFaded )
    Q_PROPERTY( DismissPolicy dismissPolicy READ dismissPolicy WRITE setDismissPolicy )
    Q_PROPERTY( FrameType frameType READ frameType WRITE setFrameType )
               
public:

    enum DefaultTimeout
    {
        NoTimeout,
        ConfirmationNoteTimeout,
        StandardTimeout,
        ContextMenuTimeout
    };

    enum DismissPolicy
    {
        NoDismiss   = 0,
        TapInside   = 1,
        TapOutside  = 2,
        TapAnywhere = TapInside | TapOutside
    };

    enum FrameType
    {
        Strong = 0,
        Weak   = 1 
    };
    
    enum Placement
    {
        TopLeftCorner     = Qt::TopLeftCorner,
        TopRightCorner    = Qt::TopRightCorner,
        BottomLeftCorner  = Qt::BottomLeftCorner,
        BottomRightCorner = Qt::BottomRightCorner,
        TopEdgeCenter,
        RightEdgeCenter,
        BottomEdgeCenter,
        LeftEdgeCenter,
        Center
    };

    explicit HbPopup(QGraphicsItem *parent = 0);
    virtual ~HbPopup();

    int timeout() const;
    void setTimeout(int timeout);
    void setTimeout(HbPopup::DefaultTimeout timeout);

    bool isModal() const;
    void setModal(bool enabled);

    void setBackgroundFaded(bool fadeBackground);
    bool isBackgroundFaded() const;

    DismissPolicy dismissPolicy() const;
    void setDismissPolicy(HbPopup::DismissPolicy dismissPolicy);

    FrameType frameType() const;
    void setFrameType(HbPopup::FrameType frameType);

    enum { Type = Hb::ItemType_Popup };
    int type() const { return Type; }

    void setPreferredPos(const QPointF& position,
                          HbPopup::Placement placement = HbPopup::TopLeftCorner);

public slots:
    void open( QObject *receiver = 0, const char *member = 0);
    void exec();
    void handlePopupPos();

signals:
    void aboutToShow();
    void aboutToHide();
    void aboutToClose();

protected:
    HbPopup(HbPopupPrivate &dd, QGraphicsItem *parent);
    QVariant itemChange(GraphicsItemChange change, const QVariant & value);

    void mousePressEvent(QGraphicsSceneMouseEvent *event );
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event );
    void keyPressEvent(QKeyEvent *event);

    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    void resizeEvent(QGraphicsSceneResizeEvent *event);
    void closeEvent(QCloseEvent *event);
    bool event(QEvent *event);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbPopup)
    Q_DISABLE_COPY(HbPopup)
#ifdef HB_EFFECTS
    Q_PRIVATE_SLOT(d_func(), void _q_delayedHide(HbEffect::EffectStatus status))
    Q_PRIVATE_SLOT(d_func(), void _q_orientationChange(Qt::Orientation orient, bool animate))
#endif // HB_EFFECTS
    Q_PRIVATE_SLOT(d_func(), void _q_timeoutFinished())
};

#endif // HbPopup_H
