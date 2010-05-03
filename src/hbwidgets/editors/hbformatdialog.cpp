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
#include "hbformatdialog.h"
#include "hbformatdialog_p.h"

#include <hbwidget.h>

#include <hbpushbutton.h>
#include <hbcombobox.h>
#include <hbdialog.h>
#include <hbcolordialog.h>
#include <hbabstractedit.h>
#include <hbabstractedit_p.h>
#include <hbstyle.h>

#include <QGraphicsItem>
#include <QGraphicsGridLayout>
#include <QTextListFormat>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextList>

/*!
    \class HbFormatDialog
    \brief HbFormatDialog is a text format dialog class for HbAbstractEdit based 
    editors. It offers the default UI and basic formatting functions. This class 
    can be customized by overloading the setContent and polish methods. Signals 
    related to the format changes are sent from the editor.

    HbFormatDialogPointer should be used as pointer to HbFormatDialog object.

    @proto
    @hbwidgets

   Simplest way of using HbFormatDialog is like this:

   \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,48}

   Example of subclassing the HbFormatDialog class:
   \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,49}

   HbFormatDialog instance shared with many editors:
   \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,50}
 */

/*!
    Constructs a HbFormatDialog.
 */
HbFormatDialog::HbFormatDialog(QGraphicsItem *parent) : HbDialog(*new HbFormatDialogPrivate, parent)
{}

/*!
    Destructor.
*/
HbFormatDialog::~HbFormatDialog()
{}

/*!
    Launches a format dialog user interface for the \a editor.

    \sa setContent
 */
void HbFormatDialog::launch(HbAbstractEdit *editor)
{
    Q_D(HbFormatDialog);

    setFlag(QGraphicsItem::ItemIsMovable);       
    setTimeout(HbPopup::NoTimeout);
    setBackgroundFaded(false);

    d->editor = editor;
    if (editor) {
        if (editor->textCursor().isNull())
            return;
        setContent(editor->textCursor());
        show();
    }
}

/*!
    Sets and updates the content widget for the format dialog. This method can be overloaded. 
 */
