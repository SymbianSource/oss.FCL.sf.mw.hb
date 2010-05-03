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

#include "hbeffect.h"
#include "hbeffect_p.h"
#include "hbeffectinternal_p.h"
#include "hbeffectabstract_p.h"
#include "hbeffectgroup_p.h"
#include "hbinstance.h"
#include "hbinstance_p.h"
#include "hbimagetraces_p.h"
#include <hbglobal.h>
#include <QObject>
#include <QGraphicsItem>
#include <QGraphicsWidget>

// Uncomment next define in order to get warning prints.
//#define HBEFFECT_WARNING

// Effect definitions for effect item strings starting with this prefix cannot
// be added with this public API. This prevents overriding HB's effects by
// mistake in 3rd party software.
#define HB_EFFECT_ITEM_PREFIX "HB_"
// This itemType is used internally to manage object specific effects and should
// not be used by 3rd party software.
#define HB_EFFECT_INTERNAL_ITEM "HB_INTERNAL"

static HbEffectPrivate d;

// ===========================================================================

class EffectMapKey
{
public:
    EffectMapKey(QGraphicsItem *item, const QString &effectEvent);

public:
    QGraphicsItem *mItem;
    QString mEffectEvent;
};

EffectMapKey::EffectMapKey(QGraphicsItem *item, const QString &effectEvent) :
    mItem(item), mEffectEvent(effectEvent)
{
}

bool operator <(const EffectMapKey &key1, const EffectMapKey &key2)
{
    // First compare item pointer address
    int result = key1.mItem - key2.mItem;
    // If that is same, compare effect event string
    if (!result) {
        result = key1.mEffectEvent.compare(key2.mEffectEvent);
    }
    return result<0;
}

static void fixEffectGroupOrder(QList<HbEffectGroup *> *groupList)
{
    // If there is only one group, no need to change order
    if (groupList->count() > 1) {
        int last = groupList->count() - 1;

        int movedCount = 0; // number of groups moved last in the list

        for (int i = last; i >= 0; --i) {
            HbEffectGroup *group = groupList->at(i);
            // If the group has a translate effect, move it last
            if (group->hasTranslateEffect()) {
                groupList->takeAt(i);
                groupList->append(group);
                movedCount++;
            }
        }

        if (last - movedCount > 0 ) {
            for (int i = last - movedCount; i >= 0; --i) {
                HbEffectGroup *group = groupList->at(i);
                // If the group has a scale effect, move it second last before groups that had translate effects
                if (group->hasScaleEffect()) {
                    groupList->takeAt(i);
                    groupList->insert(last - movedCount, group);
                }
            }
        }
    }
}

static bool privateDestroyed = false;

HbEffectPrivate::HbEffectPrivate() :
    mController(),
    mFactory(mController),
    mEventEffectList(),
    mEnabled(true),
    mDisabledItems(),
    mViewChangeConnected(false),
    mEffectsEnabled(true)
{
    // Needed for notifying effect finish asynchronously
    qRegisterMetaType<HbEffect::EffectStatus>();
}

HbEffectPrivate::~HbEffectPrivate()
{   
    privateDestroyed = true;
    mDisabledItems.clear();
}

void HbEffectPrivate::connectViewChanges()
{
    // Connect to view change signals if not done yet
    if (!mViewChangeConnected) {
        mViewChangeConnected = true;
        QList<HbMainWindow *> windowList = hbInstance->allMainWindows();
        foreach (const HbMainWindow *window, windowList) {
            connect(window, SIGNAL(currentViewChanged(HbView*)), SLOT(handleViewChanged(HbView*)));
        }
        // Need a notification when a mainwindow is added in the future.
        connect(HbInstancePrivate::d_ptr(), SIGNAL(windowAdded(HbMainWindow*)), SLOT(handleWindowAdded(HbMainWindow*)));
    }
}

void HbEffectPrivate::handleWindowAdded(HbMainWindow *window)
{
    connect(window, SIGNAL(currentViewChanged(HbView*)), SLOT(handleViewChanged(HbView*)));
}

void HbEffectPrivate::handleViewChanged(HbView *view)
{
     // Observers may be notified during cancellations which may result in starting a new
    // effect during which groups may be deleted and removed from
    // mEventEffectList. Therefore we cannot directly iterate on mEventEffectList.
    QList<HbEffectGroup *> groupsToBeHandled;
    foreach (HbEffectGroup *group, d.mEventEffectList) {
        groupsToBeHandled.append(group);
    }

    foreach (HbEffectGroup *group, groupsToBeHandled) {
    // Check if the real list still contains the group. If not then it may have been
    // deleted and removed meanwhile so do nothing.
    if (d.mEventEffectList.values().contains(group)) {
        if (group->isRunning()) {
            // Looping effect
            if (group->isLooping()) {
                if (group->view() == view) {    
                    group->resume(); // Effect's view activated, resume effect
                } else {
                    group->pause(); // Effect's view deactivated, pause effect
                }
            } else { // Non-looping effect
                if (group->view() != view) { // Effect's view deactivated, cancel the effect
                    group->cancelAll(true);
              
                    // remove group from eventEffectsList
                    QMap<EffectMapKey, HbEffectGroup *>::iterator e = d.mEventEffectList.begin();
                     while (e != d.mEventEffectList.end()) {
                         if (e.value() == group) { // found, erase from event effect list
                             d.mEventEffectList.erase(e);
							 delete group; // once removed from list, delete group
                             e = d.mEventEffectList.end();
                         }
                         else
                            e++;  // try next one
                     }
                }
            }
        }
    }    
}
}


