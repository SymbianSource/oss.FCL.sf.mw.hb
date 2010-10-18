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

#include "hbstyle_p.h"
#include "hbstyleoptionslider_p.h"
#include "hbstyleoptionsliderelement_p.h"
#include "hbstyleoptionpushbutton_p.h"
#include "hbstyleoptiontoolbutton_p.h"
#include "hbstyleoptiontooltip_p.h"
#include "hbstyleoptionprogressbar_p.h"
#include "hbstyleoptionlabel_p.h"
#include "hbstyleoptioncheckbox_p.h"
#include "hbiconloader_p.h"
#include "hbstyleoptiontitlepane_p.h"
#include "hbstyleoptionstatusbar_p.h"
#include "hbstyleoptionindicatorgroup_p.h"
#include "hbstyleoptionprogressdialog_p.h"
#include "hbstyleoptionnotificationdialog_p.h"
#include "hbstyleoptionmessagebox_p.h"
#include "hbstyleoptionnavigationbutton_p.h"
#include "hbstyleoptionindicatorbutton_p.h"
#include "hbstyleoptionsignalindicator_p.h"
#include "hbstyleoptionbatteryindicator_p.h"
#include "hbstyleloader.h"
#include "hbwidgetloader_p.h"
#include "hbwidget_p.h"
#include "hbstyleoptionratingslider_p.h"
#include "hbstyleoptiondataformviewitem_p.h"
#include "hbstyleoptiondatagroup_p.h"
#include "hbstyleoptiondatagroupheadingwidget_p.h"
#include "hbstyleoptiondataform_p.h"
#include "hbstyleoptiongroupbox_p.h"
#include "hbstyleoptioncombobox_p.h"
#include "hbstyleoptioninputdialog_p.h"
#include "hbstyleoptionprogresssliderhandle_p.h"
#include "hbstyleoptionprogressslider_p.h"


#include <hbstyleprimitivedata.h>
#include <hbstyletextprimitivedata.h>
#include <hbstylerichtextprimitivedata.h>
#include <hbstyleiconprimitivedata.h>
#include <hbstyleframeprimitivedata.h>
#include <hbstylemarqueeprimitivedata.h>
#include <hbstyletouchareaprimitivedata.h>

#include <hbicon.h>
#include <hbstyle.h>
#include <hbframedrawer.h>
#include <hbiconitem.h>
#include <hbiconitem_p.h>
#include <hbframeitem.h>
#include <hbframedrawer.h>
#include <hbframedrawer_p.h>
#include <hbframebackground.h>
#include <hbslidertrackitem_p.h>
#include <hbinstance.h>
#include <hbtextitem.h>
#include <hbtextitem_p.h>
#include <hbmarqueeitem.h>
#include <hbmarqueeitem_p.h>
#include <hbrichtextitem.h>
#include <hbrichtextitem_p.h>
#include <hbtoucharea.h>
#include <hbindicatorleveliconitem_p.h>

#include <hbglobal.h>
#include <hbstyleparameters.h>

#include "hbstyleselector_p.h"
#include "hblayeredstyleloader_p.h"
#include "hbwidgetstyleloader_p.h"
#include "hbcssparser_p.h"
#include "hbnamespace_p.h"

#include "hbanchorlayout.h"
#include <hbanchor.h>

#include <QGraphicsWidget>
#include <hbwidgetbase.h>
#include "hbdeviceprofile.h"


#include <QDebug>
#include <QMetaClassInfo>

#ifdef HB_TEXT_MEASUREMENT_UTILITY
#include "hbinstance_p.h"
#include "hbtextmeasurementutility_r.h"
#endif


Q_DECLARE_METATYPE(QGraphicsLayout*)

//Uncomment next define(s) in order to get more debug prints.
//Similar define exists also in the engine side.
//#define HBSTYLE_DEBUG

/*!
    @beta
    @hbcore
    \class HbStyle
    \brief HbStyle is the style management class for widgets

    HbStyle is used for the style management of widgets.
    It offers methods to create and update styled primitives. Briefly, styled primitives
    are HbWidgetBase-derived (which is a QGraphicsWidget) classes which draw a certain part of the user interface (for example
    the button background, or the slider thumb).

    A widget that chooses to be styled needs to create its primitives
    by using the createPrimitive method.

    Whenever the widget's state changes in a way that needs changes in drawing the primitive
    (such as button pressed down -> change the button background image) then the widget needs to
    call the updatePrimitive method.

    Generally primitives should be updated only when a state change occurs. When a widget uses primitives to construct
    itself it does not need a paint() method at all since primitives (widget's children) are doing the drawing.
    Painting for the primitives occurs from the graphics scene.

    HbStyle has some caching functionality and thus it should not be instantiated explicitly, but
    accessed only through HbWidget::style() or HbInstance::style() APIs.

*/

// TODO: margins should be defined in layout data once layout specification exists.
static const QString STYLE_LOCATION = QLatin1String(":/themes/style/hbdefault/rules/widgets/%w/%w");
static const QString COLOR_STYLE_LOCATION = QLatin1String(":/themes/style/hbdefault/rules/widgets/%w/%w_color.css");
static const int TOUCHAREA_ZVALUE = 1000;

static const QString GLOBAL_PARAMETERS_LOCATION = QLatin1String(":/themes/style/hbdefault/variables/layout/zoom/0/hbglobalparameters.css");

static const char* LAYOUT_PTR_PROPERTY = "HbStyle_layout_ptr";
static const char* LAYOUT_NAME_PROPERTY = "HbStyle_layout_name";

inline void overrideSpacing( HbAnchorLayout *layout, const QString &name, Hb::Edge edge, qreal val )
{
    if( name.isNull() ) {
        return;
    }
    QList<HbAnchor*> list = layout->anchors();
    for( int i = 0; i < list.size(); i++ ) {
        HbAnchor *anchor = list.at(i);
        if( ( anchor->startNodeId() == name ) && ( anchor->startEdge() == edge ) ) {
            anchor->setDirection( val < 0 ? HbAnchor::Negative : HbAnchor::Positive );
            anchor->setPreferredLength( qAbs(val) );
            if (anchor->anchorId().isEmpty()) {
                // assuming it's a fixed anchor
                anchor->setSizePolicy( QSizePolicy::Fixed );
            } else {
                // assuming it's a "spacer" and we want to override the minimum length
                anchor->setMinimumLength( qAbs(val) );
            }
        }
    }
}

/*!
  Must not be called explicitly. Use HbWidget::style() or HbInstance::style() instead.

  \deprecated HbStyle::HbStyle()
        is deprecated. Use HbWidget::style() or HbInstance::style() instead.
*/
HbStyle::HbStyle() :
    d_ptr(new HbStylePrivate)
{
    Q_D( HbStyle );
    d->q_ptr = this;
}

/*!
Destructor
*/
HbStyle::~HbStyle()
{
    delete d_ptr;
}
/*!
   Instantiates widget building blocks. The building blocks are called "primitives" and their main function is to perform the painting (e.g. drawing text).
   A set of base primitives are defined in HbStyle::BasePrimitive enumeration. A widget calling this method should not cast the returned QGraphicsObject
   to the actual primitive object. By following this rule the styling mechanism can be used to change the actual primitive instance to something else than the original one 
   instantiated by the createPrimitive() method. This enables customizing the painting of the UI without touching the widget code.
      
   Updating of a primitive's state is done by using HbStyle::updatePrimitive() method which uses the style primitive data classes (comparable to
   Qt styleoptions) to get the state information needed for the primitive updating.

    \param primitiveType enumeration of the base primitive
    \param itemName string which identifies a primitive in the widget (e.g. "background", or "text"). The item name matches with primitive names in widget's CSS file.
    \param parent parent for the primitive, although the default is 0 usually the widget is given as a parent for the primitive
    \return QGraphicsObject, the returned primitive should be stored as a QGraphicsObject on the widget side
    \sa HbStyle::updatePrimitive()
    \sa HbStylePrimitiveData
    \sa HbStyleTextPrimitiveData
    \sa HbStyleFramePrimitiveData
    \sa HbStyleIconPrimitiveData
    \sa HbStyleRichTextPrimitiveData
    \sa HbStyleMarqueePrimitiveData
    \sa HbStyleTouchAreaPrimitiveData

 */
QGraphicsObject *HbStyle::createPrimitive(HbStyle::PrimitiveType primitiveType, const QString &itemName, QGraphicsObject *parent) const
{
    QGraphicsObject *bp;

    switch (primitiveType) {
    case PT_TextItem:
        bp = new HbTextItem(parent);
        break;
    case PT_RichTextItem:
        bp = new HbRichTextItem(parent);
        break;
    case PT_FrameItem:
        bp = new HbFrameItem(parent);
        break;
    case PT_IconItem:
        bp = new HbIconItem(parent);
        break;
    case PT_MarqueeItem:
        bp = new HbMarqueeItem(parent);
        break;
    case PT_TouchArea:
        bp = new HbTouchArea(parent);
        break;
    default:
        bp = 0;
    }

    if (bp != 0) {
        setItemName(bp, itemName);
    }

    return bp;
}

/*!
    This method is called by Hb widgets whenever there's a widget state change that results to a change in primitive's state.
    For example if a button's text changes the button widget needs to update the button's text primitive by calling this method.
    
    Widgets need to use the HbStylePrimitiveData-derived classes to carry the primitive state information from the widget to the style.
    These data classes enable the widget to set the primitives' state information without directly using the primitive APIs. 

    Note that the updating of the primitive attributes is optimized so that if just one value from the primitive data set is updated only that one
    value will be updated to the primitive on the style side. The values on primitive data classes are stored by using HbStyleValue templated class
    which keeps track of whether a primitive data value was assigned or not. On the style side before calling the primitive API the HbStyleValue's
    isSet()-method is called first to check if the widget did assign the primitive data value.


    \param primitive the primitive to be updated
    \param data primitive data needed for updating the primitive
    \param parent optionally the parent can be given
    \return bool, true if correct primitive type was found from the style, false if not.
    \sa HbStyle::createPrimitive()
    \sa HbStylePrimitiveData
    \sa HbStyleTextPrimitiveData
    \sa HbStyleFramePrimitiveData
    \sa HbStyleIconPrimitiveData
    \sa HbStyleRichTextPrimitiveData
    \sa HbStyleMarqueePrimitiveData
    \sa HbStyleTouchAreaPrimitiveData
 */
bool HbStyle::updatePrimitive(QGraphicsObject *primitive, const HbStylePrimitiveData *data, QGraphicsObject *parent) const
{
    Q_UNUSED(parent);
    bool ret = false;

    if (HbTextItem *textItem = qgraphicsitem_cast<HbTextItem*>(primitive)) {
        const HbStyleTextPrimitiveData *td = hbstyleprimitivedata_cast<const HbStyleTextPrimitiveData*>(data);
        if (td->text.isSet())
            textItem->setText(td->text);
        if (td->textColor.isSet())
            textItem->setTextColor(td->textColor);
        if (td->alignment.isSet())
            textItem->setAlignment(td->alignment);
        if (td->elideMode.isSet())
            textItem->setElideMode(td->elideMode);
        if (td->textWrapping.isSet())
            textItem->setTextWrapping(td->textWrapping);
        if (td->isTextVisible.isSet())
            textItem->setTextVisible(td->isTextVisible);
        if (td->isTextClip.isSet())
            textItem->setTextClip(td->isTextClip);
        if (td->geometry.isSet())
            textItem->setGeometry(td->geometry);
        if (td->fadeLengths.isSet())
            textItem->setFadeLengths(td->fadeLengths);
        if (td->minimumLines.isSet())
            textItem->setMinimumLines(td->minimumLines);
        if (td->maximumLines.isSet())
            textItem->setMaximumLines(td->maximumLines);
        if (td->fontSpec.isSet())
            textItem->setFontSpec(td->fontSpec);
        ret = true;

    } else if (HbIconItem *iconItem = qgraphicsitem_cast<HbIconItem*>(primitive)) {
        const HbStyleIconPrimitiveData *id = hbstyleprimitivedata_cast<const HbStyleIconPrimitiveData*>(data);
        if (id->icon.isSet())
            iconItem->setIcon(id->icon);
        if (id->size.isSet())
            iconItem->setSize(id->size);
        if (id->aspectRatioMode.isSet())
            iconItem->setAspectRatioMode(id->aspectRatioMode);
        if (id->alignment.isSet())
            iconItem->setAlignment(id->alignment);
        if (id->iconMode.isSet())
            iconItem->setMode(id->iconMode);
        if (id->iconState.isSet())
            iconItem->setState(id->iconState);
        if (id->iconName.isSet())
            iconItem->setIconName(id->iconName);
        if (id->iconFlags.isSet())
            iconItem->setFlags(id->iconFlags);
        if (id->mirroringMode.isSet())
            iconItem->setMirroringMode(id->mirroringMode);
        if (id->brush.isSet())
            iconItem->setBrush(id->brush);
        if (id->color.isSet())
            iconItem->setColor(id->color);
        ret = true;

    } else if(HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>(primitive)) {
        const HbStyleFramePrimitiveData *fd =  hbstyleprimitivedata_cast<const HbStyleFramePrimitiveData*>(data);
        HbFrameDrawer *drawer = &(frameItem->frameDrawer()); 
        if (fd->frameGraphicsName.isSet())
            drawer->setFrameGraphicsName(fd->frameGraphicsName);
        if (fd->frameType.isSet())
            drawer->setFrameType(fd->frameType);
        if (fd->borderWidthLeft.isSet() || fd->borderWidthRight.isSet() || fd->borderWidthBottom.isSet() || fd->borderWidthTop.isSet())
            drawer->setBorderWidths(fd->borderWidthLeft, fd->borderWidthTop, fd->borderWidthRight, fd->borderWidthBottom);  
        if (fd->fillWholeRect.isSet())
            drawer->setFillWholeRect(fd->fillWholeRect);       
        if (fd->mirroringMode.isSet())
            drawer->setMirroringMode(fd->mirroringMode);
        if (fd->fileNameSuffixList.isSet())
            drawer->setFileNameSuffixList(fd->fileNameSuffixList);
        if (fd->pixmapMask.isSet())
            drawer->setMask(fd->pixmapMask);
        if (fd->bitmapMask.isSet())
            drawer->setMask(fd->bitmapMask); 
        frameItem->setFrameDrawer(drawer);
        ret = true;

    } else if (HbMarqueeItem *marqueeItem = qgraphicsitem_cast<HbMarqueeItem*>(primitive)) {
         const HbStyleMarqueePrimitiveData *md =  hbstyleprimitivedata_cast<const HbStyleMarqueePrimitiveData*>(data);
         if (md->text.isSet())
             marqueeItem->setText(md->text);
         if (md->textColor.isSet())
             marqueeItem->setTextColor(md->textColor);
         if (md->animation.isSet()) {
             if (md->animation)
                 marqueeItem->startAnimation();
             else 
                 marqueeItem->stopAnimation();
         }
         if (md->loopCount.isSet())
             marqueeItem->setLoopCount(md->loopCount);
        ret = true;

    } else if (HbTouchArea *touchArea = qgraphicsitem_cast<HbTouchArea*>(primitive)) {
        const HbStyleTouchAreaPrimitiveData *td =  hbstyleprimitivedata_cast<const HbStyleTouchAreaPrimitiveData*>(data);
        if (td->size.isSet())
            touchArea->setSize(td->size);
        if (td->geometry.isSet())
            touchArea->setGeometry(td->geometry);
        ret = true;

    } else if (HbRichTextItem *richTextItem = qgraphicsitem_cast<HbRichTextItem*>(primitive)) {
        const HbStyleRichTextPrimitiveData *rd =  hbstyleprimitivedata_cast<const HbStyleRichTextPrimitiveData*>(data);
        if (rd->text.isSet())
            richTextItem->setText(rd->text);
        if (rd->defaultColor.isSet())
            richTextItem->setTextDefaultColor(rd->defaultColor);
        if (rd->alignment.isSet())
            richTextItem->setAlignment(rd->alignment);
        if (rd->textWrappingMode.isSet())
            richTextItem->setTextWrapping(rd->textWrappingMode);
        if (rd->isTextVisible.isSet())
            richTextItem->setTextVisible(rd->isTextVisible);
        if (rd->clipping.isSet())
            richTextItem->setTextClip(rd->clipping);
        if (rd->geometry.isSet())
            richTextItem->setGeometry(rd->geometry);
        ret = true;
    }

    if (ret) {
        if (data->geometry.isSet() 
            && primitive->isWidget()) {
            static_cast<QGraphicsWidget*>(primitive)->setGeometry(data->geometry);
        }
    }

    return ret;
}


