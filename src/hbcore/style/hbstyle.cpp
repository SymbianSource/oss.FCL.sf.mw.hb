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

#include "hbstyleoptionslider.h"
#include "hbstyleoptionsliderelement_p.h"
#include "hbstyleoptionpushbutton.h"
#include "hbstyleoptiontoolbutton.h"
#include "hbstyleoptiontooltip.h"
#include "hbstyleoptionprogressbar.h"
#include "hbstyleoptionabstractviewitem.h"
#include "hbstyleoptionlistviewitem.h"
#include "hbstyleoptionmenuitem.h"
#include "hbstyleoptionlabel.h"
#include "hbstyleoptionscrollbar.h"
#include "hbstyleoptiontoolbarextension.h"
#include "hbstyleoptiongridviewitem.h"
#include "hbstyleoptioncheckbox.h"
#include "hbiconloader_p.h"
#include "hbstyleoptiontitlepane.h"
#include "hbstyleoptionstatusbar_p.h"
#include "hbstyleoptionindicatorgroup.h"
#include "hbstyleoptionprogressdialog.h"
#include "hbstyleoptionnotificationdialog.h"
#include "hbstyleoptiontreeviewitem.h"
#include "hbstyleoptioncolorgridviewitem.h"
#include "hbstyleoptionmessagebox.h"
#include "hbstyleoptionsoftkey.h"   // deprecated
#include "hbstyleoptionnavigationbutton.h"
#include "hbstyleoptionindicatorbutton.h"
#include "hbstyleoptionsignalindicator_p.h"
#include "hbstyleoptionbatteryindicator_p.h"
#include "hbstyle_p.h"
#include "hbstyleloader.h"
#include "hbwidgetloader_p.h"
#include "hbstyleoptionratingslider.h"
#include "hbstyleoptiondataformviewitem.h"
#include "hbstyleoptiondatagroup_p.h"
#include "hbstyleoptiondatagroupheadingwidget_p.h"
#include "hbstyleoptiondataform.h"
#include "hbstyleoptiongroupbox.h"
#include "hbstyleoptionindexfeedback.h"
#include "hbstyleoptioncombobox.h"
#include "hbstyleoptioninputdialog.h"

#include <hbicon.h>
#include <hbstyle.h>
#include <hbframedrawer.h>
#include <hbiconitem.h>
#include <hbiconitem_p.h>
#include <hbframeitem.h>
#include <hbframedrawer.h>
#include <hbframedrawer_p.h>
#include <hbframebackground.h>
#include <hbprogresstrackitem_p.h>
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
#include <QPluginLoader>
#include <hbstyleinterface.h>
#include <hbstyleparameters.h>

#include "hbstyleselector_p.h"
#include "hblayeredstyleloader_p.h"
#include "hbwidgetstyleloader_p.h"
#include "hbcssparser_p.h"
#include "hbrepeaticonitem_p.h"
#include "hbcolortheme_p.h"
#include "hbnamespace_p.h"


#include "hbmeshlayout_p.h"

#include <QGraphicsWidget>
#include <hbwidget.h>
#include <hbwidgetbase.h>
#include "hbdeviceprofile.h"
#include "hbrepeatitem_p.h"

#include <QDebug>

//Uncomment next define in order to get more debug prints.
//Similar define exists also in the engine side.
//#define HBSTYLE_DEBUG

