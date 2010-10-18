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

#ifndef HBVOLUMESLIDERPOPUP_H
#define HBVOLUMESLIDERPOPUP_H

#include <hbsliderpopup.h>

class HbVolumeSliderPopupPrivate;

class HB_WIDGETS_EXPORT HbVolumeSliderPopup : public HbSliderPopup
{
    Q_OBJECT
    Q_PROPERTY(bool currentValueVisible READ isCurrentValueVisible 
        WRITE setCurrentValueVisible USER true)
    Q_PROPERTY(bool muted READ isMuted WRITE setMuted)    

public:
    explicit HbVolumeSliderPopup();
    virtual ~HbVolumeSliderPopup();

    enum { Type = Hb::ItemType_VolumePopup };
    int type() const { return Type; }

    void setCurrentValueVisible(bool visible);
    bool isCurrentValueVisible()const;
    void setMuted( bool mute );
    bool isMuted( ) const;

signals:
    void iconToggled(bool checked); 

protected:
    HbVolumeSliderPopup(HbVolumeSliderPopupPrivate &dd, QGraphicsItem *parent);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbVolumeSliderPopup)
    Q_DISABLE_COPY(HbVolumeSliderPopup)
  
};

#endif // HBVOLUMESLIDERPOPUP_H
