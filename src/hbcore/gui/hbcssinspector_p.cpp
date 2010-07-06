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

#include "hbcssinspector_p.h"

#ifdef HB_CSS_INSPECTOR
#include <hbanchor.h>
#include <hbanchorarrowdrawer_p.h>
#include <hbcolorscheme.h>
#include <hbcssformatter_p.h>
#include <hbevent.h>
#include <hbframedrawer.h>
#include <hbframeitem.h>
#include <hbiconitem.h>
#include <hbinstance.h>
#include <hblayeredstyleloader_p.h>
#include <hbmainwindow_p.h>
#include <hbmarqueeitem.h>
#include <hbnamespace_p.h>
#include <hbscreen_p.h>
#include <hbtextitem.h>
#include <hbwidgetloadersyntax_p.h>
#include <hbxmlloaderabstractsyntax_p.h>
#include <hbwidgetbase_p.h>
#include <hbwidget_p.h>

#include <QBrush>
#include <QCheckBox>
#include <QGraphicsLayout>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPen>
#include <QPainter>
#include <QPointer>
#include <QRadioButton>
#include <QSizePolicy>
#include <QSplitter>
#include <QTextEdit>
#include <QTreeView>
#include <QVBoxLayout>
#include <QXmlStreamWriter>
#include <qmath.h>

#define NODEPTR_N(x) HbCss::StyleSelector::NodePtr n = {n.ptr = (void *)x};

const QString CSS_HTML_HEADER("<style type=\"text/css\"> \
                                .overridden {color:#999; text-decoration:line-through;} \
                                .filename {background-color: #e0e0e0; margin:0;} \
                                .selectors {margin:0;} \
                                .selector {color:#000;} \
                                .pseudo {font-weight:bold;} \
                                .attr {font-style:italic;} \
                                .property {color:#00f; font-weight:bold;} \
                                .value {color:#f00;} \
                                .overridden .property, .overridden .value {color:#999;} \
                                </style>");
const QString WIDGETML_HTML_HEADER("<style type=\"text/css\"> \
                                     pre {color:#999; font-family:Arial;} \
                                     span {color:#000; font-weight:bold;} \
                                     </style><pre>");
const QString WIDGETML_HTML_FOOTER("</pre>");

const QStringList SKIPPED_CHILD_ITEMS(QStringList() << "HbTextItem" << "HbTouchArea"
        << "HbIconItem" << "HbFrameItem" << "HbSpacerItem" << "HbWidgetBase" 
        << "HbMarqueeItem" << "QGraphicsWidget" << "QGraphicsItem");

const QStringList SKIPPED_GLOBAL_ITEMS(QStringList() << "HbPopupLayoutManager" << "HbAnchorArrowDrawer" << "HbCssInfoDrawer");

const qreal HOVER_BOX_PEN_WIDTH = 2.0;
const qreal GUIDE_LINE_WIDTH = 1.0;
const QChar BIG_NUMBER_CHAR = 0x221E;
const QString TEXT_COLOR("qtc_default_main_pane_normal");
const QString LINE_COLOR("qtc_view_visited_normal");
const int ABOVE_POPUP_ZVALUE = 5000;
const qreal SIZE_PREF_DRAW_SIZE = 7.0;
const qreal SIZE_PREF_MINIMUM_THRESHOLD = 4.0 * SIZE_PREF_DRAW_SIZE;
const qreal SIZE_PREF_LINE_WIDTH = 1.0;
const qreal SIZE_PREF_ALLOWED_OVERLAP = 2.0;
const qreal SIZE_PREF_BOX_SIZE = 0.4 * SIZE_PREF_DRAW_SIZE;

static QString itemClass(const QGraphicsItem *item)
{
    if (item && item->isWidget()) {
        const QGraphicsWidget *widget = static_cast<const QGraphicsWidget *>(item);
        return widget->metaObject()->className();
    }
    return "QGraphicsItem";
}

static QString objName(const QGraphicsItem *item)
{
    if (item && item->isWidget()) {
        const QGraphicsWidget *widget = static_cast<const QGraphicsWidget *>(item);
        return widget->objectName();
    }
    return QString();
}

static QString itemValue(const QGraphicsItem *item)
{
    QString itemClassName(itemClass(item));
    if (itemClassName == "HbIconItem") {
        const HbIconItem *icon = static_cast<const HbIconItem*>(item);
        return icon->iconName();
    } else if (itemClassName == "HbTextItem") {
        const HbTextItem *text = static_cast<const HbTextItem*>(item);
        return text->text();
    } else if (itemClassName == "HbFrameItem") {
        const HbFrameItem *frame = static_cast<const HbFrameItem*>(item);
        return frame->frameDrawer().frameGraphicsName();
    } else if (itemClassName == "HbMarqueeItem") {
        const HbMarqueeItem *marquee = static_cast<const HbMarqueeItem*>(item);
        return marquee->text();
    }
    return QString();
}

