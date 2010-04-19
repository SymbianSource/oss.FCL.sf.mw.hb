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

#ifndef HBFONTSPEC_H
#define HBFONTSPEC_H

#include <hbglobal.h>
#include <QFont>
#include <QExplicitlySharedDataPointer>
#include <QMetaType>
#include <QtGlobal>

class HbFontSpecPrivate;

class HB_CORE_EXPORT HbFontSpec
{
    Q_GADGET
    Q_ENUMS(Role)

public:
    enum Role
    {
        Undefined = 0,
        Primary,
        Secondary,
        Title,
        PrimarySmall,
        Digital
    };

    HbFontSpec();
    explicit HbFontSpec(HbFontSpec::Role role);

    HbFontSpec(const HbFontSpec &other);
    HbFontSpec &operator=(const HbFontSpec &other);

    ~HbFontSpec();

    bool isNull() const;

    QFont font() const;

    HbFontSpec::Role role() const;
    void setRole(HbFontSpec::Role role);

    qreal textHeight() const;
    void setTextHeight(qreal textHeight);

    //Deprecated
    int pixelSize() const;
    //Deprecated
    void setPixelSize(int pixelSize);
    //Deprecated
    int pointSize() const;
    //Deprecated
    void setPointSize(int pointSize);
    //Deprecated
    qreal textPaneHeight() const;
    //Deprecated
    void setTextPaneHeight(qreal textPaneHeight);
    //Deprecated
    QFont fontDelta() const;
    //Deprecated
    void setFontDelta(const QFont &font);

    operator QVariant() const;
    bool operator==(const HbFontSpec &other) const;
    bool operator!=(const HbFontSpec &other) const;

private:
    HbFontSpecPrivate *d;
};

Q_DECLARE_METATYPE(HbFontSpec)

#endif // HBFONTSPEC_H
