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

#include "hbabstractedit_p.h"
#include "hbabstractedit.h"
#include "hbstyleoption_p.h"
#include "hbscrollarea.h"
#include "hbvalidator.h"
#include "hbmeshlayout_p.h"
#include "hbmenu.h"
#include "hbselectioncontrol_p.h"
#include "hbcolorscheme.h"
#include "hbsmileyengine_p.h"
#include "hbtextmeasurementutility_p.h"
#include "hbfeaturemanager_p.h"
#include "hbinputeditorinterface.h"
#include "hbinputvkbhost.h"

#include <QValidator>
#include <QTextLayout>
#include <QTextBlock>
#include <QTextList>
#include <QTextTable>
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QtDebug>
#include <QClipboard>
#include <QInputContext>
#include <QRegExp>

static inline bool firstFramePosLessThanCursorPos(QTextFrame *frame, int position)
{
    return frame->firstPosition() < position;
}

static inline bool cursorPosLessThanLastFramePos(int position, QTextFrame *frame)
{
    return position < frame->lastPosition();
}

static QRectF boundingRectOfFloatsInSelection(const QTextCursor &cursor)
{
    QRectF r;
    QTextFrame *frame = cursor.currentFrame();
    const QList<QTextFrame *> children = frame->childFrames();

    const QList<QTextFrame *>::ConstIterator firstFrame = qLowerBound(children.constBegin(), children.constEnd(),
                                                                      cursor.selectionStart(), firstFramePosLessThanCursorPos);
    const QList<QTextFrame *>::ConstIterator lastFrame = qUpperBound(children.constBegin(), children.constEnd(),
                                                                     cursor.selectionEnd(), cursorPosLessThanLastFramePos);
    for (QList<QTextFrame *>::ConstIterator it = firstFrame; it != lastFrame; ++it) {
        if ((*it)->frameFormat().position() != QTextFrameFormat::InFlow)
            r |= frame->document()->documentLayout()->frameBoundingRect(*it);
    }
    return r;
}

class HbEditItem : public HbWidget
{
public:

    HbEditItem(HbAbstractEdit *parent) : HbWidget(parent), edit(parent)
    {
    };

    virtual ~HbEditItem() {};

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0)
    {
        Q_UNUSED(widget)

        painter->save();
#ifdef HB_DEBUG_EDITOR_DRAW_RECTS
        painter->setPen(Qt::yellow);
        painter->drawRect(boundingRect().adjusted(3,3,-3,-3));
#endif
        if ( option) {
            edit->drawContents(painter, *option);
        }

        painter->restore();
    }

    void changeEvent(QEvent *event)
    {
        HbWidget::changeEvent(event);

        switch (event->type()) {
        case QEvent::FontChange:
            edit->document()->setDefaultFont(font());
            break;
        default:
            ;
        }
    }

    HbAbstractEdit *edit;
};

static QTextLine currentTextLine(const QTextCursor &cursor)
{
    const QTextBlock block = cursor.block();
    if (!block.isValid())
        return QTextLine();

    const QTextLayout *layout = block.layout();
    if (!layout)
        return QTextLine();

    const int relativePos = cursor.position() - block.position();
    return layout->lineForTextPosition(relativePos);
}



QStringList HbAbstractEditMimeData::formats() const
{
    if (!fragment.isEmpty())
        return QStringList() << QString::fromLatin1("text/plain") << QString::fromLatin1("text/html");
    else
        return QMimeData::formats();
}

QVariant HbAbstractEditMimeData::retrieveData(const QString &mimeType, QVariant::Type type) const
{
    if (!fragment.isEmpty())
        setup();
    return QMimeData::retrieveData(mimeType, type);
}

void HbAbstractEditMimeData::setup() const
{
    HbAbstractEditMimeData *that = const_cast<HbAbstractEditMimeData *>(this);
#ifndef QT_NO_TEXTHTMLPARSER
    that->setData(QLatin1String("text/html"), fragment.toHtml("utf-8").toUtf8());
#endif
    that->setText(fragment.toPlainText());
    fragment = QTextDocumentFragment();
}

HbAbstractEditPrivate::HbAbstractEditPrivate () :
    HbWidgetPrivate(),
    doc(0),
    placeholderDoc(0),
    validator(0),
    imEditInProgress(false),
    imPosition(0),
    imAdded(0),
    imRemoved(0),
    interactionFlags(Qt::TextEditorInteraction),
    tapPosition(-1, -1),
    cursorOn(false),
    preeditCursor(0),
    preeditCursorVisible(true),
    apiCursorVisible(true),
    canvas(0),
    scrollArea(0),
    scrollable(false),
    hadSelectionOnMousePress(false),
    selectionControl(0),
    acceptSignalContentsChange(true),
    acceptSignalContentsChanged(true),
    validRevision(0),
    wasGesture(false),
    smileysEnabled(false),
    smileyEngine(0),
    formatDialog(0)
{
}

