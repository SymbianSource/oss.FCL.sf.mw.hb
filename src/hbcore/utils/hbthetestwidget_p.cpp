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

#include "hbthetestwidget_p.h"
#include "hbinstance.h"
#include "hbnamespace_p.h"
#include <hbapplication.h>
#include "hbtoolbutton_p.h"
#include "hbstyleoptiontoolbutton.h"
#include "hbcolorscheme.h"
#include "hbtextitem.h"
#include "hbpopup.h"
#include "hbdialog.h"
#include "hbthemecommon_p.h"
#include "hbthemeclient_p.h"


#ifdef Q_OS_SYMBIAN
#include <eikenv.h>
#include <apgtask.h>                // TApaTaskList, TApaTask
#endif

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QGraphicsWidget>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <hbaction.h>

#include <QDebug> // for qWarning

#ifdef HB_TEXT_MEASUREMENT_UTILITY
#include "hbtextmeasurementutility_p.h"
#endif //HB_TEXT_MEASUREMENT_UTILITY

//#ifdef Q_OS_SYMBIAN
//#include <coemain.h>
//#include <fbs.h>
//#endif

const int KWidth = 140; // container size, button width is KWidth/2
const int KHeight = 140; // container size, button height is KHeight/2
// how much must button be dragged before it is actually moved
const int KThreshold = 16;

HbTheTestButton::HbTheTestButton(QGraphicsItem *parent)
: HbToolButton(parent),
  mPressedDown(false)
{
    setAttribute(Hb::InteractionDisabled); // no feedback from test button
    mPosition = scenePos();
    mScreenSize = HbDeviceProfile::profile(this).logicalSize();
}

HbTheTestButton::~HbTheTestButton()
{
}

QSizeF HbTheTestButton::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    switch(which){
    case Qt::MinimumSize: //fallthrough
    case Qt::MaximumSize: //fallthrough
        return QSizeF(KWidth/2, KHeight/2);
    default:
        return HbAbstractButton::sizeHint( which, constraint );
    }
}

/*!
    \reimp
    Removed tooltip support.
 */
void HbTheTestButton::updatePrimitives()
{
    HbToolButtonPrivate *d = static_cast<HbToolButtonPrivate *>(HbToolButton::d_ptr);

    HbStyleOptionToolButton option;
    if (d->action) {
        setCheckable(d->action->isCheckable());
        setChecked(d->action->isChecked());
        setEnabled(d->action->isEnabled());
    }

    initStyleOption(&option);
    if (d->frameItem) {
        style()->updatePrimitive(d->frameItem, HbStyle::P_ToolButton_frame, &option);
    }
    if (d->textItem) {
        style()->updatePrimitive(d->textItem, HbStyle::P_ToolButton_text, &option);
    }
    if (d->iconItem) {
        style()->updatePrimitive(d->iconItem, HbStyle::P_ToolButton_icon, &option);
    }
}

/*!
  reimp
*/
void HbTheTestButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    HbToolButton::mousePressEvent(event);
    mPressedDown = true;
    mMoved = false;
    mPressedPos = event->scenePos();
    mPosition = scenePos() - pos();
    event->accept();
}

/*!
  reimp
*/
void HbTheTestButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!mMoved) {
        HbToolButton::mouseReleaseEvent(event);
    }
    mMoved = false;
    mPressedDown = false;
    event->accept();
}

/*!
  reimp

*/
void HbTheTestButton::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    HbToolButton::mouseMoveEvent(event);
    QPointF moved = mPressedPos - event->scenePos();
    if (mMoved || (mPressedDown && ((qAbs(moved.x()) > KThreshold) ||
                                   (qAbs(moved.y()) > KThreshold)))) {
        QPointF newPosition = mPosition - moved;
        if ((newPosition.x() + pos().x()) < 0) {
            newPosition.setX(0 - pos().x());
        }
        if ((newPosition.y() + pos().y()) < 0) {
            newPosition.setY(0 - pos().y());
        }

        if (newPosition.x() > (mScreenSize.width() - KWidth/2 - pos().x())) {
            newPosition.setX(mScreenSize.width() - KWidth/2 - pos().x());
        }
        if (newPosition.y() > (mScreenSize.height() - KHeight/2 - pos().y())) {
            newPosition.setY(mScreenSize.height() - KHeight/2 - pos().y());
        }
        HbTheTestWidget *parentWidget = qgraphicsitem_cast<HbTheTestWidget*>(parentItem());
        if (parentWidget) {
            mMoved = true;
            parentWidget->setPos(newPosition);
        }
        setDown(false);
    }
}




