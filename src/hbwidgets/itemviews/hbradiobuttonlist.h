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

#ifndef HBRADIOBUTTONLIST_H
#define HBRADIOBUTTONLIST_H

#include <hbglobal.h>
#include <hbnamespace.h>
#include <hblistview.h>

class HbRadioButtonListPrivate;

class HB_WIDGETS_EXPORT HbRadioButtonList: public HbListView
{    
    Q_OBJECT

    Q_ENUMS(PreviewMode)
    Q_PROPERTY(QStringList items READ items WRITE setItems)
    Q_PROPERTY(PreviewMode previewMode READ previewMode WRITE setPreviewMode)
    Q_PROPERTY(int selected READ selected WRITE setSelected NOTIFY itemSelected USER true)
  
public:

    enum PreviewMode
    {
        NoPreview,
        Preview
    };

    explicit HbRadioButtonList(QGraphicsItem *parent = 0);
    
    explicit HbRadioButtonList(
        const QStringList &list, 
        int selected = -1, 
        PreviewMode previewMode = NoPreview, 
        QGraphicsItem *parent = 0);

    ~HbRadioButtonList();

    void setItems(const QStringList &list);
    QStringList items() const;

    void setSelected(int index);
    int selected() const;

    void setPreviewMode(PreviewMode previewMode);
    PreviewMode previewMode() const;

    enum { Type = Hb::ItemType_RadioButtonList };
    int type() const;

public slots:
    virtual void reset();

signals:
    void itemSelected(int index);
    void startPreview(int index);

protected:
    HbRadioButtonList(HbRadioButtonListPrivate &dd, HbAbstractItemContainer *container, QGraphicsItem *parent);

    void emitActivated(const QModelIndex &modelIndex);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    QItemSelectionModel::SelectionFlags selectionCommand(
        const HbAbstractViewItem *item, 
        const QEvent *event);
    bool event(QEvent *e);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;

protected slots:
    virtual void rowsInserted(const QModelIndex &parent, int start, int end);
    virtual void rowsRemoved(const QModelIndex &parent, int start, int end);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbRadioButtonList)
    Q_DISABLE_COPY(HbRadioButtonList)
    Q_PRIVATE_SLOT(d_func(), void _q_itemActivated(const QModelIndex&))
    Q_PRIVATE_SLOT(d_func(), void _q_itemActivationTimerExpired())
    Q_PRIVATE_SLOT(d_func(), void _q_itemPreviewTimerExpired()) 
};

#endif // HBRADIOBUTTONLIST_H


