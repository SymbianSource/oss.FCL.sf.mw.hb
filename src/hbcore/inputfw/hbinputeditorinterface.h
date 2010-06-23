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

#ifndef HB_INPUT_EDITOR_INTERFACE_H
#define HB_INPUT_EDITOR_INTERFACE_H

#include <QObject>

#include <hbinputdef.h>
#include <hbinputfilter.h>
#include <hbinputlanguage.h>
#include <hbinputstate.h>
#include <hbsmileytheme.h>

class HbEditorInterfacePrivate;
class HbInputFilter;
class HbAction;
class HbVkbHost;

class HB_CORE_EXPORT HbEditorInterface : public QObject
{
    Q_OBJECT

public:
    HbEditorInterface(QObject *editor);
    ~HbEditorInterface();

    HbTextCase textCase() const;
    void setTextCase(HbTextCase textCase);
    HbInputModeType mode() const;
    void setMode(HbInputModeType inputMode);
    HbEditorConstraints inputConstraints() const;
    void setInputConstraints(HbEditorConstraints constraints);
    HbInputFilter *filter() const;
    void setFilter(HbInputFilter *filter);
    HbInputDigitType digitType() const;
    void setDigitType(HbInputDigitType digitType);
    void addAction(HbAction *action);
    void insertAction(HbAction *before, HbAction *action);
    void removeAction(HbAction *action);
    QList<HbAction *> actions() const;
    int extraDictionaryId() const;
    void setExtraDictionaryId(int id);
    HbInputEditorClass editorClass() const;
    void setEditorClass(HbInputEditorClass editorClass);
    HbSmileyTheme smileyTheme() const;
    void setSmileyTheme(const HbSmileyTheme &theme);
    void lastFocusedState(HbInputState &result) const;
    void setLastFocusedState(const HbInputState &state);

    bool operator==(const HbEditorInterface &editorInterface) const;
    bool operator!=(const HbEditorInterface &editorInterface) const;
    QObject *editor() const;
    HbVkbHost *vkbHost() const;

public:  // Convenience methods for setting up standard editor types.
    void setUpAsCompletingEmailField();
    void setUpAsCompletingUrlField();
    void setUpAsLatinAlphabetOnlyEditor();
    bool isNumericEditor() const;
    bool isPredictionAllowed() const;

public:
    static bool isConnected(QObject *object);

signals:
    void modified();
    void cursorPositionChanged(int oldPos, int newPos);

public slots:
    void backendModified();
    void notifyCursorPositionChange(int oldPos = 0, int newPos = 0);

private slots:
    void backendDestroyed(QObject *obj);

protected:
    HbEditorInterfacePrivate *mPrivate;

    // Prevent copying
    Q_DISABLE_COPY(HbEditorInterface)
};

#endif // HB_INPUT_EDITOR_INTERFACE_H

// End of file

