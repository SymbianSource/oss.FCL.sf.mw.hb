/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbWidgets module of the UI Extensions for Mobile.
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

#include "hbdatepickerviewitem_p.h"
#include "hbtapgesture.h"

#include "hblistviewitem_p.h"
#include <QGestureEvent>
#include <QCoreApplication>
#include <hbwidgetfeedback.h>

#include <QPainter>
#include <qmath.h>
#include <QtDebug>
#include <hbcolorscheme.h>
#include <hbevent.h>
#include <QDynamicPropertyChangeEvent>
#include <hbtextitem.h>
#include <QTextLayout>
#if 0
#define DEBUG qDebug()
#else
#define DEBUG if(0)qDebug()
#endif

#undef ZOOMTEXT
class HbRotatingText : public HbTextItem
{
public:
    HbRotatingText(QGraphicsItem* parent =0 ):HbTextItem(parent)
    {
        mTextCol = HbColorScheme::color("qtc_tumbler_normal");
        mSelTextCol = HbColorScheme::color("qtc_tumbler_selected");

        setFlag(ItemSendsGeometryChanges);
        setFlag(ItemSendsScenePositionChanges);
        if(parent){
            parent->setFlag(ItemSendsGeometryChanges);
            parent->setFlag(ItemSendsScenePositionChanges);
        }
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);
        QTransform oldTrans = painter->transform();
        QBrush oldBrush = painter->brush();
        bool atialiased = false;
        if(!painter->testRenderHint(QPainter::Antialiasing)){
            painter->setRenderHint(QPainter::Antialiasing);
            atialiased = true;
        }
        if(refereceItem){
            QRectF viewBounds = refereceItem->mapToScene(refereceItem->boundingRect()).boundingRect();
            QRectF itemBounds = mapToScene(this->boundingRect()).boundingRect();
            if(viewBounds.intersect(itemBounds).isValid()){

                QRectF refbounds = mapFromItem(refereceItem,refereceItem->boundingRect()).boundingRect();


                qreal dy = boundingRect().center().y()- refbounds.center().y();
                qreal radius = (refbounds.height()+boundingRect().height())/2;
                QLinearGradient gr(QPointF(refbounds.center().x(),refbounds.center().y()-radius),
                                   QPointF(refbounds.width()/2,refbounds.center().y() + radius));
                gr.setColorAt(0, mTextCol);
                gr.setColorAt(0.5, mSelTextCol);
                gr.setColorAt(1, mTextCol) ;
                QPen pen;
                pen.setBrush(QBrush(gr));
                painter->setPen(pen);
                painter->setBrush(QBrush(gr));

                #if QT_VERSION >= 0x040700
                if(rotate){

                    qreal ratio = dy/radius;
                    qreal absRatio = qAbs(ratio);
                    if(absRatio <= 1.0){
                        qreal angle = qAsin(ratio);
                        DEBUG<<"Angle"<<angle;
                        QTransform current = painter->transform();
                        painter->setTransform(current.rotateRadians(angle,Qt::XAxis));

                    }

                }
                dy = qAbs(dy);
                #else
                dy = (dy<0)?-dy:dy;
                #endif



                QFont thefont = font();

//                qreal len = (radius-dy);

//                if(len >0 ){
//                    qreal pixeSize = thefont.pixelSize();
//                    qreal ratio = (0.5+len/radius);

//                    pixeSize = ((ratio>1)?1:ratio) *pixeSize;
//                    thefont.setPixelSize((int)pixeSize);
//                    painter->setFont(thefont);

//                }

                QFontMetrics fm(thefont);
                QPointF center = boundingRect().center();
                QSizeF textSize = fm.size(Qt::TextSingleLine,text);
                qreal middlex = center.x()-(textSize.width()/2);
                qreal middley = center.y()-textSize.height()/2;

                textlayout.setFont(thefont);

                textlayout.setText(text);
                textlayout.beginLayout();
                textlayout.createLine();
                textlayout.endLayout();
                textlayout.draw(painter,QPointF(middlex,middley));



//                for (int i=0; i<paths.size(); ++i) {
//                    QPainterPath path = lensDeform(paths[i],
//                                                   QPointF(size().width()/2-m_pathBounds.width()/2,
//                                                           size().height()/2-m_pathBounds.height()/2));


//                    painter->drawPath(path);
//                }

            }



        }
        //HbTextItem::paint(painter,option,widget);





