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

#include "hbeffectxmlparser_p.h"
#include "hbeffectdef_p.h"
#include <QString>
#include <QStringList>
#include <QXmlStreamAttribute>

/*
  \class HbEffectXmlParser

  \brief The HbEffectXmlParser class is responsible of parsing
  xml file containing event effect definitions.
  The startwidth, startheight, endwidth and endheight correspond to startx, starty, endx and endy respectively.

  \warning This class is a part of internal library implementation and may
  be removed from the public API!

  \internal
*/

/*
  Constructor.
*/
HbEffectXmlParser::HbEffectXmlParser()
    :mFxmlData(0)
{
}

/*
  Destructor.
*/
HbEffectXmlParser::~HbEffectXmlParser()
{
}

/*
  Parses fxml document from the specified io device.
*/
bool HbEffectXmlParser::read(QIODevice *device, HbEffectFxmlData *dst)
{
    setDevice(device);
    mFxmlData = dst;

    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == FXML_LAYERS) {
                readVisuals();
            } else {
                qWarning("HbEffectXmlParser: Document element is invalid (not <layers>");
                raiseError("HbEffectXmlParser::read The document is not an valid effect definitions document.");
            }
        }        
    }

    if (error()) {
        qWarning("HbEffectXmlParser: failed with %d (%s)", error(), qPrintable(errorString()));
    }

    return !error();
}

/*
  Handles unknown elements.
*/
void HbEffectXmlParser::readUnknownElement()
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

void HbEffectXmlParser::readBlendingElement()
{
    Q_ASSERT(isStartElement());

    while (!atEnd()) {
        readNext();

        // If there is no value in the blending element, bail out.
        if (isEndElement()) {
            break;
        }

        if (isCharacters()) {
            // Store current blending value
            mCurrentBlending = text().toString().trimmed();
            // Blending value was read, find end tag and return.
            while (!atEnd()) {
                readNext();

                if (isEndElement()) {
                    break;
                }
            }
            break;
        }
    }
}

// FXML support
void HbEffectXmlParser::readVisuals()
{
    Q_ASSERT(isStartElement() && name() == FXML_LAYERS);

    while (!atEnd()) {
        readNext();

        if (isEndElement()) {
            if (name() == FXML_LAYERS)
                break;
        }

        if (isStartElement()) {
            if (name() == FXML_VISUAL) {
                readVisualData();
            }

            else if (name() == FXML_LAYERGROUP) {
                // Not needed
            }
            else if (name() == FXML_BLENDING) {
                readBlendingElement();
            }
            else if (name() == FXML_COMMENT) {
                // Comments are skipped
            }
            else if (name() == FXML_FILTER) {
#ifdef HB_FILTER_EFFECTS
                readFilterData();
#endif
            }

            else {
                readUnknownElement();
            }
        }
    }
}

void HbEffectXmlParser::readVisualData()
{
    // Read all 'param' tags inside the 'visual' tags and append them in the fxml animation list

    while (!atEnd()) {
        readNext();

        if (isEndElement() && name() == FXML_VISUAL) {
            break;
        }

        if (isStartElement()) {
            if (name() == FXML_PARAM) {
                mFxmlData->appendParamData(readParamData());
            }
            else if (name() == FXML_FILTER) {
#ifdef HB_FILTER_EFFECTS
                readFilterData();
#endif
            }
            else {
                readUnknownElement();
            }
        }
    }
}
/**
* Reads all 'param' tags inside the 'filter' tag and creates a filter data structure,
* which is then appended to the fxmlData.
*/
void HbEffectXmlParser::readFilterData()
{
#ifdef HB_FILTER_EFFECTS
    // Create filter data structure
    HbEffectFxmlFilterData filterData(mFxmlData->memoryType());

    // Parse filter type
    QXmlStreamAttributes attrs = attributes();

    foreach (const QXmlStreamAttribute &attr, attrs) {
        // "type" = ...
        if (attr.name().toString() == FXML_PARAM_TYPE) {
            filterData.setType(attr.value().toString());
            break;
        }
        // Other attributes are skipped atm
    }

    while (!atEnd()) {
        readNext();

        if (isEndElement() && name() == FXML_FILTER) {
            break;
        }

        if (isStartElement()) {
            if (name() == FXML_PARAM) {
                filterData.appendParamData(readParamData());
            } else {
                readUnknownElement();
            }
        }
    }

    // Add blending data in the filter data
    filterData.setBlending(mCurrentBlending);

    // Append filter data in the list
    mFxmlData->appendFilterData(filterData);

#endif
}

