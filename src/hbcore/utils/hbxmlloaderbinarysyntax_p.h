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

#ifndef HBXMLLOADERBINARYSYNTAX_P_H
#define HBXMLLOADERBINARYSYNTAX_P_H

#include <hbxmlloaderabstractsyntax_p.h>
#include <hbglobal.h>

class HbXmlLoaderAbstractActions;

class HB_CORE_PRIVATE_EXPORT HbXmlLoaderBinarySyntax : public HbXmlLoaderAbstractSyntax
{
public:

    HbXmlLoaderBinarySyntax( HbXmlLoaderAbstractActions *actions );
    virtual ~HbXmlLoaderBinarySyntax();

    static const char* signature();
    static QString version();

    bool isBinary( QIODevice *device );
    bool load( QIODevice *device, const QString &section );

private:
    bool validateDocument();
    bool processDocument( const QString &section );

    bool parseReset();
    bool parseCleanUp();
    bool parseDeleteAll();
    bool parsePushDocument();
    bool parsePushObject();
    bool parsePushWidget();
    bool parsePushConnect();
    bool parsePushProperty();
    bool parsePushRef();
    bool parsePushContainer();
    bool parsePop();
    bool parseSetContentsMargins();
    bool parseSetSizePolicy();
    bool parseSetSizeHint();
    bool parseSetToolTip();
    bool parseCreateAnchorLayout();
    bool parseAddAnchorLayoutItem();
    bool parseSetAnchorLayoutMapping();
    bool parseCreateGridLayout();
    bool parseAddGridLayoutCell();
    bool parseSetGridLayoutRowProperties();
    bool parseSetGridLayoutColumnProperties();
    bool parseSetGridLayoutRowHeights();
    bool parseSetGridLayoutColumnWidths();
    bool parseCreateLinearLayout();
    bool parseAddLinearLayoutItem();
    bool parseAddLinearLayoutStretch();
    bool parseSetLayoutContentsMargins();
    bool parseCreateStackedLayout();
    bool parseAddStackedLayoutItem();
    bool parseCreateNullLayout();

private:
    Q_DISABLE_COPY(HbXmlLoaderBinarySyntax)
    QDataStream mIn;
};

#endif // HBXMLLOADERBINARYSYNTAX_P_H
