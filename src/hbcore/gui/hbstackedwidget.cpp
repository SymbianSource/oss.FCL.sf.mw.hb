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

#include "hbstackedwidget.h"
#include "hbstackedlayout.h"
#include "hbwidget_p.h"

/*!
    @stable
    @hbcore
    \class HbStackedWidget
    \brief HbStackedWidget manages geometries of stacked layout contents.
    
    HbStackedLayout is a simple container that allows client add several widgets
    and then select one widget to be shown. This widget will manage visibility 
    and focus in addition to size and position. 

    Example code:
    \snippet{stackedwidgetsample.cpp,1}
    
    \sa HbStackedLayout
 */


class HbStackedWidgetPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbStackedWidget)

public:
    HbStackedWidgetPrivate() : mCurrentIndex(-1), mLayout(0) {}

    void setCurrentIndex(int index, QGraphicsWidget *prev, bool hideOld = true, bool showNew = true);
    
    int mCurrentIndex;
    HbStackedLayout *mLayout;
};

/*!
    \brief Sets current widget from specified index.
    \param index position of desired widget.
    \param prev previous widget.
 */
void HbStackedWidgetPrivate::setCurrentIndex(int index, QGraphicsWidget *prev, bool hideOld, bool showNew)
{
    Q_Q(HbStackedWidget);
    if (index < 0 || index >= mLayout->count()) {
        qWarning("HbStackedWidget::setCurrentIndex: index %i is out of range", index);
        return;
    }

    QGraphicsWidget *next = q->widgetAt(index);
    if (next == prev)
        return;

    QGraphicsWidget* focused = 0;
    if (prev) {
        focused = prev->focusWidget();
        // Set previous widget invisible. Focus will be reset after this statement.
        if (hideOld) {
            prev->setVisible(false);
        }
    }
    mCurrentIndex = index;
    if (showNew) {
        next->setVisible(true);
    }

    // set the focus if prev was focusable
    if (focused) {
        next->setFocus();
    }
    emit q->currentChanged(index);
}

/*!
    Constructor.
    \param parent parent graphics item
 */
HbStackedWidget::HbStackedWidget(QGraphicsItem *parent)
    : HbWidget(*new HbStackedWidgetPrivate, parent)
{
    Q_D(HbStackedWidget);
    d->q_ptr = this;

    HbStackedLayout *layout = new HbStackedLayout;
    setLayout( layout );
    d->mLayout = layout;
}

/*!
    Destructor
 */
HbStackedWidget::~HbStackedWidget()
{
}

/*!
    \brief Returns amount of widgets.
    \return amount of widgets.
 */
int HbStackedWidget::count() const
{
    Q_D(const HbStackedWidget);
    return d->mLayout->count();
}

/*!
    \brief Adds a widget.

    This is a convenience function which will internally call the \c insertWidget method.

    \param widget widget to be added.
    \return index of the added widget in the stack.
 */
int HbStackedWidget::addWidget(QGraphicsWidget *widget)
{
    return insertWidget(-1, widget);
}

/*!
    \brief Inserts a widget.

    Inserts the given \a widget to given position \a index.
    Takes ownership of the widget.

    If \a index is out of range, the widget is appended (in which case it is the actual
    index of the widget that is returned).

    If the stack was empty before this function is called, the given widget becomes
    the current widget and it will be set visible. Otherwise the widget will be set
    invisible.

    Inserting a new widget at an index less than or equal to the current index will increment
    the current index, but keep the current widget.

    \param index index to which to add.
    \param widget widget to be inserted.
    \return index of the added widget in the stack.
 */
int HbStackedWidget::insertWidget(int index, QGraphicsWidget *widget)
{
    Q_D(HbStackedWidget);
    int addedIndex = d->mLayout->insertItem(index, widget); // this will usually reparent 'widget'
    if ( addedIndex != -1 ) {
        // Need to store current index, since it might change
        // during "widgetAdded" signal (someone might call back
        // e.g. "setCurrentIndex".
        int currentIndex = d->mCurrentIndex;
        emit widgetAdded(addedIndex);
        if ( currentIndex == d->mCurrentIndex ) {
            // Current index not touched from outside.
            if (d->mCurrentIndex < 0) {
                setCurrentIndex(addedIndex);
            } else  {
                widget->setVisible(false);
                if (addedIndex <= d->mCurrentIndex) {
                    d->mCurrentIndex++;
                    emit currentChanged(d->mCurrentIndex);
                }
            }
        }
    }
    return addedIndex;
}

/*!
    \brief Removes a widget.

    Removes the given \a widget from the stack. The widget is not deleted.
    The widget is set invisible.

    Removing the current widget will set the next widget (or previous
    if the current widget was the last one) to be the current widget.

    Removing a widget from an index less than the current index will
    decrement the current index, but keep the current widget.

    \param widget widget to be removed.
 */