HbAnimatedItemGroup::HbAnimatedItemGroup(
    const QList<QGraphicsItem *> &items,
    QObject *receiver,
    const char *member,
    const QVariant &userData) :
    QObject(),
    mItems(items),
    mFinishedCount(0),
    mReceiver(receiver),
    mFunction(member ? QString(member) : QString()),
    mUserData(userData)
{
    // Need information of receiver destroyed because this class gets
    // the notifications asynchronously so it must not invoke the
    // receiver if it has been destroyed meanwhile.
    connect(mReceiver, SIGNAL(destroyed()), this, SLOT(receiverDestroyed())); 
}

void HbAnimatedItemGroup::finished(const HbEffect::EffectStatus &status)
{
    if (++mFinishedCount == mItems.count()) {
        if (mReceiver && !mFunction.isEmpty()) {
            QMetaObject::invokeMethod(
                mReceiver,
                mFunction.toAscii().data(),
                Qt::AutoConnection,
                QGenericReturnArgument(),
                Q_ARG(HbEffect::EffectStatus, status));
        }
        // We are done, delete self.
        deleteLater();
    }
}

void HbAnimatedItemGroup::receiverDestroyed()
{
    // Receiver object was destroyed, set pointer to NULL so it is not invoked any more.
    mReceiver = 0;
}

// ===========================================================================

/*!
  @stable
  @hbcore
  \class HbEffect

  \brief HbEffect is used to control effects on graphics components.

  HbEffect has functions to start or cancel effects (like scaling, fading,
  rotation, various filter effects). The effects are usually trigerred by
  various events and are themable.  The effect definition is read from FXML
  files. All the widgets which want to show an effect should have their effects
  defined in an FXML file.  For example, view can have an effect animation
  defined in albumitem_anim.fxml with the following content:

  \code

  <layers>
  <visual>

  <param name="scale_x" type="anim">
  <duration>0.1</duration>
  <style>inoutback</style>
  <keyframe at="0.0">1.0</keyframe>
  <keyframe at="1.0">2.0</keyframe>
  </param>
  <param name="scale_y" type="anim">
  <keyframe at="0.0">1.0</keyframe>
  <keyframe at="1.0">2.0</keyframe>
  </param>
  <param name="scale_origin_x" ref="visual.width">0.5</param>
  <param name="scale_origin_y" ref="visual.height">0.5</param>
  <param name="opacity" type="anim">
  <duration>0.1</duration>
  <style>linear</style>
  <keyframe at="0.0">1.0</keyframe>
  <keyframe at="1.0">0.5</keyframe>
  </param>

  </visual>
  </layers>

  \endcode

  Example of how to use HbEffect

  \snippet{HbShow/hbsalbumsview.cpp,1}

*/

/*!
  \struct HbEffect::EffectStatus
  \brief Stores status information of the effect that has finished.
  It is passed as an argument to the receiver slot provided by the user of the API.
*/

/*!
  \var QGraphicsItem* HbEffect::EffectStatus::item

  The graphics item that has completed its effect.
*/

/*!
  \var QString HbEffect::EffectStatus::effectEvent

  The effect event that was passed in the HbEffect::start when the effect was started.
*/

/*!
  \var Hb::EffectEvent HbEffect::EffectStatus::reason

  The reason why the effect finished.
*/

/*!
  \var QVariant HbEffect::EffectStatus::userData

  Optional user data that was passed in the HbEffect::start when the effect was started.
*/


/*!  This function can be used to register an FXML file for a particular type of
  component (so that the effect is not tied to any particular widget or graphics
  item instance).

  \param itemType Type of the item effect is targeted to. The same type
  identifier should be used when HbEffect::start is called.  Avoid
  over-simplified item type strings like "item" if there is any risk that it
  will clash with other effect definitions.  Item type strings starting with
  prefix "HB_" are reserved for effects defined internally in HB library.  Such
  definitions cannot be added with this API.
  \param filePath The path of effect definition XML file or the logical name rather than the whole path.
  The logical name corresponds to the XML filename without the extension).

  \param effectEvent Effect event identifier that the effect is targeted to.

  The same type identifier should be used when HbEffect::start is called.
*/
bool HbEffect::add(
    const QString &itemType,
    const QString &filePath,
    const QString &effectEvent)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(itemType);
    Q_UNUSED(filePath);
    Q_UNUSED(effectEvent);
    return false;
#else
    // This API cannot be used to add HB library's effect definitions.
    // There is internal API in class HbEffectInternal for that.
    if (itemType.startsWith(HB_EFFECT_ITEM_PREFIX)) 
    {
        return false;
    }
    return HbEffectInternal::add(itemType, filePath, effectEvent);

#endif //HB_EFFECT_API_OFF
}

/*!  Registers multiple FXML files at the same time and guarantees that either
  all or none of them are registered. This means that if one of the files cannot
  be found or parsed then none of the others will be registered.

  \param itemType A list of types. The number of elements in all three
  lists must be the same or else the function will fail and return \i
  false. As a special exception \a effectEvent is allowed to be empty
  even when the other lists are not.

  \param filePath A list of file names.

  \param effectEvent A list of effect event identifiers.

  \return True if all the files were registered, false if none. Also returns
  false if the lists' element counts do not match.

  \sa HbEffect::add(const QString &, const QString &, const QString &)
 */
bool HbEffect::add(
    const QStringList &itemType,
    const QStringList &filePath,
    const QStringList &effectEvent)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(itemType);
    Q_UNUSED(filePath);
    Q_UNUSED(effectEvent);
    return false;
#else
    foreach (const QString &type, itemType) {
        if (type.startsWith(HB_EFFECT_ITEM_PREFIX))
            return false;
    }
    return HbEffectInternal::add(itemType, filePath, effectEvent);
#endif // HB_EFFECT_API_OFF
}

