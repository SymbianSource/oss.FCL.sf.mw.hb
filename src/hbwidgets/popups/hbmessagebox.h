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

#ifndef HBMESSAGEBOX_H
#define HBMESSAGEBOX_H

#include <hbglobal.h>
#include <hbdialog.h>
#include <hbicon.h>

class HbMessageBoxPrivate;
class HbStyleOptionMessageBox;


class HB_WIDGETS_EXPORT HbMessageBox : public HbDialog
{
    Q_OBJECT
    Q_FLAGS(StandardButton StandardButtons)
    Q_ENUMS(MessageBoxType StandardButton )
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( HbIcon icon READ icon WRITE setIcon )
    Q_PROPERTY( bool iconVisible READ iconVisible WRITE setIconVisible )
    Q_PROPERTY(StandardButtons standardButtons READ standardButtons WRITE setStandardButtons)
public:
    enum MessageBoxType {
        MessageTypeNone,
        MessageTypeInformation,
        MessageTypeQuestion,
        MessageTypeWarning
    };

    enum StandardButton {
        NoButton           = 0x00000000,
        Ok                 = 0x00000400,
        Save               = 0x00000800,
        Open               = 0x00001000,
        Yes                = 0x00002000,
        Continue           = 0x00004000,
        Delete             = 0x00008000,
        No                 = 0x00010000,
        Retry              = 0x00020000,
        Close              = 0x00040000,
        Cancel             = 0x00080000,
        Help               = 0x00100000,
        Apply              = 0x00200000,
        Reset              = 0x00400000
    };    

    Q_DECLARE_FLAGS(StandardButtons, StandardButton)

    explicit HbMessageBox(MessageBoxType type=MessageTypeNone,QGraphicsItem *parent = 0);
    explicit HbMessageBox(const QString &text,MessageBoxType type =MessageTypeNone, QGraphicsItem *parent = 0);
    virtual ~HbMessageBox();

    void setText(const QString &text);
    QString text() const;

    void setIcon(const HbIcon &icon);
    HbIcon icon() const;

    void setIconVisible(bool visible);
    bool  iconVisible() const;

    void setStandardButtons(StandardButtons buttons);
    StandardButtons standardButtons() const;
    
    QGraphicsItem *primitive(HbStyle::Primitive primitive) const;
    enum { Type = Hb::ItemType_MessageBox };
    int type() const { return Type; }

public:
    static void question(const QString &questionText,
                            QObject *receiver = 0,
                            const char *member = 0,
                            const QString &primaryButtonText = tr("Yes"),
                            const QString &secondaryButtonText = tr("No"),
                            QGraphicsWidget *headWidget = 0,
                            QGraphicsScene *scene = 0,
                            QGraphicsItem *parent = 0 );
	 
    static void information(const QString &informationText,
                            QObject *receiver = 0,
                            const char *member = 0,
                            QGraphicsWidget *headWidget = 0,
                            QGraphicsScene *scene = 0,
                            QGraphicsItem *parent = 0 );

    static void warning(const QString &warningText,
                            QObject *receiver = 0,
                            const char *member = 0,
                            QGraphicsWidget *headWidget = 0,
                            QGraphicsScene *scene = 0,
                            QGraphicsItem *parent = 0 );

    static void question(const QString &questionText,
                            QObject *receiver ,
                            const char *member,
                            HbMessageBox::StandardButtons buttons,
                            QGraphicsWidget *headingWidget = 0,
                            QGraphicsScene *scene = 0,
                            QGraphicsItem *parent = 0);
	 
    static void information(const QString &informationText,
                            QObject *receiver,
                            const char *member,
                            HbMessageBox::StandardButtons buttons ,
                            QGraphicsWidget *headingWidget=0,
                            QGraphicsScene *scene = 0,
                            QGraphicsItem *parent = 0 );

    static void warning(const QString &warningText,
                            QObject *receiver ,
                            const char *member,
                            HbMessageBox::StandardButtons buttons,
                            QGraphicsWidget *headingWidget = 0,
                            QGraphicsScene *scene = 0,
                            QGraphicsItem *parent = 0 );

public slots:
    void updatePrimitives();

protected:
    HbMessageBox(HbMessageBoxPrivate &dd, QGraphicsItem *parent);
    void initStyleOption(HbStyleOptionMessageBox *option) const;

private:
    Q_PRIVATE_SLOT(d_func(), void _q_buttonClicked())
    Q_DECLARE_PRIVATE_D(d_ptr, HbMessageBox)
    Q_DISABLE_COPY(HbMessageBox)    
	
};
Q_DECLARE_OPERATORS_FOR_FLAGS(HbMessageBox::StandardButtons)

#endif // HB_MESSAGEBOX_H

