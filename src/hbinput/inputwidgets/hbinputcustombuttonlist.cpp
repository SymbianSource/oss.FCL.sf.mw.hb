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

#include <QGraphicsLinearLayout>
#if QT_VERSION >= 0x040600
#include <QGraphicsDropShadowEffect>
#endif

#include <hbinputvkbwidget.h>
#include <hbaction.h>
#include "hbinputtouchkeypadbutton.h"
#include "hbinputcustombuttonlist.h"
#include "hbdialog_p.h"

/// @cond


class HbInputCustomButtonListPrivate : public HbDialogPrivate
{
public:
    HbInputCustomButtonListPrivate(HbInputVkbWidget* owner) : mOwner(owner) {};
    ~HbInputCustomButtonListPrivate() {};
public:
    HbInputVkbWidget* mOwner;
    QList<HbTouchKeypadButton*> mButtonList;
};



/// @endcond

/*!
\deprecated HbInputCustomButtonList::HbInputCustomButtonList(QList<HbAction*>&, HbInputVkbWidget*, QGraphicsWidget*)
    is deprecated. HbInputCustomButtonList will be removed.
*/
HbInputCustomButtonList::HbInputCustomButtonList(QList<HbAction*>& actionList, HbInputVkbWidget* owner,
                            QGraphicsWidget* parent)
    : HbDialog(*new HbInputCustomButtonListPrivate(owner), parent)
{    
    Q_D(HbInputCustomButtonList);

    d->setPriority(HbPopupPrivate::VirtualKeyboard + 1);  // Should be shown on top of virtual keyboard.

    QGraphicsLinearLayout *linearLayout = new QGraphicsLinearLayout(Qt::Horizontal);

    QGraphicsWidget* contextWidget = new QGraphicsWidget;
    linearLayout->setContentsMargins(0.0, 0.0, 0.0, 0.0);
    linearLayout->setSpacing(0.0);
    contextWidget->setLayout(linearLayout);

#if QT_VERSION >= 0x040600
    // Make sure the custom button list never steals focus.
    setFlag(QGraphicsItem::ItemIsPanel, true);
    setActive(false);

    // enable drop shadow for the preview pane
// Effect deletion is crashing -> Effect temporarily removed
//    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
//    effect->setBlurRadius(8);
//    setGraphicsEffect(effect);
#endif

    setContentWidget(contextWidget);

    // set default values for popup
    setTimeout(HbDialog::NoTimeout);
    layout()->setContentsMargins(0.0, 0.0, 0.0, 0.0);
    setBackgroundFaded(false);
    setDismissPolicy(TapAnywhere);
    setModal(false);

    updateActions(actionList);
}

/*!
\deprecated HbInputCustomButtonList::~HbInputCustomButtonList()
    is deprecated. HbInputCustomButtonList will be removed.
*/
HbInputCustomButtonList::~HbInputCustomButtonList()
{  
}


/*!
\deprecated HbInputCustomButtonList::updateActions(QList<HbAction*>&)
    is deprecated. HbInputCustomButtonList will be removed.
*/
void HbInputCustomButtonList::updateActions(QList<HbAction*>& newActions)
{
    Q_D(HbInputCustomButtonList);

    QGraphicsLinearLayout* layout = static_cast<QGraphicsLinearLayout*>(contentWidget()->layout());
    int j = 0;

    // create new buttons
    for(int i=0; i<newActions.count(); i++)
    {     
        HbTouchKeypadButton* button = 0;

        if (newActions.at(i)->isVisible()) {
            if (j < d->mButtonList.count()){
                // update old button
                button = d->mButtonList.at(j);
                button->disconnect(SIGNAL(clicked()));
                button->setText(newActions.at(i)->text());
                button->setIcon(newActions.at(i)->icon());
            } else {
                // create new button
                if (!newActions.at(i)->icon().isNull()) {
                    button = new HbTouchKeypadButton(d->mOwner, newActions.at(i)->icon(), newActions.at(i)->text(), this);
                } else {
                    button = new HbTouchKeypadButton(d->mOwner, newActions.at(i)->text(), this);
                }
                d->mButtonList.append(button);
                layout->addItem(button);
            }

            button->setButtonType(HbTouchKeypadButton::HbTouchButtonNormal);
            button->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
            if (newActions.at(i)->text().isEmpty()) {
				button->setObjectName( "Custom button list button " + QString::number(i));
            } else {
                button->setObjectName( "Custom button list " + newActions.at(i)->text());
            }
            button->setToolTip(newActions.at(i)->toolTip());
            button->setEnabled(newActions.at(i)->isEnabled()); 
            
            // make click connection
            connect(button, SIGNAL(clicked()), newActions.at(i), SLOT(trigger()));
            ++j;
        }
    }

    // remove extra buttons from layout
    while(j < d->mButtonList.count()) {
        HbTouchKeypadButton* button = d->mButtonList.at(j);
        d->mButtonList.removeAt(j);
        layout->removeItem(button);
        button->disconnect(SIGNAL(clicked()));
        delete button;
    }
}

/*!
\deprecated HbInputCustomButtonList::count()
    is deprecated. HbInputCustomButtonList will be removed.
*/
int HbInputCustomButtonList::count()
{
    Q_D(HbInputCustomButtonList);
    return d->mButtonList.count();
}

/*!
\deprecated HbInputCustomButtonList::orientation()
    is deprecated. HbInputCustomButtonList will be removed.
*/
Qt::Orientation HbInputCustomButtonList::orientation()
{
    return static_cast<QGraphicsLinearLayout*>(contentWidget()->layout())->orientation();
}

/*!
\deprecated HbInputCustomButtonList::setOrientation(Qt::Orientation)
    is deprecated. HbInputCustomButtonList will be removed.
*/
void HbInputCustomButtonList::setOrientation(Qt::Orientation orientation)
{
    static_cast<QGraphicsLinearLayout*>(contentWidget()->layout())->setOrientation(orientation);
    updatePrimitives();
}

/*!
\deprecated HbInputCustomButtonList::setGeometry(const QRectF&)
    is deprecated. HbInputCustomButtonList will be removed.

\reimp
*/
void HbInputCustomButtonList::setGeometry(const QRectF & rect)
{
    // Override HbDialog setGeometry, which would center the widget.
    QGraphicsWidget::setGeometry(rect);
} 

// End of file