HbAbstractEditPrivate::~HbAbstractEditPrivate ()
{
}

void HbAbstractEditPrivate::init()
{
    Q_Q(HbAbstractEdit);

    canvas = new HbEditItem(q);

    setContent(Qt::RichText, QString());

    QTextOption textOption = doc->defaultTextOption();
    textOption.setTextDirection(q->layoutDirection());
    doc->setDefaultTextOption(textOption);
    doc->setUndoRedoEnabled(interactionFlags & Qt::TextEditable);

    updatePaletteFromTheme();

    scrollArea = new HbScrollArea(q);
    scrollArea->setClampingStyle(HbScrollArea::StrictClamping);
    scrollArea->setFrictionEnabled(true);
    scrollArea->setScrollDirections(Qt::Vertical);
    scrollArea->setVerticalScrollBarPolicy(HbScrollArea::ScrollBarAlwaysOff);
    scrollArea->setContentWidget(canvas);
    scrollArea->setFlag(QGraphicsItem::ItemIsFocusable, false);
    QObject::connect(scrollArea, SIGNAL(scrollingStarted()), q, SLOT(_q_scrollStarted()));
    QObject::connect(scrollArea, SIGNAL(scrollingEnded()), q, SLOT(_q_scrollEnded()));
    QObject::connect(q, SIGNAL(selectionChanged(QTextCursor,QTextCursor)), q, SLOT(_q_selectionChanged()));
    HbStyle::setItemName(scrollArea, QString("text"));

    // These are the default values which are then overridden in subclasses
    // and when different options are enabled.
    q->setFlag(QGraphicsItem::ItemIsFocusable);
    q->setFlag(QGraphicsItem::ItemAcceptsInputMethod);
    q->setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    q->setFocusPolicy(Qt::StrongFocus);
    q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    contextMenuShownOn = Hb::ShowTextContextMenuOnSelectionClicked | Hb::ShowTextContextMenuOnLongPress;

    q->grabGesture(Qt::TapGesture);
}

void HbAbstractEditPrivate::updatePaletteFromTheme()
{
    // The link color is used from application's palette
    QColor linkColor = HbColorScheme::color("qtc_view_link_normal");
    QColor linkVisitedColor = HbColorScheme::color("qtc_view_visited_normal");
    QPalette appPal = qApp->palette();
    if (linkColor.isValid()) {
        appPal.setColor(QPalette::Link, linkColor);
    }

    if (linkVisitedColor.isValid()) {
        appPal.setColor(QPalette::LinkVisited, linkVisitedColor);
    }
    qApp->setPalette(appPal);

}

void HbAbstractEditPrivate::setContent(Qt::TextFormat format, const QString &text)
{
    Q_Q(HbAbstractEdit);

    // for localization text support
    QString txt( text );
#ifdef HB_TEXT_MEASUREMENT_UTILITY
    if ( HbFeatureManager::instance()->featureStatus( HbFeatureManager::TextMeasurement ) ) {
        if (text.endsWith(QChar(LOC_TEST_END))) {
            int index = text.indexOf(QChar(LOC_TEST_START));
            q->setProperty( HbTextMeasurementUtilityNameSpace::textIdPropertyName,
                    text.mid(index + 1, text.indexOf(QChar(LOC_TEST_END)) - index - 1) );
            txt = text.left(index);
        }
    }
#endif

    // for use when called from setPlainText. we may want to re-use the currently
    // set char format then.
    const QTextCharFormat charFormatForInsertion = cursor.charFormat();

    bool clearDocument = true;
    if (!doc) {
        connectToNewDocument(new QTextDocument(q));
    }

    bool previousUndoRedoState = doc->isUndoRedoEnabled();
    doc->setUndoRedoEnabled(false);

    // avoid multiple textChanged() signals being emitted
    acceptSignalContentsChanged = false;

    if (!txt.isEmpty()) {
        // clear 'our' cursor for insertion to prevent
        // the emission of the cursorPositionChanged() signal.
        // instead we emit it only once at the end instead of
        // at the end of the document after loading and when
        // positioning the cursor again to the start of the
        // document.
        cursor = QTextCursor();

        if (format == Qt::PlainText) {
            QTextCursor formatCursor(doc);
            formatCursor.beginEditBlock();
            // put the setPlainText and the setCharFormat into one edit block,
            // so that the syntax highlight triggers only /once/ for the entire
            // document, not twice.
            doc->setPlainText(txt);
            formatCursor.select(QTextCursor::Document);
            formatCursor.setCharFormat(charFormatForInsertion);
            formatCursor.endEditBlock();
        } else {
#ifndef QT_NO_TEXTHTMLPARSER
            doc->setHtml(txt);
#else
            doc->setPlainText(txt);
#endif
        }
        cursor = QTextCursor(doc);
    } else if (clearDocument) {
        doc->clear();
    }
    cursor.setCharFormat(charFormatForInsertion);

    doc->setUndoRedoEnabled(previousUndoRedoState);

    acceptSignalContentsChanged = true;

    updateCurrentCharFormat();

    emit q->contentsChanged();

    cursorChanged(HbValidator::CursorChangeFromContentSet);

    doc->setModified(false);

    ensureCursorVisible();

    smileyEngineInstance()->setDocument(doc);
    if(q->isSmileysEnabled()) {
        smileyEngineInstance()->insertSmileys();
    }
}

