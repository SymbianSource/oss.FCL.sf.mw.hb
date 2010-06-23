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

#include <QGraphicsWidget>
#include <QPointer>
#include <QIcon>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLinearLayout>

#include <hbinputregioncollector_p.h>
#include "hbdeviceprofile.h"
#include "hbdialog.h"
#include "hblabel.h"
#include "hbstyleoptionlabel_p.h"
#include "hbinputexactwordpopup.h"
#include "hbiconitem.h"
#include "hbinputsettingproxy.h"
#include "hbframeitem.h"
#include "hbframedrawer.h"
#include "hbcolorscheme.h"
#include "hbdialog_p.h"

const qreal HbExactWordPopupHeight = 50.0;

class HbExactWordPopupPrivate : public HbDialogPrivate
{
    Q_DECLARE_PUBLIC(HbExactWordPopup)

public:
    HbExactWordPopupPrivate();
    ~HbExactWordPopupPrivate();

    void initBackground();

public:
    HbLabel *mText;
    HbFrameItem *mPopupBackground;
};

HbExactWordPopupPrivate::HbExactWordPopupPrivate()
 : mText(0), mPopupBackground(0)
{
}

HbExactWordPopupPrivate::~HbExactWordPopupPrivate()
{
}

void HbExactWordPopupPrivate::initBackground()
{
    Q_Q(HbExactWordPopup);

    mPopupBackground = static_cast<HbFrameItem*>(q->primitive(HbStyle::P_Popup_background));

    if (!mPopupBackground) {
        mPopupBackground = static_cast<HbFrameItem*>(q->style()->createPrimitive((HbStyle::Primitive)(HbStyle::P_Popup_background), q));
    }

    if (mPopupBackground->frameDrawer().isNull()) {
        HbFrameDrawer *fd = new HbFrameDrawer("qtg_fr_popup_secondary", HbFrameDrawer::NinePieces);
        mPopupBackground->setFrameDrawer(fd);
    }
}

HbExactWordPopup* HbExactWordPopup::instance(HbExactWordPopupIndicator indicatorArrow)
{
    static QPointer<HbExactWordPopup> exactWordPopup;
    if (!exactWordPopup) {
        // HbExactWordPopup is owned by the scene
        exactWordPopup = new HbExactWordPopup( 0, indicatorArrow );
    }
    return exactWordPopup;
}

/*!
    Constructor.
    \param parent An optional parameter.
*/
HbExactWordPopup::HbExactWordPopup(QGraphicsWidget *parent, HbExactWordPopupIndicator indicatorArrow)
 : HbDialog(*new HbExactWordPopupPrivate(), parent)
{
    Q_D(HbExactWordPopup);
    HbInputRegionCollector::instance()->attach(this);

    d->mText = new HbLabel();
    d->mText->setAlignment(Qt::AlignCenter);
    d->mText->setFontSpec(HbFontSpec(HbFontSpec::Primary));
    d->mText->setContentsMargins(0, 0, 0, 0);

    setContentWidget(d->mText);

    d->initBackground();

    setTimeout(HbPopup::NoTimeout);
    setBackgroundFaded(false);
    setDismissPolicy(HbPopup::TapInside);
    setFocusPolicy(Qt::ClickFocus);
    setModal(false);
    setContentsMargins(0, 0, 0, 0);

    // Make sure the exact word popup never steals focus.
    setFlag(QGraphicsItem::ItemIsPanel, true);
    setActive(false);

    setIndicatorArrow(indicatorArrow);
}

/*!
    Returns the text of the tooltip. The default value is "Exact Word popup text".

    \sa setText()
*/

QString HbExactWordPopup::text()
{
    Q_D(HbExactWordPopup);
    return d->mText->plainText();
}

/*!
    Sets the text of the label within the progress note.

    \sa text()
*/
void HbExactWordPopup::setText(const QString &newText)
{
    Q_D(HbExactWordPopup);
    d->mText->setPlainText(newText);
}

/*!
    Displays exact word popup at a given point

    \sa hideText()
*/
void HbExactWordPopup::showText(QPointF pos)
{
    Q_D(HbExactWordPopup);

    QFontMetrics fontMetrics(HbFontSpec(HbFontSpec::Primary).font());
    qreal width = fontMetrics.width(text());

    d->mText->setMinimumWidth(width);
    d->mText->setMaximumWidth(width);
  
    pos.setY(pos.y() - HbExactWordPopupHeight);
    setPos(pos);

    show();
}

/*!
    Hides exact word popup

    \sa hideText()
*/
void HbExactWordPopup::hideText()
{
    close();
}

void HbExactWordPopup::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (contains(mapFromScene(event->scenePos()))) {
        // commit word only if mouse release event happens inside popup area
        emit exactWordSelected();
        // this call closes the popup, and this is called only when mouse relase event happens
        // inside popup area
        HbDialog::mouseReleaseEvent(event);
    }
    // if mouse release event happens outside popup area, don't commit or close the popup

}

void HbExactWordPopup::updatePrimitives()
{
    Q_D(HbExactWordPopup);

    d->mPopupBackground->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
    d->mPopupBackground->frameDrawer().setFrameGraphicsName("qtg_fr_popup_secondary");
    d->mPopupBackground->setGeometry(boundingRect());

	QColor col = HbColorScheme::color( "qtc_editor_normal" ); //popupforeground
    if (col.isValid()) {
        d->mText->setTextColor(col);
    }
}

// this method is called whenever there is a switch of keypad usage from h/w to virtual
// h/w keypad needs an indicator, whereas virtual does not, hence set the image appropriately.
void HbExactWordPopup::setIndicatorArrow(HbExactWordPopupIndicator indicatorArrow)
{
    Q_D(HbExactWordPopup);

    if (indicatorArrow == HbNoIndicatorArrow) {
        d->mText->setIcon(HbIcon());
    } else {
        d->mText->setIcon(HbIcon("qtg_graf_inpu_swipe"));
   }
}
