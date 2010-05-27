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

#ifndef HBWSINIPARSER_H
#define HBWSINIPARSER_H

#include <QMap>

#include "hbscreenmode_p.h"

class HbWsiniParser
    {
public:
    static void parseModes(QMap<int, HbScreenMode> &modes, const QString &wsIniFile);

private:
    HbWsiniParser(QMap<int, HbScreenMode> &modes);

    void parseFile(const QString &filename);
    void parseLine(const QString &line);
    void parseTokens(const QString &keyword, const QStringList &line);

private:
    struct ParserEntry
        {
        const char* iKeyword;
        void (*iFunc)(HbWsiniParser* self, const QStringList &line);
        };
    static const ParserEntry mParseTable[];

private:

#define WSINI_PARSE_FUNC(name) static void call_##name(HbWsiniParser* self, const QStringList& line) \
    { self->name(line); } void name(const QStringList &line);
    
    WSINI_PARSE_FUNC(hardwareStateKeycode)
    WSINI_PARSE_FUNC(hardwareStateScreenMode)
    WSINI_PARSE_FUNC(hardwareStateAltScreenMode)
    WSINI_PARSE_FUNC(screenModeSoftkeyLocation)
    WSINI_PARSE_FUNC(screenModeStyleName)
    WSINI_PARSE_FUNC(screenWidth)
    WSINI_PARSE_FUNC(screenHeight)

private: 
    void ensureModeExists(int stateNumber);
    
private:
    QMap<int, HbScreenMode> &mDeviceModes;
    };

#endif // HBWSINIPARSER_H



