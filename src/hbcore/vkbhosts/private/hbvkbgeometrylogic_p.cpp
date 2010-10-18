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

#include "hbvkbgeometrylogic_p.h"

#include <QtGlobal>
#include <hbglobal.h>

/*!
    \class HbVkbGeometryLogicPrivate
    \brief Calculates screen movement in cases the keyboard would overlap with editor

    This class contains calculations and logic to move screen in cases, when upcoming
    keyboard would overlap the editor so, that text written could not be seen. Idea is
    to minimize the movement as much as possible.

*/


/*!
    \internal
    \brief Construct current state object screen status.

    Constructor to create status object based on given parameters. These parameters
    are used to calculate possible movement vector for container.

    \param screenSize       Size of screen.
    \param keybadSize       Size of keyboard.
    \param sceneArea        Size of view area, to which container belongs.
    \param isVkbOpen        Information about current status of VKB.
    \param hideTitlebar     Should titlebar hiding be calculated?
    \param hideStatusbar    Should statusbar hiding be calculated?
    \param containerArea    Area of editor's top item.
    \param editorArea       Area of editor itself.
    \param cursorArea       Area of cursor inside editor.
*/
HbVkbGeometryLogicPrivate::HbVkbGeometryLogicPrivate(
        const QSizeF& screenSize,
        const QSizeF& keypadSize,
        const QRectF& sceneArea,
        bool isPopupType,
        bool isVkbOpen,
        const QRectF& containerArea,
        const QRectF& editorArea,
        const QRectF& cursorArea,
        qreal margin)
            :
        mIsPopupType(isPopupType),
        mMargin(margin)
{
    // We need to consider situation, when keyboard is already on the screen, in
    // which case, titlebar and statusbar are already hidden, thus bigger visible area
    // is already in use, so no adjustments needed. Popups can always use full visible
    // area.
    if ( isVkbOpen || isPopupType ) {
        mVisibleArea = QRectF(0.0, 0.0, screenSize.width(), screenSize.height() - keypadSize.height());
        mAdjust = 0.0;
    } else {
        // When titlebar and statusbar are visible, visible area is going to be from
        // bottom of the titlebar to top of the keyboard. No container movement needed.
        mVisibleArea = QRectF(0.0, 0.0, screenSize.width(), sceneArea.height() - keypadSize.height());
        mAdjust = 0.0;
    }

    // Find out the container area. No margin adjustments for popups.
    mContainerArea = containerArea;
    if (isPopupType) {
        mContainerArea.adjust(0.0, -mMargin, 0.0, mMargin);
    }
    mContainerArea.translate(QPointF(0, mAdjust));

    // Find out the editor bounding box and add a small margin to height.
    mEditorArea = editorArea;
    mEditorArea.adjust(0.0, -mMargin, 0.0, mMargin);
    mEditorArea.translate(QPointF(0, mAdjust));

    // Finally, get cursor size and adjust it little bit
    mCursorArea = cursorArea;
    mCursorArea.adjust(0.0, -mMargin, 0.0, mMargin);
    mCursorArea.translate(QPointF(0, mAdjust));
}


/*!
    \internal
    \brief Check the source area fits inside target area.
*/
bool HbVkbGeometryLogicPrivate::minimunMovement(QPointF& vector) const
{
    vector.rx() = 0.0;
    vector.ry() = mAdjust;
    return false;
}


/*!
    \internal
    \brief Calculates vector to move given area to visible area.
    \param areaToMove contains rectangle, which needs to be moved to visible area.

    When moving editor to visible area, we need to consider situation, where
    margins of the editor border may make editor look like it is outside of the
    container. So every check we made, we need to compare against editor border
    and container border.

    \return Vector to visible area.
*/
bool HbVkbGeometryLogicPrivate::calculateVectorToVisibleArea(QPointF& vector, const QRectF& areaToMove) const
{
    qreal aTop = areaToMove.top();
    qreal vTop = mVisibleArea.top();
    qreal cTop = mContainerArea.top();

    qreal aBottom = areaToMove.bottom();
    qreal vBottom = mVisibleArea.bottom();
    qreal cBottom = mContainerArea.bottom();

    QRectF areaOfInterest = areaToMove;

    // To simplify everything, first check, whether there is need to inspect
    // container movement issues or not.
    if ( !mContainerArea.contains(areaOfInterest) ) {
        // Now we know, that area we are supposed to move, cannot be used
        // in situations, when too close to borders and movement direction
        // tells us to move container.
        bool shouldMoveUp = aBottom > vBottom;
        bool areaBelowContainer = aBottom > cBottom;
        bool shouldMoveDown = aTop < vTop;
        bool areaAboveContainer = aTop < cTop;

        if ( ( areaBelowContainer && shouldMoveUp ) ||
             ( areaAboveContainer && shouldMoveDown ) ) {
            areaOfInterest = mContainerArea;
        }
    }

    // Area is inside container. Before anything, check if we
    // can move the container at least little bit. But it cannot be moved
    // too low.
    if ( areaOfInterest != mContainerArea && aTop < vTop ) {
        qreal upward = aTop - mAdjust;

        if ( upward < 0 ) {
            vector.ry() -= upward;
            return true;
        } else {
            return false;
        }

    // Vector calculation is simple, just figure out direction.
    } else if ( aTop < vTop ) {
        vector = QPointF(0.0, -aTop);
        return true;

    } else {
        vector = QPointF(0.0, vBottom - areaOfInterest.bottom());
        return true;
    }
}