static bool parentHasAnchorLayout(const QGraphicsItem *item)
{
    if (item) {
        if (const QGraphicsItem *parent = item->parentItem()) {
            if (const QGraphicsWidget *widget = static_cast<const QGraphicsWidget*>(parent)) {
                if (const QGraphicsLayout *layout = widget->layout()) {
                    if (const HbAnchorLayout *anchor = dynamic_cast<const HbAnchorLayout*>(layout)) {
                        Q_UNUSED(anchor)
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

static QString itemInParentLayout(const QGraphicsItem *item)
{
    static QMap<const QGraphicsLayout*, QSet<const QGraphicsItem*> > itemCache;
    if (item) {
        if (const QGraphicsWidget *widget = static_cast<const QGraphicsWidget*>(item->parentItem())) {
            QGraphicsLayout *layout = widget->layout();
            QSet<const QGraphicsItem*> itemsInLayout;
            if (itemCache.contains(layout)) {
                itemsInLayout = itemCache[layout];
            } else {
                HbAnchorLayout *anchorLayout = dynamic_cast<HbAnchorLayout*>(layout);
                if (!anchorLayout) {
                    return QString(); // Non-anchor layout
                } else {
                    foreach (HbAnchor *anchor, anchorLayout->effectiveAnchors()) {
                        itemsInLayout << anchor->startItem()->graphicsItem();
                        itemsInLayout << anchor->endItem()->graphicsItem();
                    }
                    itemCache[layout] = itemsInLayout;
                }
            }
            return itemsInLayout.contains(item) ? "Yes" : "No";
        }
    }
    return QString(); // Non-widget parent
}

static QString cssItemText(const QGraphicsItem *item)
{
    QString txt(itemClass(item));
    if (item && item->isWidget()) {
        // Add objectname
        QString objectName(objName(item));
        if (objectName.length() > 0) {
            txt.append("#");
            txt.append(objectName);
        }
    }
    // Add itemname
    QString name(HbStyle::itemName(item));
    if (name.length() > 0) {
        txt.append("::");
        txt.append(name);
    }
    return txt;
}


static QString convertMeasurementToText(const QGraphicsItem *item, qreal hint)
{
    qreal unit = HbDeviceProfile::profile(item).unitValue();

    if (unit != 0) {
        hint = (hint / unit);
        }
    QString hintText(QString::number(hint, 'g', 2));
    if (hintText.contains('+')) {
        hintText = BIG_NUMBER_CHAR;
    } else {
        if (hint !=0 && unit != 0) {
            hintText += "un";
        }
    }
    return hintText;
}

static QString convertEffectiveSizeHintToText(const QGraphicsWidget *item, Qt::SizeHint which)
{
    QString hintText('(');
    if (item) {
        const QSizeF &size = item->effectiveSizeHint(which);
        hintText += convertMeasurementToText(item, size.width()) + ',';
        hintText += convertMeasurementToText(item, size.height());
    }
    hintText += ')';
    return hintText;
}

static QString cssItemHintText(const QGraphicsItem *item)
{
    QString sizeHint;
    if (item && item->isWidget()) {
        const QGraphicsWidget *widget = static_cast<const QGraphicsWidget*>(item);
        sizeHint += convertEffectiveSizeHintToText(widget, Qt::MinimumSize)+'|';
        sizeHint += convertEffectiveSizeHintToText(widget, Qt::PreferredSize)+'|';
        sizeHint += convertEffectiveSizeHintToText(widget, Qt::MaximumSize);
    }
    return sizeHint;
}

static QString cssSizePolicyText(const QGraphicsItem *item, Qt::Orientation dir)
{
    if (!item || !item->isWidget()) {
        return QString();
    }
    const HbWidget *widget = static_cast<const HbWidget*>(item);
    QSizePolicy::Policy pol = dir == Qt::Vertical
        ? widget->sizePolicy().verticalPolicy()
        : widget->sizePolicy().horizontalPolicy();

    if (pol == QSizePolicy::Fixed) {
        return "Fixed";
    } else if (pol == QSizePolicy::Minimum) {
        return "Minimum";
    } else if (pol == QSizePolicy::Maximum) {
        return "Maximum";
    } else if (pol == QSizePolicy::Preferred) {
        return "Preferred";
    } else if (pol == QSizePolicy::MinimumExpanding) {
        return "Minimum expanding";
    } else if (pol == QSizePolicy::Expanding) {
        return "Expanding";
    } else if (pol == QSizePolicy::Ignored) {
        return "Ignored";
    } else {
        return "[Unrecognised]";
    }
}

static QRectF cssItemHintRect(const QGraphicsItem *item, Qt::SizeHint which)
{
    QRectF hintRect;
    if (item && item->isWidget()) {
        const QGraphicsWidget *widget = static_cast<const QGraphicsWidget*>(item);
        const QSizeF &size = widget->effectiveSizeHint(which);
        hintRect.setWidth(size.width());
        hintRect.setHeight(size.height());
    }
    return hintRect;
}


static QString anchorEdgeName(Hb::Edge edge)
{
    QString name;
    switch (edge) {
        // these are hardwired inside the parser
        case Hb::TopEdge: name = QString("TOP"); break;
        case Hb::BottomEdge: name = QString("BOTTOM"); break;
        case Hb::LeftEdge: name = QString("LEFT"); break;
        case Hb::RightEdge: name = QString("RIGHT"); break;
        case Hb::CenterHEdge: name = QString("CENTERH"); break;
        case Hb::CenterVEdge: name = QString("CENTERV"); break;
        }
    return name;
}

QString HbCssInspectorWindow::anchorItemName(QGraphicsLayoutItem* item, QGraphicsLayout* layout, bool& isIdBased)
{
    isIdBased = true;
    QString name;
    if ( item != layout ) {
        QGraphicsItem *gItem = item->graphicsItem();
        if ( gItem ) {
            name = HbStyle::itemName( gItem );
            if (name.isEmpty()) {
                isIdBased = false;
                name = objName(gItem);
                if (name.isEmpty()) {
                    name = QString("<%1>").arg(itemClass(gItem));
                }
            }
        }
        if ( name.isEmpty() ) {
            name = QString("<unknown>");
        }
    }

    return name;
}


QString HbCssInspectorWindow::anchorsToHtmlInfo(HbAnchorLayout *anchorLayout, const QString itemName, const QString layoutName)
{
    QString widgetML;
    QXmlStreamWriter xmlWriter(&widgetML);
    xmlWriter.setAutoFormatting(true);

    HbWidgetLoaderSyntax syntax(new HbWidgetLoaderActions());

    QString str = syntax.lexemValue(HbXmlLoaderAbstractSyntax::TYPE_HBWIDGET);
    xmlWriter.writeStartElement(syntax.lexemValue(HbXmlLoaderAbstractSyntax::TYPE_HBWIDGET));
    xmlWriter.writeAttribute(
        syntax.lexemValue(HbXmlLoaderAbstractSyntax::ATTR_VERSION), 
        HbWidgetLoaderSyntax::version());
    xmlWriter.writeAttribute(syntax.lexemValue(HbXmlLoaderAbstractSyntax::ATTR_TYPE), itemName);

    xmlWriter.writeStartElement(syntax.lexemValue(HbXmlLoaderAbstractSyntax::TYPE_LAYOUT));
    xmlWriter.writeAttribute(syntax.lexemValue(HbXmlLoaderAbstractSyntax::ATTR_NAME), layoutName);
    xmlWriter.writeAttribute(
        syntax.lexemValue(HbXmlLoaderAbstractSyntax::ATTR_TYPE),
        syntax.lexemValue(HbXmlLoaderAbstractSyntax::LAYOUT_ANCHOR));

    if (anchorLayout) {
        foreach (HbAnchor *anchor, anchorLayout->effectiveAnchors()) {
            bool startIdBased, endIdBased;
            QString startName(anchorItemName(anchor->startItem(), anchorLayout, startIdBased));
            QString endName(anchorItemName(anchor->endItem(), anchorLayout, endIdBased));

            xmlWriter.writeStartElement(syntax.lexemValue(HbXmlLoaderAbstractSyntax::AL_ANCHOR));
            xmlWriter.writeAttribute(
                syntax.lexemValue( startIdBased
                    ? HbXmlLoaderAbstractSyntax::AL_SRC_ID
                    : HbXmlLoaderAbstractSyntax::AL_SRC_NAME),
                        startName);
            xmlWriter.writeAttribute(
                syntax.lexemValue(HbXmlLoaderAbstractSyntax::AL_SRC_EDGE), 
                anchorEdgeName(anchor->startEdge()));
            xmlWriter.writeAttribute(
                syntax.lexemValue( endIdBased
                    ? HbXmlLoaderAbstractSyntax::AL_DST_ID
                    : HbXmlLoaderAbstractSyntax::AL_DST_NAME),
                        endName);
            xmlWriter.writeAttribute(
                syntax.lexemValue(HbXmlLoaderAbstractSyntax::AL_DST_EDGE), 
                anchorEdgeName(anchor->endEdge()));
            if ( !anchor->anchorId().isEmpty() ) {
                xmlWriter.writeAttribute(
                    syntax.lexemValue(HbXmlLoaderAbstractSyntax::AL_SPACER),
                    anchor->anchorId());
            }
            xmlWriter.writeEndElement(); // anchoritem
        }

    }
    xmlWriter.writeEndElement(); // layout
    xmlWriter.writeEndElement(); // widgetml

    QString html(widgetML.trimmed());
    html.replace('<', "&lt;");
    html.replace('>', "&gt;");
    html.replace(QRegExp("\"([^\"]*)\""), "\"<span>\\1</span>\""); // Add span elements around things in quotes
    html.replace('\"', "&quot;");
    html.replace("\n\n", "\n");
    html.replace('\n', "<br/>");

    html.insert(0, WIDGETML_HTML_HEADER);
    html.append(WIDGETML_HTML_FOOTER);

    return html;
}



/******************************************************************************************/



HbCssInfoDrawer::HbCssInfoDrawer(QGraphicsItem *parent) :
    HbWidgetBase(parent),
    mItemRect(0,0,0,0),
    mMinHintRect(0,0,0,0),
    mPrefHintRect(0,0,0,0),
    mMaxHintRect(0,0,0,0)
{
	updateColors();
	setVisible(false);
}


HbCssInfoDrawer::~HbCssInfoDrawer()
{
}


void HbCssInfoDrawer::changeEvent(QEvent *event)
{
    if (event->type() == HbEvent::ThemeChanged)
        updateColors();
    HbWidgetBase::changeEvent(event);
}


void HbCssInfoDrawer::updateColors()
{
    mTextColor = HbColorScheme::color(TEXT_COLOR);
    mBoxColor = HbColorScheme::color(LINE_COLOR);
}


void HbCssInfoDrawer::updateFocusItem(const QGraphicsItem *item)
{
    // update text and geometry
    if (item) {
        this->setVisible(true);
        mItemRect = item->sceneBoundingRect();
        mItemText = cssItemText(item);
        mHintText = cssItemHintText(item);
        mMinHintRect = cssItemHintRect(item, Qt::MinimumSize);
        mPrefHintRect = cssItemHintRect(item, Qt::PreferredSize);
        mMaxHintRect = cssItemHintRect(item, Qt::MaximumSize);
        // Make sure this is in the same place in the scene as the window
        if (item->isWidget()) {
            const HbWidget *obj = static_cast<const HbWidget*>(item);
            this->setGeometry(obj->mainWindow()->rect());
            mItemPolicy = obj->sizePolicy();
        }
    } else {
        this->setVisible(false);
    }
    this->update();
}

void HbCssInfoDrawer::paintRect(QPainter *painter, QRectF rect)
{
    rect.adjust(
        HOVER_BOX_PEN_WIDTH/2, HOVER_BOX_PEN_WIDTH/2,
        -HOVER_BOX_PEN_WIDTH/2, -HOVER_BOX_PEN_WIDTH/2);
    painter->drawRect(rect);
}

static QPolygonF createTrianglePoints(const QPointF &pointPos, Qt::ArrowType dir)
{
    const qreal size = SIZE_PREF_DRAW_SIZE;
    const qreal half = size / 2.0;
    const qreal x = pointPos.x();
    const qreal y = pointPos.y();

    QPolygonF points;
    points << pointPos;

    switch (dir) {
        case Qt::LeftArrow:
            points << QPointF(x+size, y-half);
            points << QPointF(x+size, y+half);
            break;
        case Qt::RightArrow:
            points << QPointF(x-size, y-half);
            points << QPointF(x-size, y+half);
            break;
        case Qt::UpArrow:
            points << QPointF(x-half, y+size);
            points << QPointF(x+half, y+size);
            break;
        case Qt::DownArrow:
            points << QPointF(x-half, y-size);
            points << QPointF(x+half, y-size);
            break;
        default: // Set points to same position to avoid drawing to origin
            points << pointPos;
            points << pointPos;
    }
    return points;
}

enum LayoutPosition {
    RegularLayout,
    MirroredLayout
};

enum ArrowDirection {
    RegularArrow,
    MirroredArrow
};

static void drawTriangle(QPainter *painter, Qt::Orientation dir, const QRectF &bRect,
        int iconsFromEdge, qreal linePos, LayoutPosition layout, ArrowDirection arrowDir)
{
    QPointF point;

    qreal posFromEdge = iconsFromEdge * SIZE_PREF_DRAW_SIZE;
    if (arrowDir == MirroredArrow) {
        posFromEdge += SIZE_PREF_DRAW_SIZE;
    }

    Qt::ArrowType arrow = Qt::NoArrow;
    if (dir == Qt::Vertical) {
        if (layout == MirroredLayout) {
            posFromEdge += bRect.top();
            arrow = (arrowDir == MirroredArrow) ? Qt::DownArrow : Qt::UpArrow;
        } else {
            posFromEdge = bRect.bottom() - posFromEdge;
            arrow = (arrowDir == MirroredArrow) ? Qt::UpArrow : Qt::DownArrow;
        }
        point = QPointF(linePos, posFromEdge);
    } else {
        if (layout == MirroredLayout) {
            posFromEdge = bRect.right() - posFromEdge;
            arrow = (arrowDir == MirroredArrow) ? Qt::LeftArrow : Qt::RightArrow;
        } else {
            posFromEdge += bRect.left();
            arrow = (arrowDir == MirroredArrow) ? Qt::RightArrow : Qt::LeftArrow;
        }
        point = QPointF(posFromEdge, linePos);
    }

    painter->drawPolygon(createTrianglePoints(point, arrow));
}


static void drawPolicyIcons(
    QPainter *painter, Qt::Orientation direction, QSizePolicy policy, const QRectF &itemRect)
{
    bool vert = direction == Qt::Vertical;
    QSizePolicy::Policy pol = vert ? policy.verticalPolicy() : policy.horizontalPolicy();

    const QBrush fillBrush(Qt::gray, Qt::SolidPattern); //krazy:exclude=qenums
    const QBrush hollowBrush(Qt::white, Qt::SolidPattern);
    const QRectF rect = itemRect.adjusted(HOVER_BOX_PEN_WIDTH/2, HOVER_BOX_PEN_WIDTH/2,
                        -HOVER_BOX_PEN_WIDTH/2, -HOVER_BOX_PEN_WIDTH/2);

    const qreal vLinePos = rect.left() + (rect.width() * 0.8);
    const qreal hLinePos = rect.top() + (rect.height() * 0.2);
    const qreal linePos = qFloor(vert ? vLinePos : hLinePos); // floor to force consistent rounding

    bool drawSecondIcons;
    if (vert) {
        drawSecondIcons = (3*SIZE_PREF_DRAW_SIZE) + rect.top() <=
                hLinePos + SIZE_PREF_ALLOWED_OVERLAP;
        painter->drawLine((int)linePos, (int)(rect.top()), (int)linePos, (int)(rect.bottom()));
    } else {
        drawSecondIcons = rect.right() - (3*SIZE_PREF_DRAW_SIZE) >=
            vLinePos - SIZE_PREF_ALLOWED_OVERLAP;
        painter->drawLine((int)(rect.left()), (int)linePos, (int)(rect.right()), (int)linePos);
    }

    // Ignore icons have different rules
    if (pol & QSizePolicy::IgnoreFlag) {
        // Draw outer triangle
        painter->setBrush(fillBrush);
        drawTriangle(painter, direction, rect, 0, linePos, RegularLayout, RegularArrow);
        if (drawSecondIcons) {
            drawTriangle(painter, direction, rect, 0, linePos, MirroredLayout, RegularArrow);
        }
        // Draw inner triangle
        painter->setBrush(hollowBrush);
        drawTriangle(painter, direction, rect, 2, linePos, RegularLayout, RegularArrow);
        if (drawSecondIcons) {
            drawTriangle(painter, direction, rect, 2, linePos, MirroredLayout, RegularArrow);
        }
    } else {
        // Draw outer triangle
        if (pol & QSizePolicy::GrowFlag) {
            painter->setBrush(pol & QSizePolicy::ExpandFlag ? fillBrush : hollowBrush);
            drawTriangle(painter, direction, rect, 0, linePos, RegularLayout, RegularArrow);
            if (drawSecondIcons) {
                drawTriangle(painter, direction, rect, 0, linePos, MirroredLayout, RegularArrow);
            }
        }
        // Draw box
        painter->setBrush(pol & QSizePolicy::ExpandFlag ? hollowBrush : fillBrush);
        QRectF boxRect(0, 0, SIZE_PREF_DRAW_SIZE, SIZE_PREF_DRAW_SIZE);
        qreal midPoint = (1 + 0.5) * SIZE_PREF_DRAW_SIZE; // Middle of the second icon
        if (vert) {
            boxRect.setHeight(SIZE_PREF_BOX_SIZE);
            boxRect.moveCenter(QPointF(linePos, rect.bottom() - midPoint));
        } else {
            boxRect.setWidth(SIZE_PREF_BOX_SIZE);
            boxRect.moveCenter(QPointF(rect.left() + midPoint, linePos));
        }
        painter->drawRect(boxRect);
        if (drawSecondIcons) {
            if (vert) {
                boxRect.moveCenter(QPointF(linePos, rect.top() + midPoint));
            } else {
                boxRect.moveCenter(QPointF(rect.right() - midPoint, linePos));
            }
            painter->drawRect(boxRect);
        }
        // Draw inner triangle
        if (pol & QSizePolicy::ShrinkFlag)  {
            painter->setBrush(hollowBrush);
            drawTriangle(painter, direction, rect, 2, linePos, RegularLayout, MirroredArrow);
            if (drawSecondIcons) {
                drawTriangle(painter, direction, rect, 2, linePos, MirroredLayout, MirroredArrow);
            }
        }
    }
}

void HbCssInfoDrawer::paint(QPainter *painter,
    const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    const QPen prevPen = painter->pen();
    const Qt::LayoutDirection prevDirection = painter->layoutDirection();
    painter->setLayoutDirection(Qt::LeftToRight);

    if (mShowBox) {
        painter->setPen(QPen(mBoxColor, HOVER_BOX_PEN_WIDTH));
        paintRect(painter, mItemRect);
    }
    if (mShowMinHintBox) {
        painter->setPen(QPen(Qt::blue, HOVER_BOX_PEN_WIDTH)); //krazy:exclude=qenums
        QRectF rect = mMinHintRect;
        rect.moveCenter(mItemRect.center());
        paintRect(painter, rect);
    }
    if (mShowPrefHintBox) {
        painter->setPen(QPen(Qt::green, HOVER_BOX_PEN_WIDTH)); //krazy:exclude=qenums
        QRectF rect = mPrefHintRect;
        rect.moveCenter(mItemRect.center());
        paintRect(painter, rect);
    }
    if (mShowMaxHintBox) {
        painter->setPen(QPen(Qt::red, HOVER_BOX_PEN_WIDTH)); //krazy:exclude=qenums
        QRectF rect = mMaxHintRect;
        rect.moveCenter(mItemRect.center());
        paintRect(painter, rect);
    }

    painter->setPen(mTextColor);
    int fontSize = painter->fontInfo().pixelSize();
    int boxHeight = (int)(mItemRect.height());

    if (mShowItemText && boxHeight - fontSize > 0) {
        QPointF pos = mItemRect.topLeft();
        pos += QPointF(HOVER_BOX_PEN_WIDTH, fontSize);
        painter->drawText(pos, mItemText);
    }

    bool roomForSizeHint = (boxHeight - (2*fontSize) > 0 )
            || (!mShowItemText && (boxHeight - fontSize) > 0);
    if (mShowHintText && roomForSizeHint) {
        QPointF pos = mItemRect.bottomLeft();
        pos += QPointF(HOVER_BOX_PEN_WIDTH, -HOVER_BOX_PEN_WIDTH);
        painter->drawText(pos, mHintText);
    }

    if (mDrawGuideLines) {
        const QRectF &br = this->boundingRect();
        painter->setPen(QPen(mTextColor, GUIDE_LINE_WIDTH, Qt::DashLine));
        // Line down left hand side
        painter->drawLine((int)mItemRect.left(), 0, (int)mItemRect.left(), (int)br.height());
        // Line down right hand side
        painter->drawLine((int)mItemRect.right(), 0, (int)mItemRect.right(), (int)br.height());
        // Line across top
        painter->drawLine(0, (int)mItemRect.top(), (int)br.width(), (int)mItemRect.top());
        // Line across bottom
        painter->drawLine(0, (int)mItemRect.bottom(), (int)br.width(), (int)mItemRect.bottom());
    }

    // Draw the size prefs icons
    if (mShowSizePrefs) {
        painter->setPen(QPen(Qt::gray, SIZE_PREF_LINE_WIDTH)); //krazy:exclude=qenums
        if (mItemRect.height() > SIZE_PREF_MINIMUM_THRESHOLD) {
            drawPolicyIcons(painter, Qt::Horizontal, mItemPolicy, mItemRect);
        }
        if (mItemRect.width() > SIZE_PREF_MINIMUM_THRESHOLD) {
            drawPolicyIcons(painter, Qt::Vertical, mItemPolicy, mItemRect);
        }
    }

    painter->setLayoutDirection(prevDirection);
    painter->setPen(prevPen);
}



/******************************************************************************************/

CodeWidget::CodeWidget(const QString &title, QWidget *parent)
    : QWidget(parent), mLabel(0), mTextBox(0)
{
    mLabel = new QLabel(title, this);
    mTextBox = new QTextEdit(this);
    mTextBox->setReadOnly(true);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(mLabel);
    layout->addWidget(mTextBox);
    setLayout(layout);
}

CodeWidget::~CodeWidget()
{
}

void CodeWidget::setText(const QString &text)
{
    mTextBox->setText(text);
}

void CodeWidget::setHtml(const QString &html)
{
    mTextBox->setHtml(html);
}

void CodeWidget::setLayoutDirection(Qt::LayoutDirection dir)
{
    mLabel->setLayoutDirection(dir);
    mTextBox->setLayoutDirection(dir);
}



/******************************************************************************************/
CssInspectorModelItem::CssInspectorModelItem(QGraphicsItem *item, int row, CssInspectorModelItem *parent)
    : mItem(item), mParent(parent), mRow(row)
{
}

CssInspectorModelItem::~CssInspectorModelItem()
{
    QHash<int, CssInspectorModelItem*>::iterator it;
    for (it = mChildren.begin(); it!= mChildren.end(); ++it)
        delete it.value();
}

CssInspectorModelItem* CssInspectorModelItem::child(int i)
{
    if (mChildren.contains(i))
        return mChildren[i];

    if (i>=0 && i < mItem->childItems().count()) {
        QGraphicsItem *child = mItem->childItems().at(i);
        CssInspectorModelItem *childItem = new CssInspectorModelItem(child, i, this);
        mChildren[i] = childItem;
        return childItem;
    }
    return 0;
}

CssInspectorModelItem* CssInspectorModelItem::parent()
{
    return mParent;
}

QGraphicsItem* CssInspectorModelItem::data()
{
    return mItem;
}

int CssInspectorModelItem::row()
{
    return mRow;
}

/******************************************************************************************/
CssInspectorModel::CssInspectorModel(HbMainWindow *win, QObject *parent)
    : QAbstractItemModel(parent), mWin(win), mRootItem(0)
{
    QGraphicsItem *screen = HbMainWindowPrivate::d_ptr(win)->mClippingItem;
    mRootItem = new CssInspectorModelItem(screen, 0);
}

CssInspectorModel::~CssInspectorModel()
{
    delete mRootItem;
}

QVariant CssInspectorModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    CssInspectorModelItem *item = static_cast<CssInspectorModelItem*>(index.internalPointer());
    QGraphicsItem *graphicsItem = item->data();

    switch (index.column()) {
        case 0: return itemClass(graphicsItem);
        case 1: return objName(graphicsItem);
        case 2: return HbStyle::itemName(graphicsItem);
        case 3: return itemValue(graphicsItem);
        case 4: return itemInParentLayout(graphicsItem);
    }
    return QVariant();
}

QVariant CssInspectorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return "Classname";
            case 1: return "Objectname";
            case 2: return "Itemname";
            case 3: return "Value";
            case 4: return "In layout";
        }
    }
    return QVariant();
}

QModelIndex CssInspectorModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    CssInspectorModelItem *parentItem;
    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<CssInspectorModelItem*>(parent.internalPointer());

    CssInspectorModelItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex CssInspectorModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    CssInspectorModelItem* childItem = static_cast<CssInspectorModelItem*>(child.internalPointer());
    CssInspectorModelItem* parentItem = childItem->parent();

    if (!parentItem || parentItem == mRootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int CssInspectorModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    CssInspectorModelItem *parentItem;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<CssInspectorModelItem*>(parent.internalPointer());

    return parentItem->data()->childItems().count();
}

int CssInspectorModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 5;
}


ItemColorDelegate::ItemColorDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

ItemColorDelegate::~ItemColorDelegate()
{
}

const int ITEMNAME_COLUMN = 2;
const int INLAYOUT_COLUMN = 4;

void ItemColorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, 
                              const QModelIndex &index) const
{
    if (index.column() == ITEMNAME_COLUMN) {
        if (index.data().toString() == "") {
            CssInspectorModelItem *modelItem = static_cast<CssInspectorModelItem*>(index.internalPointer());
            QGraphicsItem *item = modelItem->data();
            if (parentHasAnchorLayout(item)) {
                painter->fillRect(option.rect, Qt::red);
            }
        }
    } else if (index.column() == INLAYOUT_COLUMN) {
        if (index.data().toString() == "No") {
            painter->fillRect(option.rect, Qt::red);
        }
    }
    QStyledItemDelegate::paint(painter, option, index);
}
/******************************************************************************************/