#ifdef HBSTYLE_DEBUG
#include <QDebug>
#endif

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
    itself it does not need a paint() method at all since primitives (widget's childs) are doing the drawing. 
    Painting for the primitives occurs from the graphics scene.

*/



// TODO: margins should be defined in layout data once layout specification exists.
static const int ItemName = 0xfffe;
static const QString STYLE_LOCATION = QLatin1String(":/themes/style/hbdefault/rules/widgets/%w/%w");
static const QString COLOR_STYLE_LOCATION = QLatin1String(":/themes/style/hbdefault/rules/widgets/%w/%w_color");
static const int TOUCHAREA_ZVALUE = 1000;

static const QString GLOBAL_PARAMETERS_LOCATION = QLatin1String(":/themes/style/hbdefault/variables/layout/zoom/0/hbglobalparameters.css");


/*!
Constructor
*/
HbStyle::HbStyle() :
    d_ptr(new HbStylePrivate)
{
    Q_D( HbStyle );
    d->q_ptr = this;
    QObject::connect(hbInstance->theme(), SIGNAL(changed()), this, SLOT(_q_onThemeChanged()));
}

/*!
Destructor
*/
HbStyle::~HbStyle()
{
    delete d_ptr;
}

/*!
    Registers the style plugin with the Style system. This method can be called by a custom widget
    or application in order to register the style plugin that implements the custom graphics primitives.
    This method results in loading of the plugin, if the plugin is registered for the first time.
    It returns the base ID for the primitives implemented by the style plugin. The custom widget
    can use the range of integers from (BaseID) to (BaseID+count-1) to refer to the custom
    primitives, where count is the number of primitives supported by the plugin. The style
    plugin must return the correct number of primitives when the primitiveCount() method is called.
    In case of errors the method returns < 0. Note also that for each registerPlugin there must be
    a unregisterPlugin call, the last unregisterPlugin call for a plugin causes the plugin to be unloaded.

    If the style plugin implementation returns valid path with layout defition files (CSS+WidgetML)
    from layoutPath() method the layout definitions CSSs gets read when calling registerPlugin().

    \param pluginName, name of the Plugin library to be dynamically loaded
    \return int the base ID to be used for the primitives implemented by the style plugin
    \sa unregisterPlugin()
 */
int HbStyle::registerPlugin(const QString &pluginName)
{
    Q_D( const HbStyle );

    // check if the plugin is already registered
    if (d->registeredPlugins.contains(pluginName)) {
        // increase the reference count
        d->registeredPlugins.value(pluginName)->refCount++;
        // return the base ID of the primitives enumeration
        return d->registeredPlugins.value(pluginName)->primitiveBaseId;
    }

    // load the plugin
    QPluginLoader* loader = new QPluginLoader(pluginName);
    if (loader == 0)
        return -1;

    // get the instance pointer
    QObject* pluginInstance = loader->instance();
    if (pluginInstance == 0) {
        delete loader;

        // try the additional paths
        QFileInfo fileInfo( pluginName );
        foreach( QString additionalPath, hbInstance->libraryPaths() ) {
            const QDir pluginDir(additionalPath);
            loader = new QPluginLoader(pluginDir.absoluteFilePath(fileInfo.fileName()));
            if ( loader ) {
                pluginInstance = loader->instance();
                if ( !pluginInstance ) {
                    delete loader;
                } else {
                    break;
                }
            }
        }

        if ( !pluginInstance ) {
            return -1;
        }
    }

    HbStyleInterface *stylePlugin = qobject_cast<HbStyleInterface *>(pluginInstance);

    int primitiveCount = stylePlugin->primitiveCount();
    if (primitiveCount <= 0) {
        delete pluginInstance;
        delete loader;
        return -1;
    }

    HbStyleInterfaceInfo* info = new HbStyleInterfaceInfo();
    info->loader = loader;
    info->primitiveBaseId = d->nextAvailableId;

    // make entries for the primitives in the hash table
    for (int i=d->nextAvailableId; i<(d->nextAvailableId+primitiveCount); i++)
    {
        d->customPrimitives.insert(i, info);
    }

    // make entry for the plugin in the registered plugins hash table
    HbStylePluginInfo* pluginInfo = new HbStylePluginInfo();
    pluginInfo->primitiveBaseId = d->nextAvailableId;
    pluginInfo->primitiveCount = primitiveCount;
    pluginInfo->refCount = 1;

    d->registeredPlugins.insert(pluginName, pluginInfo);
    d->nextAvailableId += primitiveCount;

    // register associated style files
    HbWidgetStyleLoader::instance()->addFilePath(
        stylePlugin->layoutPath(),
        HbLayeredStyleLoader::Concern_Layouts, 
        HbLayeredStyleLoader::Priority_Core);
    d->pluginStylePaths.insert(pluginName, stylePlugin->layoutPath());

    return pluginInfo->primitiveBaseId;
}


/*!
    Un-registers the style plugin.
    If the reference count becomes zero, the plugin is unloaded and the primitive IDs are de-registered
    If a client has called registerPlugin() it must unregister the style plugin with this method.

    \param pluginName, name of the Plugin library
 */
void HbStyle::unregisterPlugin(const QString &pluginName)
{
    Q_D( const HbStyle );
    if (d->registeredPlugins.contains(pluginName)) {
        HbStylePluginInfo *info = d->registeredPlugins.value(pluginName);
        info->refCount--;
        // unload plugin and remove from list
        if (info->refCount == 0) {
            HbStyleInterfaceInfo* styleInfo = d->customPrimitives.value(info->primitiveBaseId);
            delete styleInfo->loader->instance();
            delete styleInfo->loader;
            delete styleInfo;
            for (int i=info->primitiveBaseId; i< (info->primitiveBaseId+info->primitiveCount); i++) {
                  d->customPrimitives.remove(i);
            }
            d->registeredPlugins.remove(pluginName);

	        // unregister associated style files
            HbWidgetStyleLoader::instance()->removeFilePath(
                d->pluginStylePaths.value(pluginName),
                HbLayeredStyleLoader::Concern_Layouts, 
                HbLayeredStyleLoader::Priority_Core);
            d->pluginStylePaths.remove(pluginName);

            if( d->registeredPlugins.count() == 0 ){
                // no plugins loaded, can reset the id counter
                d->nextAvailableId = HbStyle::P_CustomBase;
            }

        }
    }
}


/*!
  Creates instances of primitive graphics items. This method should be used by all widgets that support styling.
  When changing the style the returned primitives can be replaced with an altered version of the primitives, or with a completely
  different primitive, to create a custom appearance. This shouldn't cause changes to the widget if the functional design
  remains the same.

  This method returns HbWidgetBase objects. A widget can store the returned items as HbWidgetBase. If a widget
  does not require any functionality from HbWidgetBase then it should store the primitives as QGraphicsItems.

  \note This method is used to replace the conventional Qt-style paint indirection via drawPrimitive.
  \sa HbStyle::updatePrimitive method which is to be used in conjunction with this method
  \sa HbWidgetBase
  \param primitive, to identify the primitive to create
  \param parent of the item
  \return HbWidgetBase is returned.
 */
QGraphicsItem *HbStyle::createPrimitive( HbStyle::Primitive primitive, QGraphicsItem *parent ) const
{

    Q_D( const HbStyle );

    if (d->customPrimitives.contains(primitive)) {
        HbStyleInterfaceInfo* info = d->customPrimitives.value(primitive);
        QObject* pluginInstance = info->loader->instance();
        HbStyleInterface *stylePlugin = qobject_cast<HbStyleInterface *>(pluginInstance);
        return stylePlugin->createPrimitive((HbStyle::Primitive)(primitive-info->primitiveBaseId), parent);
    }

        switch (primitive){
            case P_MenuItem_submenuindicator:
                {
                HbIconItem *item = new HbIconItem(parent);
                setItemName(item, QLatin1String("submenu-indicator"));
                return item;
                }

            case P_MenuItem_checkindicator:
                {
                HbIconItem *item = new HbIconItem(parent);
                setItemName(item, QLatin1String("check-indicator"));
                return item;
                }

            case P_MenuItem_separator:
                {
                HbIconItem *item = new HbIconItem(parent);
                setItemName(item, QLatin1String("separator"));
                return item;
                }
            case P_ToolButton_icon:
            case P_PushButton_icon:
			case P_ProgressDialog_icon:
            case P_CheckBox_icon:
            case P_GroupBoxHeading_icon:
            case P_Label_icon:
            case P_GridViewItem_icon: {
                HbIconItem *item = new HbIconItem(HbIcon(),parent);
                setItemName(item, QLatin1String("icon"));
                return item; 
                }
            case P_DataGroup_icon:
                {
                HbIconItem *item = new HbIconItem(HbIcon(), parent);
                setItemName(item, QLatin1String("dataGroup_Icon"));
                return item; 
                }
            case P_DataItem_icon:
                {
                HbIconItem *item = new HbIconItem(HbIcon(), parent);
                setItemName(item, QLatin1String("dataItem_Icon"));
                return item; 
                }
            case P_ComboBox_text:
                {
                    HbTextItem *textItem = new HbTextItem(parent);
                    setItemName(textItem, QLatin1String("combobox_labelfield"));
                    return  textItem;
                }
            case P_ToolButton_text:
            case P_CheckBox_text:
            case P_ProgressDialog_text:
            case P_PushButton_text:
                {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("text"));
                return  ti;
                }
            case P_DataGroup_heading:
                {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("dataGroup_HeadingLabel"));
                return  ti;
                }
            case P_DataGroup_description:
                {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("dataGroup_Description"));
                return  ti;
                }
            case P_DataForm_heading:
                {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("dataForm_Heading"));
                //ti->setZValue(2);
                return  ti;
                }
            case P_DataForm_description:
                {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("dataForm_Desc"));
                return  ti;
                }
            case P_PushButton_additionaltext:
                {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("additional-text"));
                return  ti;
                }
             case P_ProgressBar_toucharea:
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                setItemName(ta, QLatin1String("toucharea1"));
				ta->setZValue(TOUCHAREA_ZVALUE);
                return ta;
                }
			case P_ProgressSliderHandle_toucharea: 
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                setItemName(ta, QLatin1String("toucharea"));
				ta->setZValue(TOUCHAREA_ZVALUE);
                if(parent){
                    parent->setHandlesChildEvents(true); 
                }
                return ta;
                }

			case P_ProgressSlider_handle:
			    {
                HbIconItem *item = new HbIconItem(QLatin1String("qtg_graf_progslider_handle_normal"), parent);
				item->setAspectRatioMode(Qt::IgnoreAspectRatio);
                return item;									 
				}
            case P_ProgressSlider_handleicon:{
                HbIconItem *item = new HbIconItem(QLatin1String("qtg_mono_play"), parent);
                return item;
                }
            case P_PushButton_toucharea: // Generic implementation, can add other cases here
            case P_CheckBox_toucharea:
            case P_RatingSlider_toucharea:
	        case P_SliderElement_touchhandle: 
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                setItemName(ta, QLatin1String("toucharea"));
                if(parent){
                    parent->setHandlesChildEvents(true); 
                }
                return ta;
                }
            case P_ScrollBar_toucharea:
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlags(QGraphicsItem::ItemIsFocusable);
                setItemName(ta, QLatin1String("toucharea"));
                // NOTE:: This is a temporary fix
                //if(parent){
                //    parent->setHandlesChildEvents(true); 
                //}
                return ta;
                }
            case P_ComboBoxButton_toucharea:
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                setItemName(ta, QLatin1String("combobox_button_toucharea"));
                /*if(parent){
                    parent->setHandlesChildEvents(true); 
                }*/
                return ta;
                }
             case P_TitleBar_toucharea: {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                setItemName(ta, QLatin1String("toucharea"));
                return ta;
                }
             case P_SliderElement_touchdecrease:
             case P_SliderElement_touchincrease:
             case P_SliderElement_touchgroove:
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                if(parent){
                    //parent->setHandlesChildEvents(true); 
                }
                return ta;
                }
            case P_SliderElement_text:
            case P_ProgressBar_text:
            {
                HbTextItem *ti = new HbTextItem(parent);
                return  ti;
            }
            case P_DataItem_label:
            {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("dataItem_Label"));
                return  ti;
            }
            case P_DataItem_description:
            {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("dataItem_Description"));
                return  ti;
            }
            case P_ProgressBar_mintext:
            {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("min-text"));
                return  ti;
            }
            case P_ProgressBar_maxtext:
            {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("max-text"));
                return  ti;
            }
            case P_Label_text: {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("text"));
                ti->setAlignment(Qt::AlignCenter);
                return ti;
            }
                
            case P_Label_richtext: 
            {
                HbRichTextItem *rti = new HbRichTextItem(parent);
                setItemName(rti, QLatin1String("text"));
                return rti;
            }

            case P_MenuItem_text:
                {
                HbTextItem *item = new HbTextItem(parent);
                setItemName(item, QLatin1String("text"));

                return item;
                }

            case P_Slider_thumb:
                return new HbIconItem(parent);

            case P_MenuItem_frame:
            case P_MenuItem_focus:
            case P_ScrollBar_groove:
            case P_ScrollBar_handle:
            case P_Slider_groove:
            case P_Popup_background:
            case P_Popup_background_weak:
            case P_Popup_heading_frame:
            case P_NotificationDialog_frame:
            case P_ToolTip_background:
            case P_PushButton_focus:
            case P_ComboBox_background:
            case P_ToolBarExtension_background:
            case P_SliderPopup_background:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                return n;
            }

            case P_ProgressBar_frame:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                setItemName(n, "frame");
                return n;
            }

            case P_ComboBoxPopup_background:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
				setItemName(n , "dropdown_background");
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
            case P_TumbleView_highlight:{
                //return new HbIconItem(parent);
                return new HbFrameItem(parent);//TODO:make it icon once iconitem setGeomoetry works for tumbleview
            }
            case P_DateTimePicker_separator:{
                //return new HbIconItem(parent);
                HbFrameItem *frameItem= new HbFrameItem(parent);//TODO:make it icon once iconitem setGeomoetry works for tumbleview
                frameItem->frameDrawer().setFrameGraphicsName("qtg_graf_tumbler_devider");
                frameItem->frameDrawer().setFrameType(HbFrameDrawer::OnePiece);
                frameItem->setPreferredWidth(2);
                return frameItem;
            }           
            case P_DataGroup_background :
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                setItemName(n, QLatin1String("dataGroup_Background"));
                return n;
            }
            case P_DataGroupComboBackground:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                setItemName(n, QLatin1String("dataGroup_ComboBackground"));
                return n;
            }
            case P_DataForm_heading_background :
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                setItemName(n, QLatin1String("dataFormHeading_Background"));
                return n;
            }
            case P_DataItem_background:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-2);
                setItemName(n, QLatin1String("dataItem_Background"));
                return n;
            }
            case P_GroupBoxContent_background:
            case P_GroupBoxHeading_background:
            case P_PushButton_background:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                setItemName(n, QLatin1String("background"));
                return n;
            }
            case P_ProgressBar_track:
            {
                HbProgressTrackItem *n = new HbProgressTrackItem(parent);
                n->setZValue(-2);
                return n;
            }
            case P_ProgressBar_slidertrack:
            {
                HbProgressTrackItem *n = new HbProgressTrackItem(parent);
                n->setZValue(-1);
                return n;
            }        
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
                setItemName(n, QLatin1String("background"));
                return n;
            }
            case P_Softkey_background:  // deprecated
            case P_NavigationButton_background:
            case P_IndicatorButton_background:
            case P_IndicatorGroup_background:   // deprecated
            {
                HbIconItem *n = new HbIconItem(HbIcon(), parent);
                n->setZValue(-1);
                setItemName(n, QLatin1String("background"));
                return n;
            }
            case P_IndicatorButton_handleindication:
            {
                HbIconItem *n = new HbIconItem(parent);
                setItemName(n, QLatin1String("handleindication"));
                return n;
            }
            case P_IndicatorGroup_icon1:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                setItemName(n, QLatin1String("icon1"));
                return n;
            }
            case P_IndicatorGroup_icon2:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                setItemName(n, QLatin1String("icon2"));
                return n;
            }
            case P_IndicatorGroup_icon3:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                setItemName(n, QLatin1String("icon3"));
                return n;
            }
            case P_IndicatorGroup_icon4:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                setItemName(n, QLatin1String("icon4"));
                return n;
            }
            case P_IndicatorGroup_signalicon: // deprecated
            case P_SignalIndicator_icon:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                setItemName(n, QLatin1String("icon"));
                return n;
            }
            case P_SignalLevel_background:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setZValue(-1);
                setItemName(n, QLatin1String("background"));
                return n;
            }
            case P_SignalLevel_icon:
            {
                HbIndicatorLevelIconItem *n = new HbIndicatorLevelIconItem(parent);
                setItemName(n, QLatin1String("levelicon"));
                return n;
            }
            case P_IndicatorGroup_batteryicon: // deprecated
            case P_BatteryIndicator_icon:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                setItemName(n, QLatin1String("icon"));
                return n;
            }
            case P_BatteryLevel_background:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setZValue(-1);
                setItemName(n, QLatin1String("background"));
                return n;
            }
            case P_BatteryLevel_icon:
            {
                HbIndicatorLevelIconItem *n = new HbIndicatorLevelIconItem(parent);
                setItemName(n, QLatin1String("levelicon"));
                return n;
            }
            case P_TitlePane_background:
                {
                    HbFrameItem *n = new HbFrameItem(parent);
                    n->setZValue(-1);
                    setItemName(n, QLatin1String("frame"));
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
                setItemName(n, QLatin1String("text"));
                return n;
            }
            case P_TitlePane_icon:
            {
                HbIconItem *n = new HbIconItem(parent);
                setItemName(n, QLatin1String("icon"));
                return n;
            }
            case P_StatusBar_background:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                setItemName(n, QLatin1String("statusbar"));
                return n;
			}
            case P_StatusBar_timetext:
            {
                HbTextItem *textItem = new HbTextItem(parent);
                setItemName(textItem, QLatin1String("timetext"));
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
                setItemName(iconItem,QLatin1String("tickmark-majoricon"));
                return iconItem;
            }
            case P_SliderTickMark_minoricon: {
                HbIconItem *iconItem = new HbIconItem(parent);
                setItemName(iconItem,QLatin1String("tickmark-minoricon"));
                return iconItem;
            }
            case P_SliderTickMark_majorlabel: {
                HbTextItem *textItem = new HbTextItem(parent);
                setItemName(textItem,QLatin1String("tickmark-majorlabel"));
                return textItem;
            }
            case P_SliderTickMark_minorlabel: {
                HbTextItem *textItem = new HbTextItem(parent);
                setItemName(textItem,QLatin1String("tickmark-minorlabel"));
                return textItem;
            }


            case P_ProgressBar_waittrack:
                return new HbRepeatIconItem(QLatin1String("qtg_fr_progbar_wait"), parent);

            case P_RatingSlider_frame:
                return  new HbRepeatItem(parent);

            case P_RatingSlider_track:
                return new HbRepeatMaskItem(parent);
            
			case P_RatingSlider_layout:
                return new HbWidgetBase(parent);

            case P_ItemViewItem_checkbox: {
                qDebug() << "Primitive P_ItemViewItem_checkbox is deprecated and will cease to exist in the near future.";
                HbIconItem *iconItem = new HbIconItem(parent);
                setItemName(iconItem, QLatin1String("checkbox-icon"));
                return iconItem;
            }

            case P_ItemViewItem_radiobutton:
            case P_ItemViewItem_selection: {
                HbIconItem *iconItem = new HbIconItem(parent);
                setItemName(iconItem, QLatin1String("selection-icon"));
                return iconItem;
            }

            case P_Edit_text:{
                return 0;
            }
            case P_GroupBoxHeading_text:
            case P_NotificationDialog_text:{
                HbTextItem *n = new HbTextItem(parent);
                setItemName(n, QLatin1String("text"));
                return n;
            }
            case P_NotificationDialog_title:{
                HbTextItem *n = new HbTextItem(parent);
                setItemName(n, QLatin1String("title"));
                return n;
            }
            case P_NotificationDialog_icon:{
                HbIconItem *n = new HbIconItem(QString(), parent);
                setItemName(n, QLatin1String("icon"));
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
                HbIconItem *iconItem = new HbIconItem(parent);
                setItemName(iconItem, QLatin1String("background"));
                return iconItem;
            }

            case P_ListViewItem_text: {
                HbTextItem *textItem = new HbTextItem(parent);
                textItem->setMinimumLines(1);
                textItem->setMaximumLines(1);
                textItem->setTextWrapping(Hb::TextNoWrap);
                return textItem;
            }

            case P_ListViewItem_richtext: {
                HbRichTextItem *textItem = new HbRichTextItem(parent);
                return textItem;
            }

            case P_GridViewItem_text: {
                HbTextItem *textItem = new HbTextItem(parent);
                // caching do not work properly - text is not refreshed immediatelly
                //textItem->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
                setItemName(textItem, QLatin1String("text"));
                textItem->setMinimumLines(1);
                textItem->setMaximumLines(1);
                textItem->setTextWrapping(Hb::TextNoWrap);
                textItem->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Preferred );
                return textItem;
            }

            case P_ListViewItem_icon:
                return new HbIconItem(parent);

            case P_ColorGridViewItem_colorIcon:
                return new HbIconItem("qtg_graf_colorpicker_mask", parent);
            case P_ColorGridViewItem_borderIcon:
                return new HbIconItem("qtg_graf_colorpicker_filled", parent);
            case P_ColorGridViewItem_checkIcon:
                return new HbIconItem("qtg_small_tick", parent); // TODO: likely wrong

            case P_TreeViewItem_expandicon: {
                HbIconItem *iconItem = new HbIconItem(parent);
                setItemName(iconItem, QLatin1String("subitem-indicator"));
                return iconItem;
            }

            case P_SelectionControl_selectionstart://fallthrough
            case P_SelectionControl_selectionend://fallthrough
            {
                HbIconItem *iconItem = new HbIconItem(parent);
                switch (primitive) {
                    case P_SelectionControl_selectionstart:
                        iconItem->setIconName(QLatin1String("qtg_graf_editor_handle_begin"));
                        break;
                    case P_SelectionControl_selectionend:
                        iconItem->setIconName(QLatin1String("qtg_graf_editor_handle_end"));
                        break;
                    default:
                        qWarning("Unknown HbSelectionControl primitive %i", primitive);
                        break;
                }
                setItemName(iconItem, QLatin1String("handle-icon"));
                return iconItem;
            }
            case P_ComboBox_button: {
                HbIconItem *n = new HbIconItem(QString(), parent);
                return n;
            }
            case P_ItemViewItem_focus: {
                HbFrameItem *item = new HbFrameItem(parent);
                setItemName(item, QLatin1String("focus-indicator"));
                return item;
            }
            case P_ItemHighlight_background: 
                return new HbFrameItem(parent);

            case P_ItemViewItem_frame: {
                HbFrameItem *item = new HbFrameItem(parent);
                setItemName(item,"frame");
                return item;
            }
            case P_ScrollArea_continuationbottom:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                setItemName(n, QLatin1String("continuation-indicator-bottom"));
                n->setZValue(-1);
                return n;
            }
            case P_ScrollArea_continuationtop:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                setItemName(n, QLatin1String("continuation-indicator-top"));
                n->setZValue(-1);
                return n;
            }
            case P_ScrollArea_continuationleft:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                setItemName(n, QLatin1String("continuation-indicator-left"));
                n->setZValue(-1);
                return n;
            }
            case P_ScrollArea_continuationright:
                {
                HbFrameItem *n = new HbFrameItem(parent);
                setItemName(n, QLatin1String("continuation-indicator-right"));
                n->setZValue(-1);
                return n;
            }
            case P_ItemViewItem_touchmultiselection:
            {
                HbTouchArea *area = new HbTouchArea(parent);
                setItemName(area, "multiselection-toucharea");
                return area;
            }
            case P_IndexFeedback_popup_text:
            {
                HbTextItem *textItem = new HbTextItem(parent);
                textItem->setAlignment(Qt::AlignCenter);
                textItem->setTextWrapping(Hb::TextNoWrap);
                setItemName(textItem, QLatin1String("index-text"));

                //TODO:: make this a sane value
                textItem->setZValue(4);
                return textItem;
            }
            case P_IndexFeedback_popup_background:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->frameDrawer().setFrameGraphicsName("qtg_fr_popup_preview");
                n->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                qreal cornerPieceSize = 0;
                parameter(QLatin1String("hb-param-background-popup-preview"),cornerPieceSize);
                n->frameDrawer().setBorderWidths(cornerPieceSize, cornerPieceSize);
                setItemName(n, QLatin1String("index-background"));
                n->setZValue(3);
                return n;
            }
            case P_InputDialog_text:
            {
                HbTextItem *n = new HbTextItem(parent);
                setItemName(n, QLatin1String("label-1"));
                return n;
            }
            case P_InputDialog_additional_text:
            {
                HbTextItem *n = new HbTextItem(parent);
                setItemName(n, QLatin1String("label-2"));
                return n;
            }
            default:
                return 0;
        }
}