bool HbAbstractEditPrivate::setFocusToAnchor(const QTextCursor &newCursor)
{
    Q_UNUSED(newCursor);
    return false;
}
bool HbAbstractEditPrivate::setFocusToNextOrPreviousAnchor(bool next)
{
    Q_UNUSED(next);
    return false;
}

bool HbAbstractEditPrivate::findNextPrevAnchor(const QTextCursor& from, bool next, QTextCursor& newAnchor)
{
    Q_UNUSED(from);
    Q_UNUSED(next);
    Q_UNUSED(newAnchor);
    return false;
}

void HbAbstractEditPrivate::setCursorPosition(int pos, QTextCursor::MoveMode mode)
{
    cursor.setPosition(pos, mode);

    cursorChanged(HbValidator::CursorChangeFromMouse);
}

bool HbAbstractEditPrivate::cursorMoveKeyEvent(QKeyEvent *e)
{
#ifdef QT_NO_SHORTCUT
    Q_UNUSED(e);
#endif

    if (cursor.isNull())
        return false;

    QTextCursor::MoveMode mode = QTextCursor::MoveAnchor;
    QTextCursor::MoveOperation op = QTextCursor::NoMove;

    if (false) {
    }
#ifndef QT_NO_SHORTCUT
    if (e == QKeySequence::MoveToNextChar) {
            op = QTextCursor::Right;
    }
    else if (e == QKeySequence::MoveToPreviousChar) {
            op = QTextCursor::Left;
    }
    else if (e == QKeySequence::SelectNextChar) {
           op = QTextCursor::Right;
           mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectPreviousChar) {
            op = QTextCursor::Left;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectNextWord) {
            op = QTextCursor::WordRight;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectPreviousWord) {
            op = QTextCursor::WordLeft;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectStartOfLine) {
            op = QTextCursor::StartOfLine;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectEndOfLine) {
            op = QTextCursor::EndOfLine;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectStartOfBlock) {
            op = QTextCursor::StartOfBlock;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectEndOfBlock) {
            op = QTextCursor::EndOfBlock;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectStartOfDocument) {
            op = QTextCursor::Start;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectEndOfDocument) {
            op = QTextCursor::End;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectPreviousLine) {
            op = QTextCursor::Up;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectNextLine) {
            op = QTextCursor::Down;
            mode = QTextCursor::KeepAnchor;
            {
                QTextBlock block = cursor.block();
                QTextLine line = currentTextLine(cursor);
                if (!block.next().isValid()
                    && line.isValid()
                    && line.lineNumber() == block.layout()->lineCount() - 1)
                    op = QTextCursor::End;
            }
    }
    else if (e == QKeySequence::MoveToNextWord) {
            op = QTextCursor::WordRight;
    }
    else if (e == QKeySequence::MoveToPreviousWord) {
            op = QTextCursor::WordLeft;
    }
    else if (e == QKeySequence::MoveToEndOfBlock) {
            op = QTextCursor::EndOfBlock;
    }
    else if (e == QKeySequence::MoveToStartOfBlock) {
            op = QTextCursor::StartOfBlock;
    }
    else if (e == QKeySequence::MoveToNextLine) {
            op = QTextCursor::Down;
    }
    else if (e == QKeySequence::MoveToPreviousLine) {
            op = QTextCursor::Up;
    }
    else if (e == QKeySequence::MoveToPreviousLine) {
            op = QTextCursor::Up;
    }
    else if (e == QKeySequence::MoveToStartOfLine) {
            op = QTextCursor::StartOfLine;
    }
    else if (e == QKeySequence::MoveToEndOfLine) {
            op = QTextCursor::EndOfLine;
    }
    else if (e == QKeySequence::MoveToStartOfDocument) {
            op = QTextCursor::Start;
    }
    else if (e == QKeySequence::MoveToEndOfDocument) {
            op = QTextCursor::End;
    }
#endif // QT_NO_SHORTCUT
    else {
        return false;
    }

    //const QTextCursor oldCursor = cursor;
    bool visualNavigation = cursor.visualNavigation();
    cursor.setVisualNavigation(true);
    cursor.movePosition(op, mode);
    cursor.setVisualNavigation(visualNavigation);
    cursorChanged(HbValidator::CursorChangeFromOperation);

    return true;
}