void HbFormatDialog::setContent(QTextCursor cursor)
{
    Q_D(HbFormatDialog);

    QTextCharFormat format = cursor.charFormat(); // current char format

    // Set content
    if (!contentWidget()) {
        HbWidget *container = new HbWidget;

        d->layout = new QGraphicsGridLayout();

        d->comboSize = new HbComboBox(container);
        d->comboSize->addItem ("8");
        d->comboSize->addItem ("9");
        d->comboSize->addItem ("10");
        d->comboSize->addItem ("11");
        d->comboSize->addItem ("12");
        d->comboSize->addItem ("14");
        d->comboSize->addItem ("16");
        d->comboSize->addItem ("18");
        d->comboSize->addItem ("20");
        d->comboSize->addItem ("22");
        d->comboSize->addItem ("24");
        d->comboSize->addItem ("26");
        d->comboSize->addItem ("28");
        d->comboSize->addItem ("36");
        d->comboSize->addItem ("48");
        d->comboSize->addItem ("72");
        connect(d->comboSize, SIGNAL(currentIndexChanged(const QString &)), SLOT(setFontPointSize(const QString &)));
        d->layout->addItem(d->comboSize, 0, 0, 1, 3);

        d->buttonColor = new HbPushButton(container);
        connect(d->buttonColor, SIGNAL(clicked()), SLOT(_q_setColor()));
        d->layout->addItem(d->buttonColor, 0, 3, 1, 1);

        d->buttonAlignLeft = new HbPushButton(container);
        d->buttonAlignLeft->setIcon(HbIcon("qtg_mono_text_align_left"));
        d->buttonAlignLeft->setCheckable(true);
        connect(d->buttonAlignLeft, SIGNAL(toggled(bool)), SLOT(setAlignmentLeft(bool))); 
        connect(d->buttonAlignLeft, SIGNAL(toggled(bool)), SLOT(_q_setAlignment(bool))); 
        d->layout->addItem(d->buttonAlignLeft, 1, 0);

        d->buttonAlignCenter = new HbPushButton(container);
        d->buttonAlignCenter->setIcon(HbIcon("qtg_mono_text_align_center"));
        d->buttonAlignCenter->setCheckable(true);
        connect(d->buttonAlignCenter, SIGNAL(toggled(bool)), SLOT(setAlignmentCenter(bool)));
        connect(d->buttonAlignCenter, SIGNAL(toggled(bool)), SLOT(_q_setAlignment(bool)));
        d->layout->addItem(d->buttonAlignCenter, 1, 1);

        d->buttonAlignRight = new HbPushButton(container);
        d->buttonAlignRight->setIcon(HbIcon("qtg_mono_text_align_right"));
        d->buttonAlignRight->setCheckable(true);
        connect(d->buttonAlignRight, SIGNAL(toggled(bool)), SLOT(setAlignmentRight(bool)));
        connect(d->buttonAlignRight, SIGNAL(toggled(bool)), SLOT(_q_setAlignment(bool)));
        d->layout->addItem(d->buttonAlignRight, 1, 2);

        d->buttonAlignJustify = new HbPushButton(container);
        d->buttonAlignJustify->setIcon(HbIcon("qtg_mono_text_align_justify"));
        d->buttonAlignJustify->setCheckable(true);
        connect(d->buttonAlignJustify, SIGNAL(toggled(bool)), SLOT(setAlignmentJustify(bool)));
        connect(d->buttonAlignJustify, SIGNAL(toggled(bool)), SLOT(_q_setAlignment(bool)));
        d->layout->addItem(d->buttonAlignJustify, 1, 3);

        d->buttonBold = new HbPushButton(container);
        d->buttonBold->setIcon(HbIcon("qtg_mono_bold"));
        d->buttonBold->setCheckable(true);
        connect(d->buttonBold, SIGNAL(toggled(bool)), SLOT(setStyleBold(bool)));
        d->layout->addItem(d->buttonBold, 2, 0);

        d->buttonItalic = new HbPushButton(container);
        d->buttonItalic->setIcon(HbIcon("qtg_mono_italic"));
        d->buttonItalic->setCheckable(true);
        connect(d->buttonItalic, SIGNAL(toggled(bool)), SLOT(setStyleItalic(bool)));
        d->layout->addItem(d->buttonItalic, 2, 1);

        d->buttonUnderline = new HbPushButton(container);
        d->buttonUnderline->setIcon(HbIcon("qtg_mono_underline"));
        d->buttonUnderline->setCheckable(true);
        connect(d->buttonUnderline, SIGNAL(toggled(bool)), SLOT(setStyleUnderline(bool)));
        d->layout->addItem(d->buttonUnderline, 2, 2);

        d->buttonBullet = new HbPushButton(container);
        d->buttonBullet->setIcon(HbIcon("qtg_mono_bullet"));
        d->buttonBullet->setCheckable(true);
        connect(d->buttonBullet, SIGNAL(toggled(bool)), SLOT(setStyleBullet(bool)));
        d->layout->addItem(d->buttonBullet, 2, 3);

        container->setLayout(d->layout);

        setContentWidget(container);
    }

    // Update content
    d->comboSize->setCurrentIndex(d->comboSize->findText(QString::number(format.font().pointSize())));
    d->colorChanged(currentCharColor());

    d->buttonAlignLeft->setChecked(cursor.blockFormat().alignment() == Qt::AlignLeft);
    d->buttonAlignCenter->setChecked(cursor.blockFormat().alignment() == Qt::AlignCenter);
    d->buttonAlignRight->setChecked(cursor.blockFormat().alignment() == Qt::AlignRight);
    d->buttonAlignJustify->setChecked(cursor.blockFormat().alignment() == Qt::AlignJustify);

    d->buttonBold->setChecked(format.font().bold());
    d->buttonItalic->setChecked(format.font().italic());
    d->buttonUnderline->setChecked(format.font().underline());
    if (cursor.currentList()) {
        d->buttonBullet->setChecked(cursor.currentList()->format().style() == QTextListFormat::ListDisc);
    }
}

/*!
    \reimp
 */
void HbFormatDialog::polish(HbStyleParameters& params)
{
    Q_D(HbFormatDialog);

    params.addParameter("button-width");
    params.addParameter("button-height");
    params.addParameter("popup-margin");
    params.addParameter("vertical-middle-margin");
    params.addParameter("horizontal-middle-margin");
    HbDialog::polish( params );

    qreal buttonWidth = params.value("button-width").toDouble();
    qreal buttonHeight = params.value("button-height").toDouble();
    d->buttonColor->setMinimumSize(buttonWidth, buttonHeight);
    d->buttonAlignLeft->setMinimumSize(buttonWidth, buttonHeight);
    d->buttonAlignCenter->setMinimumSize(buttonWidth, buttonHeight);
    d->buttonAlignRight->setMinimumSize(buttonWidth, buttonHeight);
    d->buttonAlignJustify->setMinimumSize(buttonWidth, buttonHeight);
    d->buttonBold->setMinimumSize(buttonWidth, buttonHeight);
    d->buttonItalic->setMinimumSize(buttonWidth, buttonHeight);
    d->buttonUnderline->setMinimumSize(buttonWidth, buttonHeight);
    d->buttonBullet->setMinimumSize(buttonWidth, buttonHeight);

    qreal popupMargin = params.value("popup-margin").toDouble();
    setContentsMargins(popupMargin, popupMargin, popupMargin, popupMargin);

    d->layout->setVerticalSpacing(params.value("vertical-middle-margin").toDouble());
    d->layout->setHorizontalSpacing(params.value("horizontal-middle-margin").toDouble());
}

/*!
   Merges new format for a selection, word under cursor or new chars after cursor position 
 */
