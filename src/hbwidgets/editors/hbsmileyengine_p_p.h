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
#ifndef HBSMILEYENGINE_P_P_H
#define HBSMILEYENGINE_P_P_H

#include <hbsmileytheme.h>
#include <hbsmileyengine_p.h>
#include <QTextCursor>
#include <QTextObjectInterface>
#include <QObject>
#include <QPair>

class HbIconAnimator;
QT_FORWARD_DECLARE_CLASS(QTextDocument)

struct HbSmileyDataType {
    HbSmileyDataType():cursor(0),animator(0),engine(0){}
    QTextCursor* cursor;
    HbIconAnimator* animator;
    HbSmileyEngine* engine;
};


class HbIconTextObject : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)
public:
    enum Properties { HbSmileyData = 1 };
    //typedef QPair<QTextCursor*, HbIconAnimator*> HbSmileyDataType;

public:
    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument,
                         const QTextFormat &format);
    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc,
                    int posInDocument, const QTextFormat &format);
};

class HbSmileyEnginePrivate
{
    Q_DECLARE_PUBLIC(HbSmileyEngine)
public:
    enum { HbIconTextFormat = QTextFormat::UserObject + 1 };
    HbSmileyEnginePrivate();
    virtual ~HbSmileyEnginePrivate();
    void init();
    void cleanUp();
    void setDocument(QTextDocument *doc);
    void insertSmiley( QTextCursor cursor, const QString& name);
    void insertSmileys( QTextCursor cursor, bool insertOne = false);
    HbIconAnimator* lookupAnimator(const QString& name);
    bool isCursorValid(QTextCursor* cursor) const;

    HbIconAnimator* animatorForCursor(const QTextCursor& cursor);
    void removeCursor(const QTextCursor &cursor, HbIconAnimator* animator);
    void convertToText(QTextDocument *copyDoc) const;

    void _q_animationProgressed();
    void _q_documentContentsChanged(int position, int charsRemoved, int charsAdded);

    HbSmileyEngine *q_ptr;

    QTextDocument *mDocument;
    HbSmileyTheme mSmileyTheme;
    bool mEdited;
    QObject *mIconTextObject;
    qreal mScaleFactor;


    QHash<HbIconAnimator*, QList<QTextCursor*> > mAnimatorToCursors;
    QHash<QTextCursor*,HbIconAnimator*> mCursorToAnimator;
    QHash<QString, HbIconAnimator*> mSmileyAnimator;

    static HbSmileyTheme mDefaultTheme;
};

Q_DECLARE_METATYPE(HbIconAnimator*)
Q_DECLARE_METATYPE(HbSmileyDataType)


#endif // HBSMILEYENGINE_P_P_H