void HbAbstractEditPrivate::repaintOldAndNewSelection(const QTextCursor &oldSelection)
{
    if (cursor.hasSelection()
        && oldSelection.hasSelection()
        && cursor.currentFrame() == oldSelection.currentFrame()
        && !cursor.hasComplexSelection()
        && !oldSelection.hasComplexSelection()
        && cursor.anchor() == oldSelection.anchor()
        ) {
        QTextCursor differenceSelection(doc);
        differenceSelection.setPosition(oldSelection.position());
        differenceSelection.setPosition(cursor.position(), QTextCursor::KeepAnchor);
        canvas->update(selectionRect(differenceSelection));
    } else {
        if (!oldSelection.isNull())
            canvas->update(selectionRect(oldSelection) | cursorRectPlusUnicodeDirectionMarkers(oldSelection.position()));
        canvas->update(selectionRect() | cursorRectPlusUnicodeDirectionMarkers(cursor.position()));
    }
}

void HbAbstractEditPrivate::updateCurrentCharFormat()
{
    QTextCharFormat fmt = cursor.charFormat();
    if (fmt == lastCharFormat)
        return;
    lastCharFormat = fmt;
}

QRectF HbAbstractEditPrivate::cursorRectPlusUnicodeDirectionMarkers(int position) const
{
    return rectForPositionInCanvasCoords(position,QTextLine::Leading).adjusted(-4, 0, 4, 0);
}

void HbAbstractEditPrivate::setBlinkingCursorEnabled(bool enable)
{
    Q_Q(HbAbstractEdit);

    if (enable && QApplication::cursorFlashTime() > 0)
        cursorBlinkTimer.start(QApplication::cursorFlashTime() / 2, q);
    else
        cursorBlinkTimer.stop();

    cursorOn = enable;

    repaintCursor();
}

void HbAbstractEditPrivate::repaintCursor()
{
    canvas->update(cursorRectPlusUnicodeDirectionMarkers(cursor.position()));
}

void HbAbstractEditPrivate::ensurePositionVisible(int position)
{
    if (scrollArea && scrollable) {
        QRectF rect = rectForPositionInCanvasCoords(position, QTextLine::Leading);
        rect.adjust(0, -doc->documentMargin(), 0, doc->documentMargin());
        // TODO: it seems that scrollArea->ensureVisible() expects the point
        //       in its content coordinates. Probably it should use viewport
        //       coordinates i.e. its own item coordinate system
        //QRectF recScroll = canvas->mapToItem(scrollArea, rect).boundingRect();
        scrollArea->ensureVisible(rect.center(), rect.width(), rect.height()/2);
    }
}

void HbAbstractEditPrivate::ensureCursorVisible()
{
    ensurePositionVisible(cursor.position());
    if(selectionControl) {
        selectionControl->updatePrimitives();
    }
}

void HbAbstractEditPrivate::setTextInteractionFlags(Qt::TextInteractionFlags flags)
{
    Q_Q(HbAbstractEdit);

    if (flags == interactionFlags)
        return;
    interactionFlags = flags;

    if (q->hasFocus()) {
        setBlinkingCursorEnabled(flags & Qt::TextEditable);
    }
}

void HbAbstractEditPrivate::_q_updateRequest(QRectF rect)
{
    canvas->update(rect);
}

void HbAbstractEditPrivate::_q_updateBlock(QTextBlock block)
{
    Q_Q(HbAbstractEdit);
    _q_updateRequest(q->blockBoundingRect(block));
}

void HbAbstractEditPrivate::_q_contentsChanged()
{
    Q_Q(HbAbstractEdit);
    if(acceptSignalContentsChanged) {
        acceptSignalContentsChanged = false; // prevent recurence

        if(imPosition>=0) {
            validateAndCorrect();
        }
        updateCurrentCharFormat();

        emit q->contentsChanged();

        acceptSignalContentsChanged = true; // end of prevent recurence
    }
}

void HbAbstractEditPrivate::_q_contentsChange(int position, int charsRemoved, int charsAdded)
{
    if(acceptSignalContentsChange) {
        imPosition = position;
        imRemoved = charsRemoved;
        imAdded = charsAdded;
    }
}

void HbAbstractEditPrivate::_q_selectionChanged()
{
    Q_Q(HbAbstractEdit);

    if (cursor.hasSelection()) {   
        selectionControl = HbSelectionControl::attachEditor(q);
        selectionControl->showHandles();
    } else if (selectionControl){
        selectionControl->hideHandles();
    }
    
    QTextCursor oldSelection(selectionCursor);
    selectionCursor = cursor;
    repaintOldAndNewSelection(oldSelection);
}

void HbAbstractEditPrivate::_q_scrollStarted()
{
    if (selectionControl) {
        selectionControl->scrollStarted();
    }
}


void HbAbstractEditPrivate::_q_scrollEnded()
{
    if (selectionControl) {
        selectionControl->scrollFinished();
    }
}


