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

#ifndef HBPOPUPMANAGER_P_P_H
#define HBPOPUPMANAGER_P_P_H

#include "hbdialog.h"
#include "hbpopup.h"
#include "hbdevicefadecontrol_p.h"
#include <QGraphicsItem>
#include <QHash>
#include <QList>
#include <QPointer>

class HbDialog;
class HbGraphicsScene;
class HbPopupLayoutManager;

QT_FORWARD_DECLARE_CLASS(QEvent)
QT_FORWARD_DECLARE_CLASS(QGraphicsSceneMouseEvent)

class HbPopupManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit HbPopupManagerPrivate( HbGraphicsScene *scene, QObject * parent = 0 );
    virtual ~HbPopupManagerPrivate();

public:
    void eventHook( QEvent *event);
    void showPopup( HbPopup *popup );
    void hidePopup( HbPopup *popup );
    void addPopup( HbPopup *popup );
    void removePopup( HbPopup *popup );

private:

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent);
    bool eventBlockItem(QGraphicsItem *item);
    static qreal popupZValueForPriority(int priority, int popupCountWaterMark);
    qreal nextPopupZValueForPriority(int priority);
    void updateHash(int priority, qreal zValueOfRemovedPopup);
    void updateFading(bool unfadeFirst = false);
    void setGeometryForPopup(HbPopup *popup);

private slots:
    void deviceFadeChange( bool fadeOff );

private:

    class GraphicsItem: public QGraphicsItem
    {
        friend class HbPopupManagerPrivate;
    };

    struct PopupCountRegister
    {
        PopupCountRegister():waterMark(0), count(0){}
        int waterMark;
        int count;
    };

    QList<HbPopup *> popupList;
    HbPopup *topLevelFocusablePopup;
    QGraphicsItem *initialFocusedItem;
    QHash<int, PopupCountRegister> popupCountRegisters;
    HbDeviceFadeControl deviceFadeControl;
    HbGraphicsScene *scene;
    QMap<HbPopup *,HbPopupLayoutManager*> parentItems; 

#ifdef HB_CSS_INSPECTOR
    friend class CssInspectorModel;
    friend class CssInspectorModelItem;
#endif
};

class HbPopupLayoutSpacer: public HbWidgetBase
{
public:
    HbPopupLayoutSpacer( QGraphicsItem *parent = 0 );
    virtual ~HbPopupLayoutSpacer() {}
protected:
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
};

class HbPopupLayoutProxy : public HbWidgetBase 
{
public:
    HbPopupLayoutProxy( HbPopup *popup, QGraphicsItem *parent );
    virtual ~HbPopupLayoutProxy() {}
    void activateParentLayout();
    QPointer<HbPopup> mPopup;
    QPointF mPreferredPos;
    bool mPreferredPosSet;
    HbPopup::Placement mPlacement;

protected:
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
    void setGeometry(const QRectF &rect);
    bool event( QEvent *e );
    bool eventFilter(QObject *obj, QEvent *event);
};

class HbPopupLayoutManager : public HbWidget
{
    Q_OBJECT
public:
    HbPopupLayoutManager( HbPopup *popup, QGraphicsScene *scene );
    virtual ~HbPopupLayoutManager() {}
public slots:
      void  orientationChanged(const QRectF& rect);

protected:
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;
    void changeEvent(QEvent *event);
};

#endif // HBPOPUPMANAGER_P_P_H

