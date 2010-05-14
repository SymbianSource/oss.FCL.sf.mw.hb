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

#ifndef HBFEEDBACKNAMESPACE_H
#define HBFEEDBACKNAMESPACE_H

#include <QtGlobal>
#include <QObject>
#include <hbglobal.h>

#ifndef HB_DOXYGEN
class HB_CORE_EXPORT HbFeedback
{
    Q_GADGET
    Q_ENUMS(Type InstantEffect ContinuousEffect IntensityLevel Modality)
    Q_FLAGS(Types Modalities)

public:
#else
namespace HbFeedback
{
#endif // HB_DOXYGEN

    enum Type {
        TypeInstant = 0x001,
        TypeContinuous = 0x002
    };

    Q_DECLARE_FLAGS(Types, Type)

    enum InstantEffect {
        None,
        Basic, // For generic instant feedback for custom widgets
        Sensitive, // For generic instant feedback for custom widgets
        BasicButton,
        SensitiveButton,
        BasicKeypad,
        SensitiveKeypad,
        BasicSlider,
        SensitiveSlider,
        BasicItem,
        SensitiveItem,
        ItemScroll,
        ItemPick,
        ItemDrop,
        ItemMoveOver,
        BounceEffect,
        Checkbox,
        MultipleCheckbox,
        Editor,
        TextSelection,
        BlankSelection,
        LineSelection,
        EmptyLineSelection,
        PopUp,
        PopupOpen,
        PopupClose,
        Flick,
        StopFlick,
        MultitouchActivate,
        RotateStep,
        LongPress,
        PositiveTacticon,
        NeutralTacticon,
        NegativeTacticon,
        /* new standard instant feedbacks here */
        NumberOfInstantFeedbacks,
        InstantUser = 65535,
        /* user defined custom instant feedbacks */
        InstantMaxUser = 262140
    };

    enum ContinuousEffect {
        ContinuousNone,
        ContinuousSmooth, // For generic continuous feedback for custom widgets
        ContinuousSlider,
        ContinuousPopup,
        ContinuousInput,
        ContinuousPinch,
        /* new standard continuous feedbacks here */
        NumberOfContinuousFeedbacks,
        ContinuousUser = 1000,
        /* user defined continuous instant feedbacks */
        ContinuousMaxUser = 65535
    };

    enum IntensityLevel {
        IntensityZero = 0,
        IntensitySmooth = 50,
        IntensityFull = 100
    };

    enum Modality {
        All     = 0xFFFF,
        Audio   = 0x0001,
        Tactile = 0x0002
    };

    Q_DECLARE_FLAGS(Modalities, Modality)

    static const int StandardFeedbackTimeout = 300;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(HbFeedback::Types)
Q_DECLARE_OPERATORS_FOR_FLAGS(HbFeedback::Modalities)

#endif // HBFEEDBACKNAMESPACE_H