void HbAbstractEditPrivate::validateAndCorrect()
{
    if (validator && !imEditInProgress) {
        QValidator::State state = validateContent(imPosition, imRemoved, imAdded);
        if (state == QValidator::Invalid) {
            // workaround (undo doesn't decreases revision number in Qt 4.6 and it does in Qt 4.5.2):
            int undoCount = doc->revision() - validRevision;
            for( ;undoCount>0 && doc->isUndoRedoEnabled(); --undoCount) {
                 doc->undo();
            }
            validRevision = doc->revision();
        } else if(state == QValidator::Acceptable) {
            doc->setModified(false); // store information to use undo in case Invalid values
            validRevision = doc->revision();
        }
    }
}

QValidator::State HbAbstractEditPrivate::validateContent(int position, int charsRemoved, int charsAdded)
{
#ifdef HBVALIDATOR_DEBUG_ENABLE
    qDebug() << "HbAbstractEditPrivate::validateContent"
            << "Pos: " << position
            << "[-]: " << charsRemoved
            << "[+]: " << charsAdded;
#endif
    // acceptSignalContentsChange = false; // not needed afrer corection
    QValidator::State state = validator->validateContent(cursor, position, charsRemoved, charsAdded);
    validator->updateTextCursor(cursor);
    // acceptSignalContentsChange = true; // not needed afrer corection
#ifdef HBVALIDATOR_DEBUG_ENABLE
    qDebug() << "HbAbstractEditPrivate::validateContent"
            << ((state == QValidator::Acceptable) ? "Acceptable  " : ((state == QValidator::Intermediate) ? "Intermediate" : "Invalid     "));
#endif
    return state;
}

void HbAbstractEditPrivate::initValidator()
{
    doc->setUndoRedoEnabled(true);
    doc->setPlainText(validator->defaultValue());
}

void HbAbstractEditPrivate::cursorChanged(CursorChange origin)
{
    Q_Q(HbAbstractEdit);
    if (previousCursorPosition != cursor.position()
            || previousCursorAnchor != cursor.anchor()) {

        if (validator && !imEditInProgress) {
            QTextCursor previousCursor(cursor);
            previousCursor.setPosition(previousCursorAnchor);
            previousCursor.setPosition(previousCursorPosition, QTextCursor::KeepAnchor);
            validator->validateCursorPosition(previousCursor, cursor, origin);

            if (previousCursorPosition == cursor.position() && previousCursorAnchor == cursor.anchor()) {
                // validator can cancel cursor position change
                return;
            }
        } // if validator

        // repaint the old cursor position
        canvas->update(cursorRectPlusUnicodeDirectionMarkers(previousCursorPosition));

        emit q->cursorPositionChanged(previousCursorPosition, cursor.position());
        previousCursorAnchor = cursor.anchor();
        previousCursorPosition = cursor.position();
        nextCharCursor = cursor;
        selectionChanged();
    }
}

void HbAbstractEditPrivate::selectionChanged(bool forceEmitSelectionChanged /*=false*/)
{
    Q_Q(HbAbstractEdit);
    if (!forceEmitSelectionChanged) {
        if (selectionCursor.position() == cursor.position()
            && selectionCursor.anchor() == cursor.anchor()) {
            return;
        }
        if (!selectionCursor.hasSelection() && !cursor.hasSelection()) {
            return;
        }
    }

    emit q->selectionChanged(selectionCursor, cursor);
}

void HbAbstractEditPrivate::acceptKeyPressEvent(QKeyEvent *event)
{
    event->accept();
    cursorOn = true;
    ensureCursorVisible();

    updateCurrentCharFormat();
}

QAbstractTextDocumentLayout::PaintContext HbAbstractEditPrivate::getPaintContext() const
{
    Q_Q(const HbAbstractEdit);

    QAbstractTextDocumentLayout::PaintContext ctx;

    ctx.palette = q->palette();
    if (cursorOn && q->isEnabled()) {
        if (!isCursorVisible())
            ctx.cursorPosition = -1;
        else if (preeditCursor != 0)
            ctx.cursorPosition = - (preeditCursor + 2);
        else
            ctx.cursorPosition = cursor.position();
    }

    if (cursor.hasSelection()) {
        QAbstractTextDocumentLayout::Selection selection;
        selection.cursor = cursor;
        QPalette::ColorGroup cg = q->hasFocus() ? QPalette::Active : QPalette::Inactive;
        selection.format.setBackground(ctx.palette.brush(cg, QPalette::Highlight));
        selection.format.setForeground(ctx.palette.brush(cg, QPalette::HighlightedText));

        HbStyleOption opt;
        q->initStyleOption(&opt);

		if (qApp->style()->styleHint(QStyle::SH_RichText_FullWidthSelection, &opt, 0)) {
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        }
        ctx.selections.append(selection);
    }

    return ctx;
}

int HbAbstractEditPrivate::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const
{
    Q_Q(const HbAbstractEdit);
    return doc->documentLayout()->hitTest(q->mapToItem(canvas, point), accuracy);
}

QRectF HbAbstractEditPrivate::cursorRect(const QTextCursor &cursor) const
{
    Q_Q(const HbAbstractEdit);
    if (cursor.isNull())
        return QRectF();

    return q->rectForPosition(cursor.position());
}

