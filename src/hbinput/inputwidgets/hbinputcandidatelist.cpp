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
#include "hbinputcandidatelist.h"

#include <QLabel>
#include <QGraphicsLayout>
#include <hblistwidget.h>
#include <hblistwidgetitem.h>
#include <hbview.h>
#include <hbframeitem.h>
#include <hbframedrawer.h>

#include <hbdeviceprofile.h>

#include <hbinputmethod.h>
#include <hbinputsettingproxy.h>
#include <hbinputvkbhost.h>
#include <hbinputregioncollector_p.h>

#include "hbdialog_p.h"

const int HbCandListDefaultNumRows = 5;
const qreal HbCandListMaxWidthMultiplier = 0.8;
const qreal HbCandListMinWidth = 30.0;
const qreal HbAutoComplPopupSideMargin = 15.0;
const qreal HbAutoComplPopupVerticalMargin = 15.0;
const qreal HbAutoComplPopupMinAllowedHeight = 25.0;

/// @cond

class HbCandidateListPrivate : public HbDialogPrivate
{
    Q_DECLARE_PUBLIC(HbCandidateList)

public:
    HbCandidateListPrivate(HbInputMethod *input);
    ~HbCandidateListPrivate();
    void calculateAndSetSize(qreal maxWidth);
    void initFrameIcon();

public:
    HbListWidget *mList;
    HbInputMethod *mInput;
    int numRows;
    int numCandidates;
    int longestStringWidth;
    HbFrameItem *mFrameBackground;
    HbListWidgetItem *mSpellQueryItem;
    bool mCandidateCommitted;
    bool mSpellQueryOpenIsPending;
};

HbCandidateListPrivate::HbCandidateListPrivate(HbInputMethod *input)
    : mInput(input),
      numRows(HbCandListDefaultNumRows),
      numCandidates(0),
      longestStringWidth(0),
      mFrameBackground(0),
      mSpellQueryItem(0),
      mCandidateCommitted(false),
      mSpellQueryOpenIsPending(false)
{
    Q_Q(HbCandidateList);

    mList = new HbListWidget(q);
    mList->setEnabledAnimations(HbAbstractItemView::None);
}

HbCandidateListPrivate::~HbCandidateListPrivate()
{
}

void HbCandidateListPrivate::initFrameIcon()
{
    Q_Q(HbCandidateList);

    mFrameBackground = static_cast<HbFrameItem *>(q->primitive(HbStyle::P_Popup_background));

    if (mFrameBackground == 0) {
        mFrameBackground = static_cast<HbFrameItem *>(q->style()->createPrimitive((HbStyle::Primitive)(HbStyle::P_Popup_background), q));
    }
}


void HbCandidateListPrivate::calculateAndSetSize(qreal maxWidth)
{
    Q_Q(HbCandidateList);

    const qreal oneLineHeight = 40.0;  // temporarily as a constant, eventually we'll need to calculate this.

    QRectF geom = q->geometry();
    qreal finalWidth = 30 + longestStringWidth * 2;    // Use magic numbers for now until we can calculate this from font.

    if (finalWidth > maxWidth) {
        finalWidth = maxWidth;
    }

    qreal finalHeight = geom.height();
    const int numLines = (numCandidates < numRows ? numCandidates : numRows);
    qreal l = 0, r = 0, t = 0, b = 0;
    if (q->layout()) {
        q->layout()->getContentsMargins(&l, &t, &r, &b);
    }

    finalWidth = finalWidth + l + r ;
    finalHeight = (qreal)numLines * oneLineHeight + 5.0 + t + b;

    if (mSpellQueryItem) {
        finalHeight += oneLineHeight ; // for spell button
    }

    if (finalHeight > HbDeviceProfile::current().logicalSize().height() - 30) {
        finalHeight = HbDeviceProfile::current().logicalSize().height() - 30;
    }

    geom.setHeight(finalHeight);
    geom.setWidth(finalWidth);

    q->setGeometry(geom);
}

/// @endcond

