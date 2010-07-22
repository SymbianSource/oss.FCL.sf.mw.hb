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

#include <QPointer>

#include "hbmainwindow.h"
#include "hbwidget.h"

#ifndef HB_INPUT_MAINWINDOW
#define HB_INPUT_MAINWINDOW

class HbProxyWindow;

class HbInputMainWindow : public HbMainWindow
{
    Q_OBJECT
public:
    static HbInputMainWindow *instance();
    void showInputWindow();
    void hideInputWindow();
private:
    HbInputMainWindow();
    virtual ~HbInputMainWindow();

    bool event(QEvent *e);
    bool eventFilter(QObject *obj, QEvent *event);

public slots:
    void saveFocusWidget(QWidget * /*Old*/, QWidget *newFocus);
    void updateRegion(QRegion region);

private:
    QPointer<QWidget> mLastFocusedWidget;
    QRegion mMask;
    bool mSpellQueryLaunched;
    QPointer<HbProxyWindow > mProxyWindow;
};

class HbProxyWindow: public QWidget
{
public:
    HbProxyWindow()
    {
        setGeometry(0,0,0,0);
    }
    void setWindow(QWidget* window)
    {
        this->window = window;
        if (window) {
            window->setParent(this);
        }
    }
    ~HbProxyWindow()
    {
        if (window) {
            window->setParent(0);
        }
    }
private:
    QPointer<QWidget> window;
};

#endif //HB_INPUT_MAINWINDOW

// End of file