QRectF HbAbstractEditPrivate::cursorRect() const
{
    return cursorRect(cursor);
}

QRectF HbAbstractEditPrivate::selectionRect(const QTextCursor &cursor) const
{
    Q_Q(const HbAbstractEdit);

    QRectF r = q->rectForPosition(cursor.selectionStart());

    if (cursor.hasComplexSelection() && cursor.currentTable()) {
        QTextTable *table = cursor.currentTable();
        r = doc->documentLayout()->frameBoundingRect(table);
    } else if (cursor.hasSelection()) {
        const int position = cursor.selectionStart();
        const int anchor = cursor.selectionEnd();
        const QTextBlock posBlock = doc->findBlock(position);
        const QTextBlock anchorBlock = doc->findBlock(anchor);
        if (posBlock == anchorBlock && posBlock.isValid() && posBlock.layout()->lineCount()) {
            const QTextLine posLine = posBlock.layout()->lineForTextPosition(position - posBlock.position());
            const QTextLine anchorLine = anchorBlock.layout()->lineForTextPosition(anchor - anchorBlock.position());

            const int firstLine = qMin(posLine.lineNumber(), anchorLine.lineNumber());
            const int lastLine = qMax(posLine.lineNumber(), anchorLine.lineNumber());
            const QTextLayout *layout = posBlock.layout();
            r = QRectF();
            for (int i = firstLine; i <= lastLine; ++i) {
                r |= layout->lineAt(i).rect();
                r |= layout->lineAt(i).naturalTextRect(); // might be bigger in the case of wrap not enabled
            }
            r.translate(q->blockBoundingRect(posBlock).topLeft());
        } else {
            QRectF anchorRect = q->rectForPosition(cursor.selectionEnd());
            r |= anchorRect;
            r |= boundingRectOfFloatsInSelection(cursor);
            QRectF frameRect(doc->documentLayout()->frameBoundingRect(cursor.currentFrame()));
            r.setLeft(frameRect.left());
            r.setRight(frameRect.right());
        }
        if (r.isValid())
            r.adjust(-1, -1, 1, 1);
    }

    return r;
}

QRectF HbAbstractEditPrivate::selectionRect() const
{
    return selectionRect(selectionCursor);
}

QRectF HbAbstractEditPrivate::rectForPositionInCanvasCoords(int position, QTextLine::Edge edge) const
{
    Q_Q(const HbAbstractEdit);

    const QTextBlock block = doc->findBlock(position);
    if (!block.isValid())
        return QRectF();
    const QAbstractTextDocumentLayout *docLayout = doc->documentLayout();
    const QTextLayout *layout = block.layout();
    const QPointF layoutPos = q->blockBoundingRect(block).topLeft();
    int relativePos = position - block.position();
    if (preeditCursor != 0) {
        int preeditPos = layout->preeditAreaPosition();
        if (relativePos == preeditPos)
            relativePos += preeditCursor;
        else if (relativePos > preeditPos)
            relativePos += layout->preeditAreaText().length();
    }
    QTextLine line = layout->lineForTextPosition(relativePos);

    int cursorWidth;
    {
        bool ok = false;
#ifndef QT_NO_PROPERTIES
        cursorWidth = docLayout->property("cursorWidth").toInt(&ok);
#endif
        if (!ok)
            cursorWidth = 1;
    }

    QRectF r;

    if (line.isValid()) {
        qreal x = line.cursorToX(relativePos, edge);
        qreal w = 0;
        r = QRectF(layoutPos.x() + x, layoutPos.y() + line.y(),
                   cursorWidth + w, line.height());
    } else {
        r = QRectF(layoutPos.x(), layoutPos.y(), cursorWidth, 10); // #### correct height
    }

    if(layout->preeditAreaText().length()) {
        r.adjust(0,0,q->blockBoundingRect(block).width()/2,0);
    }

    return r;
}

/*
  Returns the viewport rectangle in editor coordinate system.
*/
QRectF HbAbstractEditPrivate::viewPortRect() const
{
    QRectF viewRect = scrollArea->geometry();
    qreal margin = doc->documentMargin();
    viewRect.adjust(0,margin,0,-margin);
    return viewRect;
}

int HbAbstractEditPrivate::contentLength() const
{
    QTextBlock block = doc->lastBlock();
    return block.position() + block.length() - 1;
}

bool HbAbstractEditPrivate::hasAcceptableInput() const
{
    QTextCursor cursorCopy = cursor;
    if (validator &&
        validator->validateContent(cursorCopy, 0, 0, 0) != QValidator::Acceptable) {
        return false;
    }
    return true;
}

bool HbAbstractEditPrivate::canPaste() const
{
#ifndef QT_NO_CLIPBOARD
    Q_Q(const HbAbstractEdit);
    if (interactionFlags & Qt::TextEditable) {
        const QMimeData *md = QApplication::clipboard()->mimeData();
        return md && q->canInsertFromMimeData(md);
    }
#endif//QT_NO_CLIPBOARD
    return false;
}