/*!
  Updates the state and content of widget's child primitives. Update for a styled primitive should happen always when
  a state change that affects drawing occurs. Such a situation can be for example pressing of a button (change background image), or 
  changing a text for a widget having text content. The updatePrimitive() implementation for each primitive element can be considered 
  as a part of widget's implementation. Note that it's up to the widget to decide what the styleoption contains and what the updatePrimitive() method 
  uses the styleoption data for.
  
  \sa HbStyle::createPrimitive
  \param item Primitive graphicsitem.
  \param primitive To identify the primitive to create.
  \param option Style option, contains all the information needed to update the primitive, this
  information is widget specific and each widget usually has a styleoption. Styleoption may include
  information about the widget's state, content etc.
 */
void HbStyle::updatePrimitive( QGraphicsItem *item, HbStyle::Primitive primitive, const QStyleOption *option ) const
{
    Q_D( const HbStyle );

    if (d->customPrimitives.contains(primitive)) {
        HbStyleInterfaceInfo* info = d->customPrimitives.value(primitive);
        QObject* pluginInstance = info->loader->instance();
        HbStyleInterface *stylePlugin = qobject_cast<HbStyleInterface *>(pluginInstance);
        return stylePlugin->updatePrimitive(item, (HbStyle::Primitive)(primitive-info->primitiveBaseId), option);
    }

    switch(primitive){
            case P_PushButton_icon:
                {
                    if (const HbStyleOptionPushButton *opt = 
                        qstyleoption_cast<const HbStyleOptionPushButton*>(option)) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                        //iconItem->setIconName(opt->icon.iconName());
                        iconItem->setIcon(opt->icon); // with this call iconitem refresh issue is there 
                        iconItem->setMode(d->iconMode(opt->state));
                        iconItem->setState(d->iconState(opt->state));
                    }
                break;
                }
            case P_GroupBoxHeading_icon:
                {
                    if (const HbStyleOptionGroupBox *opt = 
                        qstyleoption_cast<const HbStyleOptionGroupBox*>(option)) {
                            HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                            if (opt->collapsed) {
                                iconItem->setIconName(QLatin1String("qtg_small_collapse"));
                            } else {
                                iconItem->setIconName(QLatin1String("qtg_small_expand"));
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
                            iconItem->setIconName(QLatin1String("qtg_small_expand"));
                        } else {
                            iconItem->setIconName(QLatin1String("qtg_small_collapse"));
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
                        iconItem->setMode(d->iconMode(opt->state));
                        iconItem->setState(d->iconState(opt->state));
                        iconItem->setAspectRatioMode(opt->aspectRatioMode);
                        iconItem->setAlignment(opt->alignment);
                    }
                }
                break;
            case P_ToolButton_icon:
                if (const HbStyleOptionToolButton *opt = qstyleoption_cast<const HbStyleOptionToolButton*>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIcon(opt->icon);
                    iconItem->setMode(d->iconMode(opt->state));
                    iconItem->setState(d->iconState(opt->state));
                }
                break;
            case P_ComboBox_text:
                if (const HbStyleOptionComboBox *opt = qstyleoption_cast<const HbStyleOptionComboBox*>(option)) {
                    HbTextItem *comboTextItem = static_cast<HbTextItem*>(item);
                    comboTextItem->setText(opt->text);
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
                        textItem->setTextWrapping(Hb::TextWrapAnywhere);
                    }
                }
                break;
            case P_ProgressDialog_text:
                 {
                   if (const HbStyleOptionProgressDialog *opt = 
					   qstyleoption_cast<const HbStyleOptionProgressDialog*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        textItem->setAlignment( opt->textAlignment );
						if (opt->wrap) {
							textItem->setTextWrapping(Hb::TextWrapAnywhere);
						 } else {
							 textItem->setTextWrapping(Hb::TextNoWrap);
						}
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
                        additionalTextItem->setTextWrapping(Hb::TextWrapAnywhere);
                    }                    
                }
                break;
            case P_PushButton_toucharea:  // Generic implementation, can add other cases here
            case P_ComboBoxButton_toucharea:
            case P_CheckBox_toucharea:
            case P_TitleBar_toucharea:
            case P_SliderElement_touchdecrease:
            case P_SliderElement_touchincrease:
            case P_SliderElement_touchgroove:
            case P_SliderElement_touchhandle:
            case P_ScrollBar_toucharea:
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
                    }
                break;
                }
            case P_DataGroup_description:
                {
                   if (const HbStyleOptionDataGroup *opt = 
                        qstyleoption_cast<const HbStyleOptionDataGroup*>(option)) {
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
            case P_ProgressBar_text: {
                if (const HbStyleOptionProgressBar *opt = qstyleoption_cast<const HbStyleOptionProgressBar*>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setAlignment(Qt::AlignCenter);
                    textItem->setZValue(100);
                    textItem->setText(opt->text);
                }
                break;
            }
            case P_DataGroup_background: {
                if (const HbStyleOptionDataGroupHeadingWidget *opt = 
                    qstyleoption_cast<const HbStyleOptionDataGroupHeadingWidget *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
	                if(!frameItem){
	                    return;
	                }
                    frameItem->setZValue(-1.0);
                    if(opt->pressed) {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_list_pressed"));
                    } else {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_list_parent_normal"));
                    }
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->setGeometry(opt->rect);
                }
                break;
            }
            case P_DataGroupComboBackground: {
                HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                if(!frameItem){
                    return;
                }
                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_list_parent_normal"));
                frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
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
                HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                if(!frameItem){
                    return;
                }

                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_list_normal"));
                frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                break;
            }
            case P_DataItem_label: {
                if (const HbStyleOptionDataFormViewItem *opt = qstyleoption_cast<const HbStyleOptionDataFormViewItem *>(option)) {
                    HbTextItem *textitem = static_cast<HbTextItem*>( item );
                    textitem->setText(opt->label);
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
            case P_IndicatorGroup_background: { // deprecated
                const HbStyleOptionIndicatorGroup *opt = qstyleoption_cast<const HbStyleOptionIndicatorGroup *>(option);

                if (opt) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>( item );
                    iconItem->setAspectRatioMode(Qt::IgnoreAspectRatio);
                    if (opt->direction == Qt::LeftToRight) {
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
                    } else {
                        if (opt->mode == QIcon::Active) {
                            if (opt->transparent) {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_pressed_r"));
                            } else {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_pressed_r"));
                            }
                        } else if (opt->mode == QIcon::Selected) {
                            if (opt->transparent) {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_latched_r"));
                            } else {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_latched_r"));
                            }
                        } else {
                            if (opt->transparent) {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_normal_r"));
                            } else {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_normal_r"));
                            }
                        }
                    }
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

            case P_SignalIndicator_icon:
            case P_IndicatorGroup_signalicon: { // deprecated
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
                            iconItem->setIconName("qtg_status_wlan");
                            break;
                        default:
                            iconItem->setIconName("qtg_mono_offline");
                            break;
                    }
                }
                break;
            }
            case P_SignalLevel_background: {
                if (option) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIconName("qtg_graf_signal_bg");
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
                }
                break;
            }
            case P_BatteryIndicator_icon:
            case P_IndicatorGroup_batteryicon: { // deprecated
                if (option) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIconName("qtg_status_battery");
                }
                break;
            }
            case P_BatteryLevel_background: {
                if (option) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIconName("qtg_graf_battery_bg");
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
                }
                break;
            }
            case P_Softkey_background:{ // deprecated
                const HbStyleOptionSoftKey *opt = qstyleoption_cast<const HbStyleOptionSoftKey *>(option);

                if (opt) {

                    switch (opt->mKey)
                        {
                        case Hb::PrimarySoftKey:
                            // does not exist...
                            break;
                        case Hb::SecondarySoftKey:
                            {
                                if (opt) {
                                    HbIconItem *iconItem = static_cast<HbIconItem*>( item );
                                    if (opt->direction == Qt::LeftToRight) {
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
                                    } else {
                                        if (opt->transparent) {
                                            if (opt->mode == QIcon::Active) {
                                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_pressed_l"));
                                            } else {
                                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_normal_l"));
                                            }
                                        } else {
                                            if (opt->mode == QIcon::Active) {
                                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_pressed_l"));
                                            } else {
                                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_normal_l"));
                                            }
                                         }
                                    }
                                }
                            }
                            break;
                        case Hb::MiddleSoftKey:
                            // does not exist...
                            break;
                        default:
                            break;
                        }
                }
                break;
            }

            case P_NavigationButton_background: {
                const HbStyleOptionNavigationButton *opt = qstyleoption_cast<const HbStyleOptionNavigationButton *>(option);
                    if (opt) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>( item );
                        iconItem->setAspectRatioMode(Qt::IgnoreAspectRatio);
                        if (opt->direction == Qt::LeftToRight) {
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
                        } else {
                            if (opt->transparent) {
                                if (opt->mode == QIcon::Active) {
                                    iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_pressed_l"));
                                } else {
                                    iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_normal_l"));
                                }
                            } else {
                                if (opt->mode == QIcon::Active) {
                                    iconItem->setIconName(QLatin1String("qtg_fr_titlebar_pressed_l"));
                                } else {
                                    iconItem->setIconName(QLatin1String("qtg_fr_titlebar_normal_l"));
                                }
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
                    if (opt->direction == Qt::LeftToRight) {
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
                    } else {
                        if (opt->mode == QIcon::Active) {
                            if (opt->transparent) {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_pressed_r"));
                            } else {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_pressed_r"));
                            }
                        } else if (opt->mode == QIcon::Selected) {
                            if (opt->transparent) {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_latched_r"));
                            } else {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_latched_r"));
                            }
                        } else {
                            if (opt->transparent) {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_normal_r"));
                            } else {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_normal_r"));
                            }
                        }
                    }
                }
                break;
            }

            case P_TitlePane_text: {
                const HbStyleOptionTitlePane *opt = qstyleoption_cast<const HbStyleOptionTitlePane *>(option);

                if (opt) {
                    HbMarqueeItem *marqueeItem = static_cast<HbMarqueeItem*>( item );
                    if (marqueeItem) {
                        marqueeItem->setText(opt->caption);
                        marqueeItem->startAnimation();
                    }
                }
                break;
            }

            case P_TitlePane_icon: {
                const HbStyleOptionTitlePane *opt = qstyleoption_cast<const HbStyleOptionTitlePane *>(option);
                if (opt) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIconName("qgn_indi_input_arrow_down");
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
                     int margin = 4;
                     QRectF focusRect  = opt->rect.adjusted(-margin , -margin , margin , margin);
                     iconItem->setGeometry(focusRect);
                     iconItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                 }
                break;
            }
            case P_ToolButton_frame:{
                const HbStyleOptionToolButton *opt = qstyleoption_cast<const HbStyleOptionToolButton *>(option);
                HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                frameItem->frameDrawer().setFillWholeRect(true);
                frameItem->setGeometry(opt->rect);
                if (opt->orientation == Qt::Vertical) {
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                } else {
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                }

                QStringList list;
                QString frameGraphicsFooter;
                QIcon::Mode mode = d->iconMode(opt->state);
                QIcon::State state = d->iconState(opt->state);

                if (!opt->customBackground.isNull()) {
                    QString customName = opt->customBackground.iconName(mode, state);
                    frameItem->frameDrawer().setFrameGraphicsName(customName);
                    break;
                }

                if (!opt->toolBarPosition) {
                    QString frameGraphicsName;
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
                    frameGraphicsHeader = opt->orientation == Qt::Vertical ? QLatin1String("qtg_fr_tb_h_"):
                                                                             QLatin1String("qtg_fr_tb_v_");
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

                break;
            }



            case P_SliderTickMark_majoricon: {
                 if (const HbStyleOptionSlider *opt = 
                        qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbIconItem *iconItem = qgraphicsitem_cast<HbIconItem*>(item);
                        iconItem->setMode(d->iconMode(opt->state));
                        if ( opt->orientation ==Qt::Horizontal ){
                            iconItem->setIconName( "qtg_graf_slider_h_tick_major" );
                        } else {
                            iconItem->setIconName ( "qtg_graf_slider_v_tick_major" );
                        }
                        iconItem->setState(d->iconState(opt->state));
                    }
                 break;
                                             
             }

            case P_SliderTickMark_minoricon: {
                 if (const HbStyleOptionSlider *opt = 
                        qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbIconItem *iconItem = qgraphicsitem_cast<HbIconItem*>(item);
                        iconItem->setMode(d->iconMode(opt->state));
                        if ( opt->orientation ==Qt::Horizontal ){
                            iconItem->setIconName( "qtg_graf_slider_h_tick_minor" );
                        } else {
                            iconItem->setIconName ( "qtg_graf_slider_v_tick_minor" );
                        }
                        iconItem->setState(d->iconState(opt->state));
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
                        if(!opt->groovePressed){
                            name = QLatin1String("qtg_fr_slider_h_frame_normal");
                        }
                        else {
                            name = QLatin1String("qtg_fr_slider_h_frame_pressed");
                        }
                    } else if (opt->orientation == Qt::Vertical) {
                        if(!opt->groovePressed){
                            name = QLatin1String("qtg_fr_slider_v_frame_normal");
                        }
                        else {
                            name = QLatin1String("qtg_fr_slider_v_frame_pressed");
                        }
                        type = HbFrameDrawer::ThreePiecesVertical;
                    }

                    frameItem->frameDrawer().setFrameGraphicsName(name);
                    frameItem->frameDrawer().setFrameType(type);
                    frameItem->frameDrawer().setFillWholeRect(true);
                    break;
                }
            }

            case P_Slider_thumb: {
                 if (const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider *>(option)){
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    // Set pixmap name and bounding rect
                    QString thumbPath;
                    if(opt->thumbPath.isNull())
                    {
                        thumbPath=d->logicalName(HbStyle::P_Slider_thumb, option);
                    }
                    else
                    {
                        thumbPath=opt->thumbPath;
                    }
                    iconItem->setIconName(thumbPath);
                    iconItem->setAspectRatioMode(Qt::KeepAspectRatio);
                 }
                break;
            }
            case HbStyle::P_SliderElement_increase:
                {
                    if (const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                        iconItem->setZValue(1.0);
                        iconItem->setMode(d->iconMode(opt->state));
                        iconItem->setState(d->iconState(opt->state));
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
            case HbStyle::P_SliderElement_decrease:
                {
                    if (const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                        iconItem->setZValue(1.0);
                        iconItem->setMode(d->iconMode(opt->state));
                        iconItem->setState(d->iconState(opt->state));
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
            case HbStyle::P_SliderElement_icon:
                {
                    if (const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                        iconItem->setZValue(1.0);
                        iconItem->setMode(d->iconMode(opt->state));
                        iconItem->setState(d->iconState(opt->state));
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
                    textItem->setTextWrapping(Hb::TextWrapAnywhere);
                    textItem->setText(opt->text);
                }
                break;

            case P_CheckBox_icon: {
                if (const HbStyleOptionCheckBox *opt = qstyleoption_cast<const HbStyleOptionCheckBox *>(option)){
                    HbIconItem *iconItem = static_cast<HbIconItem *>(item);
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
                break;
            }
            case P_ItemViewItem_radiobutton:
            case P_ItemViewItem_checkbox:
            case P_ItemViewItem_selection:{
                if (const HbStyleOptionAbstractViewItem *opt = qstyleoption_cast<const HbStyleOptionAbstractViewItem *>(option)){
                    HbIconItem *iconItem = static_cast<HbIconItem *>(item);
                    iconItem->setAlignment(Qt::AlignCenter);

                    if (opt->viewItemType == Hb::ItemType_RadioButtonListViewItem) {
                        if (opt->checkState == Qt::Checked) {
                            iconItem->setState(QIcon::On);
                            iconItem->setIconName(QLatin1String("qtg_small_radio_selected"));
                        } else {
                            iconItem->setState(QIcon::Off);
                            iconItem->setIconName(QLatin1String("qtg_small_radio_unselected"));
                        }
                    } else {
                        if (opt->checkState == Qt::Checked) {
                            iconItem->setState(QIcon::On);
                            if (opt->singleSelectionMode) {
                                iconItem->setIconName(QLatin1String("qtg_small_tick"));
                            }
                            else {
                                iconItem->setIconName(QLatin1String("qtg_small_selected"));
                            }
                        } else if (opt->checkState == Qt::PartiallyChecked) {
                            iconItem->setState(QIcon::On);
                            iconItem->setIconName(QLatin1String("qtg_small_selected_partial"));
                        } else {
                            iconItem->setState(QIcon::Off);
                            if (opt->singleSelectionMode) {
                                iconItem->setIconName(QLatin1String(""));
                            }
                            else {
                                iconItem->setIconName(QLatin1String("qtg_small_unselected"));
                            }
                        }
                    }
                }
                break;
            }

            case P_LineEdit_frame_normal:{
                if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_lineedit_normal"));
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                }
            break;
            }

            case P_LineEdit_frame_highlight:{
                if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_lineedit_highlight"));
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                }
            break;
            }

            case P_TextEdit_frame_normal:{
                if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_textedit_normal"));
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                }
            break;
            }

            case P_TextEdit_frame_highlight:{
                if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_textedit_highlight"));
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
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

            case P_MenuItem_frame:{
                        HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_list_normal"));
                    break;
                }
            case P_MenuItem_focus:{
                        HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_list_pressed"));
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

            case P_MenuItem_submenuindicator:
                if (const HbStyleOptionMenuItem *opt = qstyleoption_cast<const HbStyleOptionMenuItem *>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    Q_UNUSED(opt)
                    iconItem->setIconName(QLatin1String("qtg_mono_options_menu"));
                }
                break;

            case P_MenuItem_checkindicator:
                if (const HbStyleOptionMenuItem *opt = qstyleoption_cast<const HbStyleOptionMenuItem *>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    if (opt->checked) {
                        iconItem->setIconName(QLatin1String("qtg_small_selected"));
                    } else {
                        iconItem->setIcon(HbIcon());
                    }
                }
                break;

            case P_MenuItem_separator:
                if (const HbStyleOptionMenuItem *opt = qstyleoption_cast<const HbStyleOptionMenuItem *>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    Q_UNUSED(opt)
                    iconItem->setAspectRatioMode(Qt::IgnoreAspectRatio);
                    iconItem->setAlignment(Qt::AlignCenter);
                    iconItem->setIconName(QLatin1String("qtg_graf_popup_separator"));
                }
                break;

            case P_ScrollBar_groove:
                if (const HbStyleOptionScrollBar *opt = qstyleoption_cast<const HbStyleOptionScrollBar *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    if (opt->interactive) {
                        if (opt->groovePressed) {
                            if (opt->orientation == Qt::Vertical) {
                                frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_v_active_frame_pressed"));
                            } else {
                                frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_h_active_frame_pressed"));
                            }
                        } else {
                            if (opt->orientation == Qt::Vertical) {
                                frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_v_active_frame_normal"));
                            } else {
                                frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_h_active_frame_normal"));
                            }
                        }
                    } else {
                        if (opt->orientation == Qt::Vertical) {
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_v_frame"));
                        } else {
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_h_frame"));
                        }
                    }

                    frameItem->frameDrawer().setFillWholeRect(true);
                }
                break;

            case P_ScrollBar_handle:
                if (const HbStyleOptionScrollBar *opt = qstyleoption_cast<const HbStyleOptionScrollBar *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    if (opt->interactive) {
                        if (opt->thumbPressed) {
                            if (opt->orientation == Qt::Vertical) {
                                frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_v_active_handle_pressed"));
                            } else {
                                frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_h_active_handle_pressed"));
                            }
                        } else {
                            if (opt->orientation == Qt::Vertical) {
                                frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_v_active_handle_normal"));
                            } else {
                                frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_h_active_handle_normal"));
                            }
                        }
                    } else {
                        if (opt->orientation == Qt::Vertical) {
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_v_handle"));
                        } else {
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_h_handle"));
                        }
                    }
                    frameItem->frameDrawer().setFillWholeRect(true);
                }
                break;

            case P_Popup_background:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup"));
                    frameItem->setGeometry(opt->boundingRect);
                }
                break;
            case P_Popup_background_weak:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_secondary"));
                    frameItem->setGeometry(opt->boundingRect);
                }
                break;

            case P_Popup_heading_frame:
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

            case P_ComboBoxPopup_background:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                        if(!frameItem){
                           return;
                        }
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_secondary"));
                    frameItem->setGeometry(opt->boundingRect);
                }
                break;

            case P_ToolTip_background:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_preview"));
                    frameItem->setGeometry(opt->boundingRect);
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
                    textItem->setTextWrapping(opt->wrappingText);
                    textItem->setText(opt->text);
                }
                break;
            case P_NotificationDialog_title:
                if (const HbStyleOptionNotificationDialog *opt =
                        qstyleoption_cast<const HbStyleOptionNotificationDialog *>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);

                    textItem->setAlignment(opt->titleAlignment);
                    textItem->setFontSpec(HbFontSpec(HbFontSpec::Primary));
                    textItem->setTextWrapping(opt->wrappingTitle);
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
                    icon->setAlignment(opt->iconAlignment);
                }

                break;

            
            case P_MessageBox_text:
                if (const HbStyleOptionMessageBox *opt = static_cast<const HbStyleOptionMessageBox *>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setAlignment(opt->textAlignment);

                    if (opt->textWrapping) {
                        textItem->setTextWrapping(Hb::TextWrapAnywhere);
                    } else {
                        textItem->setTextWrapping(Hb::TextNoWrap);
                    }
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
                                    icon->setIcon(HbIcon(QLatin1String("note_info")));
                                    break;
                                case HbStyleOptionMessageBox::MessageTypeQuestion:
                                    icon->setIcon(HbIcon(QLatin1String("qtg_large_question_mark")));
                                    break;
                                case HbStyleOptionMessageBox::MessageTypeWarning:
                                    icon->setIcon(HbIcon(QLatin1String("note_warning")));
                                    break;
                                default:
                                    break;
                            }
									                            
                        }
						else {                 
                            icon->setIcon(opt->icon);
                        }
                        icon->setAlignment(opt->iconAlignment);
                }
                break;
            case P_ItemViewItem_background:
                if (const HbStyleOptionAbstractViewItem *opt = qstyleoption_cast<const HbStyleOptionAbstractViewItem *>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setZValue(-3.0);

                    if (opt->background.canConvert<HbIcon>()){
                        iconItem->setIcon(opt->background.value<HbIcon>());
                    } else if (opt->background.canConvert<QBrush>()){
                        iconItem->setBrush(opt->background.value<QBrush>());
                    }
                }

                break;
            case P_ListViewItem_text:
                if (const HbStyleOptionListViewItem *opt = qstyleoption_cast<const HbStyleOptionListViewItem *>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    
                    // needs set word wrapping explicitly, if multiple lines supported
                    if (opt->index == 1) {
                        Hb::TextWrapping wrapping = Hb::TextNoWrap;
                        if (    opt->minimumLines != 1
                            ||  opt->maximumLines != 1) {
                            wrapping = Hb::TextWordWrap;
                        }
                        textItem->setTextWrapping(wrapping);
                        textItem->setMinimumLines(opt->minimumLines);
                        textItem->setMaximumLines(opt->maximumLines);
                    }
                    setItemName(textItem, QLatin1String("text-") + QString::number(opt->index + 1));
                    textItem->setText(opt->content.value<QString>());
                }
                break;
            case P_ListViewItem_richtext:
                if (const HbStyleOptionListViewItem *opt = qstyleoption_cast<const HbStyleOptionListViewItem *>(option)) {
                    HbRichTextItem *label = static_cast<HbRichTextItem*>(item);
                    label->setText(opt->content.value<QString>());

                    setItemName(label, QLatin1String("text-") + QString::number(opt->index + 1));
                }
                break;
            case P_ListViewItem_icon:
                if (const HbStyleOptionListViewItem *opt = qstyleoption_cast<const HbStyleOptionListViewItem *>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    
                    setItemName(iconItem, QLatin1String("icon-") + QString::number(opt->index + 1));
                    iconItem->setZValue(opt->index + 1);

                    if (opt->content.canConvert<HbIcon>()){
                        iconItem->setIcon(opt->content.value<HbIcon>());
                    } if (opt->content.canConvert<QIcon>()){
                        iconItem->setIcon(opt->content.value<QIcon>());
                    }
                }
                break;
            case P_GridViewItem_icon:
                if (const  HbStyleOptionGridViewItem*opt = qstyleoption_cast<const HbStyleOptionGridViewItem*>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIcon(opt->icon);
                    iconItem->setAlignment(Qt::AlignCenter);
                    iconItem->setAspectRatioMode(Qt::KeepAspectRatio);
                }
                break;

            case P_ColorGridViewItem_colorIcon:
                if (const  HbStyleOptionColorGridViewItem* opt = qstyleoption_cast<const HbStyleOptionColorGridViewItem*>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    if(!iconItem)
                        return;
                    iconItem->setColor(opt->color);
                    iconItem->setZValue(iconItem->parentItem()->zValue()+2);
                }
                break;
            case P_ColorGridViewItem_borderIcon:
                if (const  HbStyleOptionColorGridViewItem* opt = qstyleoption_cast<const HbStyleOptionColorGridViewItem*>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    if(!iconItem)
                        return;
                    iconItem->setIconName(opt->borderIcon);
                    iconItem->setColor(opt->borderColor);
                    iconItem->setZValue(iconItem->parentItem()->zValue()+1);
                }
                break;
            case P_ColorGridViewItem_checkIcon:
                if (const  HbStyleOptionColorGridViewItem* opt = qstyleoption_cast<const HbStyleOptionColorGridViewItem*>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    if(!iconItem)
                        return;
                    Q_UNUSED(opt);
                    iconItem->setZValue(iconItem->parentItem()->zValue()+3);
                }
                break;


            case P_GridViewItem_text:
                if (const  HbStyleOptionGridViewItem*opt = qstyleoption_cast<const HbStyleOptionGridViewItem*>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setAlignment(Qt::AlignCenter);
                    textItem->setTextWrapping(Hb::TextWrapAnywhere);
                    textItem->setText(opt->text);
                }
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
            case P_ToolBarExtension_background:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_tb_ext"));
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
                if (const HbStyleOptionProgressBar *opt = qstyleoption_cast<const HbStyleOptionProgressBar *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);

                    if(opt->isSlider) {
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::OnePiece);
                        if(opt->orientation == Qt::Horizontal){
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_graf_progslider_unrated"));
                        }
                        else{
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_graf_vprogslider_unrated"));
                        }
                    } else {
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                        if(opt->orientation == Qt::Horizontal){
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progbar_h_frame"));
                        }
                        else{
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                            // frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_vprogbar_frame"));
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progbar_v_frame"));
                        }
                    }
                    frameItem->frameDrawer().setFillWholeRect(true);
                    //frameItem->setZValue(-2.0);
                }
                break;
            case P_ProgressBar_track:

                if (const HbStyleOptionProgressBar *opt = qstyleoption_cast<const HbStyleOptionProgressBar *>(option)) {
                    HbProgressTrackItem* frameItem = static_cast<HbProgressTrackItem*>(item);
                    if(!frameItem->isVisible()) {
                        break;
                    }
                    if(opt->isSlider) {
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::OnePiece);
                        if(opt->orientation == Qt::Horizontal){
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_graf_progslider_rated"));
                        }
                        else{
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_graf_vprogslider_rated"));
                        }
                    } else {
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                        if(opt->secondoryType){
                            if(opt->orientation == Qt::Horizontal){
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progbar_down"));

                            }
                            else{

                                frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progress_v_frame_second"));
                            }
                            //frameItem->setZValue(-2.0);
                        }
                        else{
                            if(opt->orientation == Qt::Horizontal){
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progbar_h_filled"));
                            }
                            else{
                                frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progbar_v_filled"));
                            }
                        }
                    }
                    frameItem->frameDrawer().setFillWholeRect(true);
                    frameItem->setMaximum(opt->maximum);
                    frameItem->setMinimum(opt->minimum);
                    frameItem->setValue(opt->progressValue);
                    frameItem->setInverted(opt->inverted);
                    frameItem->setOrientation(opt->orientation);
                    frameItem->update();
                }
                break;
            case P_ProgressBar_waittrack:
                if (const HbStyleOptionProgressBar *opt = qstyleoption_cast<const HbStyleOptionProgressBar *>(option)) {                   
                    HbRepeatIconItem *iconItem = static_cast<HbRepeatIconItem*>(item);
                    iconItem->setOrientation(opt->orientation);
                    if( !iconItem->isVisible() ) {
                        break;
                    }
                    if(opt->orientation == Qt::Horizontal){
                        iconItem->setName(QLatin1String("qtg_graf_progbar_h_wait"));
                    }
                    else{
                        iconItem->setName(QLatin1String("qtg_graf_progbar_v_wait"));
                    }
                    if(opt->stopWaitAnimation){
                        iconItem->stopAnimation();
                    }

                /*    HbFrameDrawer drawer("qtg_fr_progbar_mask",HbFrameDrawer::ThreePiecesHorizontal);
                    drawer.setFillWholeRect(true);
                    QPixmap track(opt->frameRect.size().toSize());
                    track.fill(Qt::black);
                    QPainter p;
                    p.begin(&track);
                    drawer.paint(&p, opt->frameRect);
                    p.end();
                    iconItem->setIconHeight(opt->frameRect.height());//set height of the image drawer
                    iconItem->setGeometry(opt->frameRect);//set geometry of QGI
                    
                    QImage i=track.toImage();
                    i.invertPixels();
                    iconItem->setMask(QPixmap::fromImage(i));*/
                }
                break;

            case P_ProgressBar_slidertrack:
                if (const HbStyleOptionProgressBar *opt = qstyleoption_cast<const HbStyleOptionProgressBar *>(option)) {
                    HbProgressTrackItem* frameItem = static_cast<HbProgressTrackItem*>(item);

                    if(opt->orientation == Qt::Horizontal){
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progbar_h_filled"));
                    }
                    else{
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progbar_v_filled"));
                    }
                    frameItem->frameDrawer().setFillWholeRect(true);
                    frameItem->setMaximum(opt->maximum);
                    frameItem->setMinimum(opt->minimum);
                    frameItem->setValue(opt->sliderValue);
                    frameItem->setInverted(opt->inverted);
                    frameItem->setMaskWidth(opt->maskWidth);//maskWidth != 0 when  progress handle's moving.
                    frameItem->setOrientation(opt->orientation);
                    frameItem->update();
                    //frameItem->setZValue(-1.0);
                }
                break;

            case P_ProgressBar_mintext: {
                 if (const HbStyleOptionProgressBar *opt = 
                        qstyleoption_cast<const HbStyleOptionProgressBar*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        if(!item) {
                            return;
                        }
                        textItem->setTextWrapping(Hb::TextWrapAnywhere);
                        textItem->setText(opt->minText);                        	
                    }
                 break;
            }

            case P_ProgressBar_maxtext: {
                if (const HbStyleOptionProgressBar *opt = 
                        qstyleoption_cast<const HbStyleOptionProgressBar*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        if(!item) {
                            return;
                        }
                        textItem->setTextWrapping(Hb::TextWrapAnywhere);
                        textItem->setText(opt->maxText);                        	
                    }
                break;
            }

            case P_RatingSlider_frame:{
                if (const HbStyleOptionRatingSlider *opt = qstyleoption_cast<const HbStyleOptionRatingSlider *>(option)) {                   
                    HbRepeatItem *repeatItem = static_cast<HbRepeatItem*>(item);
                    repeatItem->setRepeatingNumber(opt->noOfStars);
                    if(opt->unRatedGraphicsName != QString()){
                        repeatItem->setName(opt->unRatedGraphicsName);
                    }
                    else {
                        repeatItem->setName(QLatin1String("qtg_graf_ratingslider_unrated"));
                    }
                    repeatItem->setGeometry(opt->boundingRect);
                    repeatItem->update();
                }
                break;
            }

            case P_RatingSlider_track:{
                if (const HbStyleOptionRatingSlider *opt = qstyleoption_cast<const HbStyleOptionRatingSlider *>(option)) {                   
                    HbRepeatMaskItem *repeatItem = static_cast<HbRepeatMaskItem*>(item);
                    repeatItem->setMaskValue(opt->progressValue);
                    repeatItem->setMaximum(opt->noOfIntervals);
                    repeatItem->setInverted(opt->inverted);
                    repeatItem->setRepeatingNumber(opt->noOfStars);
                    if(opt->ratedGraphicsName != QString()){
                        repeatItem->setName(opt->ratedGraphicsName);
                    }
                    else {
                        repeatItem->setName(QLatin1String("qtg_graf_ratingslider_rated"));
                    }
                    repeatItem->setGeometry(opt->boundingRect);
                    repeatItem->update();
                }
                break;
            }

   		case P_ProgressSlider_handle:
					 if (const HbStyleOptionProgressBar *opt = qstyleoption_cast<const HbStyleOptionProgressBar *>(option)) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>(item);
						iconItem->setIconName(QLatin1String("qtg_graf_progslider_handle_normal"));
                        Q_UNUSED(opt)
						}
					    break;
         case P_ProgressSlider_handleicon:
                    if (const HbStyleOptionProgressBar *opt = qstyleoption_cast<const HbStyleOptionProgressBar *>(option)) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>(item);

                        if(!opt->thumbIcon.isNull()) {                           
                            HbIcon icon(opt->thumbIcon.iconName());                       
                            iconItem->setIconName(opt->thumbIcon.iconName());
                        }
                        else {
                            switch(opt->sliderState) {
                            case HbStyleOptionProgressBar::SliderStatePlayNormal:                            
                                iconItem->setIconName(QLatin1String("qtg_mono_play"));
                                break;
                            case HbStyleOptionProgressBar::SliderStatePlayPressed:
                                iconItem->setIconName(QLatin1String("qtg_mono_play"));
                                break;
                            case HbStyleOptionProgressBar::SliderStatePauseNormal:
                                iconItem->setIconName(QLatin1String("qtg_mono_pause"));
                                break;
                            case HbStyleOptionProgressBar::SliderStatePausePressed:
                                iconItem->setIconName(QLatin1String("qtg_mono_pause"));
                                break;
                            }
                        }
                    }

                    break;
            case P_QueryInputMode_image:

             /*   if (const HbStyleOptionInputPopup *opt = qstyleoption_cast<const HbStyleOptionInputPopup *>(option)){
                    if (HbIconItem *iconItem = static_cast<HbIconItem*>(item)) {
                       iconItem->setIcon(*opt->modeIcon);
                       iconItem->setPos(opt->editRect.width() - opt->modeIcon->width() - 6,opt->editRect.top()-3);
                    }
                }*/
                break;
            case P_ItemViewItem_frame: 
                if (const HbStyleOptionAbstractViewItem *opt = qstyleoption_cast<const HbStyleOptionAbstractViewItem *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->setZValue(-4.0);

                    if (opt->background.canConvert<HbFrameBackground>()) {
                        HbFrameBackground frame = opt->background.value<HbFrameBackground>();
                        frameItem->frameDrawer().setFrameType(frame.frameType());
                        frameItem->frameDrawer().setFrameGraphicsName(frame.frameGraphicsName());
                    } else if (opt->viewItemType == Hb::ItemType_TreeViewItem) {
                        if (opt->modelItemType == Hb::ParentItem) {
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                            frameItem->frameDrawer().setFrameGraphicsName( opt->insidePopup ? 
                                QLatin1String("qtg_fr_popup_list_parent_normal") : QLatin1String("qtg_fr_list_parent_normal"));
                        } else if (opt->modelItemType == Hb::SeparatorItem) {
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_list_separator"));
                        } else {
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                            frameItem->frameDrawer().setFrameGraphicsName( opt->insidePopup ? 
                                QLatin1String("qtg_fr_popup_list_normal") : QLatin1String("qtg_fr_list_normal"));
                        }
                    } else if ( opt->viewItemType == Hb::ItemType_ListViewItem
                                || opt->viewItemType == Hb::ItemType_RadioButtonListViewItem) {
                        if (opt->modelItemType == Hb::SeparatorItem) {
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_list_separator"));
                        } else {
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                            frameItem->frameDrawer().setFrameGraphicsName( opt->insidePopup ? 
                                QLatin1String("qtg_fr_popup_list_normal") : QLatin1String("qtg_fr_list_normal"));
                        }
                    } else if (opt->viewItemType == Hb::ItemType_GridViewItem
								|| opt->viewItemType == HbPrivate::ItemType_ColorGridViewItem) {
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->frameDrawer().setFrameGraphicsName( opt->insidePopup ? 
                            QLatin1String("qtg_fr_popup_grid_normal") : QLatin1String("qtg_fr_grid_normal"));
                    }
                    else{
                        frameItem->frameDrawer().setFrameGraphicsName(QString());
                    }
                }
                break;
            case P_ItemViewItem_focus:
                if (const HbStyleOptionAbstractViewItem *opt = qstyleoption_cast<const HbStyleOptionAbstractViewItem *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->setZValue(-1.0);
                    frameItem->setGeometry(opt->boundingRect);

                    if (opt->viewItemType == Hb::ItemType_TreeViewItem
                        || opt->viewItemType == Hb::ItemType_ListViewItem
                        || opt->viewItemType == Hb::ItemType_RadioButtonListViewItem) {
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->frameDrawer().setFrameGraphicsName( opt->insidePopup ?
                            QLatin1String("qtg_fr_popup_list_pressed") : QLatin1String("qtg_fr_list_pressed"));
                    } else if (     opt->viewItemType == Hb::ItemType_GridViewItem
                                ||  opt->viewItemType == HbPrivate::ItemType_ColorGridViewItem) {
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->frameDrawer().setFrameGraphicsName(opt->insidePopup ?
                            QLatin1String("qtg_fr_popup_grid_pressed") : QLatin1String("qtg_fr_grid_pressed"));
                    } else {
                        setItemName(frameItem,QString());
                        frameItem->frameDrawer().setFrameGraphicsName(QString());
                    }
                }
                break;
            case P_SelectionControl_selectionstart://fallthrough
            case P_SelectionControl_selectionend://fallthrough
            {
                if (HbIconItem *iconItem = qgraphicsitem_cast<HbIconItem*>(item)) {
                    iconItem->setSize(iconItem->preferredSize());
                }
                break;
            }
            case P_TreeViewItem_expandicon:
                if (const HbStyleOptionTreeViewItem *opt = qstyleoption_cast<const HbStyleOptionTreeViewItem *>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    if (opt->expanded) {
                        iconItem->setIconName(opt->insidePopup ? QLatin1String("qtg_mono_expand") : QLatin1String("qtg_small_expand"));
                    } else {
                        iconItem->setIconName(opt->insidePopup ? QLatin1String("qtg_mono_collapse") : QLatin1String("qtg_small_collapse"));
                    }
                }
                break;
            case P_ScrollArea_continuationbottom:
                if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_graf_list_mask_b"));
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::OnePiece);
                    frameItem->setZValue(1);
                }
                break;
            case P_ScrollArea_continuationtop:
                if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_graf_list_mask_t"));
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::OnePiece);
                    frameItem->setZValue(1);
                }
                break;
            case P_ScrollArea_continuationright:
                if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_graf_list_mask_r"));
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::OnePiece);
                    frameItem->setZValue(1);
                }
                break;
            case P_ScrollArea_continuationleft:
                if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_graf_list_mask_l"));
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::OnePiece);
                    frameItem->setZValue(1);
                }
                break;

            case P_TumbleView_background:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption*>(option)) {
                    if(HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {
                        frameItem->frameDrawer().setFrameGraphicsName("qtg_fr_tumbler_bg");
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
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption*>(option)) {
                    if(HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {
                        frameItem->frameDrawer().setFrameGraphicsName("qtg_graf_tumbler_highlight_sec");
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::OnePiece);
                        frameItem->setZValue(2);
                        //TODO:temp fix, issue with css rule picking in derived class
                        
                        //frameItem->setGeometry(0,(opt->boundingRect.height()-frameItem->boundingRect().height())/2,opt->boundingRect.width(),opt->boundingRect.height());
                        Q_UNUSED(opt);
                    }
                    
                }
                break;            
            case P_IndexFeedback_popup_text:
                if (const HbStyleOptionIndexFeedback *opt = qstyleoption_cast<const HbStyleOptionIndexFeedback *>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setFontSpec(opt->fontSpec);
                    textItem->setGeometry(opt->textRect);
                    textItem->setText(opt->text);
                }
                break;
            case P_IndexFeedback_popup_background:
                if (const HbStyleOptionIndexFeedback *opt = qstyleoption_cast<const HbStyleOptionIndexFeedback *>(option)) {
                    if (HbFrameItem* frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {
                        frameItem->setGeometry(opt->popupRect);
                    }
                }
                break;
            case P_DateTimePicker_background:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption*>(option)) {
                    if(HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {
                        frameItem->frameDrawer().setFrameGraphicsName("qtg_fr_tumbler_bg");
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->setZValue(-5);
                        //TODO:temp fix, issue with css rule picking in derived class
                        //frameItem->setGeometry(opt->boundingRect);
                        Q_UNUSED(opt);
                    }
                }
                break;
            case P_DateTimePicker_frame: 
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption*>(option)) {
                    if(HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {
                        frameItem->frameDrawer().setFrameGraphicsName("qtg_fr_tumbler_overlay");
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->setZValue(1);
                        //TODO:temp fix, issue with css rule picking in derived class
                        //frameItem->setGeometry(opt->boundingRect);
                        Q_UNUSED(opt);
                    }
                }
                break;
            case P_DateTimePicker_separator:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption*>(option)) {
                    if(HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {
                        frameItem->frameDrawer().setFrameGraphicsName("qtg_graf_tumbler_highlight_pri");
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::OnePiece);
                        frameItem->setZValue(0);
                        //TODO:temp fix, issue with css rule picking in derived class

                        //frameItem->setGeometry(0,(opt->boundingRect.height()-frameItem->boundingRect().height())/2,opt->boundingRect.width(),opt->boundingRect.height());
                        Q_UNUSED(opt);
                    }

                }
                break;
            case P_InputDialog_text:
                if (const HbStyleOptionInputDialog *opt = 
                    qstyleoption_cast<const HbStyleOptionInputDialog*>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    if(!item) {
                        return;
                    }
                    textItem->setTextWrapping(Hb::TextWrapAnywhere);
                    textItem->setText(opt->text);                        	
                }
                break;
             case P_InputDialog_additional_text:
                if (const HbStyleOptionInputDialog *opt = 
                    qstyleoption_cast<const HbStyleOptionInputDialog*>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    if(!item) {
                        return;
                    }
                    textItem->setTextWrapping(Hb::TextWrapAnywhere);
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
    const HbDeviceProfile &profile)
{
    quint64 pseudoClass(0);

    if ( profile.orientation() == Qt::Horizontal ) {
        pseudoClass |= HbCss::PseudoClass_Landscape;
    } else {
        pseudoClass |= HbCss::PseudoClass_Portrait;
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
    HbWidget* widget,
    QGraphicsItem *item,
    const QString &name,
    bool layoutDefined) const
{
    if (name.isEmpty() && widget != item) {
        return;
    }

#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polish : -- --";
    if ( widget == item ) {
        qDebug() << "HbStyle::polish : -- polishItem: " << widget->metaObject()->className();
    } else if ( item ) {
        qDebug() << "HbStyle::polish : -- polishItem: " << name << "(existing item)";
    } else {
        qDebug() << "HbStyle::polish : -- polishItem: " << name << "(missing item)";
    }
#endif

    HbDeviceProfile profile(HbDeviceProfile::profile(widget));

    const HbVector<HbCss::Declaration> decl = declarations(styleRules, name, profile);
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polish : -- Number of maching CSS declarations: " << decl.count();
#endif
    HbCss::ValueExtractor extractor(decl, layoutParameters, profile);
    HbCss::GeometryValues geomValues;
    HbCss::PositionValues posValues;

    bool extracted = extractor.extractGeometry(geomValues);
#ifndef HBSTYLE_DEBUG
    Q_UNUSED(extracted);
#endif
#ifdef HBSTYLE_DEBUG
    if ( !extracted ) {
        qDebug() << "HbStyle::polish : -- No geometry information found";
    }
#endif
    extracted = extractor.extractPosition(posValues);
#ifdef HBSTYLE_DEBUG
    if ( !extracted ) {
        qDebug() << "HbStyle::polish : -- No position information found";
    }
#endif

    if ( item ) {
        if (posValues.mFlags & HbCss::ExtractedZValue) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polish : -- Setting zvalue: " << posValues.mZ;
#endif
            item->setZValue(posValues.mZ);
        }
    }

    QGraphicsWidget *gWidget = (item && item->isWidget())
        ? static_cast<QGraphicsWidget*>(item)
        : 0;
    if ( gWidget ) {
        if (posValues.mFlags & HbCss::ExtractedMirroring) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polish : -- Setting mirroring: " << posValues.mMirroring;
#endif
            if(posValues.mMirroring){
                gWidget->unsetLayoutDirection();
            }else{
                gWidget->setLayoutDirection(Qt::LeftToRight);
            }
        }
    }

    HbWidgetBase *hbWidget = qobject_cast<HbWidgetBase*>(gWidget);
    if ( hbWidget ) {
        HbWidgetBasePrivate* hbWidget_p = HbWidgetBasePrivate::d_ptr(hbWidget);
        QFont font;
        HbFontSpec fontSpec;
        int dummy;
        if (extractor.extractFont(&font, &fontSpec, &dummy)) {
            if ( !fontSpec.isNull() ) {
                if ( font == QFont() ) {
#ifdef HBSTYLE_DEBUG
                    qDebug() << "HbStyle::polish : -- Setting fontspec: " << fontSpec.role();
#endif
                    hbWidget->setFontSpec(fontSpec);
                } else {
#ifdef HBSTYLE_DEBUG
                    qDebug() << "HbStyle::polish : -- Setting fontspec with overrides: "
                             << fontSpec.role() << font;
#endif
                    hbWidget->setFont(fontSpec.font().resolve(font));
                }
            } else {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polish : -- Setting explicit font: " << font;
#endif
                hbWidget->setFont(font);
            }
        }

        HbIconItem* icon = qobject_cast<HbIconItem*>(hbWidget);
        if (icon) {
            Qt::AspectRatioMode mode;
            if (extractor.extractAspectRatioMode(&mode) 
				&& !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_IconAspectRatioMode)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polish : -- Setting aspect ratio mode: " << mode;
#endif
                icon->setAspectRatioMode(mode);
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_IconAspectRatioMode, false);
            }
            if(!hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_IconBrush)){
                QBrush brush;
                QString uri;
                HbCss::Repeat repeat = HbCss::Repeat_XY;
                Qt::Alignment alignment = Qt::AlignTop | Qt::AlignLeft;
                HbCss::Attachment attachment = HbCss::Attachment_Scroll;
                HbCss::Origin origin = HbCss::Origin_Padding;
                HbCss::Origin clip = HbCss::Origin_Border;
                if (extractor.extractBackground(&brush, &uri, &repeat, &alignment, &origin, &attachment, &clip)) {
#ifdef HBSTYLE_DEBUG
                    qDebug() << "HbStyle::polish : -- Setting icon background: " << brush;
#endif
                    icon->setBrush( brush );
                } else {
#ifdef HBSTYLE_DEBUG
                    qDebug() << "HbStyle::polish : -- Resetting icon background";
#endif
                    icon->setBrush( QBrush() );
                }
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_IconBrush, false);
            }
        }

        HbTextItem* text = qobject_cast<HbTextItem*>(hbWidget);
        if (text) {
            HbCss::TextValues textValues;
            if ( extractor.extractTextValues( textValues ) ) {
                if ( textValues.mFlags & HbCss::ExtractedLineCountMin
                    && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin)) {
#ifdef HBSTYLE_DEBUG
                    qDebug() << "HbStyle::polish : -- Setting text min lines: " << textValues.mLineCountMin;
#endif
                    text->setMinimumLines( textValues.mLineCountMin );
                    hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin, false);
                }
                if ( textValues.mFlags & HbCss::ExtractedLineCountMax
                    && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMax)) {
#ifdef HBSTYLE_DEBUG
                    qDebug() << "HbStyle::polish : -- Setting text max lines: " << textValues.mLineCountMax;
#endif
                    text->setMaximumLines( textValues.mLineCountMax );
                    hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMax, false);
                }
            }
            if ( posValues.mFlags & HbCss::ExtractedTextAlign 
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextAlign)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polish : -- Setting text alignment: " << posValues.mTextAlignment;
#endif
                text->setAlignment( posValues.mTextAlignment );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextAlign, false);
            }
            if ( posValues.mFlags & HbCss::ExtractedWrapMode 
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextWrapMode)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polish : -- Setting wrap mode : " << posValues.mTextWrapMode;
#endif
                text->setTextWrapping( posValues.mTextWrapMode );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextWrapMode, false);
            }
        }

        HbRichTextItem* richtext = qobject_cast<HbRichTextItem*>(hbWidget);
        if (richtext) {
            if ( posValues.mFlags & HbCss::ExtractedTextAlign
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextAlign)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polish : -- Setting text alignment: " << posValues.mTextAlignment;
#endif
                richtext->setAlignment( posValues.mTextAlignment );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextAlign, false);
            }
            if ( posValues.mFlags & HbCss::ExtractedWrapMode 
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextWrapMode)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polish : -- Setting wrap mode : " << posValues.mTextWrapMode;
#endif
                richtext->setTextWrapping( posValues.mTextWrapMode );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextWrapMode, false);
            }
        }

        HbFrameItem *frame = qobject_cast<HbFrameItem*>(hbWidget);
        if (frame && !frame->frameDrawer().d->testBorderApiProtectionFlag()) {
            qreal borderWidths[HbCss::NumEdges] = { 0.0,0.0,0.0,0.0 };
            QBrush borderColors[HbCss::NumEdges];
            HbCss::BorderStyle borderStyles[HbCss::NumEdges];
            QSize borderRadii[4];

            if (extractor.extractBorder(borderWidths,borderColors,borderStyles,borderRadii)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polish : -- Setting border widths (l,t,r,b):"
                    << borderWidths[HbCss::LeftEdge]
                    << borderWidths[HbCss::TopEdge]
                    << borderWidths[HbCss::RightEdge]
                    << borderWidths[HbCss::BottomEdge];
#endif
                frame->frameDrawer().setBorderWidths(
                    borderWidths[HbCss::LeftEdge],
                    borderWidths[HbCss::TopEdge],
                    borderWidths[HbCss::RightEdge],
                    borderWidths[HbCss::BottomEdge]);
                frame->frameDrawer().d->setBorderApiProtectionFlag(false);
            }
        }
        if ( hbWidget->inherits( "HbLineEdit" ) ) {
            HbCss::TextValues textValues;
            if ( extractor.extractTextValues( textValues ) ) {
                if ( textValues.mFlags & HbCss::ExtractedLineCountMin
                    && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin)) {
#ifdef HBSTYLE_DEBUG
                    qDebug() << "HbStyle::polish : -- Setting text min lines: " << textValues.mLineCountMin;
#endif
                    hbWidget->setProperty( "minRows", textValues.mLineCountMin );
                    hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin, false);
                }
                if ( textValues.mFlags & HbCss::ExtractedLineCountMax
                    && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin)) {
#ifdef HBSTYLE_DEBUG
                    qDebug() << "HbStyle::polish : -- Setting text max lines: " << textValues.mLineCountMax;
#endif
                    hbWidget->setProperty( "maxRows", textValues.mLineCountMax );
                    hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMax, false);
                }
            }
        }
    }

    QGraphicsLayoutItem* lItem = (item && item->isWidget()) ? (QGraphicsLayoutItem*)static_cast<QGraphicsWidget*>(item) : 0;
    if ( !lItem ) {
        lItem = widget->layoutPrimitive(name);
        if ( lItem && !lItem->graphicsItem() ) {
            // assume it is spacer
            static_cast<HbMeshLayout*>(widget->layout())->setItemId( lItem, name );
        }
    }
    if ( lItem ) {
        if ( geomValues.mFlags & HbCss::ExtractedMinW ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polish : -- Setting minimum width: " << geomValues.mMinW;
#endif
            lItem->setMinimumWidth( geomValues.mMinW );
        }
        if ( geomValues.mFlags & HbCss::ExtractedMinH ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polish : -- Setting minimum height: " << geomValues.mMinH;
#endif
            lItem->setMinimumHeight( geomValues.mMinH );
        }
        if ( geomValues.mFlags & HbCss::ExtractedPrefW ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polish : -- Setting preferred width: " << geomValues.mPrefW;
#endif
            lItem->setPreferredWidth( geomValues.mPrefW );
        }
        if ( geomValues.mFlags & HbCss::ExtractedPrefH ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polish : -- Setting preferred height: " << geomValues.mPrefH;
#endif
            lItem->setPreferredHeight( geomValues.mPrefH );
        }
        if ( geomValues.mFlags & HbCss::ExtractedMaxW ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polish : -- Setting maximum width: " << geomValues.mMaxW;
#endif
            lItem->setMaximumWidth( geomValues.mMaxW );
        }
        if ( geomValues.mFlags & HbCss::ExtractedMaxH ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polish : -- Setting maximum height: " << geomValues.mMaxH;
#endif
            lItem->setMaximumHeight( geomValues.mMaxH );
        }
        QSizePolicy itemPol = lItem->sizePolicy();
        if ( geomValues.mFlags & HbCss::ExtractedPolHor ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polish : -- Setting horizontal size policy: " << geomValues.mSizePolicy.horizontalPolicy();
#endif
            itemPol.setHorizontalPolicy(geomValues.mSizePolicy.horizontalPolicy());
        }
        if ( geomValues.mFlags & HbCss::ExtractedPolVer ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polish : -- Setting vertical size policy: " << geomValues.mSizePolicy.verticalPolicy();
#endif
            itemPol.setVerticalPolicy(geomValues.mSizePolicy.verticalPolicy());
        }
        lItem->setSizePolicy(itemPol);
    }

    if (layoutDefined) {
        HbMeshLayout *layout = static_cast<HbMeshLayout*>(widget->layout());
        if ( layout ) {
            if (posValues.mFlags & HbCss::ExtractedLeft) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polish : -- Setting left override: " << posValues.mLeft;
#endif
                layout->overrideSpacing(name, Hb::LeftEdge, posValues.mLeft);
            }
            if (posValues.mFlags & HbCss::ExtractedRight) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polish : -- Setting right override: " << posValues.mRight;
#endif
                layout->overrideSpacing(name, Hb::RightEdge, posValues.mRight);
            }
            if (posValues.mFlags & HbCss::ExtractedTop) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polish : -- Setting top override: " << posValues.mTop;
