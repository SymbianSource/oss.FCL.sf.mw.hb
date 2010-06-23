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
#include "hbinputmainwindow_p.h"

#include <QGraphicsWidget>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QInputContext>

#include "hbinputregioncollector_p.h"
#include "hbinstance.h"
#include "hbwidget.h"
#include "hbview.h"
#include "hbnamespace_p.h"
#include "hbstackedlayout.h"

#if defined (Q_OS_SYMBIAN)
#include <coemain.h>
#include <coecntrl.h>
#include <e32cmn.h>

TRect qt_QRect2TRect(const QRectF &rect)
{
    TRect trect;
    trect.SetRect(rect.topLeft().x(), rect.topLeft().y(),
                  rect.bottomRight().x() + 1, rect.bottomRight().y() + 1);
    return trect;
}

Q_DECLARE_TYPEINFO(TRect, Q_MOVABLE_TYPE);
#endif

class HbInputTransparentWindow : public HbWidget
{
public:

    HbInputTransparentWindow(QGraphicsItem *parent = 0);
    ~HbInputTransparentWindow();

    enum { Type = Hb::ItemType_TransparentWindow };
    int type() const {
        return Type;
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
};

HbInputTransparentWindow::HbInputTransparentWindow(QGraphicsItem *parent) :
    HbWidget(parent)
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);
}


/*!
    Destructs the transparent window.
 */
HbInputTransparentWindow::~HbInputTransparentWindow()
{
}

void HbInputTransparentWindow::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    QPainter::CompositionMode compositionMode = painter->compositionMode();
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->fillRect(option->exposedRect, QColor(0, 0, 0, 0));
    painter->setCompositionMode(compositionMode);
}

/*
creates an instance of HbInputMainWindow.
*/
HbInputMainWindow *HbInputMainWindow::instance()
{
    static HbInputMainWindow *mainWindow = new HbInputMainWindow();
    return mainWindow;
}

HbInputMainWindow::~HbInputMainWindow()
{
    delete mProxyWindow;
}

// constructor.
HbInputMainWindow::HbInputMainWindow()
// HbMainWindow creates a background QGraphicsItem, which has the background image. we need to hide it that.
    : HbMainWindow(0, Hb::WindowFlagTransparent), mLastFocusedWidget(0), mSpellQueryLaunched(false), mProxyWindow(0)
{
    // We need a window which is of type Qt::Window flag at the same time does not show
    // any decorators Qt::Tool seems to be the option, and we want this window to be always on top so Qt::WindowStaysOnTopHint.
    // And since transparency requires to have a frameless window we are setting that too.
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool | Qt::FramelessWindowHint);

    // By default QGraphicsView has a background which is white in color (Other controls eg. QPushButton
    // have a grey background), we need to make that transparent too.
    setStyleSheet("background: transparent");

    // No focus is necessary as we don't want the hbmainwindw to steal focus.
    setFocusPolicy(Qt::NoFocus);

    // add transparency begin.
    HbView *view = new HbView;
    view->hideItems(Hb::AllItems);
    view->setContentFullScreen();

#if defined (Q_OS_SYMBIAN)
    CCoeControl *c = effectiveWinId();
    c->SetFocusing(false);
    RWindow *rw = static_cast<RWindow *>(c->DrawableWindow());
    rw->SetRequiredDisplayMode(EColor16MA);
    TInt err = rw->SetTransparencyAlphaChannel();
    if (err == KErrNone) {
        rw->SetBackgroundColor(~0);
    }
#endif // Q_OS_SYMBIAN

    HbInputTransparentWindow *transparentWindow = new HbInputTransparentWindow;
    HbStackedLayout *stackedLayout = new HbStackedLayout;
    stackedLayout->addItem(transparentWindow);
    view->setLayout(stackedLayout);
    addView(view);
    // add transparency ends.

    connect(HbInputRegionCollector::instance(), SIGNAL(updateRegion(QRegion)), this, SLOT(updateRegion(QRegion)));

    // QApplication signal for getting notification of any focus change. If therer
    // is a switch between application window and HbInputMainWindow then we need to
    // set the focus back to the application window, if we don't do that it will
    // result in focusLost call inside framework.
    connect(qApp, SIGNAL(focusChanged(QWidget *, QWidget *)),
            this, SLOT(saveFocusWidget(QWidget *, QWidget *)));
}


void HbInputMainWindow::updateRegion(QRegion region)
{
    mMask = region;
#if defined (Q_OS_SYMBIAN)
    RWindowBase *rwindow = effectiveWinId()->DrawableWindow();
    if (region.isEmpty()) {
        TRegionFix<1> tregion(TRect(TPoint(0, 0), TSize(0, 0)));
        rwindow->SetShape(tregion);
    } else {
        // Using QVector assumes the memory layout is the same as RRegion
        QVector<QRect> rects = region.rects();
        QVector<TRect> trects(rects.count());
        for (int i = 0; i < trects.count(); ++i) {
            trects[i] = qt_QRect2TRect(rects.at(i));
        }
        RRegion rregion(trects.count(), trects.data());
        if (!rregion.CheckError()) {
            rwindow->SetShape(rregion);
        }
    }
#else
    setMask(region);
#endif
}