HbCssInspectorWindow *HbCssInspectorWindow::instance()
{
    static QPointer<HbCssInspectorWindow> window = 0;
    if (!window) {
        window = new HbCssInspectorWindow;
    }
    return window;
}


void HbCssInspectorWindow::refresh()
{
    if (this->isVisible()) {
        removeFilters();
        addFilters();
    }
}


void HbCssInspectorWindow::removeFilters()
{
    foreach (HoveredWidgetFilter *filter, mInstalledFilters)
        delete filter;
    mInstalledFilters.clear();
}

void HbCssInspectorWindow::updateFromTreeView(const QModelIndex &index)
{
    CssInspectorModelItem *modelItem = static_cast<CssInspectorModelItem*>(index.internalPointer());
    QGraphicsItem *item = modelItem->data();
    updateFocusItem(item);
    foreach (HoveredWidgetFilter *filter, mInstalledFilters) {
        filter->mCssInfoDrawer->updateFocusItem(item);
        filter->mArrowDrawer->updateFocusItem(item);
    }
}

void HbCssInspectorWindow::updateColumnSizes(const QModelIndex &index)
{
    Q_UNUSED(index)
    mTreeView->header()->resizeSections(QHeaderView::ResizeToContents);
}

void HbCssInspectorWindow::addFilters()
{
    foreach (HbMainWindow *window, hbInstance->allMainWindows()) {
        mTreeView->setModel(new CssInspectorModel(window));

        HoveredWidgetFilter *filter = new HoveredWidgetFilter(window->scene());
        window->scene()->installEventFilter(filter);
        mInstalledFilters.append(filter);
        connect(filter, SIGNAL(newItemHovered(const QGraphicsItem*)),
            SLOT(updateFocusItem(const QGraphicsItem*)));

        connect(mObjectNameCheck, SIGNAL(toggled(bool)), filter->mCssInfoDrawer,
            SLOT(setItemTextVisible(bool)));
        connect(mAnchorArrowsCheck, SIGNAL(toggled(bool)), filter->mArrowDrawer,
            SLOT(setDrawArrows(bool)));
        connect(mSubitemOutlinesCheck, SIGNAL(toggled(bool)), filter->mArrowDrawer,
            SLOT(setDrawOutlines(bool)));
        connect(mSpacersCheck, SIGNAL(toggled(bool)), filter->mArrowDrawer,
            SLOT(setDrawSpacers(bool)));
        connect(mGuideLinesCheck, SIGNAL(toggled(bool)), filter->mCssInfoDrawer,
            SLOT(setGuideLinesVisible(bool)));

        connect(mSizeHintTextCheck, SIGNAL(toggled(bool)), filter->mCssInfoDrawer,
            SLOT(setHintTextVisible(bool)));
        connect(mMinSizeHintCheck, SIGNAL(toggled(bool)), filter->mCssInfoDrawer,
            SLOT(setMinHintBoxVisible(bool)));
        connect(mPrefSizeHintCheck, SIGNAL(toggled(bool)), filter->mCssInfoDrawer,
            SLOT(setPrefHintBoxVisible(bool)));
        connect(mMaxSizeHintCheck, SIGNAL(toggled(bool)), filter->mCssInfoDrawer,
            SLOT(setMaxHintBoxVisible(bool)));
        connect(mSizePrefCheck, SIGNAL(toggled(bool)), filter->mCssInfoDrawer,
            SLOT(setSizePrefsVisible(bool)));

        connect(mHoverRadio, SIGNAL(toggled(bool)), filter, SLOT(setHoverMode(bool)));
        connect(mBlockRadio, SIGNAL(toggled(bool)), filter, SLOT(setBlockingMode(bool)));

        filter->mCssInfoDrawer->setItemTextVisible(mObjectNameCheck->isChecked());
        filter->mArrowDrawer->setDrawArrows(mAnchorArrowsCheck->isChecked());
        filter->mArrowDrawer->setDrawOutlines(mSubitemOutlinesCheck->isChecked());
        filter->mArrowDrawer->setDrawSpacers(mSpacersCheck->isChecked());
        filter->mCssInfoDrawer->setGuideLinesVisible(mGuideLinesCheck->isChecked());
        filter->mCssInfoDrawer->setHintTextVisible(mSizeHintTextCheck->isChecked());
        filter->mCssInfoDrawer->setMinHintBoxVisible(mMinSizeHintCheck->isChecked());
        filter->mCssInfoDrawer->setPrefHintBoxVisible(mPrefSizeHintCheck->isChecked());
        filter->mCssInfoDrawer->setMaxHintBoxVisible(mMaxSizeHintCheck->isChecked());
        filter->mCssInfoDrawer->setSizePrefsVisible(mSizePrefCheck->isChecked());
        filter->setHoverMode(mHoverRadio->isChecked());
        filter->setBlockingMode(mBlockRadio->isChecked());
    }
}