QGraphicsItem *HbStylePrivate::createPrimitive( HbStylePrivate::Primitive primitive, QGraphicsItem *parent )
{
        
        switch (primitive){
            case P_ToolButton_icon:
            case P_PushButton_icon:
            case P_ProgressDialog_icon: {
                HbIconItem *iconItem = new HbIconItem(HbIcon(),parent);
                HbStyle::setItemName(iconItem, QLatin1String("icon"));
                return iconItem;
                }

            case P_CheckBox_icon:
            case P_GroupBoxHeading_icon:
            case P_Label_icon: {
                HbIconItem *item = q_check_ptr(new HbIconItem(HbIcon(),parent));
                HbStyle::setItemName(item, QLatin1String("icon"));
                return item;
                }
            case P_GridViewItem_icon: {
                Q_ASSERT_X(false, "HbStyle" , "P_GridViewItem_icon no more supported. Use refactored styling");
                return 0;
            }
            case P_DataGroup_icon:
                {
                HbIconItem *item = new HbIconItem(HbIcon(), parent);
                HbStyle::setItemName(item, QLatin1String("dataGroup_Icon"));
                return item;
                }
            case P_DataItem_icon:
                {
                HbIconItem *item = new HbIconItem(HbIcon(), parent);
                HbStyle::setItemName(item, QLatin1String("dataItem_Icon"));
                return item;
                }
            case P_ComboBox_text:
                {
                    HbTextItem *textItem = new HbTextItem(parent);
                    HbStyle::setItemName(textItem, QLatin1String("combobox_labelfield"));
                    return  textItem;
                }
            case P_ToolButton_text:
            case P_CheckBox_text:
            case P_ProgressDialog_text:
                {
                    HbTextItem *textItem = new HbTextItem(parent);
                    HbStyle::setItemName(textItem, QLatin1String("text"));
                    textItem->setTextWrapping(Hb::TextWordWrap);
                    return  textItem;
                }
            case P_PushButton_text:
                {
                HbTextItem *ti = new HbTextItem(parent);
                HbStyle::setItemName(ti, QLatin1String("text"));
                return  ti;
                }
            case P_DataGroup_heading:
                {
                HbTextItem *ti = new HbTextItem(parent);
                HbStyle::setItemName(ti, QLatin1String("dataGroup_HeadingLabel"));
                return  ti;
                }
            case P_DataGroup_description:
                {
                HbTextItem *ti = new HbTextItem(parent);
                HbStyle::setItemName(ti, QLatin1String("dataGroup_Description"));
                return  ti;
                }
            case P_DataForm_heading:
                {
                HbTextItem *ti = new HbTextItem(parent);
                HbStyle::setItemName(ti, QLatin1String("dataForm_Heading"));
                //ti->setZValue(2);
                return  ti;
                }
            case P_DataForm_description:
                {
                HbTextItem *ti = new HbTextItem(parent);
                HbStyle::setItemName(ti, QLatin1String("dataForm_Desc"));
                return  ti;
                }
            case P_PushButton_additionaltext:
                {
                HbTextItem *ti = new HbTextItem(parent);
                HbStyle::setItemName(ti, QLatin1String("additional-text"));
                return  ti;
                }

            case P_ProgressSlider_toucharea:
                return 0;
            case P_ProgressSliderHandle_toucharea:
                return 0;

            case P_ProgressSliderHandle_icon:
                {
                HbIconItem *item = new HbIconItem(QLatin1String("qtg_graf_progslider_handle_normal"), parent);
                item->setAspectRatioMode(Qt::IgnoreAspectRatio);
                return item;
                }
            case P_PushButton_toucharea: // Generic implementation, can add other cases here
            case P_CheckBox_toucharea:
            case P_RatingSlider_toucharea:
            case P_SliderElement_touchhandle:
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                HbStyle::setItemName(ta, QLatin1String("toucharea"));
                if(parent){
                    parent->setFiltersChildEvents(true);
                }
                return ta;
                }
            case P_ComboBoxButton_toucharea:
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                HbStyle::setItemName(ta, QLatin1String("combobox_button_toucharea"));
                if(parent){
                    parent->setFiltersChildEvents(true);
                }
                return ta;
                }
             case P_TitleBar_toucharea:
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                ta->grabGesture(Qt::SwipeGesture);
                HbStyle::setItemName(ta, QLatin1String("toucharea"));
                return ta;
                }
             case P_IndicatorButton_toucharea:
             case P_NavigationButton_toucharea:
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                ta->grabGesture(Qt::TapGesture);
                HbStyle::setItemName(ta, QLatin1String("toucharea"));
                if (parent){
                    parent->setHandlesChildEvents(true);
                }
                return ta;
                }
             case P_TitlePane_toucharea:
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                ta->grabGesture(Qt::TapGesture);
                ta->grabGesture(Qt::SwipeGesture);
                ta->grabGesture(Qt::PanGesture);
                HbStyle::setItemName(ta, QLatin1String("toucharea"));
                if (parent){
                    parent->setHandlesChildEvents(true);
                }
                return ta;
                }
             case P_SliderElement_touchdecrease:
             case P_SliderElement_touchincrease:
             case P_SliderElement_touchgroove:
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                if(parent){
                    parent->setFiltersChildEvents(true);
                }
                return ta;
                }

            case P_SliderElement_text:
                {
                    HbTextItem *ti = new HbTextItem(parent);
                    return  ti;
                }

            case P_ProgressBar_text:
                return 0;

            case P_DataItem_label:
            {
                HbTextItem *ti = new HbTextItem(parent);
                HbStyle::setItemName(ti, QLatin1String("dataItem_Label"));
                return  ti;
            }
            case P_DataItem_description:
            {
                HbTextItem *ti = new HbTextItem(parent);
                HbStyle::setItemName(ti, QLatin1String("dataItem_Description"));
                return  ti;
            }
            case P_ProgressBar_mintext:
                return 0;
            case P_ProgressBar_maxtext:
                return 0;

            case P_Label_text: {
                HbTextItem *ti = new HbTextItem(parent);
                HbStyle::setItemName(ti, QLatin1String("text"));
                ti->setAlignment(Qt::AlignCenter);
                return ti;
            }

            case P_Label_richtext:
            {
                HbRichTextItem *rti = new HbRichTextItem(parent);
                HbStyle::setItemName(rti, QLatin1String("text"));
                return rti;
            }

            case P_Slider_thumb:
                return new HbIconItem(parent);

            case P_Slider_groove:            
            case P_Popup_heading_frame:
            case P_NotificationDialog_frame:
            case P_PushButton_focus:
            case P_ComboBox_background:
            case P_SliderPopup_background:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                return n;
            }

            case P_ProgressBar_frame:
                return 0;

            case P_ComboBoxPopup_background:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                HbStyle::setItemName(n , "dropdown_background");
                return n;
            }

            case P_TumbleView_background:
            case P_TumbleView_frame:
            case P_DateTimePicker_background:
            case P_DateTimePicker_frame:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-5);
                return n;
            }
            case P_TumbleView_highlight:
            {
                return new HbFrameItem(parent);//TODO:make it icon once iconitem setGeomoetry works for tumbleview
            }

            case P_DateTimePicker_separator:{
                HbFrameItem *frameItem= new HbFrameItem(parent);//TODO:make it icon once iconitem setGeomoetry works for tumbleview
                return frameItem;
            }
            case P_DataGroup_background :
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                HbStyle::setItemName(n, QLatin1String("dataGroup_Background"));
                return n;
            }
            case P_DataGroupComboBackground:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                HbStyle::setItemName(n, QLatin1String("dataGroup_ComboBackground"));
                return n;
            }
            case P_DataForm_heading_background :
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                HbStyle::setItemName(n, QLatin1String("dataFormHeading_Background"));
                return n;
            }
            case P_DataItem_background:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-2);
                HbStyle::setItemName(n, QLatin1String("dataItem_Background"));
                return n;
            }
            case P_GroupBoxContent_background:
            case P_GroupBoxHeading_background:
            case P_PushButton_background:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                HbStyle::setItemName(n, QLatin1String("background"));
                return n;
            }

            case P_ProgressBar_track:
                return 0;
            case P_ProgressSlider_slidertrack:
                return 0;

            case P_Slider_progressgroove:
            {
                HbSliderTrackItem *n=new HbSliderTrackItem(parent);
                n->setZValue(0);
                return n;
            }
            case P_ToolButton_frame:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                HbStyle::setItemName(n, QLatin1String("background"));
                return n;
            }
            case P_NavigationButton_background:
            case P_IndicatorButton_background:
            {
                HbIconItem *n = new HbIconItem(HbIcon(), parent);
                n->setZValue(-1);
                HbStyle::setItemName(n, QLatin1String("background"));
                return n;
            }
            case P_IndicatorButton_handleindication:
            {
                HbIconItem *n = new HbIconItem(parent);
                HbStyle::setItemName(n, QLatin1String("handleindication"));
                return n;
            }
            case P_IndicatorButton_eventindication:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                HbStyle::setItemName(n, QLatin1String("eventindication"));
                return n;
            }
            case P_IndicatorGroup_icon1:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                HbStyle::setItemName(n, QLatin1String("icon1"));
                return n;
            }
            case P_IndicatorGroup_icon2:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                HbStyle::setItemName(n, QLatin1String("icon2"));
                return n;
            }
            case P_IndicatorGroup_icon3:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                HbStyle::setItemName(n, QLatin1String("icon3"));
                return n;
            }
            case P_IndicatorGroup_icon4:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                HbStyle::setItemName(n, QLatin1String("icon4"));
                return n;
            }
            case P_SignalIndicator_icon:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                HbStyle::setItemName(n, QLatin1String("icon"));
                return n;
            }
            case P_SignalLevel_background:
            {
                HbIndicatorLevelIconItem *n = new HbIndicatorLevelIconItem(parent);
                n->setZValue(-1);
                HbStyle::setItemName(n, QLatin1String("background"));
                return n;
            }
            case P_SignalLevel_icon:
            {
                HbIndicatorLevelIconItem *n = new HbIndicatorLevelIconItem(parent);
                HbStyle::setItemName(n, QLatin1String("levelicon"));
                return n;
            }
            case P_BatteryIndicator_icon:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                HbStyle::setItemName(n, QLatin1String("icon"));
                return n;
            }
            case P_BatteryLevel_background:
            {
                HbIndicatorLevelIconItem *n = new HbIndicatorLevelIconItem(parent);
                n->setZValue(-1);
                HbStyle::setItemName(n, QLatin1String("background"));
                return n;
            }
            case P_BatteryLevel_icon:
            {
                HbIndicatorLevelIconItem *n = new HbIndicatorLevelIconItem(parent);
                HbStyle::setItemName(n, QLatin1String("levelicon"));
                return n;
            }
            case P_TitlePane_background:
                {
                    HbFrameItem *n = new HbFrameItem(parent);
                    n->setZValue(-1);
                    HbStyle::setItemName(n, QLatin1String("background"));
                    return n;
                }
            case P_LineEdit_frame_normal:
            case P_LineEdit_frame_highlight:
            case P_TextEdit_frame_normal:
            case P_TextEdit_frame_highlight:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                return n;
            }
            case P_GroupBoxMarquee_text:
            case P_TitlePane_text:
            {
                HbMarqueeItem *n = new HbMarqueeItem(parent);
                HbStyle::setItemName(n, QLatin1String("text"));
                return n;
            }
            case P_TitlePane_icon:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                HbStyle::setItemName(n, QLatin1String("icon"));
                return n;
            }
            case P_StatusBar_background:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                HbStyle::setItemName(n, QLatin1String("background"));
                return n;
            }
            case P_StatusBar_timetext:
            {
                HbTextItem *textItem = new HbTextItem(parent);
                HbStyle::setItemName(textItem, QLatin1String("timetext"));
                textItem->setAlignment(Qt::AlignCenter);
                return textItem;
            }
            case P_Fade_background:
                return new HbIconItem(QString(), parent);
            case P_SliderElement_icon:
            case P_SliderElement_increase:
            case P_SliderElement_decrease:
                return new HbIconItem(QString(), parent);

            case P_SliderTickMark_majoricon: {
                HbIconItem *iconItem = new HbIconItem(parent);
                HbStyle::setItemName(iconItem,QLatin1String("tickmark-majoricon"));
                return iconItem;
            }
            case P_SliderTickMark_minoricon: {
                HbIconItem *iconItem = new HbIconItem(parent);
                HbStyle::setItemName(iconItem,QLatin1String("tickmark-minoricon"));
                return iconItem;
            }
            case P_SliderTickMark_majorlabel: {
                HbTextItem *textItem = new HbTextItem(parent);
                HbStyle::setItemName(textItem,QLatin1String("tickmark-majorlabel"));
                return textItem;
            }
            case P_SliderTickMark_minorlabel: {
                HbTextItem *textItem = new HbTextItem(parent);
                HbStyle::setItemName(textItem,QLatin1String("tickmark-minorlabel"));
                return textItem;
            }

            case P_ProgressBar_waittrack:
                return 0;
            case P_RatingSlider_frame:
                return 0;
            case P_RatingSlider_track:
                return 0;
            case P_RatingSlider_layout:
                return 0;

            case P_ItemViewItem_checkbox: {
                qDebug() << "Primitive P_ItemViewItem_checkbox is deprecated and will cease to exist in the near future.";
                HbIconItem *iconItem = new HbIconItem(parent);
                HbStyle::setItemName(iconItem, QLatin1String("checkbox-icon"));
                return iconItem;
            }

            case P_ItemViewItem_radiobutton:
            case P_ItemViewItem_selection: {
                Q_ASSERT_X(false, "HbStyle" , "P_ItemViewItem_selection no more supported. Use refactored styling");
                return 0;
            }

            case P_Edit_text:{
                return 0;
            }
            case P_GroupBoxHeading_text:
            case P_NotificationDialog_text:{
                HbTextItem *n = new HbTextItem(parent);
                HbStyle::setItemName(n, QLatin1String("text"));
                n->setMinimumLines(1);
                n->setMaximumLines(1);
                n->setTextWrapping(Hb::TextNoWrap);
                return n;
            }
            case P_NotificationDialog_title:{
                HbTextItem *n = new HbTextItem(parent);
                HbStyle::setItemName(n, QLatin1String("title"));
                return n;
            }
            case P_NotificationDialog_icon:{
                HbIconItem *n = new HbIconItem(QString(), parent);
                HbStyle::setItemName(n, QLatin1String("icon"));
                return n;
            }
            case P_MessageBox_text:{    
                HbTextItem *rti = new HbTextItem(parent);
                return rti;
            }
            case P_MessageBox_icon:{
                HbIconItem *n = new HbIconItem(QString(), parent);
                return n;
            }

            case P_ItemViewItem_background: {
                Q_ASSERT_X(false, "HbStyle" , "P_ItemViewItem_background no more supported. Use refactored styling");
                return 0;
            }

            case P_ListViewItem_text: {
                Q_ASSERT_X(false, "HbStyle" , "P_ListViewItem_text no more supported. Use refactored styling");
                return 0;
            }

            case P_ListViewItem_richtext: {
                Q_ASSERT_X(false, "HbStyle" , "P_ListViewItem_richtext no more supported. Use refactored styling");
                return 0;
            }

            case P_GridViewItem_text: {
                Q_ASSERT_X(false, "HbStyle" , "P_GridViewItem_text no more supported. Use refactored styling");
                return 0;
            }

            case P_ListViewItem_icon:
                Q_ASSERT_X(false, "HbStyle" , "P_ListViewItem_icon no more supported. Use refactored styling");
                return 0;

            case P_TreeViewItem_expandicon: {
                Q_ASSERT_X(false, "HbStyle" , "P_TreeViewItem_expandicon no more supported. Use refactored styling");
                return 0;
            }

            case P_ComboBox_button: {
                HbIconItem *n = new HbIconItem(QString(), parent);
                return n;
            }
            case P_ItemViewItem_focus: {
                Q_ASSERT_X(false, "HbStyle" , "P_ItemViewItem_focus no more supported. Use refactored styling");
                return 0;
            }
            case P_ItemHighlight_background:
                return new HbFrameItem(parent);

            case P_ItemViewItem_frame: {
                Q_ASSERT_X(false, "HbStyle" , "P_ItemViewItem_frame no more supported. Use refactored styling");
                return 0;
            }
            case P_ItemViewItem_touchmultiselection:
            {
                Q_ASSERT_X(false, "HbStyle" , "P_ItemViewItem_touchmultiselection no more supported. Use refactored styling");
                return 0;
            }
            case P_IndexFeedback_popup_text:
            {
                Q_ASSERT_X(false, "HbStyle" , "P_IndexFeedback_popup_text no more supported. Use refactored styling");
                return 0;
            }
            case P_IndexFeedback_popup_background:
            {
                Q_ASSERT_X(false, "HbStyle" , "P_IndexFeedback_popup_background no more supported. Use refactored styling");
                return 0;
            }
            case P_InputDialog_text:
            {
                HbTextItem *n = new HbTextItem(parent);
                n->setTextWrapping(Hb::TextWordWrap);
                HbStyle::setItemName(n, QLatin1String("label-1"));
                return n;
            }
            case P_InputDialog_additionaltext:
            {
                HbTextItem *n = new HbTextItem(parent);
                n->setTextWrapping(Hb::TextWordWrap);
                HbStyle::setItemName(n, QLatin1String("label-2"));
                return n;
            }
            default:
                return 0;
        }
}

