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

#ifndef HBINPUTKEYMAPFACTORY_P_H
#define HBINPUTKEYMAPFACTORY_P_H

#include <QList>
#include <hbglobal.h>

class QStringList;
class QFile;
class QTextStream;
class HbKeymap;
class HbInputLanguage;

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

/// @cond

class HB_AUTOTEST_EXPORT HbKeymapFactoryPrivate
{
public:
    HbKeymapFactoryPrivate();
    ~HbKeymapFactoryPrivate();
    bool findKeymapFile(const HbInputLanguage &language, const QStringList &searchPaths, QFile &file) const;
    void parseFileNames(const QStringList &files, QList<HbInputLanguage> &languages) const;
    int keymapVersion(QFile &file) const;
    HbKeymap *parseKeymapDefinition(const HbInputLanguage &language, QTextStream &stream) const;
    bool isValid(const HbKeymap *keymap) const;
    HbKeymap *keymap(const HbInputLanguage &language) const;
public:
    QList<HbKeymap *> mKeymaps;
    QList<HbInputLanguage> mRomLanguages;
};

/// @endcond

#endif // HBINPUTKEYMAPFACTORY_P_H
