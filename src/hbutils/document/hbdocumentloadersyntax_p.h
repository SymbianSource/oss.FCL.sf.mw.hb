/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbUtils module of the UI Extensions for Mobile.
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

#ifndef HBDOCUMENTLOADERSYNTAX_P_H
#define HBDOCUMENTLOADERSYNTAX_P_H

#include <hbxmlloaderbasesyntax_p.h>

#include <hbglobal.h>
#include <hbfontspec.h> // Needed for the Role enum

#include <QHash>
#include <QList>
#include <QXmlStreamReader>
#include <QPointer>
#include <QByteArray>

class HbDocumentLoaderActions;
class HbMainWindow;

class HbDocumentLoaderSyntax : public HbXmlLoaderBaseSyntax
{
public:

    HbDocumentLoaderSyntax( HbXmlLoaderAbstractActions *actions );
    virtual ~HbDocumentLoaderSyntax();

    void setParseOnly( bool parseOnly );

    bool load( QIODevice *device, const QString &section );

    bool scanForSections( QIODevice *device, QList<QString> &sectionsList );

    static QString version();

private:
    Q_DISABLE_COPY(HbDocumentLoaderSyntax)

    bool processDocument();
    bool processObject();
    bool processWidget();
    bool processSpacerItem();
    bool processLayout();
    bool processConnect();
    bool processContainer();
    bool processContainedProperty();
    bool processProperty();
    bool processRef();
    bool processVariable();
    bool processContentsMargins();
    bool processRowHeights( int row );
    bool processColumnWidths( int column );
    bool checkEndElementCorrectness();

    HbXml::ElementType elementType( QStringRef name ) const;

    bool createVariable( HbXmlVariable& variable );

    static bool convertSizeHintType(
        const QString &type, Qt::SizeHint &resultHint, bool &resultFixed);

    bool readGeneralStartItem();
    bool readLayoutStartItem();
    bool readContainerStartItem();
    bool readContainerEndItem();

    static bool toFontSpecRole(const QString &roleString, HbFontSpec::Role &role);

};

#endif // HBDOCUMENTLOADERSYNTAX_P_H