class HbTheTestWidgetPrivate
{
public:
    HbMainWindow *mMainWindow;
    HbTheTestButton *mButton1;
    HbTheTestButton *mButton2;
    HbTheTestButton *mButton3;
    HbTheTestButton *mButton4;
    QGraphicsGridLayout *mLayout;
};

/*!
    \class HbTheTestWidget
    \internal
    \proto
*/
HbTheTestWidget::HbTheTestWidget(HbMainWindow *mainWindow, QGraphicsItem *parent)
: HbWidget(parent)
{
    d = new HbTheTestWidgetPrivate;
    d->mMainWindow = mainWindow;

    d->mLayout = new QGraphicsGridLayout();
    d->mLayout->setContentsMargins( qreal(0.0), qreal(0.0), qreal(0.0), qreal(0.0) );
    d->mLayout->setSpacing(0.0);
    setLayout(d->mLayout);

    HbAction *action1 = new HbAction(QString("1"),this);
    d->mButton1 = new HbTheTestButton(this);
    d->mButton1->setAction(action1);
    d->mButton1->setToolButtonStyle(HbToolButton::ToolButtonText);

    HbAction *action2 = new HbAction(QString("2"),this);
    d->mButton2 = new HbTheTestButton(this);
    d->mButton2->setAction(action2);
    d->mButton2->setToolButtonStyle(HbToolButton::ToolButtonText);

    HbAction *action3 = new HbAction(QString("3"),this);
    d->mButton3 = new HbTheTestButton(this);
    d->mButton3->setAction(action3);
    d->mButton3->setToolButtonStyle(HbToolButton::ToolButtonText);

    HbAction *action4 = new HbAction(QString("4"),this);
    d->mButton4 = new HbTheTestButton(this);
    d->mButton4->setAction(action4);
    d->mButton4->setToolButtonStyle(HbToolButton::ToolButtonText);

    d->mLayout->addItem(d->mButton1, 0, 0);
    d->mLayout->addItem(d->mButton2, 0, 1);
    d->mLayout->addItem(d->mButton3, 1, 0);
    d->mLayout->addItem(d->mButton4, 1, 1);

    QSize screenSize = HbDeviceProfile::profile(this).logicalSize();
    setPos((screenSize.width()-KWidth)/2,(screenSize.height()-KHeight)/2);

    QObject::connect(mainWindow, SIGNAL(orientationChanged(Qt::Orientation)),
        this, SLOT(orientationChanged(Qt::Orientation)));
}


HbTheTestWidget::~HbTheTestWidget()
{
    delete d;
}

void HbTheTestWidget::orientationChanged(Qt::Orientation newOrientation)
{
    Q_UNUSED(newOrientation)
    QSize screenSize = HbDeviceProfile::profile(this).logicalSize();
    setPos((screenSize.width()-KWidth)/2,(screenSize.height()-KHeight)/2);
    d->mButton1->mScreenSize = screenSize;
    d->mButton2->mScreenSize = screenSize;
    d->mButton3->mScreenSize = screenSize;
    d->mButton4->mScreenSize = screenSize;
}

QSizeF HbTheTestWidget::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    switch(which){
    case Qt::MinimumSize: //fallthrough
    case Qt::MaximumSize: //fallthrough
        return QSizeF(KWidth, KHeight);
    default:
        return HbWidget::sizeHint( which, constraint );
    }
}

HbTheTestButton *HbTheTestWidget::button1()
{
    if (d) {
        return d->mButton1;
    }
    return 0;
}

HbTheTestButton *HbTheTestWidget::button2()
{
    if (d) {
        return d->mButton2;
    }
    return 0;
}

HbTheTestButton *HbTheTestWidget::button3()
{
    if (d) {
        return d->mButton3;
    }
    return 0;
}

HbTheTestButton *HbTheTestWidget::button4()
{
    if (d) {
        return d->mButton4;
    }
    return 0;
}

// slots that can be used in test buttons start here

void HbTheTestWidget::textLayoutMeasure()
{
#ifdef HB_TEXT_MEASUREMENT_UTILITY
    HbTextMeasurementUtility *measureUtility = HbTextMeasurementUtility::instance();
    measureUtility->measureItems();
#endif //HB_TEXT_MEASUREMENT_UTILITY
}

void HbTheTestWidget::textLayoutWriteReport()
{
#ifdef HB_TEXT_MEASUREMENT_UTILITY
    HbTextMeasurementUtility *measureUtility = HbTextMeasurementUtility::instance();
    HbDeviceProfile profile = HbDeviceProfile::profile(d->mMainWindow);
    if (!HbApplication::applicationName().isEmpty()) {
        measureUtility->writeReport(profile, HbApplication::applicationName());
    } else {
        measureUtility->writeReport(profile, "unknown_application");
    }
    measureUtility->reset();
#endif //HB_TEXT_MEASUREMENT_UTILITY
}

