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

#ifndef HB_INPUT_MODE_CACHE_H
#define HB_INPUT_MODE_CACHE_H

#include <QObject>

#include "hbinputdef.h"
#include "hbinputlanguage.h"
#include "hbinputstate.h"
#include "hbinputmethoddescriptor.h"

class HbInputModeCachePrivate;
class HbInputMethod;

class HB_CORE_PRIVATE_EXPORT HbInputModeCache : public QObject
{
    Q_OBJECT

public:
    static HbInputModeCache* instance();

private:
    HbInputModeCache();
    ~HbInputModeCache();

public:  
    HbInputMethod* loadInputMethod(const HbInputMethodDescriptor &inputMethod);
    QList<HbInputMethodDescriptor> listCustomInputMethods();
    HbInputMethod* findStateHandler(const HbInputState& state); 
    HbInputMethod* activeMethod() const;
    QList<HbInputLanguage> listInputLanguages() const;
    bool acceptsState(const HbInputMethod *inputMethod, const HbInputState &state) const;
    HbInputMethodDescriptor descriptor(const HbInputMethod *inputMethod) const;

public slots:
    void shutdown();
    void directoryChanged(const QString &directory);

private:
    HbInputModeCachePrivate * const d_ptr;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputModeCache)
    Q_DISABLE_COPY(HbInputModeCache)
};

#endif // HB_INPUT_MODE_CACHE_H

// End of file
