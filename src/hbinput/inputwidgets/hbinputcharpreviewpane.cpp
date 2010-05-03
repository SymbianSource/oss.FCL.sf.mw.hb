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
// Qt includes
#include <QGraphicsLinearLayout>
#include <QSignalMapper>
#include <QPainter>
#include <QSizePolicy>
#include <QGraphicsDropShadowEffect>

#include <hbeffect.h>
#include <hbinputsettingproxy.h>
#include <hbmainwindow.h>
#include <hbcolorscheme.h>
#include <hbframedrawer.h>
#include <hbframeitem.h>
#include <hbdeviceprofile.h>
#include <private/hbdialog_p.h>
#include <hbtextitem.h>
#include <hbmainwindow.h>
#include <hbinstance.h>

#include "hbinputpreviewlabel.h"
#include "hbinputcharpreviewpane.h"

const int HbPreviewZoomDelta = 7;
const qreal HbPreviewBoundaryDelta = 1.5;
const qreal HbBoundaryLabelWidthFactor = 0.75;
const qreal HbLabelwidthFactor = 0.50;

/// @cond

/*
Character preview widget for Accented characters.
Implements character preview for characters mapped in the preview Pane.
*/
class HbAccentedCharPreviewPane: public HbWidget
{
public:
    /*!
    Constructor.
    @param parent of the widget.
    */
    HbAccentedCharPreviewPane(QGraphicsItem *parent = 0)
        :HbWidget(parent),
        mTextItem(0)
    {
        HbFrameItem *n = new HbFrameItem(parent);
        n->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
        n->frameDrawer().setFrameGraphicsName("qtg_fr_character_preview");


        setBackgroundItem( n );
        mTextItem = static_cast<HbTextItem*>(style()->createPrimitive(HbStyle::P_Label_text, this));
    }
    /*!
    update the text and frame primitives
    */
    void updatePrimitives()
    {
        HbWidget::updatePrimitives();
        if (mTextItem) {
            mTextItem->setFontSpec(HbFontSpec(HbFontSpec::Primary));
            mTextItem->setAlignment(Qt::AlignCenter);
        }
    }
    /*!
    Destroys the object.
    */
    ~HbAccentedCharPreviewPane() {
    }

public:
    HbTextItem* mTextItem;
};

class HbCharPreviewPanePrivate: public HbDialogPrivate
{
    Q_DECLARE_PUBLIC(HbCharPreviewPane)

public:
    HbCharPreviewPanePrivate();
    ~HbCharPreviewPanePrivate();
    void clearCharacters();
    void updateCharacters();
    void init();

// private slots
    void _q_showAccentedPreviewPane(QString character, QRectF sceneBoundingRect);
    void _q_hideAccentedPreviewPane();
    void _q_hidePreviewPanePopup();
public:
    QStringList mCharacterList;
    QSignalMapper *mReleaseMapper;
    QGraphicsLinearLayout* mCandLayout;
    QSizeF mItemSize;
    HbAccentedCharPreviewPane* mAccentedPreviewPane;
};

void HbCharPreviewPanePrivate::init()
{
    Q_Q(HbCharPreviewPane);

    HbFrameItem *n = new HbFrameItem( q );
    n->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
    n->frameDrawer().setFrameGraphicsName("qtg_fr_character_preview");

    q->setBackgroundItem( n );
}

HbCharPreviewPanePrivate::HbCharPreviewPanePrivate()
    : mReleaseMapper(0),
    mCandLayout(0)
{
    mAccentedPreviewPane = new HbAccentedCharPreviewPane();
}

HbCharPreviewPanePrivate::~HbCharPreviewPanePrivate()
{
    if(mAccentedPreviewPane) {
        delete mAccentedPreviewPane;
    }
    if(mReleaseMapper) {
        delete mReleaseMapper;
    }
}

void HbCharPreviewPanePrivate::clearCharacters()
{
    Q_Q(HbCharPreviewPane);
    for (int i = mCandLayout->count() - 1; i >= 0; i--) {
        QGraphicsLayoutItem* layoutitem = mCandLayout->itemAt(i);
        mCandLayout->removeAt(i);
        delete layoutitem;
    }
    mCandLayout->updateGeometry();
    q->adjustSize();
}

