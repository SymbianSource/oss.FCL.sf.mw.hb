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

#include "hbsmileyengine_p.h"
#include "hbsmileyengine_p_p.h"
#include <hbiconanimator.h>
#include <hbicon.h>
#include <QTextDocument>
#include <QFontMetricsF>
#include <QChar>
#include <QPainter>

HbSmileyTheme HbSmileyEnginePrivate::mDefaultTheme = HbSmileyTheme();

QSizeF HbIconTextObject::intrinsicSize(QTextDocument *doc, int posInDocument,
                                    const QTextFormat &format)
{
    Q_UNUSED(doc)
    Q_UNUSED(posInDocument)

    HbSmileyDataType smiley = qVariantValue<HbSmileyDataType>(format.property(HbSmileyData));
    HbIconAnimator *animator = smiley.animator;
    Q_ASSERT(animator);

    QFont f = static_cast<const QTextCharFormat&>(format).font();
    QFontMetricsF fm(f);

    // TODO: optimize this
    HbIcon icon = animator->icon();
    qreal iconHeight = fm.height() * smiley.scaleFactor;

    if(icon.height() != iconHeight) {
        icon.setHeight(iconHeight);
        animator->setIcon(icon);
    }
    // We are going to draw the smiley on the descent line not on the baseline
    // therefore the size needs to be adjusted accordingly.
    return (icon.size()-QSizeF(0,fm.descent()));
}

void HbIconTextObject::drawObject(QPainter *painter, const QRectF &rect,
                               QTextDocument *doc, int posInDocument,
                               const QTextFormat &format)
{
    Q_UNUSED(doc)
    Q_UNUSED(posInDocument)

    HbSmileyDataType smiley = qVariantValue<HbSmileyDataType>(format.property(HbSmileyData));

    QFont f = static_cast<const QTextCharFormat&>(format).font();
    QFontMetricsF fm(f);

    // Draw smiley on descent line
    QTransform matrix = painter->transform();
    painter->translate(0,fm.descent()-1);

    HbIconAnimator *animator = smiley.animator;
    animator->paint(painter,rect);
    painter->setTransform(matrix);
}

HbSmileyEnginePrivate::HbSmileyEnginePrivate()
    :mDocument(0),
     mEdited(true),
     mIconTextObject(new HbIconTextObject),
     mScaleFactor(1)
{
    q_ptr = 0;
}

HbSmileyEnginePrivate::~HbSmileyEnginePrivate()
{
    cleanUp();
    delete mIconTextObject;
}


void HbSmileyEnginePrivate::init()
{
    // Load default smiley theme
    if(mDefaultTheme.isNull()) {
        HbSmileyEnginePrivate::mDefaultTheme.load(":smileys/smileys_theme.sml");
        //
        //TODO: uncomment the lines below if animation definition file is provided for
        //      default smiley theme.
        //HbIconAnimationManager* m = HbIconAnimationManager::global();
        //m->addDefinitionFile(":smileys/smileys_animations.xml");
    }
    mSmileyTheme = HbSmileyEnginePrivate::mDefaultTheme;
}

void HbSmileyEnginePrivate::cleanUp()
{
    QList<QTextCursor*> cursors = mCursorToAnimator.keys();
    qDeleteAll(cursors);

    QList<HbIconAnimator*> animators = mAnimatorToCursors.keys();
    qDeleteAll(animators);

    mAnimatorToCursors.clear();
    mCursorToAnimator.clear();
    mSmileyAnimator.clear();
}

void HbSmileyEnginePrivate::setDocument(QTextDocument *doc)
{
    Q_Q(HbSmileyEngine);
    mDocument = doc;
    mDocument->documentLayout()->registerHandler(HbIconTextFormat, mIconTextObject);
    q->connect(mDocument, SIGNAL(contentsChange(int,int,int)), q, SLOT(_q_documentContentsChanged(int,int,int)));
    cleanUp();
}

