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
#include "hbfontspec.h"
#ifndef HB_BOOTSTRAPPED
#include "hbinstance_p.h"
#include "hbtypefaceinfo_p.h"
#include "hbstyle.h"
#endif

#include <QVariant>
#include <QFont>

/*!
    @stable
    @hbcore
    \class HbFontSpec
    \brief HbFontSpec is used to request a system font.

    It introduces a list of font roles, which can be used to request system
    fonts configured for the device. The actual physical fonts matching the
    different font roles may vary and also change in run-time.

    The class requests font size by text height. HbFontSpec accesses
    system typeface information to determine what value to pass to
    QFont::setPixelSize which would result in a font that will render text
    with accents descenders entirely within the given height.

    Brief example of how to create a fontspec and use it.

    \dontinclude ultimatecodesnippet/ultimatecodesnippet.cpp
    \skip Start of snippet 24
    \until End of snippet 24
*/

/*!
  \enum HbFontSpec::Role

  This enum defines the available font roles.
*/

/*!
    \var HbFontSpec::Undefined

    The font role is undefined.
*/

/*!
    \var HbFontSpec::Primary

    System's primary font.
*/

/*!
    \var HbFontSpec::Secondary

    System's secondary font.
*/

/*!
    \var HbFontSpec::Title

    System's title font.
*/

/*!
    \var HbFontSpec::PrimarySmall

    System's small primary font.
*/

/*!
    \var HbFontSpec::Digital

    System's digital font. Contains only digits and a small set of
    other characters.
*/

// Register meta type
static const int fontSpecMetaType = qRegisterMetaType<HbFontSpec>();

class HbFontSpecPrivate
{
public:

    HbFontSpecPrivate();
    ~HbFontSpecPrivate();

    qreal textHeight() const;
    QFont font();

public: // data
    HbFontSpec::Role mRole;
    mutable qreal mTextHeight;
    QString mFontName;
};

/*!
\internal
*/
HbFontSpecPrivate::HbFontSpecPrivate() :
    mRole(HbFontSpec::Undefined),
    mTextHeight(-1.0f),
    mFontName("")
{
}

/*!
\internal
*/
HbFontSpecPrivate::~HbFontSpecPrivate()
{
}

/*!
\internal

If text height is not set with setTextHeight() resolves the
default text height based on the font role.
*/
qreal HbFontSpecPrivate::textHeight() const
{
#ifdef HB_BOOTSTRAPPED
    return 0;
#else
    if ((mRole != HbFontSpec::Undefined) && (mTextHeight < 0)) {
        qreal parameterValue;
        QString parameterName;
        switch (mRole) {
        case HbFontSpec::Primary:
            parameterName = "hb-param-text-height-primary";
            break;
        case HbFontSpec::Secondary:
            parameterName = "hb-param-text-height-secondary";
            break;
        case HbFontSpec::Title:
            parameterName = "hb-param-text-height-title";
            break;
        case HbFontSpec::PrimarySmall:
            parameterName = "hb-param-text-height-tiny";
            break;
        case HbFontSpec::Digital:
        default: // Usage of Secondary as the default system typeface
            parameterName = "hb-param-text-height-secondary";
            break;
        }
        HbInstance::instance()->style()->parameter(parameterName, parameterValue);
        mTextHeight = parameterValue;
    }
    return mTextHeight;
#endif
}

/*!
\internal
*/
QFont HbFontSpecPrivate::font()
{
#ifdef HB_BOOTSTRAPPED
    return QFont();
#else
    if (mRole == HbFontSpec::Undefined && mFontName.isEmpty()) {
        return QFont();
    }
    QString typefaceFamily(mFontName);
    int weight(QFont::Normal);

    HbTypefaceInfo *tInfo = HbInstancePrivate::d_ptr()->typefaceInfo(); // Non-owning pointer

    if (mRole != HbFontSpec::Undefined) {
    tInfo->roleToTypeface(mRole, typefaceFamily, weight);
        mFontName = typefaceFamily;
    } else if (!tInfo->containsFamily(typefaceFamily)) {
        QString aliasFamily;
        if (tInfo->tryGetFamilyFromAliasName(typefaceFamily, aliasFamily, weight)) {
            typefaceFamily = aliasFamily;
        }
    } else {
        weight = tInfo->getWeight(typefaceFamily);
    }
    QFont font(typefaceFamily);

    font.setWeight(weight);

    // Sets default size if text height is not set explicitly.
    qreal height = textHeight();
    int downSizedSize = tInfo->textHeightToSizeInPixels(typefaceFamily, weight, height);
    font.setPixelSize(downSizedSize);

    return font;
#endif
}