/*!
    \internal
    \brief Check the source area fits inside target area.
*/
bool HbVkbGeometryLogicPrivate::fitsArea(const QRectF& target, const QRectF& source) const
{
    return source.width() <= target.width() && source.height() <= target.height();
}


/*!
    \internal
    \brief Checks, whether the container fits into the visible area.

    When keyboard opens, the screen will contain visible area and the keyboard area.
    This method checks, whether container will fit the visible area.

    \return True, when fits. Otherwise false.
*/
bool HbVkbGeometryLogicPrivate::containerFitsVisibleArea() const
{
     return fitsArea(mVisibleArea, mContainerArea);
}


/*!
    \internal
    \brief Checks, whether the editor fits into the visible area.

    When keyboard opens, the screen will contain visible area and the keyboard area.
    This method check, whether editor itself can be fitted to screen. This is needed
    when the container cannot fit the screen, so the editor needs to be positioned.

    \return
*/
bool HbVkbGeometryLogicPrivate::editorFitsVisibleArea() const
{
    return fitsArea(mVisibleArea, mEditorArea);
}


/*!
    \internal
    \brief Check if container is fully visible.

    \return True, when fully inside visible area, otherwise false.
*/
bool HbVkbGeometryLogicPrivate::isContainerVisible() const
{
    return mVisibleArea.contains(mContainerArea);
}


/*!
    \internal
    \return
*/
bool HbVkbGeometryLogicPrivate::isEditorVisible() const
{
    return mVisibleArea.contains(mEditorArea);
}


/*!
    \internal
    \return
*/
bool HbVkbGeometryLogicPrivate::isCursorVisible() const
{
    // Check whether cursor inside the visible area.
    return mVisibleArea.contains(mCursorArea);
}


/*!
    \internal
    \return
*/
bool HbVkbGeometryLogicPrivate::calculateContainerMovement(QPointF& vector) const
{
    // Clear any data away from the vector
    vector = QPointF(0, 0);

    // First check against invalid case. If cursor is not inside container, the
    // data provided is incorrect.
    if ( !mContainerArea.contains(mCursorArea) ) {
        return false;

    } else if ( containerFitsVisibleArea() ) {
        return calculateContainerVector(vector);

    } else if ( mIsPopupType ) {
        return calculatePopupVector(vector);

    // In case editor or cursor inside visible area, no extra movement needed.
    } else if ( isCursorVisible() ) {
        return minimunMovement(vector);

    // At this point we know, that cursor is not inside of visible area,
    // after VKB has been shown. To make it bit prettier, let's check, if we can
    // move and fit the whole editor into the screen at once.
    } else if ( !isEditorVisible() && editorFitsVisibleArea() ) {
        return calculateEditorVector(vector);


    // At this point we know, that cursor is not visible and the editor does not fit
    // into the visible area. Here we need to move editor, so that the cursor can be
    // seen in the visible area. There are two ways to do this.
    // 1) Move container, until bottom of editor is reached OR
    // 2) Move container, until cursor hits top of the screen
    } else {
        int cursorMove = static_cast<int>(mVisibleArea.top() - mCursorArea.top());
        int editorMove = static_cast<int>(mVisibleArea.bottom() - mEditorArea.bottom());

        // Choose smaller movement (notice usage of negative values)
        vector = QPointF(0.0, cursorMove >= editorMove ? cursorMove : editorMove);

        vector.ry() += mAdjust;
        return true;
    }
}


/*!
    \internal
    \return
*/
bool HbVkbGeometryLogicPrivate::calculateContainerVector(QPointF& vector) const
{
    if ( isContainerVisible() ) {
        return minimunMovement(vector);
    }

    bool result = calculateVectorToVisibleArea(vector, mContainerArea);
    vector.ry() += mAdjust;
    return result;
}


/*!
    \internal
    \brief Calculate vector to adjust popup location properly.

    Goal of these calculations are to guarrantee, that the buttons are always
    visible for user, so that the popup can be closed without closing VKB first.
    This is achieved by trying to move popup bottom to edge of visible area,
    and then check, whether the editor is still seen or not. If not, then adjust
    the editor so, that it can be seen.

    \return True, when movement needed
*/
bool HbVkbGeometryLogicPrivate::calculatePopupVector(QPointF& vector) const
{
    qreal mBottom = mContainerArea.bottom();
    qreal vBottom = mVisibleArea.bottom();
    vector.ry() += vBottom - mBottom;

    QRectF newEditorPos = mEditorArea.translated(vector);
    if (!mVisibleArea.contains(newEditorPos)) {
        QPointF temp(0,0);
        calculateVectorToVisibleArea(temp, newEditorPos);
        vector += temp;
    }

    vector.ry() += mAdjust;

    return true;
}


/*!
    \internal
    \return
*/
bool HbVkbGeometryLogicPrivate::calculateEditorVector(QPointF& vector) const
{
    bool result = calculateVectorToVisibleArea(vector, mEditorArea);
    vector.ry() += mAdjust;

    return result;
}