void HbFormatDialog::mergeCharFormat(const QTextCharFormat &format)
{
    Q_D(HbFormatDialog);

    QTextCursor cursor = d->editor->textCursor();
    if (cursor.hasSelection()) {
        cursor.mergeCharFormat(format);
    } else {
        int position = cursor.position();
        QChar currentChar = cursor.document()->characterAt(position);
        cursor.select(QTextCursor::WordUnderCursor);
        int selectionStart = cursor.selectionStart();
        int selectionEnd = cursor.selectionEnd();
        if (position == selectionStart || position == selectionEnd || currentChar.isSpace()) {            
            HbAbstractEditPrivate::d_ptr(d->editor)->nextCharCursor.mergeCharFormat(format);            
        } else {
            cursor.mergeCharFormat(format); 
        }
    }
}

/*!
   Gets current character color, which can be either default or user changed.
 */
QColor HbFormatDialog::currentCharColor() const
{
    Q_D(const HbFormatDialog);

    if (!d->editor) return QColor().convertTo(QColor::Invalid);
    QTextCharFormat format = d->editor->textCursor().charFormat();
    QVariant property = format.property(QTextFormat::ForegroundBrush);   
    bool isBrush = (property.userType() == QVariant::Brush);
    QColor color = format.colorProperty(QTextFormat::ForegroundBrush);
    QColor foregoundColor = format.foreground().color();
    QColor defaultColor = d->editor->palette().color(QPalette::Text);
    return (isBrush && !color.isValid()) ? foregoundColor : defaultColor;
}

/*!
   Sets font size as \a size for a selection, word under cursor or new chars after cursor position 
 */
void HbFormatDialog::setFontPointSize(const QString &size)
{
    Q_D(HbFormatDialog);

    qreal pointSize = size.toFloat();
    QTextCursor cursor = d->editor->textCursor();
    if (pointSize != cursor.charFormat().font().pointSize()) {
        if (pointSize > 0) {
            QTextCharFormat fmt;
            fmt.setFontPointSize(pointSize);
            mergeCharFormat(fmt);
        }
    }
}

/*!
   Sets or removes bullets for the selected paragraphs or a paragraph containing cursor.
 */
void HbFormatDialog::setStyleBullet(bool toggled)
{
    Q_D(HbFormatDialog);

    QTextCursor cursor = d->editor->textCursor();
    QTextListFormat::Style style;
    if (toggled) {
        style = QTextListFormat::ListDisc;
    } else {
        style = QTextListFormat::ListStyleUndefined;
    }
    QTextBlockFormat blockFmt;
    cursor.beginEditBlock();
    blockFmt = cursor.blockFormat();
    QTextListFormat listFmt;
    int indent = blockFmt.indent() + 1;
    if (cursor.currentList()) {
        listFmt = cursor.currentList()->format();
        if (!toggled)
            indent = 0;
     } else {
        blockFmt.setIndent(0);
        cursor.setBlockFormat(blockFmt);
    }
    listFmt.setIndent(indent);
    listFmt.setStyle(style);
    cursor.createList(listFmt);
    cursor.endEditBlock();
}

/*!
   Sets alignment to left for the selected paragraphs or a paragraph containing cursor.
 */
void HbFormatDialog::setAlignmentLeft(bool toggled)
{
    Q_D(HbFormatDialog);

    if (toggled) {
        d->setAlignment(Qt::AlignLeft);
    }
}

/*!
   Sets alignment to center for the selected paragraphs or a paragraph containing cursor.
 */
void HbFormatDialog::setAlignmentCenter(bool toggled)
{
    Q_D(HbFormatDialog);

    if (toggled) {
        d->setAlignment(Qt::AlignCenter);
    }
}

/*!
   Sets alignment to right for the selected paragraphs or a paragraph containing cursor.
 */
void HbFormatDialog::setAlignmentRight(bool toggled)
{
    Q_D(HbFormatDialog);

    if (toggled) {
        d->setAlignment(Qt::AlignRight);
    }
}

/*!
   Sets alignment to justify for the selected paragraphs or a paragraph containing cursor.
 */
void HbFormatDialog::setAlignmentJustify(bool toggled)
{
    Q_D(HbFormatDialog);

    if (toggled) {
        d->setAlignment(Qt::AlignJustify);
    }
}

/*!
   Sets text bold for a selection, word under cursor or new chars after cursor position 
 */
void HbFormatDialog::setStyleBold(bool toggled)
{
    QTextCharFormat fmt;
    fmt.setFontWeight(toggled ? QFont::Bold : QFont::Normal);
    mergeCharFormat(fmt);
}

/*!
   Sets text italic for a selection, word under cursor or new chars after cursor position 
 */
void HbFormatDialog::setStyleItalic(bool toggled)
{
    QTextCharFormat fmt;
    fmt.setFontItalic(toggled);
    mergeCharFormat(fmt);
}

/*!
   Sets text underlined for a selection, word under cursor or new chars after cursor position 
 */
void HbFormatDialog::setStyleUnderline(bool toggled)
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(toggled);
    mergeCharFormat(fmt);
}
