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
#include <hbcssinspector_p.h>

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
		AC_TextColor = 0x01,
		AC_TextAlign = 0x02,
		AC_IconBrush = 0x04,
		AC_IconAspectRatioMode = 0x08,
        AC_TextWrapMode = 0x10,
        AC_TextLinesMin = 0x20,
        AC_TextLinesMax = 0x40
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
        case Hb::InputMethodNeutral: bit = 3; break;
        default: break;
        }
        return bit;
    }

    quint32 mApiProtectionFlags;
    quint32 attributes : 4;
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
#ifdef HB_CSS_INSPECTOR
    friend class HbCssInspectorWindow;
#endif
};


#endif // HBWIDGETBASE_P_H