void HbCharPreviewPanePrivate::updateCharacters()
{
    Q_Q(HbCharPreviewPane);
	// need to set the minimum size to an invalid value here in order to make sure that the size does not become 0
	mCandLayout->setMinimumSize(-1,-1);
    for (int i = 0; i < mCharacterList.count(); i++) {
        HbPreviewLabel* label = new HbPreviewLabel(mCharacterList[i]);
        label->setPreferredHeight(mItemSize.height() + HbPreviewZoomDelta);
        if (mCharacterList.count() > 1 && i != mCharacterList.count()-1) {
            label->setPreferredWidth(HbBoundaryLabelWidthFactor * mItemSize.width());
            label->setTextGeometry(mItemSize.width(), mItemSize.height());
        } else {
            label->setPreferredWidth(HbLabelwidthFactor * mItemSize.width());
            label->setTextGeometry(mItemSize.width(), mItemSize.height());
        }

        QObject::connect(label, SIGNAL(showAccentedPreview(QString,QRectF)), q, SLOT(_q_showAccentedPreviewPane(QString,QRectF)));
        QObject::connect(label, SIGNAL(selected()), mReleaseMapper, SLOT(map()));
        QObject::connect(label, SIGNAL(hideAccentedPreview()), q, SLOT(_q_hideAccentedPreviewPane()));
        QObject::connect(label, SIGNAL(hidePreview()), q, SLOT(_q_hidePreviewPanePopup()));
        mReleaseMapper->setMapping(label, mCharacterList[i]);
        mCandLayout->addItem(label);
        mCandLayout->setItemSpacing(i, 0.0);
    }

    mCandLayout->setContentsMargins(mItemSize.width() / 4, 0, mItemSize.width() / 4, 0);
    mCandLayout->updateGeometry();
    q->adjustSize();
}

/*!
Sets the character for preview and shows in it's Pane.
@param character The character for preview.
@param itemSceneBoundingRect of the QGraphicsItem.
*/
void HbCharPreviewPanePrivate::_q_showAccentedPreviewPane(QString character, QRectF itemSceneBoundingRect)
{
    Q_Q(HbCharPreviewPane);
    mAccentedPreviewPane->setZValue(q->zValue()+1);
    q->scene()->addItem(mAccentedPreviewPane);
    // let's validate.
    if (!itemSceneBoundingRect.isValid()) {
        return;
    }
	QColor color = HbColorScheme::color("qtc_editor_normal");
    // we need to show the accented char preview preview just above the
    // passed QRectF of the item which is passed.
    QPointF pos = itemSceneBoundingRect.topLeft();
    pos.setY(pos.y() - mItemSize.height() +  HbPreviewZoomDelta + HbPreviewBoundaryDelta);

    // let's adjust x position of the character preview pane so that it
    // is aligned at the center of the items for which we want to show
    // the preview.
    pos.setX(itemSceneBoundingRect.x() - itemSceneBoundingRect.width()/4);

    // set final position for the character preview pane
    mAccentedPreviewPane->setPos(pos);
    mAccentedPreviewPane->mTextItem->setText(character);
    if (color.isValid()) {
        mAccentedPreviewPane->mTextItem->setTextColor(color);
    }
    mAccentedPreviewPane->updatePrimitives();
    QSizeF paneSize = itemSceneBoundingRect.size();
    QRectF rect;
    rect.setWidth(HbBoundaryLabelWidthFactor * mItemSize.width());
    rect.setHeight(mItemSize.height());
    rect.setY(paneSize.height()/2 - mItemSize.height()/2 - HbPreviewZoomDelta - HbPreviewBoundaryDelta);
    mAccentedPreviewPane->mTextItem->setGeometry(rect);

    // show it!
    mAccentedPreviewPane->show();
}
/*
hides the accented single character preview pane whenever user tries to hover on it
as the the character is inputed in the editor only when mouse is released from preview pane
*/
void HbCharPreviewPanePrivate::_q_hideAccentedPreviewPane()
{
    mAccentedPreviewPane->hide();
}

/*
hides the accented single character preview pane as well as charcters Preview Pane
*/
void HbCharPreviewPanePrivate::_q_hidePreviewPanePopup()
{
    Q_Q(HbCharPreviewPane);
    mAccentedPreviewPane->hide();
    if (q->isVisible()) {
        q->hide();
    }
}

/// @endcond

/*!
@proto
@hbinput
\class HbCharPreviewPane
\deprecated class HbCharPreviewPane
\brief Character preview widget for virtual keyboards.


Implements character preview for virtual keyboards. Shows a list of clickable
characters and maps the clicks to owning keyboard's charFromPreviewSelected slot.
For first level of preview popup we have a linear layout, we create a HbPreviewLabel
and add the labels to this layout. For second level of preview popup as user clicks
on the lebel of preview pane we display preview of the accented character on the preview pane.

\sa HbInputVkbWidget
\sa HbPreviewLabel
*/

