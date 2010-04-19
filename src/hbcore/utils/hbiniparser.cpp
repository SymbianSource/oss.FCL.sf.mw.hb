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

#include "hbiniparser_p.h"
#include <QByteArray>
#include <QIODevice>
#include <QtDebug>
#include <QString>
#include <QStringList>

HbIniParser::HbIniParser()
{
}

HbIniParser::~HbIniParser()
{
}
    
/*
    The main function that populates the map data.
    currently the data is case sensitive and all api behave that way.
*/
bool HbIniParser::read(QIODevice *file)
{
    if(!file->isOpen())
        return false;
    
    QByteArray line;
    
    HbIniGroup groupData;
    QString groupName;
    
    while(!file->atEnd()) {        
        line=file->readLine().trimmed();
         if (line.isEmpty() || line.at(0) == '#') {
            continue; 
        }
        
        if (line.at(0) == '[') { // found a group
            //add old group data
            if(!groupName.isEmpty()) {
                mData.insert(groupName,groupData);
                groupData.clear();
            }
                
            groupName = line.mid (1, line.indexOf(']') - 1);
            if (groupName.isEmpty()) {
                return false; //error in file
            }         
        }
        else {
            QByteArray key,value;
            int equalPosition = line.indexOf('=');
            if (equalPosition > 0) {
                key = line.left (equalPosition).trimmed();
                line.remove(0,equalPosition+1);
                value = line.trimmed();
                groupData.insert(key,value);
            }
        }                
    }
    if(!groupName.isEmpty()) {
        mData.insert(groupName,groupData);
        groupData.clear();
    }
    return true;
}
    
bool HbIniParser::setCurrentGroup(const QString &name)
{
    if (mData.contains(name)) {
        mCurrentGroup = name;
        return true;
    }
    return false;
}

QString HbIniParser::currentGroup()
{
    return mCurrentGroup;
}
    
const QString HbIniParser::value(const QString &groupName,const QString &key) const
{
    if(!mData.contains(groupName)){
        return QString();
    }
    return mData.value(groupName).value(key);
}

const QString HbIniParser::value(const QString &key) const
{
    if(mCurrentGroup.isEmpty()) {
        return QString();
    }
    return mData.value(mCurrentGroup).value(key);
}   

QStringList HbIniParser::groups() const
{
    return mData.keys();
}