/*!  This function can be used to register an FXML file for a particular object
  instance.  The registered effect will be used when HbEffect::start() is called
  with the matching graphics item and effect event parameters.

  The function takes the following parameters:
  \param item Item which the effect in bound to.
  \param filePath The path of effect definition XML file or the logical name rather than the whole path.
  The logical name corresponds to the XML filename without the extension).
  \param effectEvent Effect event identifier.

*/
bool HbEffect::add(
    QGraphicsItem *item,
    const QString &filePath,
    const QString &effectEvent)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(item);
    Q_UNUSED(filePath);
    Q_UNUSED(effectEvent);
    return false;
#else
    return HbEffectInternal::add(item, filePath, effectEvent);
#endif //HB_EFFECT_API_OFF
}

/*! Registers multiple FXML files for a particular graphics item and guarantees
 *  that either all or none of them are registered. This means that if one of
 *  the files cannot be found or parsed then none of the others will be
 *  registered.
 *
 * \param item Item to which the effect is bound.
 *
 * \param filePath Name of the FXML file.
 *
 * \param effectEvent Effect event identifier. The number of elements
 * in \a filePath and \a effectEvent must match, unless effectEvent is
 * empty.
 */
bool HbEffect::add(QGraphicsItem *item, const QStringList &filePath, const QStringList &effectEvent)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(item);
    Q_UNUSED(filePath);
    Q_UNUSED(effectEvent);
    return false;
#else
    return HbEffectInternal::add(item, filePath, effectEvent);
#endif //HB_EFFECT_API_OFF
}

/*! This function can be used to unregister an FXML file for a particular
  component. Note that you have to have the effect registered with a matching
  itemType identifier to remove it with this method.

  This will not stop any on-going effects (even if the effect is the one being unregistered),
  for that call also cancel() before remove().

  The function takes following parameters:
  \param itemType Type of the item, for example "menu", "button" or "listitem"
  \param filePath The path of effect definition XML file.
  \param effectEvent Effect event identifier.
*/
bool HbEffect::remove(
    const QString &itemType,
    const QString &filePath,
    const QString &effectEvent)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(itemType);
    Q_UNUSED(filePath);
    Q_UNUSED(effectEvent);
    return false;
#else
    // This API cannot be used to remove HB library's effect definitions.
    // There is internal API in class HbEffectInternal for that.
    if (itemType.startsWith(HB_EFFECT_ITEM_PREFIX)) 
    {
        return false;
    }
    HbEffectInternal::remove(itemType, filePath, effectEvent);
    return true;
#endif //HB_EFFECT_API_OFF
}

/*! This function can be used to unregister an FXML file for a particular item.

  This will not stop any on-going effects (even if the effect is the one being unregistered),
  for that call also cancel() before remove().

  The function takes following parameters:
  \param item The widget or graphics item
  \param filePath The path of effect definition XML file.
  \param effectEvent Effect event identifier.
*/
bool HbEffect::remove(
    QGraphicsItem *item,
    const QString &filePath,
    const QString &effectEvent)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(item);
    Q_UNUSED(filePath);
    Q_UNUSED(effectEvent);
    return false;
#else
    HbEffectInternal::remove(item, filePath, effectEvent);
    return true;
#endif //HB_EFFECT_API_OFF

}

/*! This function can be used to unregister all FXML files for a particular item.

  This will not stop any on-going effects (even if the effect is the one being unregistered),
  for that call also cancel() before remove().

  The function takes following parameters:
  \param item The instance for which all FXML files must be unregistered
*/
bool HbEffect::remove(QGraphicsItem *item)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(item);
    return false;
#else
    HbEffectInternal::remove(item);
    return true;
#endif //HB_EFFECT_API_OFF
}

/*! \brief This function is used to start an effect on a graphics item. 

  The effect definition is taken from the FXML file registered for the graphics
  item with matching itemType and effectEvent.

  \param item The target item for the effect

  \param itemType This is an string identifying type of item. For example,
  "menu", "button", "scrollbar" or "listitem". This should be same as the one
  passed in HbEffect::add() function for adding the xml.

  \param effectEvent A string identifying effect event type (e.g "appear").
  This should be same as the one passed in HbEffect::add() function for adding
  the xml.

  \param receiver This is an QObject which is notified by calling its slot
  when this effect is finished. This is optional parameter and default value is
  NULL.

  \param member The name of the function of receiver which will be called when
  effect is finished. This is an optional parameter and its default value is 0.
  The signature of the function must be the following: \c void
  memberSlotName(const HbEffect::EffectStatus &status)

  \note It is guaranteed that the slot is always invoked once, unless prohobited
  when canceling the effect.  Normally the slot is invoked asynchronously when
  the effect has finished or it has been canceled.  If the effect is not started
  at all, the slot is invoked synchronously with the reason
  Hb::EffectNotStarted.

  \param userData Any user data provided here will be passed back in the
  HbEffectStatus struct when the effect being finished is notified to the client
  via the given slot.  This may help the client to keep track of the states when
  the effect was started and ended.

  \param extRect Defines external rectangle to be used with the effect. It has
  effect only with scale and translate effects, which use the "extrect" keyword
  in the effect definition. With this parameter you can e.g. create an effect,
  which scales and translates the item from any given rectangle to its own
  rectangle.

  \sa HbEffect::EffectStatus

  The function returns \c false if some invalid parameter is passed, or if the
  effect was disabled using disable().

  Example to get notification of effect completion.
  \code
  // Create a public slot in a the widget class if you want to get effect-finished 
  // notification. In normal scenario, the widget itself can be receiver. But it is possible 
  // to use any QObject as receiver.
  class MyHbWidget : public HbWidget
  {
  Q_OBJECT
  public slots:
  void effectFinished(const HbEffect::EffectStatus &status);
  // Other stuffs 
  };
    
  void MyObserver::effectFinished(const HbEffect::EffectStatus &status)
  {
  // Effect is finished. Do something.
  }

  // Some function which starts effect..
  void MyHbWidget::myFunction()
  {
  // myFunction related other stuff
  // calling startefect
  HbEffect::start(myGraphicsItem, 
  "myItemType",
  "myEvent",
  this,
  "effectFinished");
        
  // myFunction related other stuff
  }
  \endcode
  \sa cancel()
*/
bool HbEffect::start(QGraphicsItem *item, 
                     const QString &itemType, 
                     const QString &effectEvent,
                     QObject *receiver,
                     const char *member,
                     const QVariant &userData,
                     const QRectF &extRect)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(item)
    Q_UNUSED(itemType);
    Q_UNUSED(effectEvent);
    Q_UNUSED(receiver)
    Q_UNUSED(member);
    Q_UNUSED(userData)
    Q_UNUSED(extRect)
    return false;
