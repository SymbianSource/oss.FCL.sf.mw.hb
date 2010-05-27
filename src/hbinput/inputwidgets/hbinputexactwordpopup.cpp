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

const QSizeF  HbExactWordPopupSize(10,10);
const QPointF HbExactWordPopupStartupDisplay(12, 33);

class HbExactWordPopupPrivate : public HbDialogPrivate
{
    Q_DECLARE_PUBLIC(HbExactWordPopup)

public:
    HbExactWordPopupPrivate();
    ~HbExactWordPopupPrivate();

    void initBackground();

public:
    HbLabel *mText;
    HbStyleOptionLabel *mOption;
    HbIconItem *iconPrim;
    HbFrameItem *mPopupBackground;
};

HbExactWordPopupPrivate::HbExactWordPopupPrivate(){
    mText = 0;
    mOption = 0;
    iconPrim = 0;
}

HbExactWordPopupPrivate::~HbExactWordPopupPrivate(){
    delete mOption;
    mOption = 0;
}

void HbExactWordPopupPrivate::initBackground()
{
    Q_Q(HbExactWordPopup);

    mPopupBackground = static_cast<HbFrameItem*>(q->primitive( HbStyle::P_Popup_background));

    if( mPopupBackground == 0 ) {
        mPopupBackground = static_cast<HbFrameItem*>(q->style()->createPrimitive((HbStyle::Primitive)(HbStyle::P_Popup_background), q));
    }

    if ( mPopupBackground->frameDrawer().isNull()) {
        HbFrameDrawer* fd = new HbFrameDrawer("qtg_fr_popup_secondary", HbFrameDrawer::NinePieces);
        mPopupBackground->setFrameDrawer(fd);
    }

    // the size of the layout in the base class has been set to 0, reset it by passing an invalid size to setMinimumSize
    if(mainLayout) {
        mainLayout->setMinimumSize(-1, -1);
    }

}

HbExactWordPopup* HbExactWordPopup::instance( HbExactWordPopupIndicator indicatorArrow ) {
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
HbExactWordPopup::HbExactWordPopup(QGraphicsWidget *parent, HbExactWordPopupIndicator indicatorArrow ) :
    HbDialog(*new HbExactWordPopupPrivate(), parent)
{
    Q_D(HbExactWordPopup);
    d->mText = new HbLabel(this);
    d->mText->setAlignment(Qt::AlignCenter);

    d->setPriority(HbPopupPrivate::VirtualKeyboard + 1);  // Should be shown on top of virtual keyboard.

    d->initBackground();

    setTimeout(HbPopup::NoTimeout);
    setBackgroundFaded(false);
    setVisible(false);
    setDismissPolicy(HbPopup::TapInside);
    setFocusPolicy(Qt::ClickFocus);
    setContentWidget(d->mText);
    setModal(false);

#if QT_VERSION >= 0x040600
    // Make sure the exact word popup never steals focus.
    setFlag(QGraphicsItem::ItemIsPanel, true);
    setActive(false);
#endif

    d->mOption = new HbStyleOptionLabel();
    if(d->mOption != 0 ) {
        d->mOption->text = QString(" ");
        d->mOption->boundingRect = QRectF(HbExactWordPopupStartupDisplay,HbExactWordPopupSize);
        // for hardware keypad, we need to show the arrow to indicate the word
        // and in virtual keypad this is not needed, so set the image accordingly
        setIndicatorArrow( indicatorArrow );
        d->mOption->alignment = Qt::AlignCenter;
    }

    d->iconPrim = static_cast<HbIconItem*>(primitive(HbStyle::P_Label_icon));
    if(d->iconPrim == 0) {
        d->iconPrim = static_cast<HbIconItem*>(style()->createPrimitive((HbStyle::Primitive)(HbStyle::P_Label_icon), this));
    }
    style()->updatePrimitive(d->iconPrim, (HbStyle::Primitive)(HbStyle::P_Label_icon), d->mOption);
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

    QRectF ps=QRectF(QPointF(0,0), d->mText->preferredSize()).adjusted(-9,-9,9,9);
    resize(ps.size());
}

/*!
    Displays exact word popup at a given point

    \sa hideText()
*/
void HbExactWordPopup::showText(QPointF pos)
{
    // the popup should know at this stage in which main window/scene it's been launched at.
    int screenWidth = 0;
    if ( mainWindow() ) {
        screenWidth = HbDeviceProfile::profile(this).logicalSize().width();
    } else {
        // this is the fall-back if the main window is not know - can be removed when popup
        // is not relying on the primary window anymore.
        screenWidth = HbDeviceProfile::profile(mainWindow()).logicalSize().width();
    }

    const QRectF br = boundingRect();
    const qreal brCenter = br.width()/2;
    pos.setX(pos.x()-brCenter);
    // fix x position to keep tooltip visible
    const qreal requiredWidth = pos.x()+br.width();
    if (requiredWidth > screenWidth) {
        pos.setX(pos.x()-(requiredWidth-screenWidth));
    } else if (0 > pos.x()) {
        pos.setX(0);
    }
    pos.setY(pos.y()-br.height());
    setPos(pos);

    QSizeF mySize = size();
    mySize.setHeight(HbExactWordPopupSize.height());
    resize(mySize);    

    Q_D(HbExactWordPopup);
    d->mOption->boundingRect = QRectF(rect().center().x() - (HbExactWordPopupSize.width()/2),rect().bottom(),HbExactWordPopupSize.width(),HbExactWordPopupSize.height());
    style()->updatePrimitive(d->iconPrim, (HbStyle::Primitive)(HbStyle::P_Label_icon), d->mOption);

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
    Q_D( HbExactWordPopup );

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
void HbExactWordPopup::setIndicatorArrow( HbExactWordPopupIndicator indicatorArrow )
{
    Q_D( HbExactWordPopup );

    if (indicatorArrow == HbNoIndicatorArrow) {
        d->mOption->icon = (QString(""));
    } else {
        d->mOption->icon = (QString("qtg_graf_inpu_swipe"));
   }
}