HbCssInspectorWindow::HbCssInspectorWindow(QWidget *parent)
    : QWidget(parent)
{
    QGroupBox *generalGroup = new QGroupBox(tr("General"), this);
    QVBoxLayout *genLayout = new QVBoxLayout(this);
    generalGroup->setLayout(genLayout);
    mObjectNameCheck = new QCheckBox(tr("Show object name"), this);
    mAnchorArrowsCheck = new QCheckBox(tr("Draw arrows"), this);
    mSubitemOutlinesCheck = new QCheckBox(tr("Draw subitem outlines"), this);
    mSpacersCheck = new QCheckBox(tr("Draw spacers"), this);
    mGuideLinesCheck = new QCheckBox(tr("Draw guide lines"), this);
    genLayout->addWidget(mObjectNameCheck);
    genLayout->addWidget(mAnchorArrowsCheck);
    genLayout->addWidget(mSubitemOutlinesCheck);
    genLayout->addWidget(mSpacersCheck);
    genLayout->addWidget(mGuideLinesCheck);

    QGroupBox *sizeHintOptsGroup = new QGroupBox(tr("Size Hints"), this);
    QVBoxLayout *shLayout = new QVBoxLayout(this);
    sizeHintOptsGroup->setLayout(shLayout);
    mSizeHintTextCheck = new QCheckBox(tr("Show size hint"), this);
    mMinSizeHintCheck = new QCheckBox(tr("Min size hint outline"), this);
    mPrefSizeHintCheck = new QCheckBox(tr("Pref size hint outline"), this);
    mMaxSizeHintCheck = new QCheckBox(tr("Max size hint outline"), this);
    mSizePrefCheck = new QCheckBox(tr("Size preferences"), this);
    shLayout->addWidget(mSizeHintTextCheck);
    shLayout->addWidget(mMinSizeHintCheck);
    shLayout->addWidget(mPrefSizeHintCheck);
    shLayout->addWidget(mMaxSizeHintCheck);
    shLayout->addWidget(mSizePrefCheck);

    QGroupBox *eventModeGroup = new QGroupBox(tr("Event mode"), this);
    QVBoxLayout *eventLayout = new QVBoxLayout(this);
    eventModeGroup->setLayout(eventLayout);
    mHoverRadio = new QRadioButton(tr("Hover mode"), this);
    mClickRadio = new QRadioButton(tr("Click locking mode"), this);
    mBlockRadio = new QRadioButton(tr("Blocked locking mode"), this);
    eventLayout->addWidget(mHoverRadio);
    eventLayout->addWidget(mClickRadio);
    eventLayout->addWidget(mBlockRadio);

    QHBoxLayout *settingLayout = new QHBoxLayout;
    settingLayout->addWidget(generalGroup);
    settingLayout->addWidget(sizeHintOptsGroup);
    settingLayout->addWidget(eventModeGroup);

    QGroupBox *sizeHintGroup = new QGroupBox(tr("Size hint"), this);
    QHBoxLayout *sizeHLayout = new QHBoxLayout;
    sizeHintGroup->setLayout(sizeHLayout);
    mSizeHintLabel = new QLabel("", this);
    sizeHLayout->addWidget(mSizeHintLabel);

    QGroupBox *sizePolicyGroup = new QGroupBox(tr("Size Policies"), this);
    QHBoxLayout *sizePolicyLayout = new QHBoxLayout;
    sizePolicyGroup->setLayout(sizePolicyLayout);
    mSizePolicyHoriz = new QLabel(this);
    mSizePolicyVert = new QLabel(this);
    sizePolicyLayout->addWidget(mSizePolicyHoriz);
    sizePolicyLayout->addWidget(mSizePolicyVert);

    QHBoxLayout *sizeLayout = new QHBoxLayout;
    sizeLayout->setContentsMargins(0,0,0,0);
    sizeLayout->addWidget(sizeHintGroup);
    sizeLayout->addWidget(sizePolicyGroup);

    QWidget *treeContainer  = new QWidget(this);
    QVBoxLayout *treeLayout = new QVBoxLayout;
    treeLayout->setContentsMargins(0,0,0,0);
    treeContainer->setLayout(treeLayout);
    mTreeView = new QTreeView(this);
    mTreeView->setItemDelegate(new ItemColorDelegate);
    QLabel *treeLabel = new QLabel(tr("Item tree"));
    treeLayout->addWidget(treeLabel);
    treeLayout->addWidget(mTreeView);
    
    connect(mTreeView, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(updateColumnSizes(const QModelIndex&)));
    connect(mTreeView, SIGNAL(expanded(const QModelIndex&)), this, SLOT(updateColumnSizes(const QModelIndex&)));
    connect(mTreeView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(updateFromTreeView(const QModelIndex&)));

    QSplitter *widgetSplitter = new QSplitter(this);
    mWidgetMLBox = new CodeWidget(tr("WidgetML"), this);
    widgetSplitter->addWidget(treeContainer);
    widgetSplitter->addWidget(mWidgetMLBox);

    QSplitter *cssSplitter = new QSplitter(this);
    mLayoutCssBox = new CodeWidget(tr("Layouts CSS stack (+all)"), this);
    mColorsCssBox = new CodeWidget(tr("Colors CSS stack (+all)"), this);
    cssSplitter->addWidget(mLayoutCssBox);
    cssSplitter->addWidget(mColorsCssBox);

    QSplitter *vertSplit = new QSplitter(Qt::Vertical, this);
    vertSplit->addWidget(widgetSplitter);
    vertSplit->addWidget(cssSplitter);

    mPathLabel = new QLabel(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(settingLayout);
    mainLayout->addLayout(sizeLayout);
    mainLayout->addWidget(vertSplit);
    mainLayout->addWidget(mPathLabel);

    // Lock in left-to-right mode
    mSizeHintLabel->setLayoutDirection(Qt::LeftToRight);
    mLayoutCssBox->setLayoutDirection(Qt::LeftToRight);
    mColorsCssBox->setLayoutDirection(Qt::LeftToRight);
    mWidgetMLBox->setLayoutDirection(Qt::LeftToRight);

    // Set default options
    mObjectNameCheck->setChecked(true);
    mAnchorArrowsCheck->setChecked(true);
    mSubitemOutlinesCheck->setChecked(true);
    mSpacersCheck->setChecked(true);
    mHoverRadio->setChecked(true);

    setLayout(mainLayout);
}


HbCssInspectorWindow::~HbCssInspectorWindow()
{
}


void HbCssInspectorWindow::setVisible(bool visible)
{
    if (visible) {
        addFilters();
    } else {
        removeFilters();
    }
    QWidget::setVisible(visible);
}


void HbCssInspectorWindow::updateFocusItem(const QGraphicsItem *item)
{
    if (item != 0 && item->isWidget()) {
        const QGraphicsWidget *widget = static_cast<const QGraphicsWidget *>(item);
        NODEPTR_N(widget);
        HbDeviceProfile profile(HbDeviceProfile::profile(widget));

        HbLayeredStyleLoader *layoutStack =
            HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Layouts);
        if (layoutStack) {
            // Update layout CSS box
            HbVector<HbCss::StyleRule> layoutRules =
                layoutStack->styleRulesForNode(n, profile.orientation());
            mLayoutCssBox->setHtml(CSS_HTML_HEADER + HbCssFormatter::styleRulesToHtml(layoutRules));

            // Get layoutname from CSS
            HbVector<HbCss::Declaration> decls;
            foreach (const HbCss::StyleRule &rule, layoutRules)
                decls += rule.declarations;
            HbCss::ValueExtractor extractor(decls, profile);
            QString layoutName;
            QString sectionName;
            extractor.extractLayout(layoutName, sectionName);

            // Update widgetML box
            QString html;
            if (widget->layout()) {
                HbAnchorLayout *anchorLayout = dynamic_cast<HbAnchorLayout *>(widget->layout());
                html = anchorsToHtmlInfo(anchorLayout, itemClass(widget), layoutName);
            }
            mWidgetMLBox->setHtml(html);
        }

        // Update colours CSS box
        HbLayeredStyleLoader *colorsStack =
            HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Colors);
        if (colorsStack) {
            HbVector<HbCss::StyleRule> colorRules =
                colorsStack->styleRulesForNode(n, profile.orientation());
            mColorsCssBox->setHtml(CSS_HTML_HEADER + HbCssFormatter::styleRulesToHtml(colorRules));
        }

        // Update text labels
        mSizeHintLabel->setText(cssItemHintText(item));
        mSizePolicyHoriz->setText("Horizontal: " + cssSizePolicyText(item, Qt::Horizontal));
        mSizePolicyVert->setText("Vertical: " + cssSizePolicyText(item, Qt::Vertical));
        const QGraphicsItem *pathItem = item;
        QString cssPath = cssItemText(pathItem);
        while (pathItem->parentItem()) {
            pathItem = pathItem->parentItem();
            cssPath.prepend(" > ");
            cssPath.prepend(cssItemText(pathItem));
        }
        mPathLabel->setText(cssPath);

        // Set focus in tree view
        QList<const QGraphicsItem*> ancestorItems;
        const QGraphicsItem *ancestor = item;
        while (ancestor) {
            ancestorItems << ancestor;
            ancestor = ancestor->parentItem();
        }
        bool foundRoot = false;
        QModelIndex index;
        for (int i=ancestorItems.count() -1; i>=0; i--) {
            const QGraphicsItem *thisItem = ancestorItems.at(i);
            if (!foundRoot) {
                const QGraphicsWidget *wid = static_cast<const QGraphicsWidget*>(thisItem);
                if (wid) {
                    QString name = wid->metaObject()->className();
                    if (name == "HbScreen") {
                        foundRoot = true;
                    }
                }
            } else {
                // Find and open tree node for item
                CssInspectorModel *model = static_cast<CssInspectorModel*>(mTreeView->model());
                if (model) {
                    int rowCount = model->rowCount(index);
                    for (int row=0; row<rowCount; row++) {
                        QModelIndex idx = model->index(row, 0, index);
                        CssInspectorModelItem *modelItem = static_cast<CssInspectorModelItem*>(idx.internalPointer());
                        QGraphicsItem *graphicsItem = modelItem->data();
                        if (graphicsItem == thisItem) {
                            index = idx;
                            break;
                        }
                    }
                }
            }
        }
        mTreeView->collapseAll();
        mTreeView->scrollTo(index, QAbstractItemView::EnsureVisible);
        mTreeView->expand(index);

    } else {
        mWidgetMLBox->setText("");
        mLayoutCssBox->setText("");
        mColorsCssBox->setText("");
        mPathLabel->setText("");
        mSizeHintLabel->setText("");
        mSizePolicyHoriz->setText("");
        mSizePolicyVert->setText("");
    }
}