#endif
                layout->overrideSpacing(name, Hb::TopEdge, posValues.mTop);
            }
            if (posValues.mFlags & HbCss::ExtractedBottom) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polish : -- Setting bottom override: " << posValues.mBottom;
#endif
                layout->overrideSpacing(name, Hb::BottomEdge, posValues.mBottom);
            }
            if (posValues.mFlags & HbCss::ExtractedCenterH) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polish : -- Setting centerh override: " << posValues.mCenterH;
#endif
                layout->overrideSpacing(name, Hb::CenterHEdge, posValues.mCenterH);
            }
            if (posValues.mFlags & HbCss::ExtractedCenterV) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polish : -- Setting centerv override: " << posValues.mCenterV;
#endif
                layout->overrideSpacing(name, Hb::CenterVEdge, posValues.mCenterV);
            }
        }
    }
}


#define NODEPTR_N(x) HbCss::StyleSelector::NodePtr n = {n.ptr = (void *)x};
/*!
    Polishes the appearance of the given \a widget.

    This method should be called only by HbWidget (base class). The inherited
    classes should not call this method directly, but use the HbWidget's
    polish loop instead.
    
    This method reads the CSS and WidgetML definition for a given widget and positions
    the sub-elements inside it. Note you need to set the names for the sub-elements
    with HbStyle::setItemName method before this method is called.

    Note that this method is heavy on the system resources and should be called
    absolutely when necessary by the widget.
    
    \param widget, HbWidget to be polished
    \param params, style parameters to be returned to the caller
*/
void HbStyle::polish(HbWidget *widget, HbStyleParameters &params)
{
    Q_D( HbStyle );
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polish : -- -- -- --";
#endif
    if( !widget ) {
        return;
    }
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polish : Polishing" << widget->metaObject()->className();
#endif

    HbLayeredStyleLoader *styleLoader = HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Layouts);
    if(!styleLoader){
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polish : HbLayeredStyleLoader returned a null pointer.";
#endif
        return;
    }
    HbDeviceProfile profile(HbDeviceProfile::profile(widget));
    NODEPTR_N(widget);

    HbVector<HbCss::StyleRule> styleRules;
    QVariant widgetStyleCache = widget->property( HbStyleRulesCacheId::hbStyleRulesForNodeCache );

    if(widgetStyleCache.isValid()) {
        QString widgetStyleCacheId = widgetStyleCache.toString();
        if (d->styleRulesCache.contains(widgetStyleCacheId)) {
            styleRules = d->styleRulesCache.value(widgetStyleCacheId);
        } else {
            styleRules = styleLoader->styleRulesForNode(n, profile.orientation());
            d->styleRulesCache[widgetStyleCacheId] = styleRules;
        }
    } else {
        styleRules = styleLoader->styleRulesForNode(n, profile.orientation());
    }

    if (!styleRules.count()) {
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polish : No style rules found.";
#endif
        return;
    }
    const HbVector<HbCss::Declaration> decl = declarations(styleRules, "", profile);
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polish : Number of maching CSS declarations: " << decl.count();
#endif
    d->ensureLayoutParameters(profile);

    HbCss::ValueExtractor extractor(decl, d->layoutParameters, profile);
    QString layoutName;
    QString sectionName;

    if ( params.count() ) {
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polish : Extracting parameters.";
#endif
        extractor.extractParameters( params.params(), params.values() );
    }

    bool layoutDefined = extractor.extractLayout(&layoutName, &sectionName);
