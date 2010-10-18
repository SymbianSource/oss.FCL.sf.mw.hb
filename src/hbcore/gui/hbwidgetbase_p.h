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

#ifndef HBWIDGETBASE_P_H
#define HBWIDGETBASE_P_H

#include <hbnamespace.h>
#include <hbwidgetbase.h>

#include <QGraphicsItem>
//
//  W A R N I N G
//  -------------
//
// This file is not part of the Hb API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

class HbCssInspectorWindow;
class HbWidget;

class HB_CORE_PRIVATE_EXPORT HbWidgetBasePrivate
{
    Q_DECLARE_PUBLIC(HbWidgetBase)

public:
    HbWidgetBasePrivate();
    virtual ~HbWidgetBasePrivate();
    void init();

    void fontChangeEvent();
    void handleInsidePopup(const QGraphicsItem *parent);
    bool keyNavigation() const;

    virtual void setInsidePopup(bool insidePopup);

    enum ApiCssProtectionFlags {
        AC_TextColor = 0x0001,
        AC_TextAlign = 0x0002,
        AC_IconBrush = 0x0004,
        AC_IconAspectRatioMode = 0x0008,
        AC_IconAlign = 0x0010,
        AC_TextWrapMode = 0x0020,
        AC_TextLinesMin = 0x0040,
        AC_TextLinesMax = 0x0080,
        AC_TextElideMode = 0x0100
    };

    inline void setApiProtectionFlag(HbWidgetBasePrivate::ApiCssProtectionFlags att, bool value)
    {
        if(value)
            mApiProtectionFlags |= att;
        else
            mApiProtectionFlags &= ~att;
    }

    inline bool testApiProtectionFlag(HbWidgetBasePrivate::ApiCssProtectionFlags att) const
    {
        return mApiProtectionFlags & att;
    }

    inline int attributeToBitIndex(Hb::WidgetAttribute att) const
    {
        int bit = -1;
        switch (att) {
            // Does not overlap with values set in attributeToBitIndex(Qt::WidgetAttribute)
            case Hb::InteractionDisabled: bit = 1; break;
        case Hb::InsidePopup: bit = 2; break;
        case Hb::Widget: bit = 3; break;
        default: break;
        }
        return bit;
    }

    /*
        In case when \a newAlign has no set vertical or horizontal part,
        missing part is replaced with respective part of \a oldAlign.
     */
    static inline Qt::Alignment combineAlignment (Qt::Alignment newAlign, Qt::Alignment oldAlign)
    {
        newAlign &= Qt::AlignVertical_Mask | Qt::AlignHorizontal_Mask;
        if ((newAlign & Qt::AlignHorizontal_Mask) == 0) {
            newAlign |= oldAlign & Qt::AlignHorizontal_Mask;
        }
        if ((newAlign & Qt::AlignVertical_Mask) == 0) {
            newAlign |= oldAlign & Qt::AlignVertical_Mask;
        }
        return newAlign;
    }

    quint32 mApiProtectionFlags;
    quint32 attributes : 5;
    HbFontSpec fontSpec;
    HbWidgetBase *q_ptr;
private:

    static HbWidgetBasePrivate *d_ptr(HbWidgetBase *base) {
        Q_ASSERT(base);
        return base->d_func();
    }
    friend class HbStylePrivate;
    friend class HbDocumentLoaderActions;
    friend class HbWidgetLoaderActions;
    friend class HbInputCheckBoxList; // for accessing setBackgroundItem
#ifdef HB_CSS_INSPECTOR
    friend class HbCssInspectorWindow;
#endif
};


#endif // HBWIDGETBASE_P_H
