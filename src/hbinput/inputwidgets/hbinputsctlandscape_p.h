/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbInput module of the UI Extensions for Mobile.
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

#ifndef HB_INPUT_SCT_LANDSCAPE_PRIVATE_H
#define HB_INPUT_SCT_LANDSCAPE_PRIVATE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Hb Inputs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <hbinputsctlandscape.h>
#include "hbinputvkbwidget_p.h"

class HbCharPreviewPane;

class HB_INPUT_PRIVATE_EXPORT HbInputSctLandscapePrivate : public HbInputVkbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbInputSctLandscape)

public:
    HbInputSctLandscapePrivate();
    void createSctButtons();
    void setLayoutDimensions(QSizeF dimensions);
    void setActiveView(HbInputVkbWidget::HbSctView view);

    void handleStandardButtonPress(int aButtonId);
    void handleStandardButtonRelease(int aButtonId);
    void handleStandardButtonClick(int aButtonId);
    int keyCode(int buttonId);

    void getSpecialCharacters();

    void setSctButtons(const QString& characters);
    void setAsFunctionButton(int index, const HbIcon &icon, const QString &text);
    void latchRangeButton(int buttonId);

    // private slots
    void _q_enteredInNonStickyRegion();
    void _q_mappedKeyClick(int buttonid);

    void applyEditorConstraints();

public:
    QList<HbTouchKeypadButton *> mSctButtons;
    QString mSpecialCharacterSet;
    QList<HbIcon> mSmilies;
    int mStartIndex;
    int mCurrentPage;
    HbInputSctLandscape::HbSctView mActiveView;
    HbCharPreviewPane *mPreviewPane;
    QSignalMapper *mClickMapper;
    QSizeF mSize;
};

#endif //HB_INPUT_SCT_LANDSCAPE_PRIVATE_H

// End of file
