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


#ifndef HBINPUT_DIALOG_H
#define HBINPUT_DIALOG_H

#include <hbdialog.h>
#include <hblineedit.h>

class HbInputDialogPrivate;
class HbStyleOptionInputDialog;
class HbValidator;

class HB_WIDGETS_EXPORT HbInputDialog : public HbDialog
{
    Q_OBJECT

public:
    enum InputMode {
        TextInput,
        IntInput,
        RealInput,
        IpInput
    };

    explicit HbInputDialog(QGraphicsItem *parent= 0);
    ~HbInputDialog();

    void setInputMode(InputMode mode, int row = 0);
    InputMode inputMode(int row = 0) const;

    void setPromptText(const QString &text, int row = 0);
    QString promptText(int row = 0) const;

    void setValue(const QVariant &value, int row = 0);
    QVariant value(int row = 0) const;

    void setAdditionalRowVisible(bool visible = false);
    bool isAdditionalRowVisible();

    void setValidator(HbValidator *validator, int row=0);
    HbValidator* validator(int row=0) const;

    HbLineEdit* lineEdit(int row=0) const;

    void setEchoMode(HbLineEdit::EchoMode echoMode=HbLineEdit::Normal,int row=0);
    HbLineEdit::EchoMode echoMode(int row=0) const;


public:
    static void getText(const QString &heading
            ,QObject *receiver = 0
            ,const char *member = 0
            ,const QString &text = QString()
            ,QGraphicsScene *scene = 0
            ,QGraphicsItem *parent = 0);
    static void getInteger(const QString &label
            ,QObject *receiver = 0
            ,const char *member = 0
            ,int value = 0
            ,QGraphicsScene *scene = 0
            ,QGraphicsItem *parent = 0);
    static void getDouble(const QString &label
            ,QObject *receiver = 0
            ,const char *member = 0
            ,double value = 0
            , QGraphicsScene *scene = 0
            , QGraphicsItem *parent = 0);

    static void getIp(const QString &label
            ,QObject *receiver = 0
            ,const char *member = 0
            , const QString &ipaddress = QString()
            , QGraphicsScene *scene = 0
            , QGraphicsItem *parent = 0);

    static void queryText(const QString &promptText
            ,QObject *receiver
            ,const char *member
            ,const QString &defaultText = QString()
            ,QGraphicsScene *scene = 0
            ,QGraphicsItem *parent = 0);
    static void queryInt(const QString &promptText
            ,QObject *receiver
            ,const char *member
            ,int defaultInt = 0
            ,QGraphicsScene *scene = 0
            ,QGraphicsItem *parent = 0);
    static void queryDouble(const QString &promptText
            ,QObject *receiver
            ,const char *member
            ,double defaultDouble = 0
            , QGraphicsScene *scene = 0
            , QGraphicsItem *parent = 0);
    static void queryIp(const QString &promptText
            ,QObject *receiver
            ,const char *member
            , const QString &defaultIp = QString()
            , QGraphicsScene *scene = 0
            , QGraphicsItem *parent = 0);

    QGraphicsItem* primitive(HbStyle::Primitive primitive) const;

    enum { Type = Hb::ItemType_InputDialog };
    int type() const { return Type; }

public slots:
    void updatePrimitives();
    void done(int code);

signals:
    void intValueSelected(int value);
    void doubleValueSelected(double value);
    void textValueSelected(QString value);

protected:
    HbInputDialog(HbDialogPrivate &dd, QGraphicsItem *parent);
    void initStyleOption(HbStyleOptionInputDialog *option) const;

private:
    Q_DISABLE_COPY(HbInputDialog)
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputDialog)
    Q_PRIVATE_SLOT(d_func(), void _q_notesOrientationChanged(Qt::Orientation))
    Q_PRIVATE_SLOT(d_func(), void textChange(const QString))
};

#endif //HBINPUT_DIALOG_H