/*!
@proto
@hbinput
\class HbCandidateList
\brief Hb based candidate list popup.

Shows a list of word prediction candidates. Informs the owning
input method when a word is selected

\sa HbInputVkbWidget
*/


/*!
Constructor.
@param input The input method that uses this widget.
@param parent parent of the widget.
*/
HbCandidateList::HbCandidateList(HbInputMethod *input, QGraphicsItem *parent)
    : HbDialog(*new HbCandidateListPrivate(input), parent)
{
    Q_D(HbCandidateList);

    HbInputRegionCollector::instance()->attach(this);

    d->setPriority(HbPopupPrivate::VirtualKeyboard + 1);  // Should be shown on top of virtual keyboard.
    d->initFrameIcon();

    // Make sure the preview pane never steals focus.
    setFlag(QGraphicsItem::ItemIsPanel, true);
    setActive(false);

    setTimeout(NoTimeout);
    setAttribute(Qt::WA_InputMethodEnabled, false);
    connect(d->mList, SIGNAL(activated(HbListWidgetItem *)), this, SLOT(itemActivated(HbListWidgetItem *)));
    connect(d->mList, SIGNAL(longPressed(HbListWidgetItem *, const QPointF &)), this, SLOT(itemActivated(HbListWidgetItem *)));

    setBackgroundFaded(false);
}

/*!
Destroys the object.
*/
HbCandidateList::~HbCandidateList()
{
}

/*!
Populates the candidate list with text strings given as parameter.
*/
void HbCandidateList::populateList(const QStringList &candidates, bool addSpellQuery)
{
    Q_D(HbCandidateList);
	// Only for the first time when we launch candidate list its not setting a layout, 
	// Mostly the problem is form Qt side, for the time being to resolve issue related to candidate list
	// making visible property true.  
    setVisible(true);
    setContentWidget(d->mList);

    d->setPriority(HbPopupPrivate::VirtualKeyboard + 1);  // Should be shown on top of virtual keyboard.
    d->mList->clear();

    int longestwidth = 0;
    int finalWidth = 250;
    for (int i = 0; i < candidates.count(); i++) {
        HbListWidgetItem *item = new HbListWidgetItem();
        item->setText(candidates[i]);
        d->mList->addItem(item);

        // TODO: Font has not been set corretly yet...
        QFontMetrics fontMetrics(d->mList->fontSpec().font());
        finalWidth = fontMetrics.width(candidates[i]);
        if (finalWidth > longestwidth) {
            longestwidth = finalWidth;
        }
    }

    d->mSpellQueryItem = 0;
    if (addSpellQuery) {
        d->mSpellQueryItem = new HbListWidgetItem();
        d->mSpellQueryItem->setText(tr("Spell"));
        d->mList->addItem(d->mSpellQueryItem);
        QFontMetrics fontMetrics(d->mList->fontSpec().font());
        finalWidth = fontMetrics.width(tr("Spell"));
        if (finalWidth > longestwidth) {
            longestwidth = finalWidth;
        }
    }

    d->mList->setMinimumWidth(HbCandListMinWidth);
    d->numCandidates = candidates.count();
    d->longestStringWidth = longestwidth;

    //always scroll to first item
    d->mList->setCurrentRow(0);
    d->mList->scrollTo(d->mList->currentIndex());

    d->calculateAndSetSize((qreal)HbDeviceProfile::current().logicalSize().width() * HbCandListMaxWidthMultiplier);
}

/*!
Inherited from HbDialog.
*/
void HbCandidateList::keyPressEvent(QKeyEvent *event)
{
    Q_D(HbCandidateList);

    if (event->key() == Qt::Key_Space
        || event->key() == Qt::Key_Enter
        || event->key() == Qt::Key_Return
        || event->key() == Qt::Key_Right
        || event->key() == Qt::Key_Left) {
        if (!d->mCandidateCommitted) {
            emit candidateSelected(event->key(), currentCandidate());
            d->mCandidateCommitted = true;
        }
        hide();
    }
}

/*!
Inherited from HbDialog.
*/
void HbCandidateList::closeEvent(QCloseEvent* /*event*/)
{
    hide();
    emit candidatePopupCancelled();
}