void HbStylePrivate::updatePrimitive( QGraphicsItem *item, HbStylePrivate::Primitive primitive, const QStyleOption *option )
{
    
    switch(primitive){
            case P_PushButton_icon:
                {
                    if (const HbStyleOptionPushButton *opt =
                        qstyleoption_cast<const HbStyleOptionPushButton*>(option)) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                        //iconItem->setIconName(opt->icon.iconName());
                        iconItem->setIcon(opt->icon); // with this call iconitem refresh issue is there
                        iconItem->setMode(iconMode(opt->state));
                        iconItem->setState(iconState(opt->state));
                        iconItem->setAlignment(Qt::AlignCenter);
                    }
                break;
                }
            case P_GroupBoxHeading_icon:
                {
                    if (const HbStyleOptionGroupBox *opt =
                        qstyleoption_cast<const HbStyleOptionGroupBox*>(option)) {
                            HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                            if (opt->collapsed) {
                                iconItem->setIconName(QLatin1String("qtg_small_expand"));
                            } else {
                                iconItem->setIconName(QLatin1String("qtg_small_collapse"));
                            }
                    }
                    break;
                }
            case P_DataGroup_icon:
                {
                    if (const HbStyleOptionDataGroupHeadingWidget *opt =
                            qstyleoption_cast<const HbStyleOptionDataGroupHeadingWidget*>(option)) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                        if (opt->expanded) {
                            iconItem->setIconName(QLatin1String("qtg_small_collapse"));
                        } else {
                            iconItem->setIconName(QLatin1String("qtg_small_expand"));
                        }
                    }
                break;
                }
            case P_DataItem_icon:
                {
                    const HbStyleOptionDataFormViewItem *opt = qstyleoption_cast<const HbStyleOptionDataFormViewItem *>(option);
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIcon(opt->icon);
                    break;
                }
            case P_Label_icon:
                if (const HbStyleOptionLabel *opt = qstyleoption_cast<const HbStyleOptionLabel*>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIcon(opt->icon);
                    if (!opt->icon.isNull()) {
                        iconItem->setMode(iconMode(opt->state));
                        iconItem->setState(iconState(opt->state));
                        iconItem->setAspectRatioMode(opt->aspectRatioMode);
                        iconItem->setAlignment(opt->alignment);
                    }
                }
                break;
            case P_ToolButton_icon:
                if (const HbStyleOptionToolButton *opt = qstyleoption_cast<const HbStyleOptionToolButton*>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIcon(opt->icon);
                    iconItem->setMode(iconMode(opt->state));
                    iconItem->setState(iconState(opt->state));
                }
                break;
            case P_ComboBox_text:
                if (const HbStyleOptionComboBox *opt = qstyleoption_cast<const HbStyleOptionComboBox*>(option)) {
                    HbTextItem *comboTextItem = static_cast<HbTextItem*>(item);
                    comboTextItem->setText(opt->text);
                    comboTextItem->setTextWrapping(Hb::TextNoWrap);
                }
                break;
            case P_PushButton_text:
                {
                    if (const HbStyleOptionPushButton *opt =
                        qstyleoption_cast<const HbStyleOptionPushButton*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        textItem->setText(opt->text);
                        //default alignment will come from css,
                        //if api flag is set ,then alignment from style is taken.
                        if (opt->hasTextAlignment) {
                            textItem->setAlignment( opt->textAlignment );
                        }
                        textItem->setTextWrapping(Hb::TextWordWrap);
                    }
                }
                break;
            case P_ProgressDialog_text:
                {
                    if (const HbStyleOptionProgressDialog *opt =
                        qstyleoption_cast<const HbStyleOptionProgressDialog*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        textItem->setText(opt->text);
                    }
                break;
                }
            case P_PushButton_additionaltext:
                {
                    if (const HbStyleOptionPushButton *opt =
                            qstyleoption_cast<const HbStyleOptionPushButton*>(option)) {
                        HbTextItem *additionalTextItem = static_cast<HbTextItem*>(item);
                        additionalTextItem->setText(opt->additionalText);
                        //default alignment will come from css,
                        //if api flag is set ,then alignment from style is taken.
                        if (opt->hasAdditionalTextAlignment) {
                            additionalTextItem->setAlignment( opt->additionalTextAlignment );
                        }
                        additionalTextItem->setTextWrapping(Hb::TextWordWrap);
                    }
                }
                break;
            case P_PushButton_toucharea:  // Generic implementation, can add other cases here
            case P_ComboBoxButton_toucharea:
            case P_CheckBox_toucharea:
            case P_TitleBar_toucharea:
            case P_IndicatorButton_toucharea:
            case P_TitlePane_toucharea:
            case P_NavigationButton_toucharea:
            case P_SliderElement_touchdecrease:
            case P_SliderElement_touchincrease:
            case P_SliderElement_touchgroove:
            case P_SliderElement_touchhandle:
                {
                    HbTouchArea *touchArea = static_cast<HbTouchArea*>(item);
                    touchArea->setZValue(TOUCHAREA_ZVALUE);
                }
                break;
            case P_DataGroup_heading:
                {
                   if (const HbStyleOptionDataGroupHeadingWidget *opt =
                        qstyleoption_cast<const HbStyleOptionDataGroupHeadingWidget*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        textItem->setText(opt->heading);
                        textItem->setTextWrapping(Hb::TextNoWrap);
                    }
                break;
                }
            case P_DataGroup_description:
                {
                   if (const HbStyleOptionDataGroupHeadingWidget *opt =
                        qstyleoption_cast<const HbStyleOptionDataGroupHeadingWidget*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        textItem->setText(opt->description);
                    }
                break;
                }
            case P_DataForm_heading:
                {
                   if (const HbStyleOptionDataForm *opt =
                        qstyleoption_cast<const HbStyleOptionDataForm*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        textItem->setText(opt->heading);
                    }
                break;
                }
            case P_DataForm_description:
                {
                   if (const HbStyleOptionDataForm *opt =
                        qstyleoption_cast<const HbStyleOptionDataForm*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        HbFontSpec spec(HbFontSpec::Secondary);
                        QFont descriptionFont = spec.font();
                        descriptionFont.setItalic(true);
                        textItem->setFont(descriptionFont);
                        textItem->setText(opt->description);
                    }
                break;
                }

            case P_DataForm_heading_background: {
                HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                if(!frameItem){
                    return;
                }
                frameItem->setZValue(-1.0);
                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_form_heading"));
                frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                break;
            }
            case P_ToolButton_text:
                if (const HbStyleOptionToolButton *opt = qstyleoption_cast<const HbStyleOptionToolButton*>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setText(opt->text);
                }
                break;
            case P_ProgressBar_text: 
                break;
            
            case P_DataGroup_background: {
                if (const HbStyleOptionDataGroupHeadingWidget *opt =
                    qstyleoption_cast<const HbStyleOptionDataGroupHeadingWidget *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                    if(!frameItem){
                        return;
                    }
                    QString frameGraphicsName;
                    QIcon::Mode mode = QIcon::Disabled;
                    if (opt->state & QStyle::State_Enabled) {
                        mode = QIcon::Normal;
                    }
                    if (opt->state & QStyle::State_Active) {
                        mode = QIcon::Active;
                    }
                    if (opt->state & QStyle::State_Selected) {
                        mode = QIcon::Selected;
                    }
                    frameItem->setZValue(-1.0);
                    if(mode == QIcon::Disabled) {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_list_disabled"));
                    }
                    else if(opt->pressed ) {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_list_pressed"));
                    } else {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_list_parent_normal"));
                    }
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                }
                break;
            }
            case P_DataGroupComboBackground: {
                if (const HbStyleOptionDataGroup *opt = qstyleoption_cast<const HbStyleOptionDataGroup *>(option)) {
                    QString frameGraphicsName;
                    QIcon::Mode mode = QIcon::Disabled;
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                    if(!frameItem){
                        return;
                    }
                    if (opt->state & QStyle::State_Enabled) {
                        mode = QIcon::Normal;
                    }
                    if (opt->state & QStyle::State_Active) {
                        mode = QIcon::Active;
                    }
                    if (opt->state & QStyle::State_Selected) {
                        mode = QIcon::Selected;
                    }
                    if (mode == QIcon::Disabled ) {
                            frameGraphicsName = QLatin1String("qtg_fr_list_disabled");
                        } else  {
                            frameGraphicsName = QLatin1String("qtg_fr_list_parent_normal");
                    }
                    frameItem->frameDrawer().setFrameGraphicsName(frameGraphicsName);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                }
                break;
            }

            case P_GroupBoxHeading_background: {
                if (const HbStyleOptionGroupBox *opt = qstyleoption_cast<const HbStyleOptionGroupBox *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );

                    if(!frameItem){
                        return;
                    }
                    QString frameGraphicsName;
                    if(opt->groupBoxType == HbStyleOptionGroupBox::GroupBoxSimpleLabel)
                        frameGraphicsName = QLatin1String("qtg_fr_groupbox");
                    else if(opt->groupBoxType == HbStyleOptionGroupBox::GroupBoxCollapsingContainer){
                        if (opt->state == QStyle::State_On) {
                            frameGraphicsName = QLatin1String("qtg_fr_list_pressed");
                        } else if (opt->state == QStyle::State_Off) {
                            frameGraphicsName = QLatin1String("qtg_fr_list_parent_normal");
                        } else {
                            frameGraphicsName = QLatin1String("qtg_fr_list_highlight");
                        }
                    }
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(frameGraphicsName);
                }
                break;
            }

            case P_GroupBoxContent_background: {
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );

                    if(!frameItem){
                        return;
                    }

                    QString frameGraphicsName;
                    if (opt->state == QStyle::State_On) {
                        frameGraphicsName = QLatin1String("qtg_fr_groupbox_pressed");
                    } else if (opt->state == QStyle::State_Off) {
                        frameGraphicsName = QLatin1String("qtg_fr_groupbox_normal");
                    } else {
                        frameGraphicsName = QLatin1String("qtg_fr_groupbox_highlight");
                    }
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(frameGraphicsName);
                }
                break;
            }

            case P_DataItem_background: {
                if (const HbStyleOptionDataFormViewItem *opt = qstyleoption_cast<const HbStyleOptionDataFormViewItem *>(option)) {
                    QString frameGraphicsName;
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                    if(!frameItem){
                        return;
                    }
                    QIcon::Mode mode = QIcon::Disabled;
                    if (opt->state & QStyle::State_Enabled) {
                        mode = QIcon::Normal;
                    }
                    if (opt->state & QStyle::State_Active) {
                        mode = QIcon::Active;
                    }
                    if (opt->state & QStyle::State_Selected) {
                        mode = QIcon::Selected;
                    }
                    if (mode == QIcon::Disabled ) {
                            frameGraphicsName = QLatin1String("qtg_fr_list_disabled");
                        } else  {
                            frameGraphicsName = QLatin1String("qtg_fr_list_normal");
                    }
                    frameItem->frameDrawer().setFrameGraphicsName(frameGraphicsName);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                }
                break;
            }
            case P_DataItem_label: {
                if (const HbStyleOptionDataFormViewItem *opt = qstyleoption_cast<const HbStyleOptionDataFormViewItem *>(option)) {
                    HbTextItem *textitem = static_cast<HbTextItem*>( item );
                    textitem->setText(opt->label);
                    textitem->setTextWrapping(Hb::TextNoWrap);
                }
                break;
            }
            case P_DataItem_description: {
                if (const HbStyleOptionDataFormViewItem *opt = qstyleoption_cast<const HbStyleOptionDataFormViewItem *>(option)) {
                    HbTextItem *textitem = static_cast<HbTextItem*>( item );
                    textitem->setText(opt->description);
                }
                break;
            }
            case P_PushButton_background: {
                if (const HbStyleOptionPushButton *opt =
                    qstyleoption_cast<const HbStyleOptionPushButton *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                    QString frameGraphicsName;
                    QIcon::Mode mode = QIcon::Disabled;
                    QIcon::State state = QIcon::Off;
                    if (opt->state & QStyle::State_Enabled)
                        mode = QIcon::Normal;
                    if (opt->state & QStyle::State_Active)
                        mode = QIcon::Active;
                    if (opt->state & QStyle::State_Selected)
                        mode = QIcon::Selected;
                    if (opt->state & QStyle::State_On)
                        state = QIcon::On;

                    frameItem->setZValue(-1.0);
                    if(opt->backgroundFrameDrawer &&!(opt->backgroundFrameDrawer->isNull()))
                    {
                        frameItem->setFrameDrawer(opt->backgroundFrameDrawer);
                    }
                    else if (!opt->background.isNull()) {
                        QString customName = opt->background.iconName(mode, state);
                        frameItem->frameDrawer().setFrameGraphicsName(customName);
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::OnePiece);
                        break;
                    }
                    else{
                        if (mode == QIcon::Disabled && state == QIcon::Off) {
                            frameGraphicsName = QLatin1String("qtg_fr_btn_disabled");
                        } else if (mode == QIcon::Normal && state == QIcon::On) {
                            if(!opt->isCheckable){
                                frameGraphicsName = QLatin1String("qtg_fr_btn_pressed");
                            } else {
                                frameGraphicsName = QLatin1String("qtg_fr_btn_latched");
                            }
                        } else if (mode == QIcon::Selected && state == QIcon::Off) {
                            frameGraphicsName = QLatin1String("qtg_fr_btn_highlight");
                        } else {
                            frameGraphicsName = QLatin1String("qtg_fr_btn_normal");
                        }
                    frameItem->frameDrawer().setFrameGraphicsName(frameGraphicsName);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    }
                }
                break;
            }

            case P_TitlePane_background:{
                const HbStyleOptionTitlePane *opt = qstyleoption_cast<const HbStyleOptionTitlePane *>(option);

                if (opt) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                    frameItem->frameDrawer().setFillWholeRect(true);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                    if (opt->mode == QIcon::Active) {
                        if (opt->transparent) {
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_titlebar_trans_pressed"));
                        } else {
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_titlebar_pressed"));
                        }
                    } else if (opt->mode == QIcon::Selected) {
                        if (opt->transparent) {
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_titlebar_trans_latched"));
                        } else {
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_titlebar_latched"));
                        }
                    } else {
                        if (opt->transparent) {
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_titlebar_trans_normal"));
                        } else {
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_titlebar_normal"));
                        }
                    }
                    frameItem->frameDrawer().setFileNameSuffixList(QStringList() << QLatin1String("_cl") << QLatin1String("_c") << QLatin1String("_cr"));
                }
                break;
            }
            case P_StatusBar_background: {
                const HbStyleOptionStatusBar *opt = qstyleoption_cast<const HbStyleOptionStatusBar *>(option);
                if (opt) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                    frameItem->frameDrawer().setFillWholeRect(true);
                    if (opt->transparent) {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_statusbar_trans"));
                    } else {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_statusbar"));
                    }
                }
                break;
            }
            case P_StatusBar_timetext: {
                if (const HbStyleOptionStatusBar *opt =
                    qstyleoption_cast<const HbStyleOptionStatusBar*>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setText(opt->timeText);
                    }
                break;
            }

            case P_IndicatorGroup_icon1:
            case P_IndicatorGroup_icon2:
            case P_IndicatorGroup_icon3:
            case P_IndicatorGroup_icon4:
                {
                const HbStyleOptionIndicatorGroup *opt = qstyleoption_cast<const HbStyleOptionIndicatorGroup *>(option);
                if (opt) {
                    HbIconItem *iconItem = qgraphicsitem_cast<HbIconItem*>(item);
                    iconItem->setIconName(opt->iconName);
                }
                break;
            }

            case P_IndicatorButton_handleindication: {
                const HbStyleOptionIndicatorButton *opt = qstyleoption_cast<const HbStyleOptionIndicatorButton *>(option);
                if (opt) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIconName("qtg_graf_titlebar_minimize");
                }
                break;
            }

            case P_IndicatorButton_eventindication: {
                const HbStyleOptionIndicatorButton *opt = qstyleoption_cast<const HbStyleOptionIndicatorButton *>(option);
                if (opt) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    if (opt->twoIcons) {
                        iconItem->setIconName("qtg_mono_new_event");
                        iconItem->setVisible(true);
                    } else {
                        iconItem->setVisible(false);
                    }
                }
                break;
            }

            case P_SignalIndicator_icon: {
                const HbStyleOptionSignalIndicator *opt = qstyleoption_cast<const HbStyleOptionSignalIndicator *>(option);
                if (opt) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    switch (opt->networkMode) {
                        case 0:
                            iconItem->setIconName("qtg_mono_offline");
                            break;
                        case 1:
                            iconItem->setIconName("qtg_status_signal");
                            break;
                        case 2:
                            iconItem->setIconName("qtg_status_wcdma");
                            break;
                        case 3:
                            iconItem->setIconName("qtg_status_wcdma");
                            break;
                        case 5:
                            iconItem->setIconName("qtg_mono_wlan");
                            break;
                        default:
                            iconItem->setIconName("qtg_mono_offline");
                            break;
                    }
                }
                break;
            }
            case P_SignalLevel_background: {
                const HbStyleOptionSignalIndicator *opt = qstyleoption_cast<const HbStyleOptionSignalIndicator *>(option);
                if (opt) {
                    HbIndicatorLevelIconItem *iconItem = static_cast<HbIndicatorLevelIconItem*>(item);
                    iconItem->setIconName("qtg_graf_signal_bg");
                    iconItem->setValue(100 - opt->signalValue);
                    iconItem->setDirection(HbIndicatorLevelIconItem::RightToLeft);
                    iconItem->update();
                }
                break;
            }
            case P_SignalLevel_icon: {
                const HbStyleOptionSignalIndicator *opt = qstyleoption_cast<const HbStyleOptionSignalIndicator *>(option);
                if (opt) {
                    HbIndicatorLevelIconItem *iconItem = static_cast<HbIndicatorLevelIconItem*>(item);
                    switch (opt->signalLevel) {
                        case HbStyleOptionSignalIndicator::Zero:
                            iconItem->setIconName("qtg_graf_signal_bg");
                            break;
                        case HbStyleOptionSignalIndicator::Low:
                            iconItem->setIconName("qtg_graf_signal_low");
                            break;
                        case HbStyleOptionSignalIndicator::Medium:
                            iconItem->setIconName("qtg_graf_signal_medium");
                            break;
                        case HbStyleOptionSignalIndicator::Full:
                            iconItem->setIconName("qtg_graf_signal_full");
                            break;
                        default:
                            iconItem->setIconName("qtg_graf_signal_bg");
                            break;
                    }
                    iconItem->setValue(opt->signalValue);
                    iconItem->setDirection(HbIndicatorLevelIconItem::LeftToRight);
                    iconItem->update();
                }
                break;
            }
            case P_BatteryIndicator_icon: {
                if (option) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIconName("qtg_status_battery");
                }
                break;
            }
            case P_BatteryLevel_background: {
                const HbStyleOptionBatteryIndicator *opt = qstyleoption_cast<const HbStyleOptionBatteryIndicator *>(option);
                if (opt) {
                    HbIndicatorLevelIconItem *iconItem = static_cast<HbIndicatorLevelIconItem*>(item);
                    iconItem->setIconName("qtg_graf_battery_bg");
                    iconItem->setValue(100 - opt->batteryValue);
                    iconItem->setDirection(HbIndicatorLevelIconItem::LeftToRight);
                    iconItem->update();
                }
                break;
            }
            case P_BatteryLevel_icon: {
                const HbStyleOptionBatteryIndicator *opt = qstyleoption_cast<const HbStyleOptionBatteryIndicator *>(option);
                if (opt) {
                    HbIndicatorLevelIconItem *iconItem = static_cast<HbIndicatorLevelIconItem*>(item);
                    switch (opt->batteryLevel) {
                        case HbStyleOptionBatteryIndicator::Zero:
                            iconItem->setIconName("qtg_graf_battery_bg");
                            break;
                        case HbStyleOptionBatteryIndicator::Low:
                            iconItem->setIconName("qtg_graf_battery_low");
                            break;
                        case HbStyleOptionBatteryIndicator::Medium:
                            iconItem->setIconName("qtg_graf_battery_medium");
                            break;
                        case HbStyleOptionBatteryIndicator::Full:
                            iconItem->setIconName("qtg_graf_battery_full");
                            break;
                        default:
                            iconItem->setIconName("qtg_graf_battery_bg");
                            break;
                    }
                    iconItem->setValue(opt->batteryValue);
                    iconItem->setDirection(HbIndicatorLevelIconItem::RightToLeft);
                    iconItem->update();
                }
                break;
            }

            case P_NavigationButton_background: {
                const HbStyleOptionNavigationButton *opt = qstyleoption_cast<const HbStyleOptionNavigationButton *>(option);
                    if (opt) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>( item );
                        iconItem->setAspectRatioMode(Qt::IgnoreAspectRatio);
                        if (opt->transparent) {
                            if (opt->mode == QIcon::Active) {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_pressed_r"));
                            } else {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_normal_r"));
                            }
                        } else {
                            if (opt->mode == QIcon::Active) {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_pressed_r"));
                            } else {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_normal_r"));
                            }
                        }
                    }
                break;
            }

            case P_IndicatorButton_background: {
                const HbStyleOptionIndicatorButton *opt = qstyleoption_cast<const HbStyleOptionIndicatorButton *>(option);
                if (opt) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>( item );
                    iconItem->setAspectRatioMode(Qt::IgnoreAspectRatio);
                    if (opt->mode == QIcon::Active) {
                        if (opt->transparent) {
                            iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_pressed_l"));
                        } else {
                            iconItem->setIconName(QLatin1String("qtg_fr_titlebar_pressed_l"));
                        }
                    } else if (opt->mode == QIcon::Selected) {
                        if (opt->transparent) {
                            iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_latched_l"));
                        } else {
                            iconItem->setIconName(QLatin1String("qtg_fr_titlebar_latched_l"));
                        }
                    } else {
                        if (opt->transparent) {
                            iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_normal_l"));
                        } else {
                            iconItem->setIconName(QLatin1String("qtg_fr_titlebar_normal_l"));
                        }
                    }
                    iconItem->update();
                }
                break;
            }

            case P_TitlePane_text: {
                const HbStyleOptionTitlePane *opt = qstyleoption_cast<const HbStyleOptionTitlePane *>(option);

                if (opt) {
                    HbMarqueeItem *marqueeItem = static_cast<HbMarqueeItem*>(item);
                    if (marqueeItem) {
                        marqueeItem->setText(opt->caption);
                        if (opt->margueeAnimation) {
                            marqueeItem->startAnimation();
                        }
                    }
                }
                break;
            }

            case P_TitlePane_icon: {
                const HbStyleOptionTitlePane *opt = qstyleoption_cast<const HbStyleOptionTitlePane *>(option);
                if (opt) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIconName("qtg_mono_options_menu");
                }
                break;
            }
            case P_GroupBoxMarquee_text: {
                if (const HbStyleOptionGroupBox *opt = qstyleoption_cast<const HbStyleOptionGroupBox *>(option)) {
                    HbMarqueeItem *marqueeItem = static_cast<HbMarqueeItem*>( item );
                    if(marqueeItem){
                        marqueeItem->setText(opt->heading);
                        if(opt->marqueeEnabled){
                            if(!marqueeItem->isAnimating()){
                                marqueeItem->setLoopCount(-1);
                                marqueeItem->startAnimation();
                            }
                        }else
                        {
                            marqueeItem->stopAnimation();
                        }
                    }
                }
                break;
            }
            case P_GroupBoxHeading_text: {
                if (const HbStyleOptionGroupBox *opt = qstyleoption_cast<const HbStyleOptionGroupBox *>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>( item );
                    if(textItem){
                        textItem->setText(opt->heading);
                    }
                }
                break;
            }
            case P_PushButton_focus: {
                if (const HbStyleOptionPushButton *opt =
                        qstyleoption_cast<const HbStyleOptionPushButton *>(option)) {
                     HbFrameItem *iconItem  = static_cast<HbFrameItem*>(item);
                     iconItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_btn_highlight"));
                     //int margin = 4;
                     //QRectF focusRect  = opt->rect.adjusted(-margin , -margin , margin , margin);
                     iconItem->setGeometry(opt->boundingRect);
                     iconItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                 }
                break;
            }
            case P_ToolButton_frame:{
                const HbStyleOptionToolButton *opt = qstyleoption_cast<const HbStyleOptionToolButton *>(option);
                HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                frameItem->frameDrawer().setFillWholeRect(true);
                if (opt->orientation == Qt::Vertical) {
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                } else {
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                }

                QStringList list;
                QString frameGraphicsFooter;
                QIcon::Mode mode = iconMode(opt->state);
                QIcon::State state = iconState(opt->state);

                if (!opt->customBackground.isNull()) {
                    QString customName = opt->customBackground.iconName(mode, state);
                    frameItem->frameDrawer().setFrameGraphicsName(customName);
                    break;
                }

                QString frameGraphicsName;
                if(opt->isToolBarExtension) {
                   if (mode == QIcon::Normal && state == QIcon::On) {
                        if(!opt->isCheckable){
                            frameGraphicsName = QLatin1String("qtg_fr_popup_grid_pressed");
                        } else {
                            frameGraphicsName = QLatin1String("qtg_fr_tb_ext");
                        }
                    }
                    frameItem->frameDrawer().setFrameGraphicsName(frameGraphicsName);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    return;
                }
                if (!opt->toolBarPosition) {
                    if (mode == QIcon::Disabled && state == QIcon::Off) {
                        frameGraphicsName = QLatin1String("qtg_fr_btn_disabled");
                    } else if (mode == QIcon::Normal && state == QIcon::On) {
                        if(!opt->isCheckable){
                            frameGraphicsName = QLatin1String("qtg_fr_btn_pressed");
                        } else {
                            frameGraphicsName = QLatin1String("qtg_fr_btn_latched");
                        }
                    } else if (mode == QIcon::Selected && state == QIcon::Off) {
                        frameGraphicsName = QLatin1String("qtg_fr_btn_highlight");
                    } else {
                        frameGraphicsName = QLatin1String("qtg_fr_btn_normal");
                    }
                    frameItem->frameDrawer().setFrameGraphicsName(frameGraphicsName);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    break;
                }
                // For toolbar:

                QString frameGraphicsHeader;
                if (!opt->useSecondaryGraphics){
                    if (opt->useTransparentGraphics) {
                        frameGraphicsHeader = opt->orientation == Qt::Vertical ?
                                              QLatin1String("qtg_fr_tb_trans_h_"):
                                              QLatin1String("qtg_fr_tb_trans_v_");
                    } else {
                        frameGraphicsHeader = opt->orientation == Qt::Vertical ?
                                              QLatin1String("qtg_fr_tb_h_"):
                                              QLatin1String("qtg_fr_tb_v_");
                    }
                }else {
                    frameGraphicsHeader = QLatin1String("qtg_fr_popup_sk_");
                    frameItem->frameDrawer().setMirroringMode(HbIcon::LayoutDirection);
                }

                switch (opt->toolBarPosition) {
                    case HbStyleOptionToolButton::TB_OnlyOne:
                        if (opt->orientation == Qt::Vertical) {
                            list << QLatin1String("_l") << QLatin1String("_c") << QLatin1String("_r");
                        } else {
                            list << QLatin1String("_t") << QLatin1String("_c") << QLatin1String("_b");
                        }
                        break;
                    case HbStyleOptionToolButton::TB_Beginning:
                        if (opt->orientation == Qt::Vertical) {
                            list << QLatin1String("_l") << QLatin1String("_c") << QLatin1String("_cr");
                        } else {
                            list << QLatin1String("_t") << QLatin1String("_c") << QLatin1String("_cb");
                        }
                        break;
                    case HbStyleOptionToolButton::TB_Middle:
                        if (opt->orientation == Qt::Vertical) {
                            list << QLatin1String("_cl") << QLatin1String("_c") << QLatin1String("_cr");
                        } else {
                            list << QLatin1String("_ct") << QLatin1String("_c") << QLatin1String("_cb");
                        }
                        break;
                    case HbStyleOptionToolButton::TB_End:
                        if (opt->orientation == Qt::Vertical) {
                            list << QLatin1String("_cl") << QLatin1String("_c") << QLatin1String("_r");
                        } else {
                            list << QLatin1String("_ct") << QLatin1String("_c") << QLatin1String("_b");
                        }
                        break;
                    default:

                        break;
                }
                frameItem->frameDrawer().setFileNameSuffixList(list);
                if (mode == QIcon::Disabled && state == QIcon::Off) {
                    frameGraphicsFooter = QLatin1String("disabled");
                } else if (mode == QIcon::Normal && state == QIcon::On) {
                    if(!opt->isCheckable) {
                        frameGraphicsFooter = QLatin1String("pressed");
                    } else {
                        frameGraphicsFooter = QLatin1String("latched");
                    }
                } else if (mode == QIcon::Selected && state == QIcon::Off) {
                    frameGraphicsFooter = QLatin1String("highlight");
                } else {
                    frameGraphicsFooter = QLatin1String("normal");
                }

                frameItem->frameDrawer().setFrameGraphicsName(QString ("%0%1").arg(frameGraphicsHeader).arg(frameGraphicsFooter));
                frameItem->frameDrawer().setMirroringMode(HbIcon::LayoutDirection);

                break;
            }



            case P_SliderTickMark_majoricon: {
                 if (const HbStyleOptionSlider *opt =
                        qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbIconItem *iconItem = qgraphicsitem_cast<HbIconItem*>(item);
                        iconItem->setMode(iconMode(opt->state));
                        if ( opt->orientation ==Qt::Horizontal ){
                            iconItem->setIconName( "qtg_graf_slider_h_tick_major" );
                        } else {
                            iconItem->setIconName ( "qtg_graf_slider_v_tick_major" );
                        }
                        iconItem->setState(iconState(opt->state));
                    }
                 break;

             }

            case P_SliderTickMark_minoricon: {
                 if (const HbStyleOptionSlider *opt =
                        qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbIconItem *iconItem = qgraphicsitem_cast<HbIconItem*>(item);
                        iconItem->setMode(iconMode(opt->state));
                        if ( opt->orientation ==Qt::Horizontal ){
                            iconItem->setIconName( "qtg_graf_slider_h_tick_minor" );
                        } else {
                            iconItem->setIconName ( "qtg_graf_slider_v_tick_minor" );
                        }
                        iconItem->setState(iconState(opt->state));
                    }
                 break;

             }

            case P_SliderTickMark_majorlabel: {
                 if (const HbStyleOptionSlider *opt =
                        qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbTextItem *textItem = qgraphicsitem_cast<HbTextItem*>(item);

                        textItem->setText( opt->text );

                    }
                 break;

             }

            case P_SliderTickMark_minorlabel: {
                 if (const HbStyleOptionSlider *opt =
                        qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbTextItem *textItem = qgraphicsitem_cast<HbTextItem*>(item);
                         textItem->setText( opt->text );
                    }
                 break;

             }

            case P_Slider_progressgroove: {
                if (const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider *>(option))  {
                    HbSliderTrackItem* frameItem = static_cast<HbSliderTrackItem*>(item);
                    if(!frameItem->isVisible()) {
                        break;
                    }
                    if(opt->orientation == Qt::Horizontal){
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_slider_h_filled"));
                        frameItem->setInverted(opt->upsideDown);
                    }
                    else{
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_slider_v_filled"));
                        frameItem->setInverted(!opt->upsideDown);
                        frameItem->frameDrawer().setMirroringMode(HbIcon::Prevented);
                    }
                    frameItem->frameDrawer().setFillWholeRect(true);
                    frameItem->setMaximum(opt->maximum);
                    frameItem->setMinimum(opt->minimum);
                    frameItem->setValue(opt->sliderValue);
                    frameItem->setOrientation(opt->orientation);
                    frameItem->setSpan( opt->span );
                    frameItem->setHandleRect(opt->handleRect);
                    frameItem->update();
                }
                break;
            }
            case P_Slider_groove:{

                if (const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                    HbFrameDrawer::FrameType type(HbFrameDrawer::Undefined);
                    QString name;

                    if (opt->orientation == Qt::Horizontal) {
                        type = HbFrameDrawer::ThreePiecesHorizontal;
                        if (opt->state & QStyle::State_Enabled) {
                            if(!opt->groovePressed){
                                name = QLatin1String("qtg_fr_slider_h_frame_normal");
                            }
                            else {
                                name = QLatin1String("qtg_fr_slider_h_frame_pressed");
                            }
                        } else {
                                name = QLatin1String("qtg_fr_slider_h_frame_disabled");
                        }
                    } else if (opt->orientation == Qt::Vertical) {
                        if (opt->state & QStyle::State_Enabled) {
                            if(!opt->groovePressed){
                                name = QLatin1String("qtg_fr_slider_v_frame_normal");
                            }
                            else {
                                name = QLatin1String("qtg_fr_slider_v_frame_pressed");
                            }
                        } else {
                                name = QLatin1String("qtg_fr_slider_v_frame_disabled");

                        }
                        type = HbFrameDrawer::ThreePiecesVertical;
                    }

                    frameItem->frameDrawer().setFrameGraphicsName(name);
                    frameItem->frameDrawer().setFrameType(type);
                    frameItem->frameDrawer().setFillWholeRect(true);                    
                }
                break;
            }

            case P_Slider_thumb: {
                 if (const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider *>(option)){
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    // Set pixmap name and bounding rect
                    QString thumbPath;
                    if(opt->thumbPath.isNull())
                    {
                        thumbPath=logicalName(HbStylePrivate::P_Slider_thumb, option);
                    }
                    else
                    {
                        thumbPath=opt->thumbPath;
                    }
                    iconItem->setAlignment(Qt::AlignCenter);
                    iconItem->setIconName(thumbPath);
                    iconItem->setAspectRatioMode(Qt::KeepAspectRatio);
                 }
                break;
            }
            case HbStylePrivate::P_SliderElement_increase:
                {
                    if (const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                        iconItem->setZValue(1.0);
                        iconItem->setMode(iconMode(opt->state));
                        iconItem->setState(iconState(opt->state));
                        if ( !opt->sliderElementIcon.isNull() ){
                            iconItem->setIcon(opt->sliderElementIcon);
                        } else if ( opt->sliderType == HbStyleOptionSlider::VolumeType) {
                            iconItem->setIconName(QLatin1String("qtg_mono_vol_up"));
                        } else if (opt->sliderType == HbStyleOptionSlider::ZoomType) {
                            iconItem->setIconName(QLatin1String("qtg_mono_zoom_in"));
                        }
                    }
                break;
                }
            case HbStylePrivate::P_SliderElement_decrease:
                {
                    if (const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                        iconItem->setZValue(1.0);
                        iconItem->setMode(iconMode(opt->state));
                        iconItem->setState(iconState(opt->state));
                        if ( !opt->sliderElementIcon.isNull() ){
                            iconItem->setIcon(opt->sliderElementIcon);
                        } else if ( opt->sliderType == HbStyleOptionSlider::VolumeType) {
                            iconItem->setIconName(QLatin1String("qtg_mono_vol_down"));
                        } else if (opt->sliderType == HbStyleOptionSlider::ZoomType) {
                            iconItem->setIconName(QLatin1String("qtg_mono_zoom_out"));
                        }
                    }
                break;
                }
            case HbStylePrivate::P_SliderElement_icon:
                {
                    if (const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                        iconItem->setZValue(1.0);
                        iconItem->setMode(iconMode(opt->state));
                        iconItem->setState(iconState(opt->state));
                        if ( !opt->sliderElementIcon.isNull() ){
                            iconItem->setIcon(opt->sliderElementIcon);
                        } else if ( opt->sliderType == HbStyleOptionSlider::VolumeType) {
                            if( opt->state & QStyle::State_Sunken) {
                                iconItem->setIconName(QLatin1String("qtg_mono_speaker_off"));
                            } else {
                                iconItem->setIconName(QLatin1String("qtg_mono_speaker"));
                            }
                        }
                    }
                break;
                }
            case P_SliderElement_text:
                {
                   if (const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        textItem->setZValue(1.0);
                        textItem->setText(opt->text);
                    }
                break;
                }


            case P_CheckBox_text:
                if (const HbStyleOptionCheckBox *opt = qstyleoption_cast<const HbStyleOptionCheckBox*>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setTextWrapping(Hb::TextWordWrap);
                    textItem->setText(opt->text);
                }
                break;

            case P_CheckBox_icon: {
                if (const HbStyleOptionCheckBox *opt = qstyleoption_cast<const HbStyleOptionCheckBox *>(option)){
                    HbIconItem *iconItem = static_cast<HbIconItem *>(item);
                    if (opt->state.testFlag(QStyle::State_Selected) ) {
                        if (opt->state.testFlag(QStyle::State_On)) {
                            iconItem->setState(QIcon::On);
                            iconItem->setIconName(QLatin1String("qtg_small_selected_highlight"));
                        } else if (opt->state.testFlag(QStyle::State_Off)) {
                            iconItem->setState(QIcon::Off);
                            iconItem->setIconName(QLatin1String("qtg_small_unselected_highlight"));
                        } else if (opt->state.testFlag(QStyle::State_NoChange)) {
                            iconItem->setState(QIcon::On);
                            iconItem->setIconName(QLatin1String("qtg_small_selected_partial"));
                        }
                    } else {
                        if (opt->state.testFlag(QStyle::State_On)) {
                            iconItem->setState(QIcon::On);
                            iconItem->setIconName(QLatin1String("qtg_small_selected"));
                        } else if (opt->state.testFlag(QStyle::State_Off)) {
                            iconItem->setState(QIcon::Off);
                            iconItem->setIconName(QLatin1String("qtg_small_unselected"));
                        } else if (opt->state.testFlag(QStyle::State_NoChange)) {
                            iconItem->setState(QIcon::On);
                            iconItem->setIconName(QLatin1String("qtg_small_selected_partial"));
                        }
                    }
                }
                break;
            }
            case P_ItemViewItem_radiobutton:
            case P_ItemViewItem_checkbox:
            case P_ItemViewItem_selection:{
                Q_ASSERT_X(false, "HbStyle" , "P_ItemViewItem_selection no more supported. Use refactored styling");
                break;
            }

            case P_LineEdit_frame_normal:{
                if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                    if(frameItem->isEnabled()) {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_lineedit_normal"));
                    } else {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_lineedit_disabled"));
                    }
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                }
            break;
            }

            case P_LineEdit_frame_highlight:{
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_lineedit_highlight"));
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->setGeometry(opt->boundingRect);
                    }
                }
            break;
            }

            case P_TextEdit_frame_normal:{
                if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                    if(frameItem->isEnabled()) {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_textedit_normal"));
                    } else {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_textedit_disabled"));
                    }
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                }
            break;
            }

            case P_TextEdit_frame_highlight:{
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_textedit_highlight"));
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->setGeometry(opt->boundingRect);
                    }
                }
            break;
            }

            case P_ComboBox_background:{
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                    QString frameGraphicsName;
                    if (opt->state & QStyle::State_Enabled) {
                        if (opt->state & QStyle::State_Sunken) {
                            frameGraphicsName = QLatin1String("qtg_fr_combobox_pressed");
                        } else {
                            frameGraphicsName = QLatin1String("qtg_fr_combobox_normal");
                        }
                    } else {
                        frameGraphicsName = QLatin1String("qtg_fr_combobox_disabled");
                    }
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                    frameItem->frameDrawer().setFillWholeRect(true);
                    frameItem->frameDrawer().setFrameGraphicsName(frameGraphicsName);
                    frameItem->frameDrawer().setMirroringMode ( HbIcon::LayoutDirection );

                }
            break;
            }
            case P_ComboBox_button:{
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>( item );
                    QString iconGraphicsName;
                    if (opt->state & QStyle::State_Enabled) {
                        if (opt->state & QStyle::State_Sunken) {
                            iconGraphicsName = QLatin1String("qtg_graf_combobox_button_pressed");
                        } else {
                            iconGraphicsName = QLatin1String("qtg_graf_combobox_button_normal");
                        }
                    } else {
                        iconGraphicsName = QLatin1String("qtg_graf_combobox_button_disabled");
                    }
                    iconItem->setIconName(iconGraphicsName);
                    iconItem->setMirroringMode ( HbIcon::LayoutDirection );
                    iconItem->setAspectRatioMode( Qt::IgnoreAspectRatio);
                }
            break;
            }

            case P_Edit_text:{
                break;
            }

            case P_Label_text:
                 if (const HbStyleOptionLabel *opt = qstyleoption_cast<const HbStyleOptionLabel*>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setAlignment(opt->alignment);
                    textItem->setElideMode(opt->elideMode);
                    textItem->setTextWrapping(opt->textWrapMode);
                    if(!opt->fontSpec.isNull()) {
                        textItem->setFontSpec(opt->fontSpec);
                    }
                    if (opt->color.isValid()) {
                        textItem->setTextColor(opt->color);
                    }
                    textItem->setText(opt->text);
                    textItem->setMaximumLines(opt->maximumLines);
                 }
                 break;
            case P_Label_richtext:
                if (const HbStyleOptionLabel *opt = qstyleoption_cast<const HbStyleOptionLabel*>(option)) {
                    HbRichTextItem *textItem = static_cast<HbRichTextItem*>(item);
                    textItem->setAlignment(opt->alignment);
                    textItem->setTextWrapping(opt->textWrapMode);
                    if(!opt->fontSpec.isNull()) {
                        textItem->setFontSpec(opt->fontSpec);
                    }
                    if (opt->color.isValid()) {
                        textItem->setTextDefaultColor(opt->color);
                    }
                    textItem->setText(opt->text);
                }
                break;
            case P_Popup_heading_frame: {
                if (const HbStyleOptionPopup *opt = qstyleoption_cast<const HbStyleOptionPopup *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                    frameItem->setGeometry(opt->boundingRect);
                    if (opt->headingMode == QIcon::Active) {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_heading_pressed"));
                    } else if (opt->headingMode == QIcon::Selected) {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_heading_highlight"));
                    } else {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_heading_normal"));
                    }
                }
                break;
            }

            case P_ComboBoxPopup_background: {         
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    if(!frameItem){
                           return;
                    }
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_secondary"));
                }
                break;

            case P_NotificationDialog_frame:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_preview"));
                    frameItem->setGeometry(opt->boundingRect);
                }
                break;

            case P_Fade_background:
               if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>( item );
                    iconItem->setIconName(QLatin1String("qtg_graf_dimming_image"));
                    iconItem->setAspectRatioMode (Qt::IgnoreAspectRatio);
                    iconItem->setGeometry(opt->boundingRect);
                }
                break;

            case P_NotificationDialog_text:
                if (const HbStyleOptionNotificationDialog *opt =
                        qstyleoption_cast<const HbStyleOptionNotificationDialog *>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setAlignment(opt->textAlignment);
                    textItem->setFontSpec(HbFontSpec(HbFontSpec::Secondary));
                    textItem->setTextWrapping(opt->textTextWrapping);
                    textItem->setText(opt->text);
                }
                break;
            case P_NotificationDialog_title:
                if (const HbStyleOptionNotificationDialog *opt =
                        qstyleoption_cast<const HbStyleOptionNotificationDialog *>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setAlignment(opt->titleAlignment);
                    textItem->setFontSpec(HbFontSpec(HbFontSpec::Primary));
                    textItem->setTextWrapping(opt->titleTextWrapping);
                    if (opt->titleTextWrapping == Hb::TextNoWrap) {
                        textItem->setElideMode(Qt::ElideNone);
                    } else {
                        textItem->setElideMode(Qt::ElideRight);
                    }
                    textItem->setText(opt->title);
                }
                break;
            case P_NotificationDialog_icon:
                if (const HbStyleOptionNotificationDialog *opt =
                        qstyleoption_cast<const HbStyleOptionNotificationDialog *>(option)) {
                    HbIconItem *icon = static_cast<HbIconItem*>(item);

                    icon->setIcon(opt->icon);
                    icon->setAlignment(opt->iconAlignment);
                }
                break;

            case P_ProgressDialog_icon:
                if (const HbStyleOptionProgressDialog *opt = static_cast<const HbStyleOptionProgressDialog *>(option)) {
                    HbIconItem *icon = static_cast<HbIconItem*>(item);
                    icon->setIcon(opt->icon);
                }
                break;

            case P_MessageBox_text:    //deprecated
                if (const HbStyleOptionMessageBox *opt = static_cast<const HbStyleOptionMessageBox *>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setTextWrapping(Hb::TextWordWrap);
                    textItem->setText(opt->text);
                }
                break;
            case P_MessageBox_icon:
                if (const HbStyleOptionMessageBox *opt = static_cast<const HbStyleOptionMessageBox *>(option)) {
                    HbIconItem *icon = static_cast<HbIconItem*>(item);
                    HbIcon noteIcon = opt->icon;
                    if(noteIcon.isNull()) {
                        switch (opt->messageBoxType) {
                        case HbStyleOptionMessageBox::MessageTypeInformation:
                            icon->setIcon(HbIcon(QLatin1String("qtg_large_info")));
                            break;
                        case HbStyleOptionMessageBox::MessageTypeQuestion:
                            icon->setIcon(HbIcon(QLatin1String("qtg_large_help")));
                            break;
                        case HbStyleOptionMessageBox::MessageTypeWarning:
                            icon->setIcon(HbIcon(QLatin1String("qtg_large_warning")));
                            break;
                        default:
                            break;
                        }
                    } else {
                        icon->setIcon(opt->icon);
                    }
                }
                break;
            case P_ListViewItem_text:
                Q_ASSERT_X(false, "HbStyle" , "P_ListViewItem_text no more supported. Use refactored styling");
                break;
            case P_ListViewItem_richtext:
                Q_ASSERT_X(false, "HbStyle" , "P_ListViewItem_richtext no more supported. Use refactored styling");
                break;
            case P_ListViewItem_icon:
                Q_ASSERT_X(false, "HbStyle" , "P_ListViewItem_icon no more supported. Use refactored styling");
                break;
            case P_GridViewItem_icon:
                Q_ASSERT_X(false, "HbStyle" , "P_GridViewItem_icon no more supported. Use refactored styling");
                break;

            case P_GridViewItem_text:
                Q_ASSERT_X(false, "HbStyle" , "P_GridViewItem_text no more supported. Use refactored styling");
                break;

            case P_ItemHighlight_background:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->setZValue(-2.0);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_list_highlight"));
                    frameItem->setGeometry(opt->boundingRect);
                }
                break;
            case P_SliderPopup_background:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    Q_UNUSED(opt)
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_trans"));
                }
                break;

            case P_ProgressBar_frame:
                break;
            case P_ProgressBar_track:
                break;
            case P_ProgressBar_waittrack:
                break;
            case P_ProgressSlider_frame:
                break;
            case P_ProgressSlider_track: 
                break;
            case P_ProgressSlider_slidertrack: 
                break;
            case P_ProgressBar_mintext:
                break;
            case P_ProgressBar_maxtext: 
                break;
            case P_RatingSlider_frame:
                break;
            case P_RatingSlider_track:
                break;
            case P_ProgressSliderHandle_icon:
                break;

            case P_QueryInputMode_image:

             /*   if (const HbStyleOptionInputPopup *opt = qstyleoption_cast<const HbStyleOptionInputPopup *>(option)){
                    if (HbIconItem *iconItem = static_cast<HbIconItem*>(item)) {
                       iconItem->setIcon(*opt->modeIcon);
                       iconItem->setPos(opt->editRect.width() - opt->modeIcon->width() - 6,opt->editRect.top()-3);
                    }
                }*/
                break;
            case P_ItemViewItem_background:
                Q_ASSERT_X(false, "HbStyle" , "P_ItemViewItem_background no more supported. Use refactored styling");
                break;
            case P_ItemViewItem_frame:
                Q_ASSERT_X(false, "HbStyle" , "P_ItemViewItem_frame no more supported. Use refactored styling");
                break;
            case P_ItemViewItem_focus:
                Q_ASSERT_X(false, "HbStyle" , "P_ItemViewItem_focus no more supported. Use refactored styling");
                break;
            case P_TreeViewItem_expandicon:
                Q_ASSERT_X(false, "HbStyle" , "P_TreeViewItem_expandicon no more supported. Use refactored styling");
                break;
            case P_TumbleView_background:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption*>(option)) {
                    if(HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {

                        //Temporary source to avoid introducing new style primitive for selection dialog mark widget background item.
                        QGraphicsItem *parent = frameItem->parentItem();
                        const QMetaObject *obj = parent->toGraphicsObject()->metaObject();

                        QString className;
                        if(obj){
                            className = obj->className();
                        }
                        ///////////////////////////////////////////////////////////

                        if( !className.compare("HbTumbleView") ){
                            frameItem->frameDrawer().setFrameGraphicsName("qtg_fr_tumbler_bg");
                        }
                        else if( !className.compare("HbSelectionDialogMarkWidget") )
                        {
                            frameItem->frameDrawer().setFrameGraphicsName("qtg_fr_groupbox");
                        }
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->setZValue(-5);
                        //TODO:temp fix, issue with css rule picking in derived class
                        //frameItem->setGeometry(opt->boundingRect);
                        Q_UNUSED(opt);
                    }
                }
                break;
            case P_TumbleView_frame:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption*>(option)) {
                    if(HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {
                        frameItem->frameDrawer().setFrameGraphicsName("qtg_fr_tumbler_overlay");
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->setZValue(-4);
                        //TODO:temp fix, issue with css rule picking in derived class
                        //frameItem->setGeometry(opt->boundingRect);
                        Q_UNUSED(opt);
                    }
                }
                break;

            case P_TumbleView_highlight:
               /* if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption*>(option)) {
                    if(HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {
                        frameItem->frameDrawer().setFrameGraphicsName("qtg_fr_tumbler_highlight_pri");
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                        frameItem->setZValue(-1);
                        //TODO:temp fix, issue with css rule picking in derived class

                        //frameItem->setGeometry(0,(opt->boundingRect.height()-frameItem->boundingRect().height())/2,opt->boundingRect.width(),opt->boundingRect.height());
                        Q_UNUSED(opt);
                    }

                }*/
                break;

            case P_IndexFeedback_popup_text:
                Q_ASSERT_X(false, "HbStyle" , "P_IndexFeedback_popup_text no more supported. Use refactored styling");
                break;
            case P_IndexFeedback_popup_background:
                Q_ASSERT_X(false, "HbStyle" , "P_IndexFeedback_popup_background no more supported. Use refactored styling");
                break;
            case P_DateTimePicker_background:
               /* if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption*>(option)) {
                    if(HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {
                        frameItem->frameDrawer().setFrameGraphicsName("qtg_fr_tumbler_bg");
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->setZValue(-5);
                        //TODO:temp fix, issue with css rule picking in derived class
                        //frameItem->setGeometry(opt->boundingRect);
                        Q_UNUSED(opt);
                    }
                }*/
                break;
            case P_DateTimePicker_frame:
               /* if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption*>(option)) {
                    if(HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {
                        frameItem->frameDrawer().setFrameGraphicsName("qtg_fr_tumbler_overlay");
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->setZValue(1);
                        //TODO:temp fix, issue with css rule picking in derived class
                        //frameItem->setGeometry(opt->boundingRect);
                        Q_UNUSED(opt);
                    }
                }*/
                break;
            case P_DateTimePicker_separator:
               /* if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption*>(option)) {
                    if(HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {
                        frameItem->frameDrawer().setFrameGraphicsName("qtg_graf_tumbler_divider");
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::OnePiece);
                        frameItem->setZValue(2);
                        //TODO:temp fix, issue with css rule picking in derived class

                        //frameItem->setGeometry(0,(opt->boundingRect.height()-frameItem->boundingRect().height())/2,opt->boundingRect.width(),opt->boundingRect.height());
                        Q_UNUSED(opt);
                    }

                }*/
                break;
            case P_InputDialog_text:
                if (const HbStyleOptionInputDialog *opt =
                    qstyleoption_cast<const HbStyleOptionInputDialog*>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    if(!item) {
                        return;
                    }
                    textItem->setText(opt->text);
                }
                break;
             case P_InputDialog_additionaltext:
                if (const HbStyleOptionInputDialog *opt =
                    qstyleoption_cast<const HbStyleOptionInputDialog*>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    if(!item) {
                        return;
                    }
                    textItem->setText(opt->additionalText);
                }
                break;

             default:
                return;
    };
}