        if(atialiased){
            painter->setRenderHint(QPainter::Antialiasing,false);
        }

        painter->setTransform(oldTrans);
        painter->setBrush(oldBrush);
    }
    void setReferenceItem(QGraphicsItem* item)
    {
        refereceItem = item;
    }
    void changeEvent(QEvent *event)
    {

        if (event->type() == HbEvent::ThemeChanged) {
            mTextCol = HbColorScheme::color("qtc_tumbler_normal");
            mSelTextCol = HbColorScheme::color("qtc_tumbler_selected");

        }

        // Call base class version of changeEvent()
        return HbTextItem::changeEvent(event);
    }
    QVariant itemChange(GraphicsItemChange change, const QVariant &value)
    {
        if(change == ItemScenePositionHasChanged){

        }
        return HbTextItem::itemChange(change,value);

    }
//    void gestureEvent(QGestureEvent *event)
//    {
//        HbTapGesture *gesture = static_cast<HbTapGesture *>(event->gesture(Qt::TapGesture));
//        if(gesture){
//            switch(gesture->state()){
//            case Qt::GestureUpdated:

//                HbRotatingText::rotate = !HbRotatingText::rotate;
//            }

//        }
//        event->ignore();
//    }

    void setText(const QString& text);
    QPainterPath lensDeform(const QPainterPath &source, const QPointF &offset);
    bool event(QEvent *e);
    QGraphicsItem* refereceItem;
    QColor mTextCol,mSelTextCol;
    QString text;
    QVector<QPainterPath> paths;
    QRectF m_pathBounds;
    QTextLayout textlayout;
    static bool rotate;


};
bool HbRotatingText::rotate = true;

QPainterPath HbRotatingText::lensDeform(const QPainterPath &source, const QPointF &offset)
{

    QPainterPath path;
    path.addPath(source);
    if(1){
        qreal flip = 70 / qreal(100);
        QPointF centerPos;
        qreal radius;
        if(refereceItem){
           QRectF refbounds = mapFromItem(refereceItem,refereceItem->boundingRect()).boundingRect();
           centerPos=refbounds.center();
           radius = refereceItem->boundingRect().height()/2;


        }
        else{
            centerPos=source.boundingRect().center();
            radius = source.boundingRect().height();
        }

        for (int i=0; i<path.elementCount(); ++i) {
            const QPainterPath::Element &e = path.elementAt(i);

            qreal x = e.x + offset.x();
            qreal y = e.y + offset.y();

            qreal dx =0;//x - centerPos.x();
            qreal dy = y - centerPos.y();

            qreal len = radius - qSqrt(dx * dx + dy * dy);

            if (len > 0 ) {
                path.setElementPositionAt(i,
                                          x + flip * dx *len / (radius*2),
                                          y + flip * dy * len / (radius+2*len));
            } else {
                path.setElementPositionAt(i, x, y);
            }

        }



    }

    return path;
}

bool HbRotatingText::event(QEvent *e)
{

    switch(e->type()){
        case QEvent::FontChange:
         setText(text);
         break;
         default:
             break;

        }
    return HbWidgetBase::event(e);
}

