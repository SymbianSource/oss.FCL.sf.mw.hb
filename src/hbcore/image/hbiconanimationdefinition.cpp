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

#include "hbiconanimationdefinition.h"

// Private class
class HbIconAnimationDefinitionPrivate : public QSharedData
{
public:
    HbIconAnimationDefinitionPrivate();
    ~HbIconAnimationDefinitionPrivate();

    // This constructor is used only by NULL instance
    HbIconAnimationDefinitionPrivate(bool nullInstance);

    HbIconAnimationDefinitionPrivate(const HbIconAnimationDefinitionPrivate &other);

private:
    // disabled
    HbIconAnimationDefinitionPrivate &operator=(const HbIconAnimationDefinitionPrivate &other);

public:
    QList<HbIconAnimationDefinition::AnimationFrame> frameList;
    HbIconAnimationDefinition::PlayMode playMode;
};

// Shared null private object
static HbIconAnimationDefinitionPrivate shared_null(true);

/*!
\internal
*/
HbIconAnimationDefinitionPrivate::HbIconAnimationDefinitionPrivate() :
    QSharedData(),
    frameList(),
    playMode(HbIconAnimationDefinition::PlayOnce)
{
}

/*!
\internal
*/
HbIconAnimationDefinitionPrivate::HbIconAnimationDefinitionPrivate(bool nullInstance) :
    QSharedData(),
    frameList(),
    playMode(HbIconAnimationDefinition::PlayOnce)
{
    Q_UNUSED(nullInstance)
    // Need to ref here so null instance does not get destroyed
    ref.ref();
}

/*!
\internal
*/
HbIconAnimationDefinitionPrivate::~HbIconAnimationDefinitionPrivate()
{
    frameList.clear();
}

/*!
\internal
*/
HbIconAnimationDefinitionPrivate::HbIconAnimationDefinitionPrivate(
    const HbIconAnimationDefinitionPrivate &other) :
    QSharedData(other),
    frameList(other.frameList),
    playMode(other.playMode)
{
}

/*!
    @stable
    @hbcore
    \class HbIconAnimationDefinition
    \brief HbIconAnimationDefinition contains animation definition for an animated icon.

    Global definitions are managed by theme server.
    Definitions added in an application are local to the application.
*/

/*!
  \enum HbIconAnimationDefinition::PlayMode

  This enum defines the play mode for the animation.
 */

/*!
  \var HbIconAnimationDefinition::PlayOnce

  The animation is played once.
 */

/*!
  \var HbIconAnimationDefinition::Loop

  The animation is repeated from the beginning when the end is reached, until it is
  stopped manually.
*/

/*!
  \struct HbIconAnimationDefinition::AnimationFrame

  \brief Stores the data for one frame of the animation.
*/

/*!
  \var HbIconAnimationDefinition::AnimationFrame::iconName

  Name of the icon.
 */

/*!
  \var HbIconAnimationDefinition::AnimationFrame::duration

  Duration in milliseconds defining how long the frame is shown.
 */

/*!
  \var HbIconAnimationDefinition::AnimationFrame::jumps

  Contains the list of jumps that need to be performed after showing this frame for the
  given duration.  For a simple animation that has no local loops (meaning that no frames
  are repeated more than once) this list will be empty. If there are multiple jumps
  associated with a frame then they will performed in their order in this list, e.g. the
  second jump will be performed when the first has already been executed for the specified
  number of times.
 */

/*!
  \struct HbIconAnimationDefinition::AnimationFrame::Jump

  \brief Stores the information for one jump associated with a frame.

  Jumps are used to implement local loops (where some frames are repeated for a number of
  times).  For example the animation xml parser translates loop elements into these
  lightweight jump instructions so the animation player can play such animations
  correctly.
*/

/*!
  \var HbIconAnimationDefinition::AnimationFrame::Jump::targetFrameIndex

  Index of the target frame of the jump.
 */

/*!
  \var HbIconAnimationDefinition::AnimationFrame::Jump::repeatCount

  Defines how many times the jump should be executed.
 */

/*!
    Creates an empty animation definition.
*/
HbIconAnimationDefinition::HbIconAnimationDefinition() : d(&shared_null)
{
}

/*!
    Destroys the animation definition.
*/
HbIconAnimationDefinition::~HbIconAnimationDefinition()
{
}

/*!
    Creates a copy of the /a other animation definition.
    This is just a shallow copy, a deep copy of the data is done
    if the instance is modified.
*/
HbIconAnimationDefinition::HbIconAnimationDefinition(
    const HbIconAnimationDefinition &other) : d(other.d)
{
}

/*!
    Assigns the /a other animation definition to this animation definition.
    This is just a shallow copy, a deep copy of the data is done
    if the instance is modified.
*/
HbIconAnimationDefinition &HbIconAnimationDefinition::operator=(
    const HbIconAnimationDefinition &other)
{
    if (&other != this) {
        d = other.d;
    }
    return *this;
}

/*!
    Returns true if the animation definition is an empty default constructed instance.
    Note that even if isNull() returns false, the animation frame list of the definition may be empty.
*/
bool HbIconAnimationDefinition::isNull() const
{
    return d.constData() == &shared_null;
}

/*!
    Returns the animation frame list. It contains information of the icon files for the animation
    frames and their durations.
*/
QList<HbIconAnimationDefinition::AnimationFrame> HbIconAnimationDefinition::frameList() const
{
    return d->frameList;
}

/*!
    Sets the animation frame list in the animation definition.
*/
void HbIconAnimationDefinition::setFrameList(const QList<HbIconAnimationDefinition::AnimationFrame> &list)
{
    d.detach();
    d->frameList = list;
}

/*!
    Returns the play mode of the animation.
*/
HbIconAnimationDefinition::PlayMode HbIconAnimationDefinition::playMode() const
{
    return d->playMode;
}

/*!
    Sets the play mode of the animation.
*/
void HbIconAnimationDefinition::setPlayMode(PlayMode mode)
{
    if (mode != d->playMode) {
        d.detach();
        d->playMode = mode;
    }
}


// End of File