/*!
  From qstylesheetstyle.cpp (declarations(...))
  \internal
*/
static HbVector<HbCss::Declaration> declarations(
    const HbVector<HbCss::StyleRule> &styleRules,
    const QString &part,
    HbWidget* widget,
    const HbDeviceProfile &profile)
{
    quint64 pseudoClass(0);

    if ( profile.orientation() == Qt::Horizontal ) {
        pseudoClass |= HbCss::PseudoClass_Landscape;
    } else {
        pseudoClass |= HbCss::PseudoClass_Portrait;
    }

    HbMainWindow *mainWindow = 0;
    if (widget) {
        mainWindow = widget->mainWindow();
    } else {
        QList<HbMainWindow *> mainWindows = hbInstance->allMainWindows();
        if (!mainWindows.isEmpty()) {
            mainWindow = mainWindows.at(0);
        }
    }
    if (mainWindow && mainWindow->isRightToLeft()) {
        pseudoClass |= HbCss::PseudoClass_RightToLeft;
    } else {
        pseudoClass |= HbCss::PseudoClass_LeftToRight;
    }

    HbVector<HbCss::Declaration> decls;
    for (int i = 0; i < styleRules.count(); i++) {
        const HbCss::Selector& selector = styleRules.at(i).selectors.at(0);
        // Rules with pseudo elements don't cascade. This is an intentional
        // diversion for CSS
        if (part.compare(selector.pseudoElement(), Qt::CaseInsensitive) != 0)
            continue;
        quint64 negated = 0;
        quint64 cssClass = selector.pseudoClass(&negated);
        if ((pseudoClass == HbCss::PseudoClass_Any) || (cssClass == HbCss::PseudoClass_Unspecified)
            || ((((cssClass & pseudoClass) == cssClass)) && ((negated & pseudoClass) == 0)))
            decls += styleRules.at(i).declarations;
    }
    return decls;
}