#else
    return HbEffectInternal::start(item, item, HbEffectInternal::Normal, itemType, effectEvent, receiver, member, userData, extRect);
#endif //HB_EFFECT_API_OFF
}

/*! \brief This function starts the same effect for a given graphics item.

  \param item The target item for the effect

  \param effectEvent A string identifying effect event type (e.g "appear").
  This should be same as the one passed in HbEffect::add() function for adding the xml.

  \param receiver This is a QObject which is notified once by calling its slot
  when all of the items have finished their effects. This is an optional
  parameter and its default value is NULL. Ownership is not transferred.

  \param member The name of the function of receiver which will be called when
  effect is finished. This is an optional parameter and its default value is 0.
  The signature of the function must be the following: \c void
  memberSlotName(const HbEffect::EffectStatus &status)

  \note It is guaranteed that the slot is always invoked once, unless prohobited
  when canceling the effect.  Normally the slot is invoked asynchronously when
  the effect has finished or it has been canceled.  If the effect is not started
  at all, the slot is invoked synchronously with the reason
  Hb::EffectNotStarted.

  \param userData Any user data provided here will be passed back in the
  HbEffectStatus struct when the effect being finished is notified to the client
  via the given slot.  This may help the client to keep track of the states when
  the effect was started and ended.

  \sa HbEffect::EffectStatus

  The function returns \c false if some invalid parameter is passed, or if the
  effect was disabled using disable().
*/
bool HbEffect::start(QGraphicsItem *item, 
                     const QString &effectEvent,
                     QObject *receiver,
                     const char *member,
                     const QVariant &userData,
                     const QRectF &extRect)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(item);
    Q_UNUSED(effectEvent);
    Q_UNUSED(receiver);
    Q_UNUSED(member);
    Q_UNUSED(userData);
    Q_UNUSED(extRect);
    return false;
#else
    return HbEffect::start(
        item, HB_EFFECT_INTERNAL_ITEM, effectEvent, receiver, member, userData, extRect);
#endif //HB_EFFECT_API_OFF
}

/*!  \brief This function starts the same effect simultaneously for a given list
  of graphics items.

  The effect definition is taken from the FXML file registered to the given item
  type.

  \param items The target items for the effect

  \param itemType A string identifying the type of the items. For example,
  "button".  This should be same as the one passed in HbEffect::add() function
  for adding the xml.

  \param effectEvent A string identifying effect event type (e.g "appear").
  This should be same as the one passed in HbEffect::add() function for adding
  the xml.

  \param receiver This is a QObject which is notified once by calling its slot
  when all of the items have finished their effects. This is an optional
  parameter and its default value is NULL. Ownership is not transferred.

  \param member The name of the function of receiver which will be called when
  effect is finished. This is an optional parameter and its default value is 0.
  The signature of the function must be the following: \c void
  memberSlotName(const HbEffect::EffectStatus &status)

  \note It is guaranteed that the slot is always invoked once, unless prohobited
  when canceling the effect.  Normally the slot is invoked asynchronously when
  the effect has finished or it has been canceled.  If the effect is not started
  at all, the slot is invoked synchronously with the reason
  Hb::EffectNotStarted.

  \param userData Any user data provided here will be passed back in the
  HbEffectStatus struct when the effect being finished is notified to the client
  via the given slot.  This may help the client to keep track of the states when
  the effect was started and ended.

  \sa HbEffect::EffectStatus

  The function returns \c false if some invalid parameter is passed, or if the
  effect was disabled using disable().
*/
bool HbEffect::start(
    const QList<QGraphicsItem *> &items, 
    const QString &itemType, 
    const QString &effectEvent,
    QObject *receiver,
    const char *member,
    const QVariant &userData)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(items);
    Q_UNUSED(itemType); 
    Q_UNUSED(effectEvent);
    Q_UNUSED(receiver);
    Q_UNUSED(member);
    Q_UNUSED(userData);
    return false;
#else
    if (privateDestroyed) {
        return false;
    }

    // Connect to view changes if not done yet
    d.connectViewChanges();

    // This object deletes itself when the effect has finished.
    HbAnimatedItemGroup *group = new HbAnimatedItemGroup(items, receiver, member, userData);

    bool ret = false;

    QObject *groupReceiver = 0;
    const char *groupMember = "";

    // If caller has given observer for the group of items,
    // set the group object as observer in HbEffect::start calls.
    // It gives callback to the caller when the whole group has finished.

    if (receiver) {
        groupReceiver = group;
        groupMember = "finished";
    }

    foreach (QGraphicsItem *item, items) {
        if (HbEffect::start(item, itemType, effectEvent, groupReceiver, groupMember, userData)) {
            // If at least one item succeeded to start, set return value to true
            ret = true;
        }
    }

    return ret;
#endif //HB_EFFECT_API_OFF
}

/*! Reports whether the given graphics \a item is currently running an effect.

  \param item pointer to the graphics item in question.

  \param effectEvent String identifying effect-event type (eg "appear"). If this
  string is empty, the function returns true if the item is running any effect.

  The function returns \c false if some invalid parameter is passed.
*/
bool HbEffect::effectRunning(QGraphicsItem *item, const QString &effectEvent)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(item);
    Q_UNUSED(effectEvent);
    return false;