#ifdef HBSTYLE_DEBUG
    if (!layoutDefined) {
        qDebug() << "HbStyle::polish : Couldn't find layout name for the widget.";
    } else {
        qDebug() << "HbStyle::polish : Layout name: " << layoutName;
    }
#endif

    QStringList meshIds;
    HbMeshLayout *meshLayout(0);
    if (layoutDefined) {

        QString cachedLayoutName = d->widgetLayoutNames[widget];
        bool cached = (cachedLayoutName == layoutName );
        if ( !cached ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "LayoutName cache miss.";
#endif
            HbWidgetStyleLoader *loader = HbWidgetStyleLoader::instance();
            if ( !loader->loadWidgetML(widget, layoutName, sectionName)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polish : Failed to load WidgetML";
#endif
                return;       
            }
            d->widgetLayoutNames[widget] = layoutName;
            if (cachedLayoutName.isNull()) {
                // Cached for the first time. Connect to destroyed signal.
                QObject::connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(widgetDestroyed(QObject*)));
#ifdef HBSTYLE_DEBUG
                qDebug() << "Cached first time. Connected to destroy signal.";
            } else {
                qDebug() << "Cached nth time.";
#endif
            }
        }
        meshLayout = static_cast<HbMeshLayout*>(widget->layout());
        if (cached) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "LayoutName cache hit.";