bool HbAbstractEditPrivate::canCopy() const
{
#ifndef QT_NO_CLIPBOARD
    return true;
#else
    return false;
#endif//QT_NO_CLIPBOARD
}


bool HbAbstractEditPrivate::canCut() const
{
    return (canCopy() && (interactionFlags & Qt::TextEditable));
}


bool HbAbstractEditPrivate::canFormat() const
{
    return formatDialog != 0;
}

bool HbAbstractEditPrivate::isCursorVisible() const
{
    return preeditCursorVisible && apiCursorVisible;
}

void HbAbstractEditPrivate::sendMouseEventToInputContext(const QPointF &tapPos) const
{
    Q_Q(const HbAbstractEdit);

    QTextLayout *layout = cursor.block().layout();
    int cursorPos = hitTest(tapPos, Qt::ExactHit);

    if (cursorPos == -1) {
            cursorPos = cursor.position() + layout->preeditAreaText().length();
    }

    if (layout && !layout->preeditAreaText().isEmpty()) {
        QInputContext *ctx = qApp->inputContext();
        if (ctx) {
            QMouseEvent ev(QEvent::MouseButtonPress, tapPos.toPoint(), q->mapToScene(tapPos).toPoint(),
                           Qt::NoButton, Qt::NoButton, Qt::NoModifier);
            ctx->mouseHandler(cursorPos - cursor.position(), &ev);
        }
    }
}
void HbAbstractEditPrivate::updateEditingSize()
{
}

void HbAbstractEditPrivate::removeCurrentDocument()
{
    Q_Q(HbAbstractEdit);

    if( doc ) {
        doc->disconnect(q);
        doc->documentLayout()->disconnect(q);
        doc->documentLayout()->setPaintDevice(0);

        if (doc->parent() == q) {
            delete doc;
        }

        doc = 0;
    }
}

void HbAbstractEditPrivate::connectToNewDocument(QTextDocument *newDoc)
{
    Q_Q(HbAbstractEdit);

    doc = newDoc;
    cursor = QTextCursor(doc);

    QObject::connect(doc, SIGNAL(contentsChanged()), q, SLOT(_q_contentsChanged()));
    QObject::connect(doc, SIGNAL(contentsChange(int, int, int)), q, SLOT(_q_contentsChange(int, int, int)));
    QObject::connect(doc, SIGNAL(documentLayoutChanged()), q, SLOT(documentLayoutChanged()));
    QObject::connect(doc, SIGNAL(blockCountChanged(int)), q, SLOT(blockCountChanged(int)));

    doc->setModified(false);

    q->documentLayoutChanged();
}

void HbAbstractEditPrivate::longTapGesture(const QPointF &point)
{
    Q_Q(HbAbstractEdit);

    if(contextMenuShownOn.testFlag(Hb::ShowTextContextMenuOnLongPress)) {

        int cursorPos = hitTest(point, Qt::FuzzyHit);
        if (cursorPos == -1)
            return;

        // don't do anything if longpress inside the selection
        if (cursor.hasSelection()
            && cursorPos >= cursor.selectionStart()
            && cursorPos <= cursor.selectionEnd()){
            return;
        }
        q->showContextMenu(q->mapToScene(point));
    }
}

void HbAbstractEditPrivate::tapGesture(const QPointF &point)
{
    Q_Q(HbAbstractEdit);

    if (interactionFlags & Qt::NoTextInteraction)
        return;

    bool removeSelection = (hitTest(point, Qt::ExactHit) == -1);

    if (removeSelection && cursor.hasSelection()) {
        const QTextCursor oldCursor = cursor;
        cursor.clearSelection();
        emit q->selectionChanged(oldCursor, cursor);
    }

    int newCursorPos = hitTest(point, Qt::FuzzyHit);

    if (cursor.hasSelection() &&
        newCursorPos >= cursor.selectionStart() &&
        newCursorPos <= cursor.selectionEnd()){
        // we have a selection under mouse click
        if (contextMenuShownOn.testFlag(Hb::ShowTextContextMenuOnSelectionClicked)) {
            q->showContextMenu(q->mapToScene(point));
        }
    } else {
        // Currently focused widget to listen to InputContext before updating the cursor position
        sendMouseEventToInputContext(point);
        // need to get the cursor position again since input context can change the document
        newCursorPos = hitTest(point, Qt::FuzzyHit);
        setCursorPosition(newCursorPos);

        if (interactionFlags & Qt::TextEditable) {
            updateCurrentCharFormat();
        }
        cursorChanged(HbValidator::CursorChangeFromMouse);

        QString anchor(q->anchorAt(point));
        if(!anchor.isEmpty()) {
            emit q->anchorTapped(anchor);
        }
    }
}



void HbAbstractEditPrivate::gestureReceived()
{
    wasGesture = true;
}