#else
    bool ret = false;

    // Return true if the given item is running any effect
    if (effectEvent.isEmpty()) {
        for (QMap<EffectMapKey, HbEffectGroup *>::const_iterator e = d.mEventEffectList.constBegin();
             e != d.mEventEffectList.constEnd();
             ++e ) {
            HbEffectGroup *group = *e;
            if (group->registrationItem() == item && group->isRunning()) {
                ret = true;
                break;
            }
        }
    } else {
        // effectEvent was specified, check if effect matching that is running
        EffectMapKey key(item, effectEvent);
        QMap<EffectMapKey, HbEffectGroup *>::const_iterator e = d.mEventEffectList.find(key);
    
        // Effect found in the list?
        if(e != d.mEventEffectList.constEnd()) {
            HbEffectGroup* group = *e;
            ret = group->isRunning();
        }
    }

    return ret;
#endif //HB_EFFECT_API_OFF
}


/*! This function is used to cancel a running effect. The end state of the item
  is reached after cancelation of effect.

  \param item  pointer to a QGraphicsItem

  \param effectEvent  String identifying effect-event type (eg "appear"). If
  not passed, all the effects running currently on the item are canceled.

  \param clearEffect   If false, the effect is left in its end state in the
                       graphics item.  If true, the effect is hidden from the
                       graphics item.

  \param sendCallback   Optional boolean parameter (true if omitted), which
                        controls if the cancel call results to our
                        effectFinished slot being called.  This should be false
                        in (for example) in destructor where you are cancelling
                        effects but not wanting any callbacks.

  \param itemIsValid This parameter tells whether the passed item pointer points
  to a valid object and can accessed when canceling the effect. For example, if
  this method is called from a graphics item base class destructor, the item
  object may be already partially destructed and this parameter should be set to
  false.

  The function returns \c false if some invalid parameter is passed.

  \sa start()
*/
bool HbEffect::cancel(
    QGraphicsItem *item,
    const QString &effectEvent,
    bool clearEffect,
    bool sendCallback,
    bool itemIsValid)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(item);
    Q_UNUSED(effectEvent);
    Q_UNUSED(clearEffect);
    Q_UNUSED(sendCallback);
    Q_UNUSED(itemIsValid);
    return false;
#else 
    if (privateDestroyed) {
        return false;
    }

    bool ret = false;

    if (effectEvent.isEmpty()) {
        // Stop all the effects running on item at that point of time.
        // (iterator way had some problem and caused crash so using foreach)
        QList<HbEffectGroup *> groupsToBeCanceled;
        foreach (const EffectMapKey &key, d.mEventEffectList.keys()) {
            if (key.mItem == item) {
                HbEffectGroup* group = d.mEventEffectList.take(key);
                groupsToBeCanceled.append(group);
            }
        }

        // Before blindly calling cancelAll to the effect groups, try to fix their order so that
        // combining different transformation end states works as well as possible. Note that there
        // may still be incorrect end state if there are multiple effect groups which each have multiple effects,
        // but this at least fixes cases, where there are multiple effect groups which each consist of a single effect.
        fixEffectGroupOrder(&groupsToBeCanceled);

        bool first = true;
        foreach (HbEffectGroup *group, groupsToBeCanceled) {
            // If clearEffect is false then it is important to pass a default transform
            // first so the matrix multiplication in cancelAll ends up correct.
            group->cancelAll(sendCallback, itemIsValid, clearEffect, first || !itemIsValid ? QTransform() : item->transform());
            if (first) {
                first = false;
            }
            delete group;
            // return true if any of the effect was canceled
            ret = true;
        }
        return ret;
    } else {
        EffectMapKey key(item, effectEvent);
        HbEffectGroup* group = d.mEventEffectList.take(key);
        if (group) {
            group->cancelAll(sendCallback, itemIsValid, clearEffect);
            delete group;
            return true;
        } else {
            return false;
        }
    }
#endif //HB_EFFECT_API_OFF
}

/*! This function can be used to reverse the effect of disable(). So if the
  effect for a graphicsitem was disabled this method can be used to enable the
  effect again for that item.

  The function takes following parameters:
  \param item pointer to QGraphicsItem for which the effect will be enabled.
    
  \sa disable()
*/
void HbEffect::enable(QGraphicsItem *item)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(item);
#else
    d.mDisabledItems.removeOne(item);
#endif //HB_EFFECT_API_OFF
}

/*! This function can be used to disable an effect for a graphicsitem. This is
  useful in cases where you have a component which usually shows an effect but
  you want to skip the effect for your object.
    
  You can enable the effect using enable(), and you should always call enable()
  once you do not need the object to be disabled, at latest when you're deleting
  your animated graphicsitem. This should be done to clean up the list of
  disabled items. Note that if the animated item is an HbWidget, then the
  enable() call happens automatically at the HbWidget destructor.
    
  Unlike remove()-methods, this won't touch the effect definition at all. The
  effect is just skipped when trying to start the effect.  Note also that
  start()-method returns false if the effect is disabled.

  \param item pointer to QGraphicsItem for which the effect will be disabled
    
  \sa enable()
  \sa start()
*/
void HbEffect::disable(QGraphicsItem *item)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(item);
#else
    if (!d.mDisabledItems.contains(item)) {
        d.mDisabledItems.append(item);
    }
#endif //HB_EFFECT_API_OFF
}

/*!
  \internal
*/
HbGVWrapperItem::HbGVWrapperItem():mMainWindow(0)
{
}

void HbGVWrapperItem::setMainWindow(HbMainWindow& mainWindow)
{
    mMainWindow = &mainWindow;
}

HbMainWindow *HbGVWrapperItem::mainWindow() const
{
    return mMainWindow;
}