void HbSmileyEnginePrivate::insertSmiley( QTextCursor cursor, const QString& name)
{
    QTextCharFormat hbiconFormat;
    QTextCursor *tmpCursor = new QTextCursor(cursor);
    hbiconFormat.setObjectType(HbIconTextFormat);

    HbSmileyDataType smiley;
    HbIconAnimator *animator = lookupAnimator(name);
    smiley.cursor = tmpCursor;
    smiley.animator = animator;
    smiley.scaleFactor = mScaleFactor;

    hbiconFormat.setProperty(HbIconTextObject::HbSmileyData, qVariantFromValue(smiley));

    mEdited = false;
    tmpCursor->insertText(QString(QChar::ObjectReplacementCharacter), hbiconFormat);
    mEdited = true;
    tmpCursor->setPosition(tmpCursor->position()-1);
    mAnimatorToCursors[animator] << tmpCursor;
    mCursorToAnimator[tmpCursor] = animator;
}

void HbSmileyEnginePrivate::insertSmileys( QTextCursor cursor, bool insertOne)
{
    QTextCursor initialCursor = cursor;
    initialCursor.beginEditBlock();

    QString regexpStr;
    foreach (QString pattern, mSmileyTheme.patterns()) {
        regexpStr += QRegExp::escape(pattern) + "|";
    }
    regexpStr.remove(regexpStr.count()-1, 1);

    QRegExp rx(regexpStr);
    cursor = mDocument->find(rx, cursor);
    while ( !cursor.isNull()){
        insertSmiley(cursor, mSmileyTheme.smiley(cursor.selectedText()));
        if (insertOne) {
            break;
        }
        cursor = mDocument->find(rx, cursor);
    }
    initialCursor.endEditBlock();
}


HbIconAnimator* HbSmileyEnginePrivate::lookupAnimator(const QString& name)
{
    Q_Q(HbSmileyEngine);
    HbIconAnimator *animator = mSmileyAnimator.value(name);

    // Init icon if it doesn't already exits
    if (!animator) {
        HbIconAnimator *newAnimator = new HbIconAnimator();
        animator = newAnimator;
        HbIcon icon = HbIcon(name);
        icon.setFlags(HbIcon::NoAutoStartAnimation);
        animator->setIcon(icon);
        q->connect(animator,SIGNAL(animationProgressed()),q,SLOT(_q_animationProgressed()));
        animator->startAnimation();
        mSmileyAnimator[name] = animator;
    }

    return animator;
}


bool HbSmileyEnginePrivate::isCursorValid(QTextCursor* cursor) const
{
    bool ret = true;

    if (cursor) {
        if (mDocument->characterAt(cursor->position()) != QChar::ObjectReplacementCharacter) {
            ret = false;
         } else {
             QTextCursor tmpCursor(*cursor);
             tmpCursor.setPosition(tmpCursor.position()+1);
             QTextCharFormat format = tmpCursor.charFormat();

             HbSmileyDataType smiley = qVariantValue<HbSmileyDataType>(format.property(HbIconTextObject::HbSmileyData));
             if (cursor != smiley.cursor) {
                ret = false;
             }
         }
     } else {
        ret = false;
     }
    return ret;
}

void HbSmileyEnginePrivate::convertToText(QTextDocument *copyDoc) const
{
    QList<QTextCursor> cursors;
    // copy the cursors to copy document so that the positions get automatically updated
    foreach(QTextCursor *cursor, mCursorToAnimator.keys()) {
        if(isCursorValid(cursor)) {
            QTextCursor copyCursor(copyDoc);
            copyCursor.setPosition(cursor->position());
            cursors << copyCursor;
        }
    }

    foreach(QTextCursor copyCursor, cursors) {
        copyCursor.setPosition(copyCursor.position()+1, QTextCursor::KeepAnchor);
        QTextFormat format = copyCursor.charFormat();
        HbSmileyDataType smiley = qVariantValue<HbSmileyDataType>(format.property(HbIconTextObject::HbSmileyData));
        HbIconAnimator *animator = smiley.animator;
        Q_ASSERT(animator);

        QString pattern = mSmileyTheme.patterns(mSmileyAnimator.key(animator)).first();
        copyCursor.deleteChar();
        copyCursor.insertText(pattern);
    }
}

void HbSmileyEnginePrivate::_q_animationProgressed()
{
    Q_Q(HbSmileyEngine);
    HbIconAnimator *animator = qobject_cast<HbIconAnimator *>(q->sender());
    Q_ASSERT(animator);

    foreach(QTextCursor *cursor, mAnimatorToCursors.value(animator)) {
        QTextCursor tmpCursor(*cursor);
        tmpCursor.setPosition(tmpCursor.position()+1, QTextCursor::KeepAnchor);

        // update a bogus property, which will trigger a paint
        QTextCharFormat format;
        format.setProperty(HbIconTextObject::HbSmileyData+1, QString("Dummy"));
        mEdited = false;
        tmpCursor.mergeCharFormat(format);
        mEdited = true;
    }
}