void HbTheTestWidget::toggleOrientation()
{
    if (d->mMainWindow->orientation() == Qt::Horizontal)
        d->mMainWindow->setOrientation(Qt::Vertical);
    else 
        d->mMainWindow->setOrientation(Qt::Horizontal);
}

void HbTheTestWidget::toggleMirroring()
{
    Qt::LayoutDirection dir = d->mMainWindow->layoutDirection();
    d->mMainWindow->setLayoutDirection(dir == Qt::LeftToRight ? Qt::RightToLeft : Qt::LeftToRight);
}

void HbTheTestWidget::setApplicationBackground()
{
#ifdef Q_OS_SYMBIAN
    TApaTask task(CEikonEnv::Static()->WsSession());
    task.SetWgId(CEikonEnv::Static()->RootWin().Identifier());
    task.SendToBackground();
#endif
}

void HbTheTestWidget::showThemeServerMemoryInfo()
{
    HbDialog popup;
    popup.setTimeout(HbPopup::StandardTimeout);   
    popup.setPreferredPos(QPointF(10,10));
    
    HbWidget widget;
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);
    widget.setLayout(layout);
    
    int freeSharedMemory = HbThemeClient::global()->freeSharedMemory();
    int allocatedSharedMemory = HbThemeClient::global()->allocatedSharedMemory();
    int allocatedHeapMemory = HbThemeClient::global()->allocatedHeapMemory();
    QString message("Free shared mem: ");
    message += QString::number(freeSharedMemory / 1024);
    message += " kB \nAllocated shared mem: ";
    message += QString::number(allocatedSharedMemory / 1024);
    message += " kB \nAllocated heap: ";
    message += QString::number(allocatedHeapMemory / 1024);
    message += " kB \n";
    HbTextItem *textItem = new HbTextItem(message);
    textItem->setTextColor(HbColorScheme::color("qtc_default_main_pane_normal"));
    layout->addItem(textItem); 
    
    popup.setContentsMargins(0,20,0,20);
    popup.setContentWidget(&widget);
    popup.exec();
}

void HbTheTestWidget::screenCapture()
{
    setVisible(false);
    QTimer::singleShot(1000, this, SLOT(doScreenCapture()));
}

void HbTheTestWidget::doScreenCapture()
{
#if defined (Q_OS_WIN32)
    QString filePath("c:\\");
    filePath = QDir::toNativeSeparators(filePath);
#elif defined (Q_OS_SYMBIAN)
    QString filePath("f:\\");
    filePath = QDir::toNativeSeparators(filePath);
    if (!QDir(filePath).exists()) {
        filePath = "e:\\";
        filePath = QDir::toNativeSeparators(filePath);
        if (!QDir(filePath).exists()) {
            filePath = "c:\\"; // this should always exist
            filePath = QDir::toNativeSeparators(filePath);
        }
    }
#elif defined (Q_OS_UNIX)
    QString filePath(QDir::tempPath());
    filePath.append(QDir::separator());
#endif

    filePath.append("data");
    filePath.append(QDir::separator());
    filePath.append("hb_screenshots");

    QDir dir(filePath);
    if (!dir.exists()) {
        dir.mkpath(filePath);
    }

    if (!HbApplication::applicationName().isEmpty()) {
        filePath.append(HbApplication::applicationName());
    } else {
        filePath.append("unknown_application");
    }
    filePath.append(".png");

//#ifdef Q_OS_SYMBIAN
    // todo: not fail-safe code
//    TSize screenSize = CCoeEnv::Static()->ScreenDevice()->SizeInPixels();
//    TDisplayMode displayMode = CCoeEnv::Static()->ScreenDevice()->DisplayMode();

//    CFbsBitmap *bitmap = new (ELeave) CFbsBitmap();
//    User::LeaveIfError(bitmap->Create(screenSize, displayMode));

//    CCoeEnv::Static()->ScreenDevice()->CopyScreenToBitmap(bitmap);
//    QPixmap screenPixmap = QPixmap::fromSymbianCFbsBitmap(bitmap);
//#else
    QPixmap screenPixmap = QPixmap::grabWindow(
        QApplication::activeWindow()->winId());
//#endif
    QString format = "png";
    screenPixmap.save(filePath.toLatin1(), format.toLatin1());
    setVisible(true);
}