void HbAbstractEditPrivate::drawSelectionEdges(QPainter *painter, QAbstractTextDocumentLayout::PaintContext ctx)
{
    if (cursor.hasSelection() && selectionControl && selectionControl->isVisible()){
        painter->setPen(ctx.palette.color(QPalette::Text));
        painter->setBrush(ctx.palette.color(QPalette::Text));
        painter->drawRect(rectForPositionInCanvasCoords(cursor.selectionStart(), QTextLine::Leading));
        painter->drawRect(rectForPositionInCanvasCoords(cursor.selectionEnd(), QTextLine::Trailing));
    }
}

/*
 * Prepares the document for pasting. Derived classes can override this method,
 * e.g. HbLineEdit clears the document before pasting when in password edit mode
 */
void HbAbstractEditPrivate::prepDocForPaste()
{
}

/*
 * this method should draw text background. it is temporary solution until
 * HbTexdDocumentlayout will be implemented
 *
 * this method is reimplemented by HbTextEdit to draw text base lines
 */
void HbAbstractEditPrivate::drawContentBackground(QPainter *,
                                                  const QStyleOptionGraphicsItem &) const
{
    // no implementation is needed
}

HbSmileyEngine* HbAbstractEditPrivate::smileyEngineInstance() const
{
    Q_Q(const HbAbstractEdit);
    if(smileyEngine == 0) {
        smileyEngine = new HbSmileyEngine(const_cast <HbAbstractEdit*>(q));
        smileyEngine->setDocument(doc);

        HbEditorInterface editorInterface(const_cast<HbAbstractEdit*>(q));

        if(smileysEnabled) {
            editorInterface.setSmileyTheme(smileyEngine->theme());
        } else {
            editorInterface.setSmileyTheme(HbSmileyTheme());
        }
    }
    return smileyEngine;
}

void HbAbstractEditPrivate::updatePlaceholderDocProperties()
{

    if(placeholderDoc) {
        placeholderDoc->setDocumentMargin(doc->documentMargin());
        placeholderDoc->setDefaultTextOption(doc->defaultTextOption());
        placeholderDoc->setDefaultFont(doc->defaultFont());
        placeholderDoc->setTextWidth(doc->textWidth());
    }
}

void HbAbstractEditPrivate::filterInputText(QString &text) const
{
    Q_Q(const HbAbstractEdit);
    HbEditorInterface editorInterface(const_cast<HbAbstractEdit*>(q));
    HbInputFilter *inputFilter = editorInterface.filter();

    if (!text.isEmpty() && inputFilter) {
        for(int i(text.length() - 1); i>=0; --i) {
            if (!inputFilter->filter(text.at(i))) {
                text.remove(i,1);
            }
        }
    }
}


Qt::Alignment HbAbstractEditPrivate::alignmentFromString(const QString &text)
{
    Qt::Alignment align(0);
    static const struct {
        Qt::Alignment align;
        const char* pattern;
    } stringToAlign[] = {
        { Qt::AlignLeft,      "\\bleft\\b" },
        { Qt::AlignRight,     "\\bright\\b" },
        { Qt::AlignHCenter,   "\\bhcenter\\b" },
        { Qt::AlignAbsolute,  "\\babsolute\\b" },
        { Qt::AlignJustify,   "\\bjustify\\b" },
        { Qt::AlignTop,       "\\btop\\b" },
        { Qt::AlignBottom,    "\\bbottom\\b" },
        { Qt::AlignVCenter,   "\\bvcenter\\b" },
        { Qt::AlignCenter,    "\\bcenter\\b" }
    };
    static const int count = sizeof(stringToAlign)/sizeof(stringToAlign[0]);


    QRegExp regExp;
    regExp.setCaseSensitivity(Qt::CaseInsensitive);

    for(int i=0; i<count; ++i) {
        regExp.setPattern(stringToAlign[i].pattern);
        if(text.indexOf(regExp)>=0) {
            align |= stringToAlign[i].align;
        }
    }
    return align;
}

// Send open input panel event.
void HbAbstractEditPrivate::sendInputPanelEvent(QEvent::Type type)
{
#if QT_VERSION >= 0x040600
    QInputContext *ic = qApp->inputContext();
    if (ic) {
        QEvent *openEvent = new QEvent(type);
        ic->filterEvent(openEvent);
        delete openEvent;
    }
#endif
}

// Send open input panel event.
void HbAbstractEditPrivate::openInputPanel()
{
    sendInputPanelEvent(QEvent::RequestSoftwareInputPanel);
}

// Send close input panel event.
void HbAbstractEditPrivate::closeInputPanel()
{
    sendInputPanelEvent(QEvent::CloseSoftwareInputPanel);
}

#include "hbinputeditorinterface.h"
#include "hbinputvkbhost.h"

void HbAbstractEditPrivate::minimizeInputPanel()
{
    Q_Q(HbAbstractEdit);

    HbEditorInterface ei(q);
    HbVkbHost* vkbHost = ei.vkbHost();
    vkbHost->minimizeKeypad();
}

#include "moc_hbabstractedit.cpp"
