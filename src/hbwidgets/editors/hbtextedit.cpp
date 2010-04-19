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

#include "hbtextedit.h"
#include "hbtextedit_p.h"

#include <QTextCursor>

#ifdef HB_TEXT_MEASUREMENT_UTILITY
#include "hbtextmeasurementutility_p.h"
#include "hbfeaturemanager_p.h"
#endif //HB_TEXT_MEASUREMENT_UTILITY

/*!
\class HbTextEdit
\brief Multiline rich-text editor supporting HTML-style tags and WYSIWYG editing.
@proto
@hbwidgets

Internally it uses QTextDocument to handle the content. It is also exposed in the API and it can be manipulated directly.
HbTextEdit also has one cursor that can be controlled with the API.

HbTextEdit can load either plaintext or HTML formatted files. See Qt documentation about available formatting.

 */
/*!
 Constructs an empty HbLineEdit with parent \a parent.
 */
HbTextEdit::HbTextEdit (QGraphicsItem *parent) :
    HbAbstractEdit(*new HbTextEditPrivate, parent)
{
    Q_D(HbTextEdit);
    d->q_ptr = this;

    d->init();
}

/*!
 Constructs a HbTextEdit with parent \a parent.
 The text edit will display the text \a text.
 */
HbTextEdit::HbTextEdit (const QString &text, QGraphicsItem *parent) :
    HbAbstractEdit(*new HbTextEditPrivate, parent)
{
    Q_D(HbTextEdit);
    d->q_ptr = this;

    d->init();

    setPlainText(text);
}

HbTextEdit::HbTextEdit (HbTextEditPrivate &dd, QGraphicsItem *parent) :
    HbAbstractEdit(dd, parent)
{
    Q_D(HbTextEdit);
    d->q_ptr = this;

    d->init();
}

/*!
 Destructor.
 */
HbTextEdit::~HbTextEdit ()
{
}

/*!
    \reimp
 */
int HbTextEdit::type () const
{
    return Type;
}

/*!
    Replaces the document used in the editor.

    This allows
 */
void HbTextEdit::setDocument(QTextDocument *document)
{
    HbAbstractEdit::setDocument(document);
}

QTextDocument *HbTextEdit::document() const
{
    return HbAbstractEdit::document();
}

void HbTextEdit::setTextCursor(const QTextCursor &cursor)
{
    HbAbstractEdit::setTextCursor(cursor);
}

QTextCursor HbTextEdit::textCursor() const
{
    return HbAbstractEdit::textCursor();
}

/*!
    \reimp
 */
QString HbTextEdit::toHtml() const
{
    return HbAbstractEdit::toHtml();
}

/*!
    \reimp
 */
void HbTextEdit::setHtml(const QString &text)
{
    HbAbstractEdit::setHtml(text);
}

/*!
    \reimp
 */
QString HbTextEdit::toPlainText () const
{
    return HbAbstractEdit::toPlainText();
}

/*!
    \reimp
 */
void HbTextEdit::setPlainText (const QString &text)
{
    QString txt( text );
#ifdef HB_TEXT_MEASUREMENT_UTILITY
    if ( HbFeatureManager::instance()->featureStatus( HbFeatureManager::TextMeasurement ) ) {
        if (text.endsWith(QChar(LOC_TEST_END))) {
            int index = text.indexOf(QChar(LOC_TEST_START));
            setProperty( HbTextMeasurementUtilityNameSpace::textIdPropertyName,  text.mid(index + 1, text.indexOf(QChar(LOC_TEST_END)) - index - 1) );
            setProperty( HbTextMeasurementUtilityNameSpace::textMaxLines, -1 );
            txt = text.left(index);
        } else {
            setProperty( HbTextMeasurementUtilityNameSpace::textIdPropertyName,  QVariant::Invalid );
        }
    }
#endif //HB_TEXT_MEASUREMENT_UTILITY
    HbAbstractEdit::setPlainText(txt);
}

/*!
    \reimp
 */
void HbTextEdit::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    HbAbstractEdit::resizeEvent(event);

    document()->setTextWidth(primitive(HbStyle::P_Edit_text)->boundingRect().width());
}


/*!
    \reimp
    reads custom parameters.
 */
void HbTextEdit::polish( HbStyleParameters& params )
{
    Q_D(HbTextEdit);

    const QString KTextRowLinesColorCSSName   = "line-color";
    const QString KTextRowLinesWidthCSSName   = "line-width";
    const QString KTextRowLinesEnabledCSSName = "line-enabled";

    // ------ adding css parameters ------
    params.addParameter(KTextRowLinesColorCSSName, Qt::magenta);
    params.addParameter(KTextRowLinesWidthCSSName, 0.0);
    params.addParameter(KTextRowLinesEnabledCSSName);

    HbAbstractEdit::polish(params);

    // ------ interpreting css parameters ------
    QVariant param = params.value( KTextRowLinesEnabledCSSName );
    d->mShowTextBaseLine = param.toBool();

    param = params.value(KTextRowLinesColorCSSName);
    if(param.canConvert(QVariant::Color)) {
        d->mTextBaseLinePen.setColor(param.value<QColor>());
    }

    param = params.value(KTextRowLinesWidthCSSName);
    if(param.canConvert(QVariant::Double)) {
        d->mTextBaseLinePen.setWidthF(qMax(param.toDouble(), 0.0));
    }
}

/*!
    \reimp
 */
void HbTextEdit::focusOutEvent(QFocusEvent * event)
{
    HbAbstractEdit::focusOutEvent(event);
    setBackgroundItem(HbStyle::P_TextEdit_frame_normal);
}

/*!
    \reimp
 */
void HbTextEdit::focusInEvent(QFocusEvent * event)
{
    HbAbstractEdit::focusInEvent(event);
    setBackgroundItem(HbStyle::P_TextEdit_frame_highlight);
}