#endif
            meshLayout->clearItemIds();
        }

        meshLayout->setItemId(meshLayout, "");
        meshIds = meshLayout->nodeIds();

    }

    // polish widget and subitems
    d->polishItem(styleRules, widget, widget, "", false);
    QList<QGraphicsItem*> list = widget->childItems();
    foreach (QGraphicsItem* item, list) {
        QString name = HbStyle::itemName(item);
        if ( meshLayout && !name.isEmpty() ) {
            // Assuming that all items with "itemName" are widgets.
            meshLayout->setItemId(static_cast<QGraphicsWidget*>(item), name);
            // Remove from "meshIds" so that we don't call polishItem
            // twice for this item.
            meshIds.removeAll(name);
        }
        d->polishItem(styleRules, widget, item, name, layoutDefined);
    }
    foreach (QString meshId, meshIds) {
        // These are the "missing" mesh items. Need to call polishItem
        // for them, too, for getting the mesh anchor spacings right.
        // if there are mesh ids, layoutDefined is always true.
        if ( !meshId.isEmpty() ) {
            d->polishItem(styleRules, widget, 0, meshId, true);
        }
    }
}

/*!
    updateThemedItems updates themable children items of widget

    \param styleRules, style-rules of the widget
    \param variableRules, variable-rules of the widget
    \param widget, widget whose themed parameters are supposed to be updated
    \param item, graphics item whose themed parameters are supposed to be updated
*/
void HbStylePrivate::updateThemedItems( const HbVector<HbCss::StyleRule> &styleRules,
    QGraphicsItem *item ) const
{
    
    QString name = HbStyle::itemName(item);
    if (name.isEmpty() ) {
        return ;
    }
  
    HbTextItem* text = qgraphicsitem_cast<HbTextItem*>( item );
    HbIconItem* iconItem = qgraphicsitem_cast<HbIconItem*>( item );
    HbMarqueeItem* marqueeItem = qgraphicsitem_cast<HbMarqueeItem*>( item );
    if(! (text || iconItem || marqueeItem ) ){
        return;
    }
    
    HbDeviceProfile profile;
    const HbVector<HbCss::Declaration> decl = declarations(styleRules, name, profile);

#ifdef HBSTYLE_DEBUG
   qDebug() << "HbStyle::updateThemedItems : -- Number of maching CSS declarations: " << decl.count();
#endif
   ensureColorParameters();
   HbCss::ValueExtractor extractor(decl, colorParameters, profile);
   
   QColor col;
   bool extracted = extractor.extractColor( &col );
   //apply the themed color to text-item
   if(text) {   
#ifdef HBSTYLE_DEBUG
       if ( !extracted ) {
            qDebug() << "HbStyle::getColor : -- No color information found";
       }
#endif    
       if( extracted && col.isValid() && !HbWidgetBasePrivate::d_ptr(text)->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor)){
           text->setTextColor(col);
           HbWidgetBasePrivate::d_ptr(text)->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor, false);
       }
   }   
   if(iconItem){
       //applying color to mono-colorised icons from theme
       if( extracted && col.isValid()) {
           iconItem->setColor( col ); 
       }
   }
   if(marqueeItem){
       //applying color to the marquee-item from theme
       if(extracted && col.isValid() && !HbWidgetBasePrivate::d_ptr(marqueeItem)->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor)) {
           marqueeItem->setTextColor( col ); 
           HbWidgetBasePrivate::d_ptr(marqueeItem)->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor, false);
       }
   }
} 
 
