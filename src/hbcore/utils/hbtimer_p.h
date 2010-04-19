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

#ifndef HBTIMER_P_H
#define HBTIMER_P_H

#include <hbglobal.h>
#include <QObject>

class HbTimerAnim;

class HB_AUTOTEST_EXPORT HbTimerEntry // do not inherit from anything, must not be a QObject
{
public:
    HbTimerEntry(int interval);
    virtual ~HbTimerEntry();

    int interval() const;

    virtual void timerFired() = 0;

protected:
    int mInterval;
    HbTimerAnim *mAnim;
    bool mDeleteWhenFinishedNormally;
    friend class HbTimerAnim;
    friend class HbTimer;
};

class HB_AUTOTEST_EXPORT HbTimerSignalEntry : public QObject, public HbTimerEntry
{
    Q_OBJECT
    
public:
    HbTimerSignalEntry(int interval);

signals:
    void timeout();

protected:
    void timerFired();
};

class HB_AUTOTEST_EXPORT HbTimer : public QObject
{
public:
    static HbTimer *instance();
    void registerEntry(HbTimerEntry *entry, bool deleteWhenFinishedNormally = false);
    bool unregisterEntry(HbTimerEntry *entry);
    HbTimerSignalEntry *addTimeout(int interval, QObject *receiver, const char *slot);

private:
    HbTimer(QObject *parent);
    QList<HbTimerEntry *> entryList;
};

#endif // HBTIMER_P_H
