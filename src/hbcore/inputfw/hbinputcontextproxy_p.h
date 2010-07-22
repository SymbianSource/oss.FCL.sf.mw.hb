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

#ifndef HB_INPUT_CONTEXT_PROXY_H
#define HB_INPUT_CONTEXT_PROXY_H

#include <QList>
#include <QInputContext>
#include <QPointer>

#include <hbinputmethod.h>

class HbInputMethod;
class QWidget;

class HB_CORE_EXPORT HbInputContextProxy : public QInputContext
{
    Q_OBJECT

public:
    explicit HbInputContextProxy(HbInputMethod *target);
    ~HbInputContextProxy();

public: // From QInputContext
    QList<QAction *> actions();
    bool filterEvent(const QEvent *event);
    QFont font() const;
    QString identifierName();
    bool isComposing() const;
    QString language();
    void mouseHandler(int x, QMouseEvent *event);
    void reset();
    void sendEvent(const QInputMethodEvent &event);
    void update();
    void widgetDestroyed(QWidget *widget);
    void setFocusWidget(QWidget *widget);

private:
    inline bool hasAlreadyInputFrameworkFocus(HbInputMethod *activeMethod, QObject *editorWidget) const;
    void setInputFrameworkFocus(QObject *widget);

private:
    QPointer<HbInputMethod> mTarget;
};

#endif // HB_INPUT_CONTEXT_PROXY_H

// End of file
