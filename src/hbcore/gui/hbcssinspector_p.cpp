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
#include <hbanchor_p.h>
#include <hbanchorarrowdrawer_p.h>
#include <hbcolorscheme.h>
#include <hbcssformatter_p.h>
#include <hbevent.h>
#include <hbinstance.h>
#include <hblayeredstyleloader_p.h>
#include <hbmeshlayoutdebug_p.h>
#include <hbnamespace_p.h>
#include <hbwidgetloadersyntax_p.h>
#include <hbxmlloaderabstractsyntax_p.h>
#include <hbwidgetbase_p.h>
#include <hbwidget_p.h>

#include <QBrush>
#include <QCheckBox>
#include <QGraphicsLayout>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPen>
#include <QPainter>
#include <QPointer>
#include <QRadioButton>
#include <QTextEdit>
#include <QXmlStreamWriter>


#define NODEPTR_N(x) HbCss::StyleSelector::NodePtr n = {n.ptr = (void *)x};

const QString CSS_HTML_HEADER = "<style type=\"text/css\"> \
                                .overridden {color:#999; text-decoration:line-through;} \
                                .filename {background-color: #e0e0e0; margin:0;} \
                                .selectors {margin:0;} \
                                .selector {color:#000;} \
                                .pseudo {font-weight:bold;} \
                                .attr {font-style:italic;} \
                                .property {color:#00f; font-weight:bold;} \
                                .value {color:#f00;} \
                                .overridden .property, .overridden .value {color:#999;} \
                                </style>";
const QString WIDGETML_HTML_HEADER = "<style type=\"text/css\"> \
                                     pre {color:#999; font-family:Arial;} \
                                     span {color:#000; font-weight:bold;} \
                                     </style><pre>";
const QString WIDGETML_HTML_FOOTER = "</pre>";

const int ITEMNAME = 0xfffe; // Copied from hbstyle.cpp!!
const qreal HOVER_BOX_PEN_WIDTH = 2.0;
const qreal GUIDE_LINE_WIDTH = 1.0;
const QChar BIG_NUMBER_CHAR = 0x221E;
const QString TEXT_COLOR = "qtc_default_main_pane_normal";
const QString LINE_COLOR = "qtc_view_visited_normal";
const int ABOVE_POPUP_ZVALUE = 5000;


static QString cssItemText(const QGraphicsItem *item)
{
    QString txt;
    if (item->isWidget()) {
        const QGraphicsWidget *widget = static_cast<const QGraphicsWidget *>(item);
        // Add classname
        txt += widget->metaObject()->className();

        // Add objectname
        QString objName = widget->objectName();
        if (objName.length() > 0) {
            txt.append("#");
            txt.append(objName);
        }

        // Add itemname
        QString itemName = widget->data(ITEMNAME).toString();
        if (itemName.length() > 0) {
            txt.append("::");
            txt.append(itemName);
        }
    }
    return txt;
}


static QString convertMeasurementToText(const QGraphicsItem *item, qreal hint)
{
    QString hintText;
    qreal unit = HbDeviceProfile::profile(item).unitValue();
    
    if (unit != 0) {
        hint = (hint / unit);
        }
    hintText = QString::number(hint, 'g', 2);
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
    QString hintText("(");
    const QSizeF &size = item->effectiveSizeHint(which);
    hintText += convertMeasurementToText(item, size.width()) + ",";
    hintText += convertMeasurementToText(item, size.height()) + ")";
    return hintText;
}

static QString cssItemHintText(const QGraphicsItem *item)
{
    QString sizeHint;
    if (item->isWidget()) {
        const QGraphicsWidget *widget = static_cast<const QGraphicsWidget*>(item);
        sizeHint += convertEffectiveSizeHintToText(widget, Qt::MinimumSize)+"|";
        sizeHint += convertEffectiveSizeHintToText(widget, Qt::PreferredSize)+"|";
        sizeHint += convertEffectiveSizeHintToText(widget, Qt::MaximumSize);
    }
    return sizeHint;
}