QRectF HbGVWrapperItem::boundingRect() const
{
    QRectF bRect;
    // viewportitem is used as a boundingrect since viewport item gets resized when changing orientation
    if( mMainWindow ) {
        bRect = mMainWindow->layoutRect();
    }
    return bRect;
}
void HbGVWrapperItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) 
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);
};

/*!
  \internal
*/
bool HbEffectInternal::add(const QString &itemType, const QString &filePath, const QString &effectEvent)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(itemType);
    Q_UNUSED(filePath);
    Q_UNUSED(effectEvent);
    return false;
#else
    // Add and try to use sharing.
    bool ret = d.mController.addFXML(itemType, filePath, effectEvent, true);
    if (ret) {
        // Mark all effects using the given item type dirty,
        // so that they get re-created when the effect is shown next time
        for (QMap<EffectMapKey, HbEffectGroup *>::iterator e = d.mEventEffectList.begin();
             e != d.mEventEffectList.end();
             ++e ) {
            HbEffectGroup *group = *e;

            if (group->itemType() == itemType) {
                group->setDirty(true);
            }
        }
    }

    return ret;
#endif //HB_EFFECT_API_OFF
}

/*!
  \internal
*/
bool HbEffectInternal::add(const QStringList &itemType, const QStringList &filePath, const QStringList &effectEvent)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(itemType);
    Q_UNUSED(filePath);
    Q_UNUSED(effectEvent);
    return false;
#else
    int count = itemType.count();
    if (count != filePath.count() || (count != effectEvent.count() && !effectEvent.isEmpty())) {
        return false;
    }
    bool ret = false;
    // Try to add all the files, stop when one fails.
    for (int i = 0; i < count; ++i) {
        ret = add(itemType.at(i), filePath.at(i), effectEvent.isEmpty() ? QString() : effectEvent.at(i));
        if (!ret) {
            break;
        }
    }
    // If any of the files were not added then remove all the others too.
    if (!ret) {
        for (int i = 0; i < count; ++i) {
            remove(itemType.at(i), filePath.at(i), effectEvent.isEmpty() ? QString() : effectEvent.at(i));
        }
    }
    return ret;
#endif // HB_EFFECT_API_OFF
}

/*!
  \internal
*/
bool HbEffectInternal::add(QGraphicsItem *item, const QString &filePath, const QString &effectEvent)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(item);
    Q_UNUSED(filePath);
    Q_UNUSED(effectEvent);
    return false;
#else
    bool ret = false;

    // FXML
    // Add and try to use sharing.
    ret = d.mController.addFXML(item, filePath, effectEvent, true);

    if (ret) {
        // Mark all effects using the given item type dirty,
        // so that they get re-created when the effect is shown next time
        for (QMap<EffectMapKey, HbEffectGroup *>::iterator e = d.mEventEffectList.begin();
             e != d.mEventEffectList.end();
             ++e ) {
            HbEffectGroup *group = *e;

            if (group->registrationItem() == item) {
                group->setDirty(true);
            }
        }
    }

    return ret;
#endif //HB_EFFECT_API_OFF
}

/*!
  \internal
*/
bool HbEffectInternal::add(QGraphicsItem *item, const QStringList &filePath, const QStringList &effectEvent)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(item);
    Q_UNUSED(filePath);
    Q_UNUSED(effectEvent);
    return false;
#else
    int count = filePath.count();
    if (count != effectEvent.count() && !effectEvent.isEmpty()) {
        return false;
    }
    bool ret = false;
    // Try to add all the files, stop when one fails.
    for (int i = 0; i < count; ++i) {
        ret = add(item, filePath.at(i), effectEvent.isEmpty() ? QString() : effectEvent.at(i));
        if (!ret) {
            break;
        }
    }
    // If any of the files were not added then remove all the others too.
    if (!ret) {
        for (int i = 0; i < count; ++i) {
            remove(item, filePath.at(i), effectEvent.isEmpty() ? QString() : effectEvent.at(i));
        }
    }
    return ret;
#endif // HB_EFFECT_API_OFF
}

/*!
  \internal
*/
void HbEffectInternal::remove(const QString &itemType, const QString &filePath, const QString &effectEvent)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(itemType);
    Q_UNUSED(filePath);
    Q_UNUSED(effectEvent);
#else
    // FXML
    // If extension is not given then try both with and without .fxml
    if (!filePath.endsWith(".fxml")) {
        d.mController.removeFXML(itemType, filePath + ".fxml", effectEvent);       
    }
    d.mController.removeFXML(itemType, filePath, effectEvent);
    // Clean up mEventEffectList.
    foreach (const EffectMapKey &key, d.mEventEffectList.keys()) {
        if (key.mEffectEvent == effectEvent) {
            HbEffectGroup *group = d.mEventEffectList.value(key);
            group->setDirty(true); // so the next start() will try to recreate the effect instead of re-using it
        }
    }
#endif //HB_EFFECT_API_OFF
}

/*!
  \internal
*/
void HbEffectInternal::remove(QGraphicsItem *item, const QString &filePath, const QString &effectEvent)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(item);
    Q_UNUSED(filePath);
    Q_UNUSED(effectEvent);
#else
    // FXML
    // If extension is not given then try both with and without .fxml
    if (!filePath.endsWith(".fxml")) {
        d.mController.removeFXML(item, filePath + ".fxml", effectEvent);       
    }
    d.mController.removeFXML(item, filePath, effectEvent);
    // Clean up mEventEffectList.
    foreach (const EffectMapKey &key, d.mEventEffectList.keys()) {
        if (key.mItem == item && key.mEffectEvent == effectEvent) {
            HbEffectGroup *group = d.mEventEffectList.value(key);
            group->setDirty(true);
        }
    }
#endif //HB_EFFECT_API_OFF
}