/*!
Called when an item on the list is activated. Hides the list and informs the input.
*/
void HbCandidateList::itemActivated(HbListWidgetItem *item)
{
    Q_D(HbCandidateList);
    if (!d->mCandidateCommitted)  {
        if (d->mSpellQueryItem != item) {
            emit candidateSelected(0, currentCandidate());
        } else if (d->mSpellQueryItem) {
            d->mSpellQueryOpenIsPending = true;
        }
        d->mCandidateCommitted = true;
    }
    hide();
}

/*!
Returns the currently selected candidate on the list.
*/
QString HbCandidateList::currentCandidate()
{
    Q_D(HbCandidateList);
    if (d->mList->currentItem()) {
        return d->mList->currentItem()->text();
    }

    return QString();
}

/*!
Sets the maximum number of visible candidates.
*/
void HbCandidateList::setNumberOfVisibleLines(int numLines)
{
    Q_D(HbCandidateList);

    d->numRows = numLines;
    update();
}

/*!
this event handler is called, for Hide events, is delivered after the widget has been hidden.
*/
void HbCandidateList::hideEvent(QHideEvent *event)
{
    Q_D(HbCandidateList);
    d->mCandidateCommitted = false;
    HbDialog::hideEvent(event);
    // If we try to open spell query in itemActivated, first spell query tries to
    // open, and then candidatel list is closed. This creates problems if we
    // quickly double click on the Spell button. So open spell query only after candidate
    // list is closed.
    if (d->mSpellQueryOpenIsPending) {
        d->mSpellQueryOpenIsPending = false;
        emit launchSpellQueryDialog();
    }
}

void HbCandidateList::updatePrimitives()
{
    Q_D(HbCandidateList);

    d->mFrameBackground->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
    d->mFrameBackground->frameDrawer().setFrameGraphicsName("qtg_fr_popup_secondary");
    d->mFrameBackground->setGeometry(boundingRect());
}

/*!
Finds correct size and position for auto-completion popup. It checks if there is enough space to display candidate popup,
finds out whether it is display above or below the editor and sets correct width.
Returns true if suitable position was found and set.
*/
bool HbCandidateList::setSizeAndPositionForAutoCompletion(HbVkbHost *vkbHost)
{
    Q_D(HbCandidateList);

    if (vkbHost && d->mInput && d->mInput->focusObject()) {
        QRectF freeViewRect = vkbHost->applicationArea();
        QRectF microFocus = d->mInput->focusObject()->microFocus();

        if (freeViewRect.isValid() &&
            microFocus.isValid() &&
            freeViewRect.contains(microFocus)) {
            QRectF topRect = freeViewRect;
            topRect.setBottom(microFocus.top());
            topRect.adjust(HbAutoComplPopupSideMargin, HbAutoComplPopupVerticalMargin, -HbAutoComplPopupSideMargin, -HbAutoComplPopupVerticalMargin);

            QRectF bottomRect = freeViewRect;
            bottomRect.setTop(microFocus.bottom());
            bottomRect.adjust(HbAutoComplPopupSideMargin, HbAutoComplPopupVerticalMargin, -HbAutoComplPopupSideMargin, -HbAutoComplPopupVerticalMargin);

            if (topRect.height() > bottomRect.height()) {
                if (topRect.height() > HbAutoComplPopupMinAllowedHeight) {
                    qreal finalHeight = (size().height() < topRect.height() ? size().height() : topRect.height());
                    topRect.setTop(topRect.bottom() - finalHeight);
                    resize(topRect.width(), topRect.height());
                    setPos(topRect.topLeft());
                    return true;
                }
            } else {
                if (bottomRect.height() > HbAutoComplPopupMinAllowedHeight) {
                    qreal finalHeight = (size().height() < bottomRect.height() ? size().height() : bottomRect.height());
                    bottomRect.setHeight(finalHeight);
                    resize(bottomRect.width(), bottomRect.height());
                    setPos(bottomRect.topLeft());
                    return true;
                }
            }
        }
    }

    return false;
}

// End of file