/*!
  used by the polish() method
  \internal
*/
void HbStylePrivate::polishItem(
    const HbVector<HbCss::StyleRule> &styleRules,
    HbWidget *widget,
    QGraphicsItem *item,
    const QString &name,
    HbDeviceProfile &profile,
    bool layoutDefined) const
{
    if (name.isEmpty() && widget != item) {
        return;
    }

#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polish : -- --";
    if ( widget == item ) {
        qDebug() << "HbStyle::polishItem : -- item name: " << widget->metaObject()->className();
    } else if ( item ) {
        qDebug() << "HbStyle::polishItem : -- item name: " << name << "(existing item)";
    } else {
        qDebug() << "HbStyle::polishItem : -- item name: " << name << "(missing item)";
    }
#endif

    const HbVector<HbCss::Declaration> decl = declarations(styleRules, name, widget, profile);
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polishItem : -- Number of matching CSS declarations: " << decl.count();
#endif
    HbCss::ValueExtractor extractor(decl, profile);
    extractor.setLayoutParameters(layoutParameters);
    HbCss::KnownProperties prop;

    if ( !extractor.extractKnownProperties(prop) ) {
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polishItem : -- No polish overrides found";
#endif
        return;
    }

    if ( item ) {
        if (prop.mFlags & HbCss::ExtractedZValue) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting zvalue: " << prop.mZ;
#endif
            item->setZValue(prop.mZ);
        }
    }

    QGraphicsWidget *gWidget = (item && item->isWidget())
        ? static_cast<QGraphicsWidget*>(item)
        : 0;
    if ( gWidget ) {
        if (prop.mFlags & HbCss::ExtractedLayoutDir) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting layout direction: " << prop.mLayoutDir;
#endif
            if (prop.mLayoutDir == HbCss::LayoutDirection_LeftToRight) {
                gWidget->setLayoutDirection(Qt::LeftToRight);
            } else if (prop.mLayoutDir == HbCss::LayoutDirection_RightToLeft) {
                gWidget->setLayoutDirection(Qt::RightToLeft);
            } else {
                gWidget->unsetLayoutDirection();
            }
        }
    }

    HbWidgetBase *hbWidget = qobject_cast<HbWidgetBase*>(gWidget);
    if ( hbWidget ) {
        HbWidgetBasePrivate* hbWidget_p = HbWidgetBasePrivate::d_ptr(hbWidget);
        if ( prop.mFlags & HbCss::ExtractedFontSpec ) {
            if ( !(prop.mFlags & HbCss::ExtractedFont) ) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting fontspec: " << prop.mFontSpec.role() << prop.mFontSpec.textHeight();
#endif
                hbWidget->setFontSpec(prop.mFontSpec);
            } else {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting fontspec with overrides: "
                         << prop.mFontSpec.role() << prop.mFontSpec.textHeight() << prop.mFont;
#endif
                hbWidget->setFont(prop.mFontSpec.font().resolve(prop.mFont));
            }
        } else if ( prop.mFlags & HbCss::ExtractedFont ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting explicit font: " << prop.mFont;
#endif
            hbWidget->setFont(prop.mFont);
        }

        HbIconItem* icon = qobject_cast<HbIconItem*>(hbWidget);
        if (icon) {
            if (prop.mFlags & HbCss::ExtractedAspectRatioMode
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_IconAspectRatioMode)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting aspect ratio mode: " << prop.mAspectRatioMode;
#endif
                icon->setAspectRatioMode(prop.mAspectRatioMode);
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_IconAspectRatioMode, false);
            }
            if ( prop.mFlags & HbCss::ExtractedAlignment
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_IconAlign)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting icon alignment: " << prop.mAlignment;
#endif
                icon->setAlignment( prop.mAlignment );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_IconAlign, false);
            }
        }

        HbTextItem* text = qobject_cast<HbTextItem*>(hbWidget);
        if (text) {
            if ( prop.mFlags & HbCss::ExtractedMinLines
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting text min lines: " << prop.mMinLines;
#endif
                text->setMinimumLines( prop.mMinLines );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin, false);
            }
            if ( prop.mFlags & HbCss::ExtractedMaxLines
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMax)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting text max lines: " << prop.mMaxLines;
#endif
                text->setMaximumLines( prop.mMaxLines );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMax, false);
            }
            if ( prop.mFlags & HbCss::ExtractedAlignment
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextAlign)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting text alignment: " << prop.mAlignment;
#endif
                text->setAlignment( prop.mAlignment );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextAlign, false);
            }
            if ( prop.mFlags & HbCss::ExtractedWrapMode
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextWrapMode)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting wrap mode : " << prop.mTextWrapMode;
#endif
                text->setTextWrapping( prop.mTextWrapMode );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextWrapMode, false);
            }
            if ( prop.mFlags & HbCss::ExtractedElideMode
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextElideMode)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting elide mode : " << prop.mTextElideMode;
#endif
                text->setElideMode( prop.mTextElideMode );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextElideMode, false);
            }
        }

        HbRichTextItem* richtext = qobject_cast<HbRichTextItem*>(hbWidget);
        if (richtext) {
            if ( prop.mFlags & HbCss::ExtractedAlignment
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextAlign)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting text alignment: " << prop.mAlignment;
#endif
                richtext->setAlignment( prop.mAlignment );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextAlign, false);
            }
            if ( prop.mFlags & HbCss::ExtractedWrapMode
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextWrapMode)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting wrap mode : " << prop.mTextWrapMode;
#endif
                richtext->setTextWrapping( prop.mTextWrapMode );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextWrapMode, false);
            }
        }

        HbFrameItem *frame = qobject_cast<HbFrameItem*>(hbWidget);
        if (frame && !frame->frameDrawer().d->testBorderApiProtectionFlag()) {
            if (prop.mFlags & HbCss::ExtractedBorderWidths) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting border widths (l,t,r,b):"
                    << prop.mBorderWidths[HbCss::LeftEdge]
                    << prop.mBorderWidths[HbCss::TopEdge]
                    << prop.mBorderWidths[HbCss::RightEdge]
                    << prop.mBorderWidths[HbCss::BottomEdge];
#endif
                frame->frameDrawer().setBorderWidths(
                    prop.mBorderWidths[HbCss::LeftEdge],
                    prop.mBorderWidths[HbCss::TopEdge],
                    prop.mBorderWidths[HbCss::RightEdge],
                    prop.mBorderWidths[HbCss::BottomEdge]);
                frame->frameDrawer().d->setBorderApiProtectionFlag(false);
            }
        }
        if ( hbWidget->inherits( "HbLineEdit" ) ) {
            if ( prop.mFlags & HbCss::ExtractedMinLines
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting text min lines: " << prop.mMinLines;
#endif
                hbWidget->setProperty( "minRows", prop.mMinLines );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin, false);
            }
            if ( prop.mFlags & HbCss::ExtractedMaxLines
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting text max lines: " << prop.mMaxLines;
#endif
                hbWidget->setProperty( "maxRows", prop.mMaxLines );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMax, false);
            }
        }
    }

    QGraphicsLayoutItem* lItem = (item && item->isWidget())
        ? (QGraphicsLayoutItem*)static_cast<QGraphicsWidget*>(item)
        : 0;
    if ( lItem ) {
        if ( prop.mFlags & HbCss::ExtractedMinW ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting minimum width: " << prop.mMinW;
#endif
            lItem->setMinimumWidth( prop.mMinW );
        }
        if ( prop.mFlags & HbCss::ExtractedMinH ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting minimum height: " << prop.mMinH;
#endif
            lItem->setMinimumHeight( prop.mMinH );
        }
        if ( prop.mFlags & HbCss::ExtractedPrefW ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting preferred width: " << prop.mPrefW;
#endif
            lItem->setPreferredWidth( prop.mPrefW );
        }
        if ( prop.mFlags & HbCss::ExtractedPrefH ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting preferred height: " << prop.mPrefH;
#endif
            lItem->setPreferredHeight( prop.mPrefH );
        }
        if ( prop.mFlags & HbCss::ExtractedMaxW ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting maximum width: " << prop.mMaxW;
#endif
            lItem->setMaximumWidth( prop.mMaxW );
        }
        if ( prop.mFlags & HbCss::ExtractedMaxH ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting maximum height: " << prop.mMaxH;
#endif
            lItem->setMaximumHeight( prop.mMaxH );
        }
        QSizePolicy itemPol = lItem->sizePolicy();
        if ( prop.mFlags & HbCss::ExtractedPolHor ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting horizontal size policy: " << prop.mSizePolicy.horizontalPolicy();
#endif
            itemPol.setHorizontalPolicy(prop.mSizePolicy.horizontalPolicy());
        }
        if ( prop.mFlags & HbCss::ExtractedPolVer ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting vertical size policy: " << prop.mSizePolicy.verticalPolicy();
#endif
            itemPol.setVerticalPolicy(prop.mSizePolicy.verticalPolicy());
        }
        lItem->setSizePolicy(itemPol);
    }

    if (layoutDefined) {
        HbAnchorLayout *layout = static_cast<HbAnchorLayout*>(widget->layout());
        if ( layout ) {
            if (prop.mFlags & HbCss::ExtractedLeft) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting left override: " << prop.mLeft;
#endif
                overrideSpacing(layout, name, Hb::LeftEdge, prop.mLeft);
            }
            if (prop.mFlags & HbCss::ExtractedRight) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting right override: " << prop.mRight;
#endif
                overrideSpacing(layout, name, Hb::RightEdge, prop.mRight);
            }
            if (prop.mFlags & HbCss::ExtractedTop) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting top override: " << prop.mTop;