void HbRotatingText::setText(const QString& text)
{
    HbTextItem::setText(text);
    this->text = text;
//    QFontMetrics fm(font());

//    paths.clear();
//    m_pathBounds = QRect();

//    QPointF advance(0, 0);

//    bool do_quick = true;
//    for (int i=0; i<text.size(); ++i) {
//        if (text.at(i).unicode() >= 0x4ff && text.at(i).unicode() <= 0x1e00) {
//            do_quick = false;
//            break;
//        }
//    }

//    if (do_quick) {
//        for (int i=0; i<text.size(); ++i) {
//            QPainterPath path;
//            path.addText(advance, font(), text.mid(i, 1));
//            m_pathBounds |= path.boundingRect();
//            paths << path;
//            advance += QPointF(fm.width(text.mid(i, 1))+2, 0);
//        }
//    } else {
//        QPainterPath path;
//        path.addText(advance, font(), text);
//        m_pathBounds |= path.boundingRect();
//        paths << path;
//    }

//    for (int i=0; i<paths.size(); ++i){
//        paths[i] = paths[i] * QMatrix(1, 0, 0, 1, -m_pathBounds.x(), -m_pathBounds.y());
//       // paths[i] = paths[i] * QMatrix(1, 0, 0, 1, 0, m_pathBounds.height()/2);
//    }

    update();
}

class HbDatePickerViewItemPrivate:public HbListViewItemPrivate
{
public:
    HbDatePickerViewItemPrivate(HbDatePickerViewItem *prototype);
    void tapTriggered(QGestureEvent *event);
#ifdef ZOOMTEXT
    HbRotatingText *zoomText;
#endif

    Q_DECLARE_PUBLIC(HbDatePickerViewItem)
};







HbDatePickerViewItemPrivate::HbDatePickerViewItemPrivate(HbDatePickerViewItem *prototype):
        HbListViewItemPrivate(prototype)
#ifdef ZOOMTEXT
        ,zoomText(0)
#endif
{
}

void HbDatePickerViewItemPrivate::tapTriggered(QGestureEvent *event)
{
    Q_Q(HbDatePickerViewItem);

    HbTapGesture *gesture = static_cast<HbTapGesture *>(event->gesture(Qt::TapGesture));
    QPointF position = event->mapToGraphicsScene(gesture->hotSpot());
    position = q->mapFromScene(position);

    switch (gesture->state()) {
        case Qt::GestureStarted: {
            setPressed(true, true);
            emit q->pressed(position);
            break;
        }
        case Qt::GestureUpdated: {
            if (gesture->tapStyleHint() == HbTapGesture::TapAndHold
                && mSharedData->mItemView
                && mSharedData->mItemView->longPressEnabled()) {
                setPressed(false, true);

            }
            break;
        }
        case Qt::GestureFinished: {
            if (gesture->tapStyleHint() == HbTapGesture::Tap
                || (mSharedData->mItemView
                && !mSharedData->mItemView->longPressEnabled())) {
                setPressed(false, true);

                HbWidgetFeedback::triggered(q, Hb::InstantReleased, 0);
                HbWidgetFeedback::triggered(q, Hb::InstantClicked);
                QPointer<HbAbstractViewItem> item = q;
              //  emit item->activated(position);
                // this viewItem may be deleted in the signal handling, so guarded pointer is used to
                // to ensure that the item still exists when item is used
                if (item) {
                    q->emitReleased(position);

                }
            } else {
                HbWidgetFeedback::triggered(q, Hb::InstantReleased,0);
                q->emitReleased(position);
            }

            break;
        }
        case Qt::GestureCanceled: {
            // hides focus immediately
            setPressed(false, false);

            emit q->released(position);
            break;
        }
        default:
            break;
    }

    event->accept();

}
HbDatePickerViewItem::HbDatePickerViewItem(QGraphicsItem* parent)
    :HbListViewItem(*(new HbDatePickerViewItemPrivate(this)),parent)
{
    HB_SDD(HbAbstractViewItem);
    sd->mItemType = QString("datepickerviewitem");
    setFocusPolicy(Qt::NoFocus);
    setFlag(ItemSendsGeometryChanges);
    setFlag(ItemSendsScenePositionChanges);
}

HbDatePickerViewItem::HbDatePickerViewItem(const HbDatePickerViewItem& other)
    :HbListViewItem(*(new HbDatePickerViewItemPrivate(*other.d_func())),0)
{
    Q_D(HbDatePickerViewItem);
    d->q_ptr = this;

    d->init();

}