/*
we should always set the focus back to the application window.
*/
bool HbInputMainWindow::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::WindowActivate:
        if (mLastFocusedWidget && !mSpellQueryLaunched) {
            qApp->setActiveWindow(mLastFocusedWidget);
        }
        break;
    default:
        break;
    }
    return HbMainWindow::event(e);
}

/*
This function checks for any modal dialog present in the HbGraphicsScene by checking DynamicPropertyChange
and blocks the events to the application window by launching a widget which sits exactly in between the applicaion
and HbInputMainWindow.
*/
bool HbInputMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::DynamicPropertyChange) {
        const QString p = static_cast<QDynamicPropertyChangeEvent *>(event)->propertyName();
        if (p == "SpellQueryLaunched") {
            QVariant variant = obj->property("SpellQueryLaunched");
            if (variant.isValid()) {
                mSpellQueryLaunched = variant.toBool();
                if (mSpellQueryLaunched) {
                    qApp->setActiveWindow(this);
                    setFocus(Qt::OtherFocusReason);
                } else {
                    if (mLastFocusedWidget) {
                        qApp->setActiveWindow(mLastFocusedWidget);
                    }
                }
            }
            // return true as we are interested party!
            return true;
        }
    }

    // we need to only check for spontaneous events.
    if (event->spontaneous() && (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease)) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent) {
            // get the top level widget at the point, and see if that widget is a HbMainWindow,
            // If it is a HbMainWindow then do not do any thing, as events will propagate
            // correctly. But when it is clicked inside application window then send the event to
            // viewport as we might want to close a popup.
            if (!mMask.contains(mouseEvent->globalPos())) {
                qApp->sendEvent(viewport(), event);
            }
        }
    }

    return HbMainWindow::eventFilter(obj, event);
}

/*
Since hbmainwindow is overlapped on top of the application window, we need to
set the focus back to the application window. Not doing so will result in a focus
lost.
*/
void HbInputMainWindow::saveFocusWidget(QWidget * /*Old*/, QWidget *newFocus)
{
    if (newFocus && !this->isAncestorOf(newFocus)) {
        mLastFocusedWidget = newFocus;
    }
}

void HbInputMainWindow::showInputWindow()
{
    // installing event filter to the application.. this is needed to get
    // the events happening in other vanilla windows.
    qApp->installEventFilter(this);
    QInputContext *ic = qApp->inputContext();
    QWidget *fw = ic ? ic->focusWidget() : 0 ;
    QWidget *win = 0;
    if (fw) {
       win = fw->window();
    }
#ifdef Q_WS_WIN
    // As in windows OS HbMainWindow can come anywhere on the screen.
    // so we need to launch main window exactly at the top windows position.
    if (win) {
        move(win->frameGeometry().x(), win->pos().y());
    }
#endif

    HbInputRegionCollector::instance()->setEnabled(true);
    if (win && win->windowModality() != Qt::NonModal) {
        if (!mProxyWindow) {
            mProxyWindow = new HbProxyWindow();
        }
        mProxyWindow->setWindow(this);
        // since the focused widget is inside a modal dialog which blocks events to other_window.
        // and since hbinputmainwindow also comes under the other_window. It does will not get the 
        // mouse click events.
        mProxyWindow->setParent(win);
        // setParent resets the window flags, so we have to set the flags once again before show() is called.
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool | Qt::FramelessWindowHint);
        show();
    } else {
        if (mProxyWindow && mProxyWindow->isAncestorOf(this)) {
            mProxyWindow->setWindow(0);
            setParent(0);
            // setParent resets the window flags, so we have to set the flags once again before show is called.
            setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool | Qt::FramelessWindowHint);
        }
        show();
    }

#if defined(Q_OS_SYMBIAN)
    // this is done to come on top of all the controls in symbian OS, done to overlap soft keys as well.
    RWindow *rWindow = static_cast<RWindow *>(effectiveWinId()->DrawableWindow());
    const int positionForeground(0);
    rWindow->SetOrdinalPosition(positionForeground,
                                ECoeWinPriorityAlwaysAtFront);
#endif
}

void HbInputMainWindow::hideInputWindow()
{
    if (mSpellQueryLaunched) {
        return;
    }

    if (isVisible()) {
        hide();
#if defined(Q_OS_SYMBIAN)
        RWindow *rWindow = static_cast<RWindow *>(effectiveWinId()->DrawableWindow());
        const int positionBackground(-1);
        rWindow->SetOrdinalPosition(positionBackground, ECoeWinPriorityNormal);
#endif
    }

    HbInputRegionCollector::instance()->setEnabled(false);

    // installing event filter to the application.. this is needed to get
    // the events happening in other vanilla windows.
    qApp->removeEventFilter(this);
}

//EOF