#endif
                overrideSpacing(layout, name, Hb::TopEdge, prop.mTop);
            }
            if (prop.mFlags & HbCss::ExtractedBottom) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting bottom override: " << prop.mBottom;
#endif
                overrideSpacing(layout, name, Hb::BottomEdge, prop.mBottom);
            }
            if (prop.mFlags & HbCss::ExtractedCenterH) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting centerh override: " << prop.mCenterH;
#endif
                overrideSpacing(layout, name, Hb::CenterHEdge, prop.mCenterH);
            }
            if (prop.mFlags & HbCss::ExtractedCenterV) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting centerv override: " << prop.mCenterV;
#endif
                overrideSpacing(layout, name, Hb::CenterVEdge, prop.mCenterV);
            }
        }
    }
}

void HbStylePrivate::polishAnchor(
    const HbVector<HbCss::StyleRule> &styleRules,
    HbWidget *widget,
    HbAnchor *anchor,
    HbDeviceProfile &profile) const
{
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polish : -- --";
    qDebug() << "HbStyle::polishAnchor : -- anchor id: " << anchor->anchorId();
#endif

    const HbVector<HbCss::Declaration> decl = declarations(styleRules, anchor->anchorId(), widget, profile);
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polishAnchor : -- Number of matching CSS declarations: " << decl.count();
#endif
    HbCss::ValueExtractor extractor(decl, profile);
    extractor.setLayoutParameters(layoutParameters);
    HbCss::KnownProperties prop;

    if ( !extractor.extractKnownProperties(prop) ) {
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polishAnchor : -- No polish overrides found";
#endif
        return;
    }


    if ( prop.mFlags & HbCss::ExtractedMinH || prop.mFlags & HbCss::ExtractedMinW ) {
        qreal minLength = prop.mFlags & HbCss::ExtractedMinH ? prop.mMinH : prop.mMinW;
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polishAnchor : -- Setting minimum length: " << minLength;
#endif
        anchor->setMinimumLength( minLength );
    }
    if ( prop.mFlags & HbCss::ExtractedPrefH || prop.mFlags & HbCss::ExtractedPrefW ) {
        qreal prefLength = prop.mFlags & HbCss::ExtractedPrefH ? prop.mPrefH : prop.mPrefW;
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polishAnchor : -- Setting preferred length: " << prefLength;
#endif
        anchor->setPreferredLength( prefLength );
    }
    if ( prop.mFlags & HbCss::ExtractedMaxH || prop.mFlags & HbCss::ExtractedMaxW ) {
        qreal maxLength = prop.mFlags & HbCss::ExtractedMaxH ? prop.mMaxH : prop.mMaxW;
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polishAnchor : -- Setting maximum length: " << maxLength;
#endif
        anchor->setMaximumLength( maxLength );
    }
    if ( prop.mFlags & HbCss::ExtractedPolVer || prop.mFlags & HbCss::ExtractedPolHor ) {
        QSizePolicy::Policy policy = prop.mFlags & HbCss::ExtractedPolVer
            ? prop.mSizePolicy.verticalPolicy() : prop.mSizePolicy.horizontalPolicy();
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polishAnchor : -- Setting size policy: " << policy;
#endif
        anchor->setSizePolicy( policy );
    }
    if ( prop.mFlags & HbCss::ExtractedAnchorDir ) {
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polishAnchor : -- Setting anchor direction: " << prop.mAnchorDir;
#endif
        anchor->setDirection( prop.mAnchorDir );
    }
}