static QRectF cssItemHintRect(const QGraphicsItem *item)
{
    QRectF hintRect;
    if (item->isWidget()) {
        if (item->isWidget()) {
            const QGraphicsWidget *widget = static_cast<const QGraphicsWidget*>(item);
            const QSizeF &size = widget->effectiveSizeHint(Qt::PreferredSize);
            hintRect.setWidth(size.width());
            hintRect.setHeight(size.height());
        }
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


QString HbCssInspectorWindow::meshItemsToHtmlInfo(HbMeshLayout *mesh, const QString itemName, const QString layoutName)
{
    QString html;
    QString widgetML;
    QXmlStreamWriter xmlWriter(&widgetML);
    xmlWriter.setAutoFormatting(true);

    HbWidgetLoaderSyntax syntax(new HbWidgetLoaderActions());

    QString str = syntax.lexemValue(HbXmlLoaderAbstractSyntax::TYPE_HBWIDGET);
    xmlWriter.writeStartElement(syntax.lexemValue(HbXmlLoaderAbstractSyntax::TYPE_HBWIDGET));
    xmlWriter.writeAttribute(syntax.lexemValue(HbXmlLoaderAbstractSyntax::ATTR_VERSION), HbWidgetLoaderSyntax::version());
    xmlWriter.writeAttribute(syntax.lexemValue(HbXmlLoaderAbstractSyntax::ATTR_TYPE), itemName);

    xmlWriter.writeStartElement(syntax.lexemValue(HbXmlLoaderAbstractSyntax::TYPE_LAYOUT));
    xmlWriter.writeAttribute(syntax.lexemValue(HbXmlLoaderAbstractSyntax::ATTR_NAME), layoutName);
    xmlWriter.writeAttribute(syntax.lexemValue(HbXmlLoaderAbstractSyntax::ATTR_TYPE), syntax.lexemValue(HbXmlLoaderAbstractSyntax::LAYOUT_MESH));

    if (mesh) {
        QList<HbAnchor*> anchors = HbMeshLayoutDebug::getAnchors(mesh);
        for (int i=0; i<anchors.count(); i++) {
            HbAnchor* anchor = anchors.at(i);

            QString startName = HbStyle::itemName(anchor->mStartItem->graphicsItem());
            QString endName = HbStyle::itemName(anchor->mEndItem->graphicsItem());
            QString spacingText;

            QGraphicsItem *asGraphicsItem = mesh->parentLayoutItem()->graphicsItem();
            if ( asGraphicsItem && asGraphicsItem->isWidget() ){
                HbWidget *asWidget = qobject_cast<HbWidget*>( static_cast<QGraphicsWidget*>(asGraphicsItem) );
                if( asWidget ) {
                    HbWidgetPrivate*priv = static_cast<HbWidgetPrivate*>(HbWidgetBasePrivate::d_ptr(asWidget));

                    if (startName.isEmpty()) {
                        startName = priv->mSpacers.key(anchor->mStartItem);
                    } 
                    if (endName.isEmpty()) {
                        endName = priv->mSpacers.key(anchor->mEndItem);
                    }

                    if(qAbs<qreal>(anchor->mValue) > 0.01)
                        spacingText = convertMeasurementToText(asWidget, anchor->mValue);
                }
            }

            xmlWriter.writeStartElement(syntax.lexemValue(HbXmlLoaderAbstractSyntax::ML_MESHITEM));

            xmlWriter.writeAttribute(
                syntax.lexemValue(HbXmlLoaderAbstractSyntax::ML_SRC_NAME), startName);
            xmlWriter.writeAttribute(
                syntax.lexemValue(HbXmlLoaderAbstractSyntax::ML_SRC_EDGE), anchorEdgeName(anchor->mStartEdge));
            xmlWriter.writeAttribute(
                syntax.lexemValue(HbXmlLoaderAbstractSyntax::ML_DST_NAME), endName);
            xmlWriter.writeAttribute(
                syntax.lexemValue(HbXmlLoaderAbstractSyntax::ML_DST_EDGE), anchorEdgeName(anchor->mEndEdge));
            if ( !spacingText.isEmpty() ) {
                xmlWriter.writeAttribute(
                syntax.lexemValue(HbXmlLoaderAbstractSyntax::ML_SPACING), spacingText);
            }
            xmlWriter.writeEndElement(); // meshitem
        }

    }
    xmlWriter.writeEndElement(); // layout
    xmlWriter.writeEndElement(); // widgetml

    html = widgetML;
    html.remove(0, html.indexOf("<")); // trim whitespace
    html.replace("<", "&lt;");
    html.replace(">", "&gt;");
    html.replace(QRegExp("\"([^\"]*)\""), "\"<span>\\1</span>\""); // Add span elements around things in quotes
    html.replace("\"", "&quot;");
    html.replace("\n\n", "\n");
    html.replace("\n", "<br/>");

    html.insert(0, WIDGETML_HTML_HEADER);
    html.append(WIDGETML_HTML_FOOTER);

    return html;
}



/******************************************************************************************/



HbCssInfoDrawer::HbCssInfoDrawer(QGraphicsItem *parent)
    : HbWidgetBase(parent), 
    mShowItemText(true),
    mShowHintText(true), 
    mShowBox(true),
    mShowHintBox(true),
    mDrawGuideLines(true), 
    mItemRect(0,0,0,0)
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
        mHintRect = cssItemHintRect(item);
        // Make sure this is in the same place in the scene as the window
        if (item->isWidget()) {
            const HbWidget *obj = static_cast<const HbWidget*>(item);
            this->setGeometry(obj->mainWindow()->rect());
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
    if (mShowHintBox) {
        painter->setPen(QPen(Qt::green, HOVER_BOX_PEN_WIDTH));
        QRectF prefRect = mHintRect;
        prefRect.moveCenter(mItemRect.center());
        paintRect(painter, prefRect);
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

    painter->setLayoutDirection(prevDirection);
    painter->setPen(prevPen);
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


void HbCssInspectorWindow::addFilters()
{
    foreach (HbMainWindow *window, hbInstance->allMainWindows()) {
        HoveredWidgetFilter *filter = new HoveredWidgetFilter(window->scene());
        window->scene()->installEventFilter(filter);
        mInstalledFilters.append(filter);
        connect(filter, SIGNAL(newItemHovered(const QGraphicsItem*)), SLOT(updateFocusItem(const QGraphicsItem*)));
        connect(mNameCheck, SIGNAL(toggled(bool)), filter->mCssInfoDrawer, SLOT(setItemTextVisible(bool)));
        connect(mSizeHintCheck, SIGNAL(toggled(bool)), filter->mCssInfoDrawer, SLOT(setHintTextVisible(bool)));
		connect(mHintOutlinesCheck, SIGNAL(toggled(bool)), filter->mCssInfoDrawer, SLOT(setHintBoxVisible(bool)));
        connect(mGuideLinesCheck, SIGNAL(toggled(bool)), filter->mCssInfoDrawer, SLOT(setGuideLinesVisible(bool)));
        connect(mArrowsCheck, SIGNAL(toggled(bool)), filter->mArrowDrawer, SLOT(setDrawArrows(bool)));
        connect(mOutlinesCheck, SIGNAL(toggled(bool)), filter->mArrowDrawer, SLOT(setDrawOutlines(bool)));
        connect(mSpacersCheck, SIGNAL(toggled(bool)), filter->mArrowDrawer, SLOT(setDrawSpacers(bool)));
        connect(mLiveRadio, SIGNAL(toggled(bool)), filter, SLOT(setLiveMode(bool)));
        connect(mBlockRadio, SIGNAL(toggled(bool)), filter, SLOT(setBlockingMode(bool)));
    }    
}


HbCssInspectorWindow::HbCssInspectorWindow(QWidget *parent)
    : QWidget(parent), mLayoutWidgetMLBox(0), mLayoutCssBox(0), mColorsCssBox(0)
{
    QGroupBox *settings = new QGroupBox(tr("Settings"), this);
    QGridLayout *settingLayout = new QGridLayout(settings);
    mArrowsCheck = new QCheckBox(tr("Draw arrows"), this);
    mOutlinesCheck = new QCheckBox(tr("Draw subitem outlines"), this);
	mHintOutlinesCheck = new QCheckBox(tr("Draw sizehint outlines"), this);
    mSpacersCheck = new QCheckBox(tr("Draw spacers"), this);
    mNameCheck = new QCheckBox(tr("Show object name"), this);
    mSizeHintCheck = new QCheckBox(tr("Show size hint"), this);
    mGuideLinesCheck = new QCheckBox(tr("Draw guide lines"), this);
    mLiveRadio = new QRadioButton(tr("Live mode"), this);
    mClickRadio = new QRadioButton(tr("Click locking mode"), this);
    mBlockRadio = new QRadioButton(tr("Click locking mode (block events)"), this);
    settingLayout->addWidget(mArrowsCheck, 0, 0);
    settingLayout->addWidget(mOutlinesCheck, 1, 0);
	settingLayout->addWidget(mHintOutlinesCheck, 2, 0);
    settingLayout->addWidget(mSpacersCheck, 3, 0);
    settingLayout->addWidget(mNameCheck, 0, 1);
    settingLayout->addWidget(mSizeHintCheck, 1, 1);
    settingLayout->addWidget(mGuideLinesCheck, 2, 1);
    settingLayout->addWidget(mLiveRadio, 0, 2);
    settingLayout->addWidget(mClickRadio, 1, 2);
    settingLayout->addWidget(mBlockRadio, 2, 2);
    mArrowsCheck->setChecked(true);
    mOutlinesCheck->setChecked(true);
	mHintOutlinesCheck->setChecked(true);
    mSpacersCheck->setChecked(true);
    mNameCheck->setChecked(true);
    mSizeHintCheck->setChecked(true);
    mGuideLinesCheck->setChecked(true);
    mLiveRadio->setChecked(true);

    QLabel *lblWidgetML = new QLabel(tr("WidgetML"), this);
    mLayoutWidgetMLBox = new QTextEdit(this);
    mLayoutWidgetMLBox->setReadOnly(true);

    QLabel *lblLayout = new QLabel(tr("Layouts CSS stack (+all)"), this);
    mLayoutCssBox = new QTextEdit(this);
    mLayoutCssBox->setReadOnly(true);

    QLabel *lblColors = new QLabel(tr("Colors CSS stack (+all)"), this);
    mColorsCssBox = new QTextEdit(this);
    mColorsCssBox->setReadOnly(true);

    mPathLabel = new QLabel("", this);
    mSizeHintLabel = new QLabel("", this);
    mSizeHintLabel->setAlignment(Qt::AlignRight);

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(settings, 0, 0, 1, 4);
    layout->addWidget(lblWidgetML, 1, 0);
    layout->addWidget(mLayoutWidgetMLBox, 2, 0, 1, 4);
    layout->addWidget(lblLayout, 3, 0, 1, 2);
    layout->addWidget(lblColors, 3, 2, 1, 2);
    layout->addWidget(mLayoutCssBox, 4, 0, 1, 2);
    layout->addWidget(mColorsCssBox, 4, 2, 1, 2);
    layout->addWidget(mPathLabel, 5, 0, 1, 3);
    layout->addWidget(mSizeHintLabel, 5, 3, 1, 1);
    layout->setRowStretch(2, 2);
    layout->setRowStretch(4, 3);

    // Lock in left-to-right mode
    mSizeHintLabel->setLayoutDirection(Qt::LeftToRight);
    lblColors->setLayoutDirection(Qt::LeftToRight);
    lblLayout->setLayoutDirection(Qt::LeftToRight);
    mLayoutCssBox->setLayoutDirection(Qt::LeftToRight);
    mColorsCssBox->setLayoutDirection(Qt::LeftToRight);
    mLayoutWidgetMLBox->setLayoutDirection(Qt::LeftToRight);

    setLayout(layout);
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

        HbLayeredStyleLoader *layoutStack = HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Layouts);
        if (layoutStack) {
            // Update layout CSS box
            HbVector<HbCss::StyleRule> layoutRules = layoutStack->styleRulesForNode(n, profile.orientation());
            mLayoutCssBox->setHtml(CSS_HTML_HEADER + HbCssFormatter::styleRulesToHtml(layoutRules));

            // Get layoutname from CSS
            HbVector<HbCss::Declaration> decls;
            foreach (const HbCss::StyleRule &rule, layoutRules)
                decls += rule.declarations;
            HbCss::ValueExtractor extractor(decls, profile);
            QString layoutName;
            QString sectionName;
            extractor.extractLayout(&layoutName, &sectionName);

            // Update widgetML box
            QString html;
            if (widget->layout()) {
                QString itemName = widget->metaObject()->className();
                HbMeshLayout *mesh = dynamic_cast<HbMeshLayout *>(widget->layout());
                html = meshItemsToHtmlInfo(mesh, itemName, layoutName);
            }
            mLayoutWidgetMLBox->setHtml(html);
        }

        // Update colours CSS box
        HbLayeredStyleLoader *colorsStack = HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Colors);
        if (colorsStack) {
            HbVector<HbCss::StyleRule> colorRules = colorsStack->styleRulesForNode(n, profile.orientation());
            mColorsCssBox->setHtml(CSS_HTML_HEADER + HbCssFormatter::styleRulesToHtml(colorRules));
        }

        // Update text labels
        mSizeHintLabel->setText(cssItemHintText(item));
        const QGraphicsItem *pathItem = item;
        QString cssPath = cssItemText(pathItem);
        while (pathItem->parentItem()) {
            pathItem = pathItem->parentItem();
            cssPath.prepend(" > ");
            cssPath.prepend(cssItemText(pathItem));
        }
        mPathLabel->setText(cssPath);
    } else {
        mLayoutWidgetMLBox->setText("");
        mLayoutCssBox->setText("");
        mColorsCssBox->setText("");
        mPathLabel->setText("");
        mSizeHintLabel->setText("");
    }
}



/******************************************************************************************/



bool HoveredWidgetFilter::eventFilter(QObject *obj, QEvent *event)
{
    if ((event->type() == QEvent::GraphicsSceneMouseMove && mLiveMode)
            || (event->type() == QEvent::GraphicsSceneMousePress && !mLiveMode)){
        QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
        QPointF eventPos = mouseEvent->scenePos();

        // Skip the drawn box/texts, arrow drawers, and popup managers
        QList<QGraphicsItem *> items = mScene->items(eventPos);
        QGraphicsItem *item = 0;
        foreach (QGraphicsItem *thisItem, items) {
            QString widgetName;
            if (thisItem->isWidget()) {
                const QGraphicsWidget *widget = static_cast<const QGraphicsWidget *>(thisItem);
                widgetName = widget->metaObject()->className();
                if (thisItem != mCssInfoDrawer 
                        && thisItem != mArrowDrawer
                        && widgetName != "HbPopupLayoutManager"
                        && widgetName != "HbAnchorArrowDrawer") {
                    item = thisItem;
                    break;
                }
            }
        }

        if (item) {
            const QGraphicsWidget *widget = static_cast<const QGraphicsWidget *>(item);
            QString itemText(widget->metaObject()->className());

            // Ignore primitives
            while (itemText == "QGraphicsWidget" 
                    || itemText == "HbWidgetBase" 
                    || itemText == "HbTextItem" 
                    || itemText == "HbIconItem" 
                    || itemText == "HbFrameItem"
                    || itemText == "HbMarqueeItem"
                    || itemText == "HbSpacerItem") {
                item = item->parentItem();
                widget = static_cast<const QGraphicsWidget *>(item);
                itemText = widget ? widget->metaObject()->className() : "";
            }

            if (item && item != mCurrentItem) {
                mCurrentItem = item;
                emit newItemHovered(item);
            }
        }

        if (mBlockingMode) {
            return true;
        }
    } else if(event->type() == QEvent::Leave && mLiveMode) {
        emit newItemHovered(0);
        mCurrentItem = 0;
#ifdef HB_GESTURE_FW
    } else if(mBlockingMode && event->type() == QEvent::Gesture) {
        QGestureEvent *gesEvent = static_cast<QGestureEvent*>(event);
        QGesture *tap = gesEvent->gesture(Qt::TapGesture);
        if (tap) {
            return true;
        }
#endif
    }

    return QObject::eventFilter(obj, event);
}

HoveredWidgetFilter::HoveredWidgetFilter(QGraphicsScene *scene)
    : mScene(scene), mCurrentItem(0), mArrowDrawer(0), mCssInfoDrawer(0), mLiveMode(true), mBlockingMode(false)
{
    mCssInfoDrawer = new HbCssInfoDrawer(0);
    mScene->addItem(mCssInfoDrawer);
    mArrowDrawer = new HbAnchorArrowDrawer(0);
    mScene->addItem(mArrowDrawer);

    // Ensure CSS inspector items are above popups in z-order
    mCssInfoDrawer->setZValue(HbPrivate::PopupZValueRangeEnd + ABOVE_POPUP_ZVALUE);
    mArrowDrawer->setZValue(HbPrivate::PopupZValueRangeEnd + ABOVE_POPUP_ZVALUE);

    connect(this, SIGNAL(newItemHovered(const QGraphicsItem*)), mCssInfoDrawer, SLOT(updateFocusItem(const QGraphicsItem*)));
    connect(this, SIGNAL(newItemHovered(const QGraphicsItem*)), mArrowDrawer, SLOT(updateFocusItem(const QGraphicsItem*)));
}

HoveredWidgetFilter::~HoveredWidgetFilter()
{
    mScene->removeItem(mCssInfoDrawer);
    mScene->removeItem(mArrowDrawer);
    delete mCssInfoDrawer;
    delete mArrowDrawer;
}

#endif
