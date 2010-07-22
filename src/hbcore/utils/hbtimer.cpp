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

#include "hbtimer_p.h"
#include <QApplication>

#if QT_VERSION < 0x040600
#include <QtAbstractAnimation>
#define QAbstractAnimation QtAbstractAnimation
#else
#include <QAbstractAnimation>
#endif

// A custom animation. We will use this instead of creating a timer ourselves, in
// order to use the animation fw's QUnifiedTimer under the hood.
class HbTimerAnim : public QAbstractAnimation
{
public:
    HbTimerAnim(HbTimerEntry *entry) : mEntry(entry), mDuration(entry->interval()) { }
    int duration() const {
        return mDuration;
    }
    void updateCurrentTime(int) { }
    void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState) {
        if (oldState != QAbstractAnimation::Stopped && newState == QAbstractAnimation::Stopped && mEntry) {
            mEntry->timerFired();
            mEntry->mAnim = 0; // to prevent confusing unregisterEntry() and double deletion
            HbTimer::instance()->unregisterEntry(mEntry);
            if (mEntry->mDeleteWhenFinishedNormally) {
                delete mEntry;
            }
        }
    }
    HbTimerEntry *mEntry;
    int mDuration;
};

/*!
 * \class HbTimer
 *
 * \brief Provides a unified timer to avoid creating several separate singleshot
 * timers.
 *
 * \internal
 *
 * For example, the following code
 * \code
 *   QTimer *timer = new QTimer(this);
 *   timer->setSingleShot(true);
 *   connect(timer, SIGNAL(timeout()), SLOT(onTimeout()));
 *   timer->start(milliseconds);
 * \code
 * simply becomes
 * \code
 *   HbTimer::instance()->addTimeout(milliseconds, this, SLOT(onTimeout()));
 * \endcode
 *
 * Alternatively a class C can inherit from HbTimerEntry, pass the timer
 * interval to the HbTimerEntry constructor, and implement timerFired().  In
 * this case HbTimer::instance()->registerEntry(instance_of_class_C) can be
 * called to have timerFired() invoked on instance_of_class_C after the number
 * of milliseconds that has been passed to the HbTimerEntry constructor.
 */

HbTimer::HbTimer(QObject *parent)
    : QObject(parent)
{
}

HbTimer *HbTimer::instance()
{
    static HbTimer *inst = new HbTimer(qApp);
    return inst;
}

/*!
 * Requests a notification after the interval specified in \entry.
 *
 * The notification will mean calling timerFired() on \a entry.
 *
 * If \a deleteWhenFinishedNormally is true then the entry is
 * automatically deleted after invoking timerFired(). Note that the
 * entry is not deleted when unregisterEntry() is called explicitly.
 *
 * \sa addTimeout
 */
void HbTimer::registerEntry(HbTimerEntry *entry, bool deleteWhenFinishedNormally)
{
    if (!entryList.contains(entry)) {
        entryList << entry;
        entry->mDeleteWhenFinishedNormally = deleteWhenFinishedNormally;
        entry->mAnim = new HbTimerAnim(entry);
        entry->mAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

/*!
 * Cancels a pending timer notification. The HbTimerEntry instance (entry) is
 * not deleted.
 *
 * \return true if the entry was successfully unregistered, false if the entry
 * was not found amongst the registered ones.
 */
bool HbTimer::unregisterEntry(HbTimerEntry *entry)
{
    int idx = entryList.indexOf(entry);
    if (idx >= 0) {
        // Stop the animation but first set the anim's entry backptr to null
        // so no callback and deletion will occur in updateState().
        if (entry->mAnim) {
            entry->mAnim->mEntry = 0;
            entry->mAnim->stop(); // this will also result in deleting the HbTimerAnim instance
            entry->mAnim = 0;
        }
        entryList.removeAll(entry);
        return true;
    }
    return false;
}

/*!
 * Convenience wrapper for registerEntry().  The given \a slot on the
 * given QObject will be invoked after \a interval miliseconds.  The
 * returned HbTimerSignalEntry instance can be used to cancel the
 * timer; note that it will automatically be deleted after the slot
 * has been invoked, however it has to be deleted manually if
 * unregisterEntry() was called explicitly.
 *
 * \sa registerEntry
 */
HbTimerSignalEntry *HbTimer::addTimeout(int interval, QObject *receiver, const char *slot)
{
    HbTimerSignalEntry *entry = new HbTimerSignalEntry(interval);
    connect(entry, SIGNAL(timeout()), receiver, slot);
    registerEntry(entry, true);
    return entry;
}

/*!
 * Constructs a new timer entry instance. Interval is in milliseconds.
 */
HbTimerEntry::HbTimerEntry(int interval)
    : mInterval(interval), mAnim(0), mDeleteWhenFinishedNormally(false)
{
}

HbTimerEntry::~HbTimerEntry()
{
    if (mAnim) {
        delete mAnim;
    }
}

int HbTimerEntry::interval() const
{
    return mInterval;
}

HbTimerSignalEntry::HbTimerSignalEntry(int interval)
    : HbTimerEntry(interval)
{
}

void HbTimerSignalEntry::timerFired()
{
    emit timeout();
}