/*!
    Polishes the appearance of the given \a widget.

    This method should be called only by HbWidget (base class). The inherited
    classes should not call this method directly, but use the HbWidget's
    polish loop instead.

    This method reads the CSS and WidgetML definition for a given widget and positions
    the sub-elements inside it. Note you need to set the names for the sub-elements
    with HbStyle::HbStyle::setItemName method before this method is called.

    Note that this method is heavy on the system resources and should be called
    absolutely when necessary by the widget.

    \param widget HbWidget to be polished
    \param params style parameters to be returned to the caller
*/
void HbStyle::polish(HbWidget *widget, HbStyleParameters &params)
{
    Q_D( HbStyle );
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polish : -- -- -- -- ENTER";
#endif
    if( !widget ) {
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polish : -- -- -- -- EXIT";
#endif
        return;
    }
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polish : Polishing" << widget->metaObject()->className();
#endif

    HbLayeredStyleLoader *styleLoader = HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Layouts);
    if(!styleLoader){
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polish : HbLayeredStyleLoader returned a null pointer.";
        qDebug() << "HbStyle::polish : -- -- -- -- EXIT";
#endif
        return;
    }
    HbDeviceProfile profile(HbDeviceProfile::profile(widget));
    HbCss::StyleSelector::NodePtr n(widget);

    HbVector<HbCss::StyleRule> styleRules = styleLoader->styleRulesForNode(n, profile.orientation());

#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polish : Number of style rules:" << styleRules.count();
#endif
    if (!styleRules.count()) {
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polish : -- -- -- -- EXIT";
#endif
        return;
    }
    const HbVector<HbCss::Declaration> decl = declarations(styleRules, "", widget, profile);
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polish : Number of matching CSS declarations: " << decl.count();
#endif
    d->layoutParameters.init(profile);

    HbCss::ValueExtractor extractor(decl, profile);
    extractor.setLayoutParameters(d->layoutParameters);
    QString layoutName;
    QString sectionName;

    if ( params.count() ) {
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polish : Extracting custom properties.";
#endif
        extractor.extractCustomProperties( params.keys(), params.values() );
    }

    bool layoutDefined = extractor.extractLayout(layoutName, sectionName);
#ifdef HBSTYLE_DEBUG
    if (!layoutDefined) {
        qDebug() << "HbStyle::polish : Couldn't find layout name for the widget.";
    } else {
        qDebug() << "HbStyle::polish : Layout name: " << layoutName;
    }
#endif

    QStringList nodeIds;
    HbAnchorLayout *anchorLayout(0);
    if (layoutDefined) {

        // check that we do not override the user defined layout
        const QVariant layoutPtrByStyleV = widget->property( LAYOUT_PTR_PROPERTY );
        const QGraphicsLayout *layoutPtrByStyle = layoutPtrByStyleV.isValid() ? layoutPtrByStyleV.value<QGraphicsLayout*>() : 0;
        const QGraphicsLayout *existingLayoutPtr = widget->layout();

        if ( existingLayoutPtr && (existingLayoutPtr!=layoutPtrByStyle) ) {
            // widget has a layout, but it is not created by the style -> we cannot override.
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polish : layout overridden by user";
#endif // HBSTYLE_DEBUG
        } else {
            const QVariant existingLayoutNameV = widget->property( LAYOUT_NAME_PROPERTY );
            const QString existingLayoutName = existingLayoutNameV.isValid() ? existingLayoutNameV.toString() : QString();
            const bool cached = (existingLayoutName == layoutName );
            if ( !cached ) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "LayoutName cache miss.";
#endif
                HbWidgetStyleLoader *loader = HbWidgetStyleLoader::instance();
                if ( !loader->loadWidgetML(widget, layoutName, sectionName)) {
#ifdef HBSTYLE_DEBUG
                    qDebug() << "HbStyle::polish : Failed to load WidgetML";
                    qDebug() << "HbStyle::polish : -- -- -- -- EXIT";
#endif
                    return;
                }
                const QVariant ptrVariant = QVariant::fromValue( widget->layout() );
                widget->setProperty( LAYOUT_PTR_PROPERTY, ptrVariant );
                widget->setProperty( LAYOUT_NAME_PROPERTY, QVariant( layoutName ) );
            }

            anchorLayout = static_cast<HbAnchorLayout*>(widget->layout());
            if (cached) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "LayoutName cache hit.";
#endif
                anchorLayout->removeMappings();
            }

            anchorLayout->setMapping(anchorLayout, "");
            nodeIds = anchorLayout->nodeIds();
        }
    }

    // polish widget and subitems
    d->polishItem(styleRules, widget, widget, "", profile, false);
    QList<QGraphicsItem*> list = widget->childItems();
    foreach (QGraphicsItem* item, list) {
        QString name = HbStyle::itemName(item);
        if ( anchorLayout && !name.isEmpty() ) {
            // Assuming that all items with "itemName" are widgets.
            anchorLayout->setMapping(static_cast<QGraphicsWidget*>(item), name);
            // Remove from "nodeIds" so that we don't call polishItem
            // twice for this item.
            nodeIds.removeAll(name);
        }
        d->polishItem(styleRules, widget, item, name, profile, layoutDefined);
    }
    foreach (const QString &nodeId, nodeIds) {
        // These are the "missing" anchor items. Need to call polishItem
        // for them, too, for getting the anchor spacings right.
        // if there are anchor node ids, layoutDefined is always true.
        if ( !nodeId.isEmpty() ) {
            d->polishItem(styleRules, widget, 0, nodeId, profile, true);
        }
    }
    if ( anchorLayout ) {
        QList<HbAnchor*> anchors = anchorLayout->anchors();
        foreach (HbAnchor* anchor, anchors) {
            if ( !anchor->anchorId().isEmpty() ) {
                d->polishAnchor(styleRules, widget, anchor, profile);
            }
        }
    }