void HbSmileyEnginePrivate::_q_documentContentsChanged(int position, int charsRemoved, int charsAdded)
{
    Q_UNUSED(position);
    Q_UNUSED(charsRemoved);
    Q_UNUSED(charsAdded);

    if(charsRemoved > 0 && mEdited) {
        foreach (QTextCursor* cursor, mCursorToAnimator.keys()) {          
            if (!isCursorValid(cursor)) {
                HbIconAnimator * animator = mCursorToAnimator.value(cursor);
                mCursorToAnimator.remove(cursor);
                QList<QTextCursor*> & cursorList = mAnimatorToCursors[animator];
                cursorList.removeFirst();
                if (!cursorList.count()) {
                    mAnimatorToCursors.remove(animator);
                    mSmileyAnimator.remove(mSmileyAnimator.key(animator));
                    animator->deleteLater();
                }
                delete cursor;
            }
        }
    }
}


HbSmileyEngine::HbSmileyEngine(QObject *parent)
    :QObject(parent),
    d_ptr(new HbSmileyEnginePrivate)

{
    Q_D(HbSmileyEngine);
    d->q_ptr = this;
    d->init();
}

HbSmileyEngine::HbSmileyEngine(HbSmileyEnginePrivate &dd, QObject *parent)
    :QObject(parent),
    d_ptr(&dd)
{
    Q_D(HbSmileyEngine);
    d->q_ptr = this;
    d->init();
}

HbSmileyEngine::~HbSmileyEngine()
{
    delete d_ptr;
}

void HbSmileyEngine::setDocument(QTextDocument *doc)
{
    Q_D(HbSmileyEngine);
    d->setDocument(doc);
}

void HbSmileyEngine::setTheme(const HbSmileyTheme& theme)
{
    Q_D(HbSmileyEngine);
    d->mSmileyTheme = theme;
}

HbSmileyTheme HbSmileyEngine::theme() const
{
    Q_D(const HbSmileyEngine);
    return d->mSmileyTheme;
}

HbSmileyTheme HbSmileyEngine::defaultTheme() const
{
    return HbSmileyEnginePrivate::mDefaultTheme;
}


QString HbSmileyEngine::toPlainText() const
{
    Q_D(const HbSmileyEngine);
    QTextDocument *copyDoc = d->mDocument->clone();
    d->convertToText(copyDoc);
    QString plainText = copyDoc->toPlainText();
    delete copyDoc;
    return plainText;
}

QString HbSmileyEngine::toHtml() const
{
    Q_D(const HbSmileyEngine);
    QTextDocument *copyDoc = d->mDocument->clone();
    d->convertToText(copyDoc);
    QString htmlString = copyDoc->toHtml();
    return htmlString;
}

void HbSmileyEngine::setSmileyScaleFactor(qreal scaleFactor)
{
    Q_D(HbSmileyEngine);
    if (d->mScaleFactor != scaleFactor) {
        // Note: it is assumed that the scale factor is not changed on the fly hence we do not need to redraw the smileyes.
        d->mScaleFactor = scaleFactor;
    }
}

void HbSmileyEngine::startAnimation()
{
    Q_D(HbSmileyEngine);
    foreach (HbIconAnimator *animator, d->mAnimatorToCursors.keys()) {
        animator->startAnimation();
    }
}

void HbSmileyEngine::stopAnimation()
{
    Q_D(HbSmileyEngine);
    foreach (HbIconAnimator *animator, d->mAnimatorToCursors.keys()) {
        animator->stopAnimation();
    }
}

void HbSmileyEngine::insertSmileys()
{
    Q_D(HbSmileyEngine);

    QTextCursor cursor(d->mDocument);
    d->insertSmileys(cursor);
}

void HbSmileyEngine::insertSmileys(const QTextCursor& cursor)
{
    Q_D(HbSmileyEngine);
    d->insertSmileys(cursor);
}

void HbSmileyEngine::insertSmiley(const QTextCursor& cursor)
{
    Q_D(HbSmileyEngine);
    d->insertSmileys(cursor,true);
}

#include "moc_hbsmileyengine_p.cpp"
