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
#ifndef HBSMILEYENGINE_P_H
#define HBSMILEYENGINE_P_H

#include <hbglobal.h>
#include <hbsmileytheme.h>

#include <QObject>

class HbSmileyEnginePrivate;
QT_FORWARD_DECLARE_CLASS(QTextDocument)
QT_FORWARD_DECLARE_CLASS(QTextCursor)

class HB_AUTOTEST_EXPORT HbSmileyEngine : public QObject
{
    Q_OBJECT
public:
    explicit HbSmileyEngine(QObject *parent = 0);
    ~HbSmileyEngine();

    void setDocument(QTextDocument *doc);
    void startAnimation();
    void stopAnimation();
    void setTheme(const HbSmileyTheme& theme);
    HbSmileyTheme theme() const;
    HbSmileyTheme defaultTheme() const;

    QString toPlainText() const;
    QString toHtml() const;
    void setSmileyScaleFactor(qreal scaleFactor);
    qreal smileyScaleFactor() const;
    void updateSmileys(QList<QTextCursor*> cursors);

public slots:
    void insertSmileys();
    void insertSmileys(const QTextCursor& cursor);
    void insertSmiley(const QTextCursor& cursor);


protected:
    HbSmileyEnginePrivate * const d_ptr;
    explicit HbSmileyEngine(HbSmileyEnginePrivate &dd, QObject *parent = 0);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbSmileyEngine)
    Q_DISABLE_COPY(HbSmileyEngine)
    Q_PRIVATE_SLOT(d_func(), void _q_animationProgressed())
    Q_PRIVATE_SLOT(d_func(), void _q_documentContentsChanged(int position, int charsRemoved, int charsAdded))

};

Q_DECLARE_METATYPE(HbSmileyEngine*)
#endif // HBSMILEYENGINE_P_H