HbAbstractViewItem *HbDatePickerViewItem::createItem()
{
    HbDatePickerViewItem* item = new HbDatePickerViewItem(*this);
    item->setFocusPolicy(Qt::NoFocus);
    connect(item,SIGNAL(released(QPointF)),item->itemView(),SLOT(_q_itemSelected(QPointF)));
    return item;
}

void HbDatePickerViewItem::updateChildItems()
{
    Q_D(HbDatePickerViewItem);
    if (d->mIndex.data(Qt::DisplayRole).isNull())
         return;

#ifdef ZOOMTEXT
    if(!d->zoomText){
        d->zoomText = new HbRotatingText(this);
        style()->setItemName(d->zoomText,"text-1");
        d->zoomText->setReferenceItem(itemView());
        //d->zoomText->setReferenceItem((itemView()->primitive("highlight")));
    }
    d->zoomText->setText(d->mIndex.data(Qt::DisplayRole).toString());
#else
    HbListViewItem::updateChildItems();
#endif
    if(d->mSelectionItem){
        d->mSelectionItem->hide();
    }
}

void HbDatePickerViewItem::gestureEvent(QGestureEvent *event)
{
    HbTapGesture *gesture = static_cast<HbTapGesture *>(event->gesture(Qt::TapGesture));
    if (gesture) {
        Q_D(HbDatePickerViewItem);
        if(gesture->state() == Qt::GestureCanceled){
            d->setPressed(false, false);
            return;
        }
        if(itemView()->isScrolling()){
            event->ignore();
            event->ignore(Qt::TapGesture);
            return;
        }
        d->tapTriggered(event);
    } else {
        HbWidget::gestureEvent(event);

    }
}

QSizeF HbDatePickerViewItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint)
{
    HB_SDD(const HbAbstractViewItem);
    static qreal height=0;

    QSizeF sh=HbListViewItem::sizeHint(which,constraint);
    if(which == Qt::PreferredSize && sh.height()<=0) {
        //TODO:remove this check once refresh issue in DTP is solved on removeRows/insertRows
        if(height<=0) {
            //Let's create a temporary item and take the height for the size hint
            HbAbstractViewItem *tempitem = sd->mPrototype->createItem();
            if(sd->mItemView) {
                QAbstractItemModel *model = sd->mItemView->model();
                if(model) {
                    int rowCount=model->rowCount();
                    QModelIndex ind=model->index(rowCount,0);
                    if(ind.isValid()) {
                        tempitem->setModelIndex(ind);
                    }
                }
            }


            //call polish
            QEvent polishEvent(QEvent::Polish);
            QCoreApplication::sendEvent(const_cast<HbAbstractViewItem *>(tempitem), &polishEvent);
            height=tempitem->effectiveSizeHint(which,constraint).height();
            delete tempitem;
        }
        return QSizeF(sh.width(),height);
    }
    return sh;
}

void HbDatePickerViewItem::emitReleased(const QPointF& point)
{
     emit HbAbstractViewItem::released(point);
}

/*!
    \reimp
*/
int HbDatePickerViewItem::type() const
{
    return Type;
}
void HbDatePickerViewItem::polish(HbStyleParameters &params)
{


    HbListViewItem::polish(params);

}

bool HbDatePickerViewItem::event(QEvent *e)
{
#ifdef ZOOMTEXT
    Q_D(HbDatePickerViewItem);
    switch(e->type()){

    case QEvent::DynamicPropertyChange:{
                QDynamicPropertyChangeEvent *dynProp = static_cast<QDynamicPropertyChangeEvent *>(e);
                if (!qstrcmp(dynProp->propertyName(), "state")) {
                    d->zoomText->setProperty(dynProp->propertyName(),property(dynProp->propertyName()));
                }
                break;
            }
            default:
                break;

        }
#endif
    return HbListViewItem::event(e);
}

QVariant  HbDatePickerViewItem::itemChange(GraphicsItemChange change, const QVariant &value)
{

    return HbListViewItem::itemChange(change,value);
}
