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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Hb API.  It exists purely as an
// implementation detail.  This file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
#include <QTextBlock>
#include "hbtextedit_p.h"
#include "hbtextedit.h"
#include "hbcolorscheme.h"

HbTextEditPrivate::HbTextEditPrivate () :
    HbAbstractEditPrivate(),
    mShowTextBaseLine(false),
    mShowTextBaseLineApiProtectionFlag(false)
{
}

HbTextEditPrivate::~HbTextEditPrivate ()
{
}

void HbTextEditPrivate::init()
{
    Q_Q(HbTextEdit);

//    mTextBaseLinePen.setColor(Qt::magenta);
    mTextBaseLinePen.setCapStyle(Qt::RoundCap);
    mTextBaseLinePen.setWidthF((qreal)1.0);

    q->setScrollable(true);
    setBackgroundItem(HbStylePrivate::P_TextEdit_frame_normal);
    setFocusHighlight((HbStyle::Primitive)HbStylePrivate::P_TextEdit_frame_highlight,HbWidget::FocusHighlightActive);
}

void HbTextEditPrivate::updatePaletteFromTheme()
{
    HbAbstractEditPrivate::updatePaletteFromTheme();
    Q_Q(HbTextEdit);

    QColor textColor = HbColorScheme::color("qtc_textedit_normal");
    QColor selectedColor = HbColorScheme::color("qtc_textedit_selected");
    QColor selectedBackground = HbColorScheme::color("qtc_textedit_marker_normal");
    QColor hintText = HbColorScheme::color("qtc_textedit_hint_normal");
    mTextBaseLinePen.setColor(HbColorScheme::color("qtc_textedit_normal"));
    setCursorColor(HbColorScheme::color("qtc_textedit_cursor"));

    QPalette pal = q->palette();

    if (textColor.isValid()) {
        pal.setColor(QPalette::Text, textColor);
    }

    if (selectedColor.isValid()) {
        pal.setColor(QPalette::HighlightedText, selectedColor);
    }

    if (selectedBackground.isValid()) {
        pal.setColor(QPalette::Highlight, selectedBackground);
    }

    if (hintText.isValid()) {
        pal.setColor(QPalette::NoRole, hintText);
    }

    q->setPalette(pal);
}

void HbTextEditPrivate::updateEditingSize()
{
    Q_Q(HbTextEdit);
    if (scrollArea) {
        scrollArea->setMinimumHeight(QFontMetrics(q->font()).height() + 2 * doc->documentMargin());
        scrollArea->setPreferredHeight(doc->documentLayout()->documentSize().height());
        scrollArea->setMaximumHeight(QWIDGETSIZE_MAX);
    }
}

/*
 * \reimp
 *
 */
void HbTextEditPrivate::drawContentBackground(QPainter *painter,
                                              const QStyleOptionGraphicsItem &) const
{
    if(mShowTextBaseLine) {
        drawTextBaseLines(painter);
    }
}

/*
 * @proto
 * this method draws base lines for text
 */
void HbTextEditPrivate::drawTextBaseLines(QPainter *painter) const
{
    // Save painter's state
    QPen oldPen = painter->pen();

    const QRegion clipReg = painter->clipRegion();

    painter->setPen(mTextBaseLinePen);

    const QAbstractTextDocumentLayout* const docLayout = doc->documentLayout();

    QRectF lineRect;
    qreal lastAscent((qreal)0.0);

    // iterate through paragraphs
    QTextBlock textBlock = doc->begin();
    for(;textBlock.isValid(); textBlock = textBlock.next() ) {
        QRectF blockRect = docLayout->blockBoundingRect(textBlock);

        if(!clipReg.intersects(blockRect.toRect())) {
            continue; // dont try to draw unneeded paragraphs
        }
        // iterate through lines of paragraph
        const int n = textBlock.layout()->lineCount();
        for( int i=0; i<n; ++i ){
            QTextLine line = textBlock.layout()->lineAt(i);

            lineRect = line.rect();
            lineRect.translate(blockRect.topLeft());
            lastAscent = line.ascent();
            drawBaseLineAt(painter, lineRect.left(), lineRect.right(), lineRect.top()+lastAscent);
        } // end of for loop: iterate through lines

    } // end of for loop: iterate through paragraphs

    // try draw extra lines where ther is no text enymore
    if(!lineRect.isEmpty()) { // last paragraph was drawn
        lineRect.translate(0,lineRect.height());
        while(lineRect.top()<=canvas->size().height()) {
            drawBaseLineAt(painter, lineRect.left(), lineRect.right(), lineRect.top()+lastAscent);
            lineRect.translate(0,lineRect.height());
        }
    }

    // Restore painter's state
    painter->setPen(oldPen);
}

/*
 * @proto
 * draws on \a painter single base line for one line text bounded by \a rect
 */
void HbTextEditPrivate::drawBaseLineAt(QPainter *painter,
                                           qreal x1, qreal x2, qreal y) const
{
    painter->drawLine(QPointF(x1,y), QPointF(x2,y));
}

void HbTextEditPrivate::setBaseLineStyleFromString(const QString &str)
{
    // if parsing is failed then set solid line
    mTextBaseLinePen.setStyle(Qt::SolidLine);

    if(!str.isEmpty()) {
        static const char *const suportedLineStyles[] = {
            "Solid",
            "Dash",
            "Dot",
            "DashDot",
            "DashDotDot"
        };

        static const int LineStyleCount = sizeof(suportedLineStyles)
                                          /sizeof(suportedLineStyles[0]);

        for (int i=0;i<LineStyleCount; ++i) {
            if (str==suportedLineStyles[i]) {
                mTextBaseLinePen.setStyle(
                        static_cast<Qt::PenStyle>(Qt::SolidLine+i));
                break;
            }
        }
    }
}