// This parses information inside one <param> field.
// E.g. "scale_x", "scale_y", "scale_origin_x"
// 
HbEffectFxmlParamData HbEffectXmlParser::readParamData()
{
    Q_ASSERT(isStartElement() && name() == FXML_PARAM);
    
    HbEffectFxmlParamData param(mFxmlData->memoryType());
    HbKeyFrame kf(mFxmlData->memoryType());

    QXmlStreamAttributes attrs = attributes();

    // Populate the PARAM attributes
    foreach (const QXmlStreamAttribute &attr, attrs) {
        // "name" = ...
        if (attr.name().toString() == FXML_PARAM_NAME) {
            param.setName(attr.value().toString());
        }
        // "type" = ...
        else if (attr.name().toString() == FXML_PARAM_TYPE) {
            // Nothing to do here atm...
        }
        // Insert other elements to param data, e.g. "ref" is stored here
        else {
            param.setAttribute(attr.name().toString(), attr.value().toString());
        }
    }

    while (!atEnd()) {
        readNext();

        if (isEndElement() && name() == FXML_PARAM) {
            break;
        }

        // Elements inside params
        if (isStartElement()) {
            // <duration> tag
            if (name() == FXML_DURATION) {
                param.setDuration(readElementText());
            }
            // <style> tag
            else if (name() == FXML_STYLE) {
                param.setAttribute(FXML_STYLE, readElementText());
            }
            // <marker> tag
            else if (name() == FXML_MARKER) {
                QXmlStreamAttributes attrs = attributes();
                
                enum {
                    Undefined = 0,
                    Start,
                    End
                } loopType = Undefined;

                // Fetch "type" attribute from <marker> tag
                foreach (const QXmlStreamAttribute &attr, attrs) {
                    if (attr.name().toString() == FXML_PARAM_TYPE) {
                        QString s = attr.value().toString();
                        if (s == FXML_LOOP_START) {
                            loopType = Start;
                        } else if (s == FXML_LOOP_END) {
                            loopType = End;
                        }
                        break;
                    }
                }

                if (loopType != Undefined) {
                    // Fetch "at" attribute from <marker> tag
                    foreach (const QXmlStreamAttribute &attr, attrs) {
                        if (attr.name().toString() == FXML_PARAM_AT) {
                            QString s = attr.value().toString();
                            bool ok = false;
                            float f = s.toFloat(&ok);
                            if (ok) {
                                if (loopType == Start) {
                                    param.setLoopStart(f);
                                } else if (loopType == End) {
                                    param.setLoopEnd(f);
                                }
                            }
                        }
                        break;
                    }
                }

                // Advance until end element
                while (!atEnd()) {
                    if (readNext() == QXmlStreamReader::EndElement) {
                        break;
                    }
                }
            }

            // <keyframe> tag
            else if (name() == FXML_KEYFRAME) {
                QXmlStreamAttributes attrs = attributes();
                
                bool ok = false;

                // Fetch "at" attribute from <keyframe> tag
                foreach (const QXmlStreamAttribute &attr, attrs) {
                    if (attr.name().toString() == FXML_PARAM_AT) {
                        QString s = attr.value().toString();
                        kf.pos = s.toFloat(&ok);
                        break;
                    }
                }

                // Fetch value of the <keyframe> tag
                QString s = readElementText();
                if (ok) {
                    // Color value cannot be stored in float so store it as a string
                    if (param.name() == "color") {
                        kf.stringValue = s;
                    } else {
                        kf.val = s.toFloat(&ok);
                    }
                }

                // Append keyframe to the param data if values were ok.
                // Verify also here that the position is between 0.0 and 1.0.
                if (ok && kf.pos >= 0.0 && kf.pos <= 1.0) {
                    param.append(kf);
                }
            }
            //<start> element
            else if(name() == FXML_KEYWORD_START) {

                QXmlStreamAttributes attrs = attributes();
                foreach (const QXmlStreamAttribute &attr, attrs) {
                    if( attr.name() == FXML_PARAM_REF ) {
                        param.setStartRef(attr.value().toString());
                    }
                }
                param.setAttribute(FXML_KEYWORD_START, readElementText());
            }
            //<end> element
            else if(name() == FXML_KEYWORD_END) {                
                QXmlStreamAttributes attrs = attributes();
                foreach (const QXmlStreamAttribute &attr, attrs) {
                    if( attr.name() == FXML_PARAM_REF ) {
                        param.setEndRef(attr.value().toString());
                    }
                }
                param.setAttribute(FXML_KEYWORD_END, readElementText());
            }
            else {
                readUnknownElement();
            }
        }
        // Value of the param
        else if (isCharacters()) {
            param.setValue(text().toString().trimmed());
        }
    }

    return param;
}

// End of File