#ifdef HB_TEXT_MEASUREMENT_UTILITY
    if ( d->mLocTestMode == -1 ) {
        d->mLocTestMode = HbTextMeasurementUtility::instance()->locTestMode();
    }
    if ( d->mLocTestMode == HbTextMeasurementUtility::Automatic ) {
        HbInstancePrivate::d_ptr()->startLocalizationMeasurement();
    }
#endif // HB_TEXT_MEASUREMENT_UTILITY

#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polish : -- -- -- -- EXIT";
#endif
}

/*!
    updateThemedItems updates themable children items of widget

    \param styleRules style-rules of the widget
    \param variableRules variable-rules of the widget
    \param widget widget whose themed parameters are supposed to be updated
    \param item graphics item whose themed parameters are supposed to be updated
*/
void HbStylePrivate::updateThemedItems(
    const HbVector<HbCss::StyleRule> &styleRules,
    QGraphicsItem *item,
    HbDeviceProfile &profile) const
{

    QString name = HbStyle::itemName(item);
    if (name.isEmpty() ) {
        return;
    }

    HbTextItem* text = qgraphicsitem_cast<HbTextItem*>( item );
    HbRichTextItem* richtext = qgraphicsitem_cast<HbRichTextItem*>( item );
    HbIconItem* iconItem = qgraphicsitem_cast<HbIconItem*>( item );
    HbMarqueeItem* marqueeItem = qgraphicsitem_cast<HbMarqueeItem*>( item );
    if(! (text || richtext || iconItem || marqueeItem ) ){
        return;
    }

    const HbVector<HbCss::Declaration> decl = declarations(styleRules, name, 0, profile);

#ifdef HBSTYLE_DEBUG
   qDebug() << "HbStyle::updateThemedItems : -- Number of matching CSS declarations: " << decl.count();
#endif

   HbCss::ValueExtractor extractor(decl, profile);

    QColor col;
    bool extracted = extractor.extractColor( col );
    if (!extracted || !col.isValid()) {
        // Setting non black or white default color to make it visisble in black or white theme
        col.setRgb(255,0,255);
    }
   //apply the themed color to text-item
    if(text) {
#ifdef HBSTYLE_DEBUG
        if ( !extracted ) {
            qDebug() << "HbStyle::getColor : -- No color information found";
        }
#endif
        if ( !HbWidgetBasePrivate::d_ptr(text)->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor ) ){
            text->setTextColor(col);
            HbWidgetBasePrivate::d_ptr(text)->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor, false);
        }
    }

    if(richtext) {
#ifdef HBSTYLE_DEBUG
        if ( !extracted ) {
            qDebug() << "HbStyle::getColor : -- No color information found";
        }
#endif
        if ( !HbWidgetBasePrivate::d_ptr(richtext)->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor ) ){
            richtext->setTextDefaultColor(col);
            HbWidgetBasePrivate::d_ptr(richtext)->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor, false);
        }
    }

    if (iconItem) {
        // Applying color to mono-colorised icons from theme. Using setColor()
        // here would be wrong. It would lead to loosing the user-supplied color
        // in some cases so use the special setThemedColor() instead.
        HbIconItemPrivate::d_ptr(iconItem)->setThemedColor( col );
    }
    if(marqueeItem){
        //applying color to the marquee-item from theme
        if(!HbWidgetBasePrivate::d_ptr(marqueeItem)->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor)) {
            marqueeItem->setTextColor( col );
            HbWidgetBasePrivate::d_ptr(marqueeItem)->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor, false);
        }
    }
}

/*!
    HbStyle::updateThemedParams()

    Updates the themable parameters of widget (specially those which are specified in css files)

    \param widget widget whose themed parameters are supposed to be updated
*/
void HbStyle::updateThemedParams(HbWidget *widget)
{
    Q_D( HbStyle );
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::updateThemedParams : -- -- -- -- ENTER";
#endif
    if( !widget ) {
        return;
    }
    if(!widget->childItems().count()){
        return;
    }
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::updateThemedParams : " << widget->metaObject()->className();
#endif

    HbLayeredStyleLoader *styleLoader = HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Colors);
    if(!styleLoader){
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::updateThemedParams : HbLayeredStyleLoader returned a null pointer.";
        qDebug() << "HbStyle::updateThemedParams : -- -- -- -- EXIT";
#endif
        return;
    }
    HbCss::StyleSelector::NodePtr n(widget);
    HbDeviceProfile profile(HbDeviceProfile::profile(widget));
    HbVector<HbCss::StyleRule> styleRules = styleLoader->styleRulesForNode(n, profile.orientation());
    if (!styleRules.count()) {
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::updateThemedParams : No style rules found.";
        qDebug() << "HbStyle::updateThemedParams : -- -- -- -- EXIT";
#endif
        return;
    }

    // update themed items
    QList<QGraphicsItem*> list = widget->childItems();
    foreach (QGraphicsItem* item, list) {
        d->updateThemedItems(styleRules, item, profile);
    }

#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::updateThemedParams : -- -- -- -- EXIT";
#endif
}

/*!
    Checkes whether given widget has orienation specific style
    rules. This can be used for optimizing orientation switch.
*/
bool HbStyle::hasOrientationSpecificStyleRules(HbWidget *widget)
{
    HbLayeredStyleLoader *styleLoader =
        HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Layouts);
    HbCss::StyleSelector::NodePtr n(widget);
    return styleLoader->hasOrientationSpecificStyleRules(n);
}

/*!
    For clearing caches.
*/
void HbStyle::widgetDestroyed(QObject* obj)
{
    Q_UNUSED( obj );
}

/*!

    Sets the item name of a graphics item. Item name is not the same as object name.
    It's a textual identifier that uniquely identifies QGraphicsItem's child items.
    This method is used to map HbWidget's subelements to sub-elements defined in
    CSS/WidgetML layouts.

    Note that polish() method requires widget's children to be set with a correct item name.
    \sa HbStyle::polish()

    This method uses QGraphicsItem::setData() with id 0xfffe.

    \param item graphics item
    \param name name to be set
*/
void HbStyle::setItemName( QGraphicsItem *item, const QString &name )
{
    QString originalName = itemName(item);
    if (item && originalName != name) {
        item->setData( ItemName, name );
        QGraphicsItem* parent = item->parentItem();
        QGraphicsLayoutItem* lItem = (item->isWidget()) ? (QGraphicsLayoutItem*)static_cast<QGraphicsWidget*>(item) : 0;
        if (lItem && parent && parent->isWidget()) {
            QGraphicsWidget* parentW = static_cast<QGraphicsWidget*>(parent);
            const QVariant layoutPtrByStyleV = parentW->property( LAYOUT_PTR_PROPERTY );
            const QGraphicsLayout *layoutPtrByStyle = layoutPtrByStyleV.isValid() ? layoutPtrByStyleV.value<QGraphicsLayout*>() : 0;
            if ( parentW->layout() && (parentW->layout()==layoutPtrByStyle)) {
                HbAnchorLayout* layout = static_cast<HbAnchorLayout*>(parentW->layout());
                if ( layout->indexOf(lItem) != -1 ) {
                    if( name.isEmpty() ) {
                        layout->removeMapping(originalName);
                    } else {
                        layout->setMapping(lItem, name);
                    }
                }
            }
        }
    }
}

/*!

    Returns the item name of a graphics item. Item name is not the same as object name.
    It's a textual identifier that uniquely identifies QGraphicsItem's child items.

    \param item graphics item
    \return item name or an empty string if name's not set
*/
QString HbStyle::itemName( const QGraphicsItem *item )
{
    return item
        ? item->data( ItemName ).toString()
        : QString();
}

/*!
    Returns a value of a global style parameter. The value is returned in pixels.

    Available parameters can be found from hbglobalparameters.css. By using these
    parameters applications get consistent look.

    \param param Name of the global parameter.
    \param value Returns value of the global parameter.
    \param profile Device profile of the used HbMainWindow. Primary display's.
            device profile HbDeviceProfile::current() is used if the value is omitted.
    \return true if the variable was found.
*/
bool HbStyle::parameter(const QString& param, qreal& value, const HbDeviceProfile &profile) const
{
    HbDeviceProfile effectiveProfile = profile;
    if ( effectiveProfile.isNull() ) {
        effectiveProfile = HbDeviceProfile::current();
    }
    Q_D( const HbStyle );
    d->layoutParameters.init(effectiveProfile);
    HbCss::ValueExtractor valueExtractor(effectiveProfile);
    valueExtractor.setLayoutParameters(d->layoutParameters);

    if ( param.endsWith(QLatin1String(")")) &&
        ( param.startsWith(QLatin1String("var(")) ||
          param.startsWith(QLatin1String("-var(")) ||
          param.startsWith(QLatin1String("expr(")) ||
          param.startsWith(QLatin1String("-expr(")) ) ) {
        // Expression parser can deal with "functions"
        return valueExtractor.extractExpressionValue(param, value);
    }
    return valueExtractor.extractVariableValue(param, value);
}

bool HbStylePrivate::valueFromTokens(const QList<int> &tokens, qreal &value, const HbDeviceProfile &profile) const
{
    HbDeviceProfile effectiveProfile = profile;
    if ( effectiveProfile.isNull() ) {
        effectiveProfile = HbDeviceProfile::current();
    }
    layoutParameters.init(effectiveProfile);
    HbCss::ValueExtractor valueExtractor(effectiveProfile);
    valueExtractor.setLayoutParameters(layoutParameters);
    return valueExtractor.extractExpressionValue(tokens, value);
}


/*!
    Returns copy of all global style parameters. Both names and values
    of the parameters are returned. The values are returned in pixels.

    Available parameters can be found from hbglobalparameters.css. By using these
    parameters applications get consistent look. Usage of this API (instead of parameter)
    is recommended if an application needs to fetch several parameters in one place.

    \param params Contains names and values of all global style parameters.
    \param profile Device profile of the used HbMainWindow. Primary display's
            device profile HbDeviceProfile::current() is used if the value is omitted.
*/

void HbStyle::parameters(HbStyleParameters &params, const HbDeviceProfile &profile) const
{
    HbDeviceProfile effectiveProfile = profile;
    if ( effectiveProfile.isNull() ) {
        effectiveProfile = HbDeviceProfile::current();
    }

    Q_D( const HbStyle );
    d->layoutParameters.init(effectiveProfile);
    HbCss::ValueExtractor valueExtractor(effectiveProfile);
    valueExtractor.setLayoutParameters(d->layoutParameters);
    qreal value = 0;
    HbLayoutParameters::const_iterator i = d->layoutParameters.constBegin();
    while (i != d->layoutParameters.constEnd()) {
        QString name(d->layoutParameters.name(i));
        if (valueExtractor.extractVariableValue(name, value)) {
            params.addParameter(name, value);
        }
        ++i;
    }
}

/*!
    Returns values for widget specific style parameters. The names of the parameters
    are passed in with \a params.

    This method should be used only if you need to access widget specific parameters out
    of the polish loop. It is more efficient to use HbWidget::polish(HbStyleParameters &params)
    if you don't need to access the parameters before the (first) polish event.

    \param parameters style parameters to be returned to the caller
    \param widget HbWidget to be polished
    \sa HbStyle::polish, HbStyle::parameters and HbWidget::polish
*/
void HbStyle::widgetParameters(HbStyleParameters &params, HbWidget* widget) const
{
    Q_D( const HbStyle );
    if( !widget || !params.count() ) {
        return;
    }
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::widgetParameters : Parameters for" << widget->metaObject()->className();
#endif

    HbLayeredStyleLoader *styleLoader = HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Layouts);
    if(!styleLoader){
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::widgetParameters : HbLayeredStyleLoader returned a null pointer.";
#endif
        return;
    }
    HbDeviceProfile profile(HbDeviceProfile::profile(widget));
    HbCss::StyleSelector::NodePtr n(widget);

    HbVector<HbCss::StyleRule> styleRules = styleLoader->styleRulesForNode(n, profile.orientation());

#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::widgetParameters : Number of style rules:" << styleRules.count();
#endif
    if (!styleRules.count()) {
        return;
    }
    const HbVector<HbCss::Declaration> decl = declarations(styleRules, "", widget, profile);
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::widgetParameters : Number of matching CSS declarations: " << decl.count();
#endif
    d->layoutParameters.init(profile);

    HbCss::ValueExtractor extractor(decl, profile);
    extractor.setLayoutParameters(d->layoutParameters);
    extractor.extractCustomProperties( params.keys(), params.values() );
}


/*!
\internal
*/
HbStylePrivate::HbStylePrivate() : mLocTestMode(-1)
{
    HbWidgetStyleLoader *loader = HbWidgetStyleLoader::instance();
    if(loader){
        loader->addFilePath(STYLE_LOCATION, HbLayeredStyleLoader::Concern_Layouts,
                HbLayeredStyleLoader::Priority_Core);
        loader->addFilePath(COLOR_STYLE_LOCATION, HbLayeredStyleLoader::Concern_Colors,
                HbLayeredStyleLoader::Priority_Core);
    }
}

/*!
\internal
*/
HbStylePrivate::~HbStylePrivate()
{
    HbWidgetStyleLoader *loader = HbWidgetStyleLoader::instance();
    if(loader){
        loader->removeFilePath(STYLE_LOCATION, HbLayeredStyleLoader::Concern_Layouts,
                HbLayeredStyleLoader::Priority_Core);
        loader->removeFilePath(COLOR_STYLE_LOCATION, HbLayeredStyleLoader::Concern_Colors,
                HbLayeredStyleLoader::Priority_Core);
    }
}

/*!
\internal
*/
QString HbStylePrivate::logicalName(HbStylePrivate::Primitive primitive, const QStyleOption *option)
{
    switch (primitive) {

    case HbStylePrivate::P_Slider_thumb: {
        const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider *>(option);
        if (opt) {
            QString iconPath;
            switch (opt->orientation) {

            case Qt::Horizontal:
                if (opt->state & QStyle::State_Sunken)
                    iconPath = "qtg_graf_slider_h_handle_pressed";
                else
                    iconPath = "qtg_graf_slider_h_handle_normal";
                return iconPath;

            case Qt::Vertical:
                if (opt->state & QStyle::State_Sunken)
                    iconPath = "qtg_graf_slider_v_handle_pressed";
                else
                    iconPath = "qtg_graf_slider_v_handle_normal";
                return iconPath;

            default:
                break;

            }
        }
    }

    default:
        break;

    }

    return QString();
}

/*!
\internal
*/
QIcon::Mode HbStylePrivate::iconMode(QStyle::State state)
{
    QIcon::Mode mode = QIcon::Disabled;
    if (state & QStyle::State_Enabled)
        mode = QIcon::Normal;
    if (state & QStyle::State_Active)
        mode = QIcon::Active;
    if (state & QStyle::State_Selected)
        mode = QIcon::Selected;
    return mode;
}

/*!
\internal
*/
QIcon::State HbStylePrivate::iconState(QStyle::State state)
{
    QIcon::State icon = QIcon::Off;
    if (state & QStyle::State_On)
        icon = QIcon::On;
    return icon;
}

/*!
\internal
*/
void HbStylePrivate::clearStyleSheetCaches()
{
    styleRulesCache.clear();
}

HbWidgetBasePrivate *HbStylePrivate::widgetBasePrivate(HbWidgetBase *widgetBase)
{
    return HbWidgetBasePrivate::d_ptr(widgetBase);
}


#include "moc_hbstyle.cpp"
