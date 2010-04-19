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

#ifndef HB_INPUT_CHAR_PREVIEW_PANE_H
#define HB_INPUT_CHAR_PREVIEW_PANE_H


#include <hbinputdef.h>
#include <hbdialog.h>

class HbCharPreviewPanePrivate;

class HB_INPUT_EXPORT HbCharPreviewPane : public HbDialog
{
    Q_OBJECT

public:
    explicit HbCharPreviewPane(QGraphicsItem* parent = NULL);
    virtual ~HbCharPreviewPane();
    
    void showCharacters(const QStringList& characterList, const QRectF &itemSceneBoundingRect);

    enum { Type = Hb::ItemType_InputCharPreviewPane };
    int type() const { return Type; }

signals:
    void charFromPreviewSelected(QString character);

public slots:
   void updatePrimitives();

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbCharPreviewPane)
	Q_DISABLE_COPY(HbCharPreviewPane)

	Q_PRIVATE_SLOT(d_func(), void _q_showAccentedPreviewPane(QString, QRectF))
	Q_PRIVATE_SLOT(d_func(), void _q_hideAccentedPreviewPane())
	Q_PRIVATE_SLOT(d_func(), void _q_hidePreviewPanePopup())
};

#endif // HB_INPUT_CHAR_PREVIEW_PANE_H