/*!
    Default constructor. The font role is HbFontSpec::Undefined and
    the font matches a default constructed QFont instance.
*/
HbFontSpec::HbFontSpec()
    : d(new HbFontSpecPrivate())
{
}

/*!
    Constructs a new font spec with the given font \a role.
*/
HbFontSpec::HbFontSpec(HbFontSpec::Role role)
    : d(new HbFontSpecPrivate())
{
    d->mRole = role;
}
HbFontSpec::HbFontSpec(const QString fontName) 
    : d(new HbFontSpecPrivate())
{
    d->mFontName = fontName;
}

/*!
    Copy constructs a new font spec using the \a other font spec.
*/
HbFontSpec::HbFontSpec(const HbFontSpec &other)
    : d(new HbFontSpecPrivate())
{
    d->mRole = other.d->mRole;
    d->mTextHeight = other.d->mTextHeight;
}

/*!
    Assigns the \a other font spec to this font and returns a
    reference to this font spec.
*/
HbFontSpec &HbFontSpec::operator=(const HbFontSpec &other)
{
    if (&other != this) {
        d->mRole = other.d->mRole;
        d->mTextHeight = other.d->mTextHeight;
    }
    return *this;
}

/*!
    Destroys the font spec.
*/
HbFontSpec::~HbFontSpec()
{
    delete d;
}

/*!
    Returns true if the font spec role is Undefined.
*/
bool HbFontSpec::isNull() const
{
    return (d->mRole == HbFontSpec::Undefined);
}

/*!
    Returns the role of the font.
    \sa HbFontSpec::setRole
*/
HbFontSpec::Role HbFontSpec::role() const
{
    return d->mRole;
}

/*!
    Sets the \a role of the font.
    The font is initialized to the default font for the given role.

    \note If the size of the font has not been set explicitly with
    setTextHeight() the size of the font is set to the default size
    of the given role.

    \sa HbFontSpec::role
*/
void HbFontSpec::setRole(HbFontSpec::Role role)
{
    d->mRole = role;
}

/*!
    Returns the font based on the parameters defined in the font spec.
    It is defined as follows.

    - 0) If font role is undefined returns default constructed QFont.
    - 1) Current physical font matching the defined font role is resolved.
    - 2) Size is overridden with size set by setTextHeight() or by
    default value defined by the font role.
*/
QFont HbFontSpec::font() const
{
    return d->font();
}

/*!
    Returns the fontspec as a QVariant.
*/
HbFontSpec::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    Returns true if this fontSpec is equal to \a other fontSpec; otherwise returns false.
*/
bool HbFontSpec::operator==(const HbFontSpec &other) const
{
    return ((d->mRole == other.d->mRole)
            && qFuzzyCompare(d->textHeight(), other.d->textHeight()));
}

/*!
    Returns true if this fontSpec is not equal to \a other fontSpec; otherwise returns false.
*/
bool HbFontSpec::operator!=(const HbFontSpec &other) const
{
    return (!(*this == other));
}

/*!
    Returns the text height that was set using \a setTextHeight.

    If text height is not set returns default value defined by the font role.

    \sa HbFontSpec::setTextHeight
*/
qreal HbFontSpec::textHeight() const
{
    return d->textHeight();
}

/*!
    Sets the height of the text into which the requested font is to fit.
    When the \a font method is called, the returned font will have been
    requested with a reduced size, such that accents and long descenders
    will fit within the passed-in text height.

    \note The input parameter is a real value. The size requested for
    the \a QFont will be an integer value.

    Setting the text height to a negative value resets the font's size to a
    default value defined by the font role.

    \sa HbFontSpec::textHeight
*/
void HbFontSpec::setTextHeight(qreal textHeight)
{
    d->mTextHeight = textHeight;
}

void HbFontSpec::setTypefaceFamily(QString fontName)
{
    d->mFontName = fontName;
}

QString HbFontSpec::typefaceFamily() const
{
   return d->mFontName;
}
// End of File