/******************************************************************************************/

bool HoveredWidgetFilter::eventFilter(QObject *obj, QEvent *event)
{
    if ((event->type() == QEvent::GraphicsSceneMouseMove && mHoverMode)
            || (event->type() == QEvent::GraphicsSceneMousePress && !mHoverMode)){
        QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
        QPointF eventPos = mouseEvent->scenePos();

        // Skip the drawn box/texts, arrow drawers, and popup managers
        QList<QGraphicsItem *> items = mScene->items(eventPos);
        QGraphicsItem *hoveredItem = 0;
        foreach (QGraphicsItem *thisItem, items) {
            if (!SKIPPED_GLOBAL_ITEMS.contains(itemClass(thisItem))) {
                hoveredItem = thisItem;
                break;
            }
        }

        if (hoveredItem) {
            // Ignore primitives
            while (hoveredItem && SKIPPED_CHILD_ITEMS.contains(itemClass(hoveredItem)))
                hoveredItem = hoveredItem->parentItem();

            if (hoveredItem && hoveredItem != mCurrentItem) {
                mCurrentItem = hoveredItem;
                emit newItemHovered(hoveredItem);
            }
        }

        if (mBlockingMode) {
            return true;
        }
    } else if(event->type() == QEvent::Leave && mHoverMode) {
        emit newItemHovered(0);
        mCurrentItem = 0;
#ifdef HB_GESTURE_FW
    } else if(mBlockingMode && event->type() == QEvent::Gesture) {
        QGestureEvent *gesEvent = static_cast<QGestureEvent*>(event);
        if (gesEvent->gesture(Qt::TapGesture)) {
            return true;
        }
#endif
    }

    return QObject::eventFilter(obj, event);
}

