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

#ifndef HBXMLLOADERBASESYNTAX_P_H
#define HBXMLLOADERBASESYNTAX_P_H

#include "hbxmlloaderabstractactions_p.h"
#include "hbxmlloaderabstractsyntax_p.h"

#include <hbglobal.h>

#include <QHash>
#include <QList>
#include <QXmlStreamReader>
#include <QPointer>
#include <QByteArray>

class HbXmlLoaderAbstractActions;
class HbWidget;

class HB_CORE_PRIVATE_EXPORT HbXmlLoaderBaseSyntax : public HbXmlLoaderAbstractSyntax
{

public:
    enum TopState {
        TS_READ_DOCUMENT,
        TS_READ_METADATA,
        TS_ERROR,
        TS_EXIT
    };

    enum DocumentState {
        DS_START_DOCUMENT,
        DS_READ_SECTIONS,
        DS_END_DOCUMENT
    };

    enum ElementState {
        ES_GENERAL_ITEM,
        ES_LAYOUT_ITEM,
        ES_CONTAINER_ITEM
    };

public:

    HbXmlLoaderBaseSyntax( HbXmlLoaderAbstractActions *actions );
    virtual ~HbXmlLoaderBaseSyntax();

    virtual bool load( QIODevice *device, const QString &section );
    bool scanForSections( QIODevice *device, QList<QString> &sectionsList );

public:
    virtual bool processDocument();
    virtual bool processLayout();
    virtual bool processContainer();
    virtual bool checkEndElementCorrectness();

    virtual HbXml::ElementType elementType( QStringRef name ) const;

    virtual QString attribute( HbXmlLoaderAbstractSyntax::DocumentLexems lexem ) const;

    virtual bool readDocument();
    virtual bool readAlienSection();
    virtual bool readTargetSection();

    virtual bool readGeneralStartItem();
    virtual bool readGeneralEndItem();
    virtual bool readLayoutStartItem();
    virtual bool readAnchorLayoutStartItem(bool idBased);
    virtual bool readLayoutEndItem();
    virtual bool readContainerStartItem();
    virtual bool readContainerEndItem();

protected :
    bool toBool( const QString &boolString, bool &value ) const;
    bool toAnchorEdge( const QString &edgeString, Hb::Edge &edge ) const;
    bool toAnchorDir( const QString &dirString, HbAnchor::Direction &dir ) const;
    bool toSizePolicy( const QString& policyS, QSizePolicy::Policy &policy ) const;
    bool loadDevice(QIODevice *device, const QString &section);

protected:

    TopState mTopState;
    DocumentState mDocumentState;
    ElementState mElementState;

    QStringList mCurrentSection;
    QStringList mRequiredSection;
    QStringList mCurrentContainerNames;

    QList<HbXmlVariable*> mCurrentContainer;

    QXmlStreamReader::TokenType mCurrentTokenType;
    HbXml::ElementType mCurrentElementType;

    DocumentLexems mCurrentLayoutType;
    DocumentLexems mCurrentContainerType;

    QXmlStreamReader mReader;

private:

    Q_DISABLE_COPY(HbXmlLoaderBaseSyntax)
};

#endif // HBXMLLOADERBASESYNTAX_P_H