/*!
  \internal
*/
void HbEffectInternal::remove(QGraphicsItem *item)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(item);
#else
    if (item) {
        d.mDisabledItems.removeOne(item); // clean the disabled list as well
        d.mController.removeFXML(item);
        // Clean up mEventEffectList.
        foreach (const EffectMapKey &key, d.mEventEffectList.keys()) {
            if (key.mItem == item) {
                HbEffectGroup *group = d.mEventEffectList.value(key);
                group->setDirty(true);
            }
        }
    }
#endif //HB_EFFECT_API_OFF
}

void HbEffectInternal::reloadFxmlFiles()
{
    QList<HbEffectInfo> parsedDataList = d.mController.effectEntries();
    //ToDo Currently all the parsed Fxml Data is being deleted with themechange.
    //     Make this function intelligent enough to delete only theme specific Fxml
    //     data.
    //parsedDataList.clear();
    QList<HbEffectInfo> newDataList;
    for (int i=0; i < parsedDataList.count(); i++) {
        if (parsedDataList[i].fromTheme()) {
            newDataList.append(parsedDataList[i]);
            parsedDataList.removeAt(i);
            i--;
        }
    }
    for (int i=0; i < newDataList.count(); i++) {
        HbEffectInfo effectData = newDataList.at(i);            
        bool ret = false;
        QString relativename = QFileInfo(effectData.xmlFileFullPath()).baseName();
        if (effectData.item() != 0) {
            ret = d.mController.addFXML(effectData.item(), relativename,
                                        effectData.effectEvent(), true);
        }
        else if (!effectData.componentType().isEmpty()) {
            ret = d.mController.addFXML(effectData.componentType(), relativename,
                                        effectData.effectEvent(), true);
        }
        if (ret) {
            // Mark all effects using the given item type dirty,
            // so that they get re-created when the effect is shown next time
            for (QMap<EffectMapKey, HbEffectGroup *>::iterator e = d.mEventEffectList.begin();
                 e != d.mEventEffectList.end();
                 ++e ) {
                HbEffectGroup *group = *e;
                if (group->registrationItem() == effectData.item()) {
                    group->setDirty(true);
                }
            }
        }
    }
    newDataList.clear();
}

/*!
  Cancels all effects. List of graphicsitems can be given as an exception which are not cancelled.
  This is used for example when changing orientation; all but the orientation change effects
  are cancelled.
*/
void HbEffectInternal::cancelAll(const QList<QGraphicsItem*> *exceptionList, bool ignoreLooping)
{
    // Observers may be notified during cancelations which may result in starting a new
    // effect during which groups may be deleted and removed from
    // mEventEffectList. Therefore we cannot directly iterate on mEventEffectList.
    QList<HbEffectGroup *> groupsToBeCanceled;
    foreach (HbEffectGroup *group, d.mEventEffectList) {
        groupsToBeCanceled.append(group);
    }
    foreach (HbEffectGroup *group, groupsToBeCanceled) {
        // Check if the real list still contains the group. If not then it may have been
        // deleted and removed meanwhile so do nothing.
        if (d.mEventEffectList.values().contains(group)) {
            if (!exceptionList || !exceptionList->contains(group->registrationItem())) {
                // Already cancelled effects may refer to graphics items that do not
                // exist anymore. Cancel only "running" (that is, running or paused,
                // i.e. non-canceled) effects.
                if (group->isRunning() && (!ignoreLooping || !group->isLooping())) {
                    group->cancelAll(true);
                }
            }
        }
    }
}

/*!
  Like cancelAll() but no observers are invoked and items are not assumed to
  be valid anymore.
 */
void HbEffectInternal::safeCancelAll(bool clear)
{
    QList<HbEffectGroup *> groupsToBeCanceled;
    foreach (HbEffectGroup *group, d.mEventEffectList) {
        groupsToBeCanceled.append(group);
    }
    foreach (HbEffectGroup *group, groupsToBeCanceled) {
        if (d.mEventEffectList.values().contains(group)) {
            group->cancelAll(false, false, clear);
        }
    }
}

/*!
  This is a master switch, which stops all effects in the application.
  Looping effects are paused and other effects are moved in their end state.

  The effect framework automatically stops effects when the application moves in background
  or display is turned off.

  \sa HbEffectInternal::resumeEffects()
*/
void HbEffectInternal::stopEffects()
{
    // Pause looping effects and stop others
    foreach (HbEffectGroup *group, d.mEventEffectList) {
        if (group->isRunning()) {
            if (group->isLooping()) {
                group->pause();
            } else {
                group->cancelAll(true);
            }
        }
    }
}

/*!
  This is a master switch, which resumes effects in the application.
  Any looping effects that were paused due to HbEffect::stopEffects() call are resumed.

  The effect framework automatically resumes effects when the application comes back to
  foreground or display is turned back on.

  \sa HbEffectInternal::stopEffects()
*/
void HbEffectInternal::resumeEffects()
{
    // Resume any looping effects that were paused with stopEffects call.
    foreach (HbEffectGroup *group, d.mEventEffectList) {
        if (group->isRunning() && group->isLooping()) {
            group->resume();
        }
    }
}

inline void updateGroup(HbEffectGroup *group,
                        QObject *receiver,
                        const char *member,
                        const QVariant &userData,
                        const QRectF &extRect,
                        HbEffectInternal::EffectFlags flags)

{
    group->setObserver(receiver, member ? QString(member) : QString());
    group->setUserData(userData);
    group->setExtRect(extRect);
    group->setEffectFlags(flags);
}

