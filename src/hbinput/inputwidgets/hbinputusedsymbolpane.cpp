/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbInput module of the UI Extensions for Mobile.
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

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPointer>

#include <hbinputsettingproxy.h>
#include <hbfontspec.h>
#include <hbinputfilter.h>
#include <hbcolorscheme.h>

#include "hbinputvkbwidget.h"
#include "hbinputvkbwidget_p.h"
#include "hbinputusedsymbolpane.h"

/*!
    @proto
    @hbinput
    \class HbInputUsedSymbolPane
    \deprecated class HbInputUsedSymbolPane
    \brief A widget for displaying most frequently used special characters.
    
    This is a widget that knows how to display most frequently used special characters.
    It uses setting proxy for tracking top candidates. User is able to select one
    of the characters and when that happens, setting proxy frequency list is
    updated and the widget emits the signal sctCharacterSelected. 

    \sa HbInputSettingProxy
*/

/// @cond

const int HbMaxSctLineChars = 7;

class HbInputUsedSymbolPanePrivate
{
public:
    HbInputUsedSymbolPanePrivate(HbInputVkbWidget* aOwner);
    qreal cellWidth(QGraphicsWidget* aParent);
    QChar mapClickedCharacter(QPointF aClickPoint, QGraphicsWidget* aParent);

public:
    int mNumChrs;
    HbInputVkbWidget* mOwner;
    QPixmap mBackground;
    QString mCharSet;
    int mNumberOfCharsDisplayed;
    QPointer<HbInputFilter> mFilter;
};

HbInputUsedSymbolPanePrivate::HbInputUsedSymbolPanePrivate(HbInputVkbWidget* aOwner)
    : mNumChrs(HbMaxSctLineChars),
      mOwner(aOwner),mFilter(0)
{
    mBackground = QPixmap(HbSctLineWidth, HbSctLineHeight);
	QColor color = HbColorScheme::color("inputmethod_color_usedsymbolpane_background");
	if (color.isValid()) {
		mBackground.fill(color);
	} else {
		mBackground.fill(QColor(Qt::gray));
	}
}

qreal HbInputUsedSymbolPanePrivate::cellWidth(QGraphicsWidget* aParent)
{
    if (mNumChrs) {
        return aParent->size().width() / (qreal)mNumChrs;
    }

    return 0.0;
}

QChar HbInputUsedSymbolPanePrivate::mapClickedCharacter(QPointF aClickPoint, QGraphicsWidget* aParent)
{
    qreal xstep = cellWidth(aParent);

    if (xstep > 0.0) {
        int chrIndex = (int)(aClickPoint.x() / xstep);

        QString characters;

		if((characters.count() > mNumberOfCharsDisplayed) && (characters.count() < HbMaxSctLineChars)) {
			mCharSet = characters;
		}

		if((mCharSet.compare(characters))) {
			characters = mCharSet;
		}

        if (chrIndex < characters.size()) {
            return characters[chrIndex];
        }
    }

    return 0;
}

/// @endcond

/*!
\deprecated HbInputUsedSymbolPane::HbInputUsedSymbolPane(HbInputVkbWidget*, QGraphicsWidget*)
    is deprecated.
*/
HbInputUsedSymbolPane::HbInputUsedSymbolPane(HbInputVkbWidget* aOwner, QGraphicsWidget* aParent)
    : QGraphicsWidget(aParent)
{
    mPrivate = new HbInputUsedSymbolPanePrivate(aOwner);
	mPrivate->mNumberOfCharsDisplayed = mPrivate->mCharSet.count();
}

/*!
\deprecated HbInputUsedSymbolPane::~HbInputUsedSymbolPane()
    is deprecated.
*/
HbInputUsedSymbolPane::~HbInputUsedSymbolPane()
{
    delete mPrivate;
}

/*!
\deprecated HbInputUsedSymbolPane::setNumberOfCharacters(int)
    is deprecated.
*/
void HbInputUsedSymbolPane::setNumberOfCharacters(int aNumChrs)
{
    mPrivate->mNumChrs = aNumChrs;
}

/*!
\deprecated HbInputUsedSymbolPane::mousePressEvent(QGraphicsSceneMouseEvent*)
    is deprecated.
*/
void HbInputUsedSymbolPane::mousePressEvent(QGraphicsSceneMouseEvent* aEvent)
{
    mPrivate->mOwner->d_func()->redirectMousePressEvent(aEvent);

    QChar chr = mPrivate->mapClickedCharacter(aEvent->pos(), this);
    if (chr > 0) {
        emit sctCharacterSelected(chr);
    }
	aEvent->accept();
}

/*!
\reimp
\deprecated HbInputUsedSymbolPane::mouseReleaseEvent(QGraphicsSceneMouseEvent*)
    is deprecated.
*/
void HbInputUsedSymbolPane::mouseReleaseEvent(QGraphicsSceneMouseEvent* aEvent)
{
    mPrivate->mOwner->d_func()->redirectMouseReleaseEvent(aEvent);
}

/*!
\reimp
\deprecated HbInputUsedSymbolPane::mouseDoubleClickEvent(QGraphicsSceneMouseEvent*)
    is deprecated.
*/
void HbInputUsedSymbolPane::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* aEvent)
{
    QChar chr = mPrivate->mapClickedCharacter(aEvent->pos(), this);
    if (chr > 0) {
        emit sctCharacterSelected(chr); 
    }
}

/*!
\deprecated HbInputUsedSymbolPane::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*)
    is deprecated.
*/
void HbInputUsedSymbolPane::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    painter->drawPixmap(boundingRect(), mPrivate->mBackground, boundingRect());

    painter->setPen(Qt::black);
    painter->drawRect(rect());

    qreal xstep = mPrivate->cellWidth(this);
    QRectF chrRect(QPointF(0.0, 0.0), QPointF(xstep, size().height()));

    QString characters;

	if((characters.count() <= HbMaxSctLineChars) && (characters.count() > mPrivate->mNumberOfCharsDisplayed)) {
		mPrivate->mNumberOfCharsDisplayed++;
		mPrivate->mCharSet = characters;
	}

	if((mPrivate->mCharSet.compare(characters))) {
		characters = mPrivate->mCharSet;
	}

    HbFontSpec spec(HbFontSpec::Primary);
    painter->setFont(spec.font());
    for (int i = 0; i < mPrivate->mNumChrs && i < characters.size(); i++) {
        painter->drawText(chrRect, Qt::AlignCenter, QString(characters[i]));
        chrRect.moveRight(chrRect.right() + xstep);
    }
}

/*!
\deprecated HbInputUsedSymbolPane::restoreSctLine(HbInputFilter*)
    is deprecated.
*/
void HbInputUsedSymbolPane::restoreSctLine(HbInputFilter *aFilter)
{
        if (aFilter != mPrivate->mFilter)
		mPrivate->mNumberOfCharsDisplayed = mPrivate->mCharSet.count();
	mPrivate->mFilter = aFilter;

	this->update();
}
// End of file
