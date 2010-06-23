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

#ifndef HB_INPUT_FOCUS_OBJECT_H
#define HB_INPUT_FOCUS_OBJECT_H

#include <QRectF>
#include <QString>
#include <QFont>
#include <QVariant>
#include <QEvent>

#include <hbinputdef.h>
#include <hbinputeditorinterface.h>

class HbInputFocusObjectPrivate;
class HbVkbHost;
class HbInputMethod;
class QGraphicsProxyWidget;
class QGraphicsWidget;

class HB_CORE_EXPORT HbInputFocusObject : public QObject
{
    Q_OBJECT

public:
    explicit HbInputFocusObject(QObject *focusedObject);
    ~HbInputFocusObject();

    void sendPreEditString(const QString &string);
    void sendCommitString(const QString &string);
    void sendEvent(QEvent &event);
    void postEvent(QEvent &event);
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
    int editorCursorPosition();
    QFont editorFont();
    QString editorTextSelection();
    QString editorSurroundingText();
    HbEditorInterface &editorInterface() const;
    void cursorLeft(Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    void cursorRight(Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    void releaseFocus();
    bool filterAndCommitCharacter(QChar character);
    QRectF editorGeometry() const;
    QRectF microFocus() const;
    QString preEditString() const;
    qreal findVkbZValue() const;
    Qt::InputMethodHints inputMethodHints() const;
    void setInputMethodHints(Qt::InputMethodHints hints);

    QPointF scenePos() const;
    void filterStringWithEditorFilter(const QString &source, QString &result);
    bool characterAllowedInEditor(QChar character) const;
    bool stringAllowedInEditor(const QString &string) const;
    void commitSmiley(QString smiley);
    QObject *object() const;
    void setFocus();

    static bool isReadOnlyWidget(QObject *editorobject);
    static bool isEditor(QObject *object);

protected:
    HbInputFocusObjectPrivate *const d_ptr;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputFocusObject)
    Q_DISABLE_COPY(HbInputFocusObject)
};

#endif // HB_INPUT_FOCUS_OBJECT_H

// End of file