/*!
    HbStyle::updateThemedParams()

    Updates the themable parameters of widget (specially those which are specified in css files)

    \param widget, widget whose themed parameters are supposed to be updated
*/
void HbStyle::updateThemedParams(HbWidget *widget)
{
    Q_D( HbStyle );
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::updateThemedParams : -- -- -- --";
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
#endif
        return;
    }
    NODEPTR_N(widget);
    HbDeviceProfile profile(HbDeviceProfile::profile(widget));
    HbVector<HbCss::StyleRule> styleRules = styleLoader->styleRulesForNode(n, profile.orientation());
    if (!styleRules.count()) {
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::updateThemedParams : No style rules found.";
#endif
        return;
    }
  
    // update themed items
    QList<QGraphicsItem*> list = widget->childItems();
    foreach (QGraphicsItem* item, list) {
        d->updateThemedItems(styleRules, item);
    }
}

/*!
    Checkes whether given widget has orienation specific style 
    rules. This can be used for optimizing orientation switch.
*/
bool HbStyle::hasOrientationSpecificStyleRules(HbWidget *widget)
{
    HbLayeredStyleLoader *styleLoader = 
        HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Layouts);
    NODEPTR_N(widget);
    return styleLoader->hasOrientationSpecificStyleRules(n);
}

