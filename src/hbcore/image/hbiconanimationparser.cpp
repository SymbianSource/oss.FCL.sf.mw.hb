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

#include "hbiconanimationparser_p.h"
#include "hbiconanimationmanager_p.h"
#include "hbimagetraces_p.h"
#include "hbtheme_p.h"
#include "hbthemeclient_p.h"
#include "hbinstance.h"
#include <QDebug>
#include <QFile>
#include <QBuffer>
#include <QByteArray>
#include <QPair>
#include <QStack>

// Private class
HbIconAnimationParser::HbIconAnimationParser() :
    QXmlStreamReader()
{
}

HbIconAnimationParser::~HbIconAnimationParser()
{
}

bool HbIconAnimationParser::parseDefinitionFileShared(
    const QString &fileName, AnimHash &animations, const QString &realFileName)
{
#ifdef HB_ICON_TRACES
    qDebug() << "HbIconAnimationParser: Trying to get" << realFileName << "via themeserver";
#endif
    QByteArray arr = HbThemeClient::global()->getSharedBlob(realFileName);

    if (!arr.isEmpty()) {
#ifdef HB_ICON_TRACES
        qDebug() << "HbIconAnimationParser: Got data from themeserver for" << realFileName
                 << "size=" << arr.size();
#endif
        QBuffer buf(&arr);
        setDevice(&buf);
        return doParseFile(fileName, animations);
    }
#ifdef HB_ICON_TRACES
    qDebug() << "HbIconAnimationParser: Getting" << realFileName << "via themeserver failed";
#endif
    return false;
}

bool HbIconAnimationParser::parseDefinitionFile(
    const QString &fileName, AnimHash &animations, const QString &realFileName)
{
    QFile file(realFileName);

    if (!file.exists()) {
#ifdef HB_ICON_TRACES
        qDebug() << "HbIconAnimationParser: File does not exist:" << realFileName;
#endif
        return false;
    }

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
#ifdef HB_ICON_TRACES
        qDebug() << "HbIconAnimationParser: File opening failed:" << realFileName;
#endif
        return false;
    }

    setDevice(&file);
    bool ret = doParseFile(fileName, animations);
    file.close();

    return ret;
}

bool HbIconAnimationParser::doParseFile(
    const QString &fileName, AnimHash &animations)
{
    bool ret = false;

    while (!atEnd()) {
        readNext();

        if (isStartElement()) {
            if (name() == "animations") {
                ret = readAnimations(fileName, animations);
            }
        }
    }

#ifdef HB_ICON_TRACES
    qDebug() << "Parsing of" << *fileName << "finished with success code" << ret;
#endif

    return ret;
}

/*!
* Reads animation elements.
* Returns true if at least one animation definition was added in the hash.
*/
bool HbIconAnimationParser::readAnimations(
    const QString &fileName, AnimHash &animations)
{
    Q_ASSERT(isStartElement() && name() == "animations");

    bool ret = false;

    while (!atEnd()) {
        readNext();

        if (isEndElement()) {
            break;
        }

        if (isStartElement()) {
            if (name() == "icon") {
                HbIconAnimationData anim;
                anim.definitionFileName = fileName;

                // Read icon element
                QString iconName = readIconData(anim);

                if (!anim.def.isNull() && !iconName.isEmpty()) {
                    // Remove old definition to avoid duplicates
                    animations.remove(iconName);
                    // Insert the new animation definition in the hash
                    animations.insert(iconName, anim);
                    ret = true;
                }
            } else {
                readUnknownElement();
            }
        }
    }

    return ret;
}

/*!
 * Parses the icon data.
 */
QString HbIconAnimationParser::readIconData(HbIconAnimationData &data)
{
    Q_ASSERT(isStartElement() && name() == "icon");

    QString iconName;
    int defaultDuration = 0;
    QStack< QPair<int, int> > repStk;

    QXmlStreamAttributes attrs = attributes();

    // Get old frame list from definition. It should be empty at this point.
    QList<HbIconAnimationDefinition::AnimationFrame> frames = data.def.frameList();

    // Read icon attributes
    foreach(const QXmlStreamAttribute & attr, attrs) {
        // "name" = ...
        if (attr.name().toString() == "name") {
            iconName = attr.value().toString();
        } else if (attr.name().toString() == "playmode") {
            // Defaults to "playonce"
            if (attr.value().toString() == "loop") {
                data.def.setPlayMode(HbIconAnimationDefinition::Loop);
            }
        } else if (attr.name().toString() == "frame_duration") {
            // E.g. frame_duration="500". This means that 500 is used as the duration for
            // all frames for which the duration is not explicitly given.
            defaultDuration = attr.value().toString().toInt();
        }
    }

    // Read frame elements inside icon element.
    while (!atEnd()) {
        readNext();

        if (isEndElement()) {
            // Check if reached </icon>
            if (name() == "icon") {
                break;
            }
            // Check if reached </loop>
            if (name() == "loop") {
                QPair<int, int> repData = repStk.pop();
                // Assign these values to the last frame of the loop that has just ended.
                if (!frames.isEmpty()) {
                    // Instead of a single index/count pair there is a list because one
                    // frame may contain multiple jumps (when multiple </loop> elements
                    // come directly after each other).
                    HbIconAnimationDefinition::AnimationFrame::Jump jump;
                    jump.targetFrameIndex = repData.first;
                    jump.repeatCount = repData.second;
                    frames.last().jumps.append(jump);
                }
            }
        }

        if (isStartElement()) {

            // Frame element inside icon or loop element.
            if (name() == "frame") {
                HbIconAnimationDefinition::AnimationFrame frame;
                frame.duration = defaultDuration;

                QXmlStreamAttributes attrs = attributes();
                // Read attributes
                foreach(const QXmlStreamAttribute & attr, attrs) {
                    // duration attribute
                    if (attr.name().toString() == "duration") {
                        frame.duration = attr.value().toString().toInt();
                        if (frame.duration < 0) {
                            frame.duration = 0;
                        }
                    }
                }

                frame.iconName = readElementText();
                frames.append(frame);

            } else if (name() == "loop") {
                // <loop count="2">
                //     <frame src=":/graphicsitemdemo/frame2.svg"/>
                //     ...
                // </loop>
                int repeatCount = 0;
                QXmlStreamAttributes attrs = attributes();
                foreach(const QXmlStreamAttribute & attr, attrs) {
                    if (attr.name().toString() == "count") {
                        repeatCount = attr.value().toString().toInt();
                    }
                }
                // If count attribute is not given or is <= 0 then the loop element is ignored.
                if (repeatCount > 0) {
                    // Target frame index for this loop's last frame will be the index of
                    // the frame that comes next.
                    repStk.push(QPair<int, int>(frames.count(), repeatCount));
                }

            } else {
                readUnknownElement();
            }
        }
    }

    // Set framelist in the definition.
    data.def.setFrameList(frames);

    return iconName;
}

/*!
    Handles unknown elements.
*/
void HbIconAnimationParser::readUnknownElement()
{
    Q_ASSERT(isStartElement());

    while (!atEnd()) {
        readNext();

        if (isEndElement()) {
            break;
        }

        if (isStartElement()) {
            readUnknownElement();
        }
    }
}

// End of File
