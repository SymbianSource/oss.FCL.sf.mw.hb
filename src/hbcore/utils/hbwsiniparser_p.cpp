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

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QMap>

#include "hbwsiniparser_p.h"

// ======== MEMBER FUNCTIONS ========

#define WSINI_PARSE_ENTRY(keyword, func) { keyword, &HbWsiniParser::call_##func }
const HbWsiniParser::ParserEntry HbWsiniParser::mParseTable[] = 
    {
    WSINI_PARSE_ENTRY("S60_HWSTATE_KEYCODE", hardwareStateKeycode),        //S60_HWSTATE_KEYCODEn <KeyCode>
    WSINI_PARSE_ENTRY("S60_HWSTATE_SCREENMODE", hardwareStateScreenMode),      //S60_HWSTATE_SCREENMODEn <ScreenModeNumber>
    WSINI_PARSE_ENTRY("S60_HWSTATE_ALT_SCREENMODE", hardwareStateAltScreenMode),       //S60_HWSTATE_ALT_SCREENMODEn <ScreenModeNumber>
    WSINI_PARSE_ENTRY("S60_SCR_SOFTKEY_LOCATION", screenModeSoftkeyLocation),      //S60_SCR_SOFTKEY_LOCATIONn <Location>
    WSINI_PARSE_ENTRY("S60_SCR_STYLE_NAME", screenModeStyleName),      //S60_SCR_STYLE_NAMEn <ScreenStyleName>
    WSINI_PARSE_ENTRY("SCR_WIDTH", screenWidth),      //SCR_WIDTHn <ScreenStyleName>
    WSINI_PARSE_ENTRY("SCR_HEIGHT", screenHeight)      //SCR_HEIGHTn <ScreenStyleName>
    };   
    
/*!
    Parses the wsini file and puts the found screen modes into the referenced map
    
    On non-Symbian OSes will normally insert no screen modes. If, on Windows, 
    a wsini file is put in the directory c:\hb\data, then that wsini will 
    simulate the behavior on a Symbian device.
*/
void HbWsiniParser::parseModes(QMap<int, HbScreenMode> &modes)
{
    HbWsiniParser parser(modes);
#if defined(Q_WS_S60)
    parser.parseFile("z:\\system\\data\\wsini.ini");
#elif defined(Q_OS_WIN32)
    parser.parseFile("c:/hb/data/wsini.ini"); 
#endif 
}

HbWsiniParser::HbWsiniParser(QMap<int, HbScreenMode> &modes)
    : mDeviceModes(modes)
{
}

void HbWsiniParser::parseFile(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    
    // Wsini.ini file is always UTF-16.  Use QTextStream and set the codec
    QTextStream input(&file);
    input.setCodec("UTF-16");
    
    while (!input.atEnd()) {
        QString line = input.readLine();
        // Is it all whitespace?
        QString trimmedLine = line.trimmed();
        if (!trimmedLine.isEmpty()){
            parseLine(trimmedLine);
        }
    }
  
    return;
}

void HbWsiniParser::parseLine(const QString &line)
{
    // Split at whitespace
    QStringList tokens = line.split(QRegExp("\\s+"));
    
    // strip off square brackets from first token
    QString first = tokens[0].remove(QRegExp("[^A-Z0-9_]"));
    
    if (!first.isEmpty()) {
        QString number;
        // Keywords have screen mode number appended to them. Need to separate
        // the number part from the end
        int pos = first.lastIndexOf( QRegExp("[A-Z_]") );
        int lastCharacter = first.length()-1;
        if (pos >= 0 && pos < lastCharacter) {
            int numDigits = lastCharacter - pos;
            number = first.right(numDigits);
            first.chop(numDigits);
        }
        
        if (!number.isEmpty()){
            tokens.replace(0, number); // Number becomes first token
        }
        else{
            tokens.removeAt(0); // remove the keyword from the tokens
        }
        parseTokens(first, tokens);
    }
}

void HbWsiniParser::parseTokens(const QString &keyword, const QStringList &line)
    {
    if (keyword.isEmpty())
        return;
    
    int targetCount = sizeof(mParseTable)/sizeof(ParserEntry);
    for (int ii=0; ii<targetCount; ii++)
        {
        const ParserEntry& parseEntry = mParseTable[ii];
        QString header(parseEntry.iKeyword);
        if (keyword == header)
            {
            (*parseEntry.iFunc)(this, line);
            break;
            }
        }
    }

/*!
    Parser method for screen width.
    Implementation is not refactored to help show correspondence between parsed content 
    and the parsing steps.
*/
void HbWsiniParser::screenWidth(const QStringList &line)
{
    bool convertedOk;
    int stateNumber = line[0].toInt(&convertedOk);
    ensureModeExists(stateNumber);
    int width = line[1].toInt(&convertedOk);
    mDeviceModes[stateNumber].setPixelWidth(width);
}

/*!
    Parser method for screen height.
    Implementation is not refactored to help show correspondence between parsed content 
    and the parsing steps.
*/
void HbWsiniParser::screenHeight(const QStringList &line)
{   
    bool convertedOk;
    int stateNumber = line[0].toInt(&convertedOk);
    ensureModeExists(stateNumber);
    int height = line[1].toInt(&convertedOk);
    mDeviceModes[stateNumber].setPixelHeight(height);
}

/*!
    Parses a line containing the screen mode name
*/
void HbWsiniParser::screenModeStyleName(const QStringList &line)
{   
    bool convertedOk;
    int stateNumber = line[0].toInt(&convertedOk);
    ensureModeExists(stateNumber);
    mDeviceModes[stateNumber].setName(line[1]);
}

/*!
    Stub parser for currently unparsed lines
*/
void HbWsiniParser::hardwareStateKeycode(const QStringList &line)
{
    Q_UNUSED(line);
}

/*!
    Stub parser for currently unparsed lines
*/
void HbWsiniParser::hardwareStateScreenMode(const QStringList &line)
{
    Q_UNUSED(line);
}

/*!
    Stub parser for currently unparsed lines
*/
void HbWsiniParser::hardwareStateAltScreenMode(const QStringList &line)
{   
    Q_UNUSED(line);
}

/*!
    Stub parser for currently unparsed lines
*/
void HbWsiniParser::screenModeSoftkeyLocation(const QStringList &line)
{
    Q_UNUSED(line);
}


/*!
    Creates a new mode if necessary
*/
void HbWsiniParser::ensureModeExists(int stateNumber)
{
    if (!mDeviceModes.contains(stateNumber)) {
        HbScreenMode mode;
        HbScreenMode &m(mode);
        int s(stateNumber);
        mDeviceModes.insert( s, m ); 
    }
}