/*!
    For clearing caches.
*/
void HbStyle::widgetDestroyed(QObject* obj)
{
    Q_D( HbStyle );
    d->widgetLayoutNames.remove((const QGraphicsWidget *)obj);
}

/*!

    Sets the item name of a graphics item. Item name is not the same as object name.
    It's a textual identifier that uniquely identifies QGraphicsItem's child items.
    This method is used to map HbWidget's subelements to sub-elements defined in
    CSS/WidgetML layouts.

    Note that polish() method requires widget's children to be set with a correct item name.
    \sa HbStyle::polish()

    This method uses QGraphicsItem::setData() with id 0xfffe.

    \param item, graphics item
    \param name, name to be set
*/
void HbStyle::setItemName( QGraphicsItem *item, const QString &name )
{
    if (item) {
        item->setData( ItemName, name );
    }
}

/*!

    Returns the item name of a graphics item. Item name is not the same as object name.
    It's a textual identifier that uniquely identifies QGraphicsItem's child items.

    \param item, graphics item
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

    \param variable Name of the global parameter.
    \param value Returns value of the global parameter.
    \param profile Device profile of the used HbMainWindow. Primary display's.
            device profile HbDeviceProfile::current() is used if the value is omitted.
    \return true if the variable were found.
*/
bool HbStyle::parameter(const QString& parameter, qreal& value, const HbDeviceProfile &profile) const
{
    HbDeviceProfile effectiveProfile = profile;
    if ( effectiveProfile.isNull() ) {
        effectiveProfile = HbDeviceProfile::current();
    }
    Q_D( const HbStyle );
    d->ensureLayoutParameters(effectiveProfile);
    HbCss::ValueExtractor valueExtractor(d->layoutParameters, true, effectiveProfile);
    // todo: parsing variable/expression is done here so that there is no need to change API
    // also parameters method not changed (this change is done for docml/widgetml parsing)
    if (parameter.startsWith("var(") && parameter.endsWith(")")) {
        return valueExtractor.extractValue(parameter.mid(4,parameter.length()-5), value);
    } else if (parameter.startsWith("-var(") && parameter.endsWith(")")) {
        bool retVal = valueExtractor.extractValue(parameter.mid(5,parameter.length()-6), value);
        value = -value;
        return retVal;
    } else if (parameter.startsWith("expr(") && parameter.endsWith(")")) {
        QString expressionString = parameter.mid(5,parameter.length()-6);
        return valueExtractor.extractExpressionValue(expressionString, value);
    } else if (parameter.startsWith("-expr(") && parameter.endsWith(")")) {
        QString expressionString = parameter.mid(6,parameter.length()-7);
        bool retVal = valueExtractor.extractExpressionValue(expressionString, value);
        value = -value;
        return retVal;
    }

    return valueExtractor.extractValue(parameter, value);    
}

/*!
    Returns copy of all global style parameters. Both names and values 
    of the parameters are returned. The values are returned in pixels.

    Available parameters can be found from hbglobalparameters.css. By using these
    parameters applications get consistent look. Usage of this API (instead of parameter)
    is recommended if an application needs to fetch several parameters in one place.

    \param parameters Contains names and values of all global style parameters.
    \param profile Device profile of the used HbMainWindow. Primary display's
            device profile HbDeviceProfile::current() is used if the value is omitted.
*/

void HbStyle::parameters(HbStyleParameters &parameters, const HbDeviceProfile &profile) const
{
    HbDeviceProfile effectiveProfile = profile;
    if ( effectiveProfile.isNull() ) {
        effectiveProfile = HbDeviceProfile::current();
    }

    Q_D( const HbStyle );
    d->ensureLayoutParameters(effectiveProfile);
    HbCss::ValueExtractor valueExtractor(d->layoutParameters, true, effectiveProfile);
    qreal value = 0;
    for (int i = 0; i < d->layoutParameters.count(); i++) {
        if (valueExtractor.extractValue(d->layoutParameters[i].property, value))
            parameters.addParameter(d->layoutParameters[i].property, value);
    }
}


/*!
\internal
*/
HbStylePrivate::HbStylePrivate()
{
    nextAvailableId = HbStyle::P_CustomBase;
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
    layoutParameters.clear();
	colorParameters.clear();

    HbWidgetStyleLoader *loader = HbWidgetStyleLoader::instance();
    if(loader){
        loader->removeFilePath(STYLE_LOCATION, HbLayeredStyleLoader::Concern_Layouts, 
                HbLayeredStyleLoader::Priority_Core);
        loader->removeFilePath(COLOR_STYLE_LOCATION, HbLayeredStyleLoader::Concern_Colors, 
                HbLayeredStyleLoader::Priority_Core);
    }
}

void HbStylePrivate::_q_onThemeChanged()
{
    colorParameters.clear();
}

/*!
\internal
*/
QString HbStylePrivate::logicalName(HbStyle::Primitive primitive, const QStyleOption *option) const
{
    switch(primitive){
    case HbStyle::P_Slider_thumb:{
        const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider *>(option);
        QString iconPath;

                switch (opt->orientation){
                case Qt::Horizontal:{
                        if( opt->state&QStyle::State_Sunken)
                            iconPath= "qtg_graf_slider_h_handle_pressed";
                        else
                            iconPath= "qtg_graf_slider_h_handle_normal";
                        return (iconPath);
                }
                case Qt::Vertical:{
                        if( opt->state&QStyle::State_Sunken)
                            iconPath= "qtg_graf_slider_v_handle_pressed";
                        else
                            iconPath= "qtg_graf_slider_v_handle_normal";
                        return (iconPath);
                }
                default: break;
        }
    }
        default: break;
    }

return QString();
}

/*!
\internal
*/
QIcon::Mode HbStylePrivate::iconMode(QStyle::State state) const
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
QIcon::State HbStylePrivate::iconState(QStyle::State state) const
{
    QIcon::State icon = QIcon::Off;
    if (state & QStyle::State_On)
        icon = QIcon::On;
    return icon;
}

/*!
\internal
*/
HbStyleInterface *HbStylePrivate::stylePluginInterface( HbStyle::Primitive primitive ) const
{
    
    if (customPrimitives.contains(primitive)) {
        HbStyleInterfaceInfo* info = customPrimitives.value(primitive);
        QObject* pluginInstance = info->loader->instance();
        HbStyleInterface *stylePlugin = qobject_cast<HbStyleInterface *>(pluginInstance);
        return stylePlugin;
    }
    return 0;
}

/*!
\internal
*/
void HbStylePrivate::ensureLayoutParameters(const HbDeviceProfile &profile) const
{
    bool firstParse(layoutParameters.isEmpty());
    bool addSpecialParams(firstParse);
    if (firstParse) {
        HbCss::Parser parser;
        parser.init(GLOBAL_PARAMETERS_LOCATION, true);
        HbCss::StyleSheet *styleSheet = new(HbCss::StyleSheet);
        parser.parse(styleSheet);

        HbStyleSelector selector;
        selector.styleSheets.append(styleSheet);
        layoutParameters = selector.variableRuleSets();
    }
    if (!firstParse && (layoutParametersProfileName != profile.name())) {
        layoutParameters.remove(layoutParameters.count()-4, 4);
        addSpecialParams = true;
    }
    if (addSpecialParams) {
        layoutParametersProfileName = profile.name();
        QSizeF pSize = profile.logicalSize();

        {
            HbCss::Declaration decl;
            decl.property = "hb-param-screen-width";
            decl.propertyId = HbCss::UnknownProperty;
            HbCss::Value val;
            val.type = HbCss::Value::Number;
            val.variant = HbVariant((double)pSize.width(),HbMemoryManager::HeapMemory);
            decl.values.append(val);
            layoutParameters.append(decl);
        }
        {
            HbCss::Declaration decl;
            decl.property = "hb-param-screen-height";
            decl.propertyId = HbCss::UnknownProperty;
            HbCss::Value val;
            val.type = HbCss::Value::Number;
            val.variant = HbVariant((double)pSize.height(),HbMemoryManager::HeapMemory);
            decl.values.append(val);
            layoutParameters.append(decl);
        }
        {
            HbCss::Declaration decl;
            decl.property = "hb-param-screen-short-edge";
            decl.propertyId = HbCss::UnknownProperty;
            HbCss::Value val;
            val.type = HbCss::Value::Number;
            val.variant = HbVariant((double)qMin(pSize.height(),pSize.width()),HbMemoryManager::HeapMemory);
            decl.values.append(val);
            layoutParameters.append(decl);
        }
        {
            HbCss::Declaration decl;
            decl.property = "hb-param-screen-long-edge";
            decl.propertyId = HbCss::UnknownProperty;
            HbCss::Value val;
            val.type = HbCss::Value::Number;
            val.variant = HbVariant((double)qMax(pSize.height(),pSize.width()),HbMemoryManager::HeapMemory);
            decl.values.append(val);
            layoutParameters.append(decl);
        }
    }

}

void HbStylePrivate::ensureColorParameters() const
{
    if (colorParameters.isEmpty()) {
        HbLayeredStyleLoader *styleLoader = HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Colors);
        colorParameters = styleLoader->variableRuleSets();
    }
}

/*!
\internal
*/
void HbStylePrivate::clearStyleSheetCaches()
{
    widgetLayoutNames.clear();
    styleRulesCache.clear();
    colorParameters.clear();
}

#include "moc_hbstyle.cpp"
