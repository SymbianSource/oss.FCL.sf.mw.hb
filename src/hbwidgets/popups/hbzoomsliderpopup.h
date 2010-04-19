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

#ifndef HBZOOM_SLIDERPOPUP_H
#define HBZOOM_SLIDERPOPUP_H

#include <hbsliderpopup.h>

class HbZoomSliderPopupPrivate;

class HB_WIDGETS_EXPORT HbZoomSliderPopup : public HbSliderPopup
{
    Q_OBJECT
    Q_PROPERTY(int defaultZoomLevel READ defaultZoomLevel WRITE setDefaultZoomLevel)

 public:
    explicit HbZoomSliderPopup();
    virtual ~HbZoomSliderPopup();

    int defaultZoomLevel() const;
    void setDefaultZoomLevel(int value);
    enum { Type = Hb::ItemType_ZoomPopup };
    int type() const { return Type; }

protected:
    HbZoomSliderPopup(HbZoomSliderPopupPrivate &dd, QGraphicsItem *parent);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbZoomSliderPopup)
    Q_DISABLE_COPY(HbZoomSliderPopup)
};


#endif // HBSLIDER_H