HoveredWidgetFilter::HoveredWidgetFilter(QGraphicsScene *scene)
    : mScene(scene), mCurrentItem(0), mArrowDrawer(0),
    mCssInfoDrawer(0), mHoverMode(true), mBlockingMode(false)
{
    mCssInfoDrawer = new HbCssInfoDrawer(0);
    mScene->addItem(mCssInfoDrawer);
    mArrowDrawer = new HbAnchorArrowDrawer(0);
    mScene->addItem(mArrowDrawer);

    // Ensure CSS inspector items are above popups in z-order
    mCssInfoDrawer->setZValue(HbPrivate::PopupZValueRangeEnd + ABOVE_POPUP_ZVALUE);
    mArrowDrawer->setZValue(HbPrivate::PopupZValueRangeEnd + ABOVE_POPUP_ZVALUE);

    connect(this, SIGNAL(newItemHovered(const QGraphicsItem*)),
        mCssInfoDrawer, SLOT(updateFocusItem(const QGraphicsItem*)));
    connect(this, SIGNAL(newItemHovered(const QGraphicsItem*)),
        mArrowDrawer, SLOT(updateFocusItem(const QGraphicsItem*)));
}

HoveredWidgetFilter::~HoveredWidgetFilter()
{
    mScene->removeItem(mCssInfoDrawer);
    mScene->removeItem(mArrowDrawer);
    delete mCssInfoDrawer;
    delete mArrowDrawer;
}

#endif // HB_CSS_INSPECTOR
