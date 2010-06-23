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

#ifndef HB_INPUT_CANDIDATE_LIST_H
#define HB_INPUT_CANDIDATE_LIST_H

#include <hbdialog.h>

#include <hbinputdef.h>

class HbInputMethod;
class HbCandidateListPrivate;
class HbListWidgetItem;
class HbVkbHost;
class HbInputMethod;

class HB_INPUT_EXPORT HbCandidateList : public HbDialog
{
    Q_OBJECT

public:
    explicit HbCandidateList(HbInputMethod *input, QGraphicsItem *parent = 0);
    virtual ~HbCandidateList();

    void populateList(const QStringList &candidates, bool addSpellQuery = false);
    QString currentCandidate();
    void setNumberOfVisibleLines(int numLines);
    bool setSizeAndPositionForAutoCompletion(HbVkbHost *vkbHost);

signals:
    void candidatePopupCancelled();
    void candidateSelected(int key, const QString &candidate);
    void launchSpellQueryDialog();

protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void hideEvent(QHideEvent *event);
    void updatePrimitives();

public slots:
    void itemActivated(HbListWidgetItem *item);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbCandidateList)
    Q_DISABLE_COPY(HbCandidateList)
};

#endif // HB_INPUT_CANDIDATE_LIST_H

// End of file
