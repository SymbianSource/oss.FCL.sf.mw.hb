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

#include <QApplication>
#include <QHash>

#include "hbsoftkeygroup_p.h"
#include "hbsoftkey_p.h"
#include "hbaction.h"
#include "hbnamespace_p.h"

class HbSoftKeyGroupPrivate : public QObject
{
    Q_OBJECT
public:
    HbSoftKeyGroupPrivate(HbSoftKeyGroup *group);

    QHash<HbPrivate::SoftKeyId, HbSoftKey *> mSoftKeys;
    HbSoftKeyGroup *q;
    HbAction *mDefaultSecondaryAction;

private slots:
    void launchMenu();
};

HbSoftKeyGroupPrivate::HbSoftKeyGroupPrivate(HbSoftKeyGroup *group)
    : q(group), mDefaultSecondaryAction(0)
{
}

void HbSoftKeyGroupPrivate::launchMenu()
{
    // only the primary softkey is connected so it is safe to assume
    // the sender is q->primary
    emit q->launchMenu(mSoftKeys.value(HbPrivate::PrimarySoftKey)->scenePos());

}

/*
    \class HbSoftKeyGroup
    \brief HbSoftKeyGroup manages soft keys.

    HbSoftKeyGroup creates and manages soft keys. HbSoftKeyGroup
    also provides access to individual soft keys. In the default
    constructor it creates primary, secondary and middle softkeys.

    Typically the softkey group is created by \a HbDecoratorGroup.
    The softkey group is accessible via HbDecoratorGroup::indicatorGroup().

    A brief example of how to use this class:
    \code
    HbSoftKeyGroup *softkeys = decorators->softKeyGroup();
    HbSoftKey *primary = softkeys->softKey(HbPrivate::PrimarySoftKey);
    \endcode

    \sa HbDecoratorGroup
*/


/*
    Default constructor.
	Creates PrimarySoftkey, SecondarySoftkey and MiddleSoftkey named
	"primary", "secondary" and "middle" respectively. Primary softkey
	connects \a launchMenu() slot and secondary to \a quit() slot.
*/
HbSoftKeyGroup::HbSoftKeyGroup(QObject *parent)
    : QObject(parent), d(new HbSoftKeyGroupPrivate(this))
{
    HbSoftKey* secondary = new HbSoftKey(HbPrivate::SecondarySoftKey);  
  
    // add default quit action to secondary softkey
    d->mDefaultSecondaryAction = new HbAction(Hb::QuitNaviAction,this);
    d->mDefaultSecondaryAction->setText("Quit");
    connect(d->mDefaultSecondaryAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    secondary->addAction(d->mDefaultSecondaryAction);

    d->mSoftKeys.insert(HbPrivate::SecondarySoftKey, secondary);

    // Create primary and middle soft keys only for non-touch
    if (!HbDeviceProfile::current().touch()) {
        HbSoftKey* primary = new HbSoftKey(HbPrivate::PrimarySoftKey);
         // add default launch menu action to primary softkey
        HbAction *action = new HbAction(this);
        action->setText("Options");
        connect(action, SIGNAL(triggered()), d, SLOT(launchMenu()));
        primary->addAction(action);
        d->mSoftKeys.insert(HbPrivate::PrimarySoftKey, primary);

        HbSoftKey* middle = new HbSoftKey(HbPrivate::MiddleSoftKey);
        d->mSoftKeys.insert(HbPrivate::MiddleSoftKey, middle);
    }
}

/*
    Destructs the softkey group.
 */
HbSoftKeyGroup::~HbSoftKeyGroup()
{
    delete d;
}

/*
    Returns a list of all soft keys.

    \sa HbSoftKey
 */
QList<HbSoftKey *> HbSoftKeyGroup::softKeys() const
{
    return d->mSoftKeys.values();
}

/*
    Returns the standard soft \a key.

    \sa HbPrivate::SoftKeyId, HbSoftKey
 */
HbSoftKey *HbSoftKeyGroup::softKey(HbPrivate::SoftKeyId key) const
{
    return d->mSoftKeys.value(key);
}

/*!
  Returns the default action for the secondary softkey (typically
  an action to quit the application).
*/
HbAction *HbSoftKeyGroup::defaultSecondaryAction() const
{
    return d->mDefaultSecondaryAction;
}

#include "hbsoftkeygroup.moc"
