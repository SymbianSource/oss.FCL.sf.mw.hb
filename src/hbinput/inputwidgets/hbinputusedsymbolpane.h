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

#ifndef HB_INPUT_USED_SYMBOL_PANE_H
#define HB_INPUT_USED_SYMBOL_PANE_H

#include <QGraphicsWidget>

#include "hbinputdef.h"

const int HbSctLineWidth = 700;
const int HbSctLineHeight = 50;

class HbInputVkbWidget;
class HbInputUsedSymbolPanePrivate;
class HbInputFilter;

class HB_INPUT_EXPORT HbInputUsedSymbolPane : public QGraphicsWidget
{
    Q_OBJECT

public:
    explicit HbInputUsedSymbolPane(HbInputVkbWidget* aOwner, QGraphicsWidget* aParent = NULL);
    virtual ~HbInputUsedSymbolPane();

    void setNumberOfCharacters(int aNumChrs);
    void restoreSctLine(HbInputFilter *aFilter);

protected: // From QGraphicsItem
    void mousePressEvent(QGraphicsSceneMouseEvent* aEvent);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* aEvent);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *aEvent);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

signals:
    void sctCharacterSelected(QChar aChar);

private:
    Q_DISABLE_COPY(HbInputUsedSymbolPane)
    HbInputUsedSymbolPanePrivate* mPrivate;
};

#endif // HB_INPUT_USED_SYMBOL_PANE_H

// End of file