void HbStackedWidget::removeWidget(QGraphicsWidget *widget)
{
    Q_D(HbStackedWidget);
    int index = indexOf(widget);
    if ( index == -1 ) {
        return;
    }
    d->mLayout->removeAt(index);

    if ( index == d->mCurrentIndex ) {
        d->mCurrentIndex = -1;
        int c = count();
        if ( c > 0 ) {
            int newIndex = (index == c) ? index-1 : index;
            d->setCurrentIndex(newIndex, widget);
        } else {
            // The last widget was removed.
            widget->setVisible(false);
            emit currentChanged( -1 );
        }
    } else if ( index <= d->mCurrentIndex ) {
        d->mCurrentIndex--;
        emit currentChanged(d->mCurrentIndex);
    }
    widget->setParentItem(0);
    emit widgetRemoved(index);
    emit widgetRemoved(widget);
}

/*!
    \brief Removes a widget from specified position.

    This is a convenience function which will internally call the \c removeWidget method.

    The removed widget is not deleted but returned to caller.

    \param index position of the widget to be removed.
    \return the removed widget.
 */
QGraphicsWidget *HbStackedWidget::removeAt(int index)
{
    QGraphicsWidget *widget = widgetAt(index);
    if ( widget ) {
        removeWidget(widget);
    }
    return widget;
}


/*!
    \brief Returns position of specified widget.
    \param widget widget to look for.
 */
int HbStackedWidget::indexOf(QGraphicsWidget *widget) const
{
    Q_D(const HbStackedWidget);
    return d->mLayout->indexOf(widget);
}

/*!
    \brief Returns index of current widget.
    \return current index, or -1 if undefined.
 */
int HbStackedWidget::currentIndex() const
{
    Q_D(const HbStackedWidget);
    return d->mCurrentIndex;
}

/*!
    \brief Returns widget at specified position.
    \param index position of widget.
 */
QGraphicsWidget *HbStackedWidget::widgetAt(int index) const
{
    Q_D(const HbStackedWidget);
    return static_cast<QGraphicsWidget*>(d->mLayout->itemAt(index));
}

/*!
    \brief Returns current widget.
    \return current widget, or 0 if undefined.
 */
QGraphicsWidget *HbStackedWidget::currentWidget() const
{
    Q_D(const HbStackedWidget);
    return widgetAt(d->mCurrentIndex);
}

/*! \fn void HbStackedWidget::widgetAdded(int index)
    \brief This signal is emitted when a widget is added to this widget.
    \param index position of the added widget.
 */

/*! \fn void HbStackedWidget::widgetRemoved(int index)
    \brief This signal is emitted when a widget is removed from this widget.
    \param index position of the removed widget.
 */

/*! \fn void HbStackedWidget::widgetRemoved(QGraphicsWidget *widget)
    \brief This signal is emitted when a widget is removed from this widget.
    \param widget the removed widget.
 */

/*! \fn void HbStackedWidget::currentChanged(int index)
    \brief This signal is emitted when the current widget is changed.

    The signal is emitted whenever either the current index or the current
    widget changes. In some cases (e.g. when removing a widget before the
    current widget) the current index changes but current widget doesn't and
    in some other cases (e.g. when removing the current widget) the current
    widget changes but the current index doesn't.

    \param index position of the new current widget.
 */

 /*!
    \brief Sets current widget from specified index.

    This method will set the widget at given \a index visible and 
    the previous widget invisible.

    \param index position of desired widget.
 */
void HbStackedWidget::setCurrentIndex(int index)
{
    Q_D(HbStackedWidget);
    d->setCurrentIndex(index, currentWidget());
}

/*!
    \brief Sets current widget.

    This is a convenience function, same as finding the widget's index
    and calling setCurrentIndex().

    \param widget desired widget.
 */
void HbStackedWidget::setCurrentWidget(QGraphicsWidget *widget)
{
    Q_D(HbStackedWidget);
    int index = d->mLayout->indexOf(widget);
    if (index == -1) {
        qWarning("HbStackedWidget::setCurrentWidget: Widget %p not contained in stack", widget);
        return;
    }
    d->setCurrentIndex(index, currentWidget());
}

/*!
    \brief Sets current widget.

    This is a convenience function which will internally call the \c setCurrentIndex method.

    \param widget Desired widget.
    \param hideOld Indicates of the old widget (from which we are switching away) should be made invisible or not.
    \param showNew Indicates if the new widget (\a widget) should be made visible or not.
 */
void HbStackedWidget::setCurrentWidget(QGraphicsWidget *widget, bool hideOld, bool showNew)
{
    Q_D(HbStackedWidget);
    int index = d->mLayout->indexOf(widget);
    if (index == -1) {
        qWarning("HbStackedWidget::setCurrentWidget: Widget %p not contained in stack", widget);
        return;
    }
    d->setCurrentIndex(index, currentWidget(), hideOld, showNew);
}