/*!
  The real implementation for HbEffect::start().

  Watch out for the difference between the registration and target items. Even though in
  most cases HbEffect::start() will invoke this by passing the same pointer to both, there
  are certain situations when they can be different (e.g. when a view switch effect is
  registered via add() by the user for the HbView instance, but effect has to operate on
  another graphics item, e.g. the view's content widget or some other "screen" item, etc.)

  \a registrationItem is used for all bookeeping (for fxml lookup, in the event-effect
  list, in effect groups, etc.).  \a targetItem is used only for the actual transformation
  or filter effect implementations.

  For functions that operate with one graphics item parameter (e.g. cancel()), that
  parameter corresponds to the \a registrationItem. The internal difference (and
  possibility) of having separate registration and target items is not exposed to the
  users of the HbEffect API in any way.

  When ClearEffectWhenFinished is set (it is typically not set when coming from the standard
  HbEffect APIs) the effect is always removed when it is finished or canceled (similarly to
  when cancel() is called with clearEffect set to true).

  When ShowItemOnFirstUpdate is set, the target and registration items are set to visible
  after the effects have been first applied. This flag is useful in certain cases to make
  sure there is no flickering.

  \sa HbEffect::start()
 */
bool HbEffectInternal::start(QGraphicsItem *registrationItem,
                             QGraphicsItem *targetItem,
                             EffectFlags flags,
                             const QString &itemType, 
                             const QString &effectEvent,
                             QObject *receiver,
                             const char *member,
                             const QVariant &userData,
                             const QRectF &extRect)
{
#ifdef HB_EFFECT_API_OFF
    Q_UNUSED(registrationItem);
    Q_UNUSED(targetItem);
    Q_UNUSED(itemType);
    Q_UNUSED(effectEvent);
    Q_UNUSED(receiver)
    Q_UNUSED(member);
    Q_UNUSED(userData)
    Q_UNUSED(extRect)
    return false;
#else
    // Connect to view changes if not done yet
    d.connectViewChanges();

    // Test error conditions first.
    bool error = false;
    if (!targetItem || !registrationItem) {
#ifdef HBEFFECT_WARNING
        qWarning("HbEffect Warning: Item is NULL");
#endif //HBEFFECT_WARNING
        error = true;
    }

    if (!d.mEffectsEnabled && targetItem && targetItem->type() != HbGVWrapperItemType) {
        // Everything was disabled via setEffectsEnabled().
        error = true;
    }

    if (!d.mEnabled) {
#ifdef HBEFFECT_WARNING
        qWarning("HbEffect Warning: Effect is not enabled");
#endif //HBEFFECT_WARNING
        error = true;
    }

    if (d.mDisabledItems.contains(registrationItem)) {
#ifdef HBEFFECT_WARNING
        qWarning("HbEffect Warning: Effect for this item was disabled by an effect client");
#endif //HBEFFECT_WARNING
        error = true;
    }

    // If there was an error in parameters, give notification that the effect was not started.
    if (error) {
        if (receiver && member) {
            HbEffect::EffectStatus status;
            status.item = registrationItem;
            status.effectEvent = effectEvent;
            status.userData = userData;
            status.reason = Hb::EffectNotStarted;

            QMetaObject::invokeMethod(
                receiver,
                member,
                Qt::AutoConnection,
                QGenericReturnArgument(),
                Q_ARG(HbEffect::EffectStatus, status));
        }
        if (flags.testFlag(HbEffectInternal::ShowItemOnFirstUpdate)) {
            if (targetItem) {
                targetItem->setVisible(true);
            }
            if (registrationItem && registrationItem != targetItem) {
                registrationItem->setVisible(true);
            }
        }
        return false;
    }

    // Warning conditions here
    if (!targetItem->boundingRect().isValid()) {
#ifdef HBEFFECT_WARNING
        qWarning("HbEffect Warning: Item size not valid in %s", qPrintable(itemType));
#endif //HBEFFECT_WARNING
    }

    bool started = false;

    EffectMapKey key(registrationItem, effectEvent);
    QMap<EffectMapKey, HbEffectGroup *>::iterator e = d.mEventEffectList.find(key);

    // Same effect found in the list?
    if(e != d.mEventEffectList.end()) {
        HbEffectGroup *group = e.value();
        if (!group->dirty()) {
            // if the effect group is not dirty, restart it. Cancel possible earlier
            // effect first so that end position gets correct.
            group->cancelAll(true);
            
            // Update with given notification parameters
            updateGroup(group, receiver, member, userData, extRect, flags);

            group->startAll();
            started = true;
        } else {
            // Delete the effect group if it was dirty
            delete d.mEventEffectList.take(key);
        }
    }

    if (!started) {
        // Create the effect.
        HbEffectGroup *group = d.mFactory.createEffect(registrationItem, targetItem, itemType, effectEvent);

        updateGroup(group, receiver, member, userData, extRect, flags);

        // Store effect to support cancelation and deletion of effect
        d.mEventEffectList.insert(key, group);
        group->startAll();
        // check if definition had effects
        if (group->effectCount() == 0) {
#ifdef HBEFFECT_WARNING
            qWarning("HbEffect Warning: Starting effect with effect count zero");
#endif //HBEFFECT_WARNING
        }
    }

    return true;
#endif //HB_EFFECT_API_OFF
}

/*!
  A simpler version without itemType.
 */
bool HbEffectInternal::start(QGraphicsItem *registrationItem,
                             QGraphicsItem *targetItem,
                             EffectFlags flags,
                             const QString &effectEvent,
                             QObject *receiver,
                             const char *member,
                             const QVariant &userData,
                             const QRectF &extRect)
{
    return HbEffectInternal::start(registrationItem, targetItem, flags,
                                   HB_EFFECT_INTERNAL_ITEM,
                                   effectEvent, receiver, member, userData, extRect);
}

/*!
  By default effects are enabled. However with this function every effect can
  be disabled, meaning that HbEffect::start() will have no effect (and will
  always return false).

  Note that "every effect" is not really every effect, it does not include the
  orientation switch effect (or any effect performed in the graphics view
  wrapper item, actually), that is always enabled regardless of this setting.

  \internal
 */
void HbEffectInternal::setEffectsEnabled(bool enabled)
{
    d.mEffectsEnabled = enabled;
    if (!d.mEffectsEnabled) {
        cancelAll();
    }
}