/*!
\deprecated HbCharPreviewPane::HbCharPreviewPane(QGraphicsItem*)
    is deprecated.
*/
HbCharPreviewPane::HbCharPreviewPane(QGraphicsItem* parent)
    : HbDialog(*new HbCharPreviewPanePrivate, parent)
{
    Q_D(HbCharPreviewPane);

    d->q_ptr = this;

    d->mCandLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    setLayout(d->mCandLayout);

    d->setPriority(HbPopupPrivate::VirtualKeyboard + 1);  // Should be visible on top of VKB

    // set some properties
    setFocusPolicy(Qt::ClickFocus);
    setModal(false);
    setDismissPolicy(HbPopup::TapAnywhere);
    setBackgroundFaded(false);

#if QT_VERSION >= 0x040600
    // Make sure the preview pane never steals focus.
    setFlag(QGraphicsItem::ItemIsPanel, true);
    setActive(false);

    // enable drop shadow for the preview pane
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(8);
    setGraphicsEffect(effect);
#endif

    // signal mapper for getting events
    d->mReleaseMapper = new QSignalMapper(this);

    QObject::connect(d->mReleaseMapper, SIGNAL(mapped(QString)), this, SIGNAL(charFromPreviewSelected(QString)));
    setTimeout(NoTimeout);
    d->init();

    // This is important we need to switch off all the effects.
    // if we dont switch off the effect, and if there is a call to 'show' while a 'hide' animation is
    // in progress the hide call will be ignored.
    // this will lead to a problem where we will not able to see all the character
    // preview in case the user is typing very fast.
#ifdef HB_EFFECTS
    HbEffect::disable(this);
#endif // HB_EFFECTS
}

/*!
\deprecated HbCharPreviewPane::~HbCharPreviewPane()
    is deprecated.
*/
HbCharPreviewPane::~HbCharPreviewPane()
{
}

/*!
\deprecated HbCharPreviewPane::showCharacters(const QStringList&, const QRectF &)
    is deprecated.
*/
void HbCharPreviewPane::showCharacters(const QStringList& characterList, const QRectF &itemSceneBoundingRect)
{
    Q_D(HbCharPreviewPane);
    // let's validate.
    if (!itemSceneBoundingRect.isValid()) {
        return;
    }

    if (characterList.size() > 1) {
        setModal(true);
    } else {
        setModal(false);
    }

    d->mCharacterList = characterList;
    // let's store items size.
    d->mItemSize = itemSceneBoundingRect.size();

    // we need to clear previous character.
    d->clearCharacters();

    // update new candidates
    d->updateCharacters();

    updateGeometry();

    // we need to show the preview just above the
    // passed QRectF of the item which is passed.
    QPointF pos = itemSceneBoundingRect.topLeft();
    pos.setY(pos.y() - size().height());

    // let's adjust x position of the character preview pane so that it
    // is aligned at the center of the items for which we want to show
    // the preview.
    pos.setX(itemSceneBoundingRect.center().x() - size().width()/2);

    // We need to adjust the character preview pane's postion
    // such that it is visible in in the current screen.
    if (pos.x() < 0) {
        pos.setX(0);
    } else {
        const int screenWidth = HbDeviceProfile::profile(this).logicalSize().width();
        const qreal requiredWidth = pos.x() + (geometry().width());
        if ( requiredWidth > screenWidth) {
            pos.setX(screenWidth-(geometry().width()));
        }
    }

    // set final position for the character preview pane
    setPos(pos);

    // we need to reset mousePressLocation to None.
    // We are handling character preview pane a bit differently than a usual popup.
    // A HbDialog closes the popup on a mouse release. And there is a problem  when we
    // have long pressed on a button and the preview pane is showing  set of characters,
    // same time if we do a long press on another button preview will be shown on the next
    // button but as soon as we release the button HbDialog's internal logic closes the
    // preview pane. since popup closes on mouse release event. To aviod this situation we
    // need to reset mousePressLocation. Since this behaviour is very specific to preview pane
    // we need to fix it here.
    d->mousePressLocation = HbPopupPrivate::None;

	// set the background as a panel if the foreground is a panel to provide focus handling
	if ((flags() & QGraphicsItem::ItemIsPanel) && isModal()) {
		d->backgroundItem->setFlag(QGraphicsItem::ItemIsPanel);
	}
    // show it!
    show();
}

#include "moc_hbinputcharpreviewpane.cpp"
// End Of File

