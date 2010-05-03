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

#ifndef HBCSSINSPECTOR_P_H
#define HBCSSINSPECTOR_P_H

#ifdef HB_CSS_INSPECTOR
#include <QWidget>
#include <hbanchorlayout.h>
#include <hbwidgetbase.h>
QT_FORWARD_DECLARE_CLASS(QTextEdit)
QT_FORWARD_DECLARE_CLASS(QGraphicsScene)
QT_FORWARD_DECLARE_CLASS(QGraphicsItem)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QRadioButton)
QT_FORWARD_DECLARE_CLASS(HbAnchorArrowDrawer)
QT_FORWARD_DECLARE_CLASS(HbMeshLayout)


class HbCssInfoDrawer : public HbWidgetBase
{
    Q_OBJECT

public:
    HbCssInfoDrawer(QGraphicsItem *parent=0);
    virtual ~HbCssInfoDrawer();

public slots:
    void setItemTextVisible(bool visible) { mShowItemText = visible; };
    void setHintTextVisible(bool visible) { mShowHintText = visible; };
    void setBoxVisible(bool visible) { mShowBox = visible; };
	void setHintBoxVisible(bool visible) { mShowHintBox = visible; };
    void setGuideLinesVisible(bool visible) { mDrawGuideLines = visible; };
    void updateFocusItem(const QGraphicsItem* item);

protected:
    void changeEvent(QEvent *event);
    void updateColors();
    void paintRect(QPainter *painter, QRectF rect);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    bool mShowItemText;
    bool mShowHintText;
    bool mShowBox;
	bool mShowHintBox;
    bool mDrawGuideLines;
    QColor mTextColor;
    QColor mBoxColor;
    QString mItemText;
    QString mHintText;
    QRectF mItemRect;
    QRectF mHintRect;
};


class HoveredWidgetFilter : public QObject
{
    Q_OBJECT

public:
    HoveredWidgetFilter(QGraphicsScene *scene);
    virtual ~HoveredWidgetFilter();

signals:
    void newItemHovered(const QGraphicsItem* item);

public slots:
    void setLiveMode(bool enabled) { mLiveMode = enabled; };
    void setBlockingMode(bool enabled) { mBlockingMode = enabled; };

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    QGraphicsScene *mScene;
    QGraphicsItem *mCurrentItem;
    HbAnchorArrowDrawer *mArrowDrawer;
    HbCssInfoDrawer *mCssInfoDrawer;
    bool mLiveMode;
    bool mBlockingMode;

friend class HbCssInspectorWindow;
};


class HbCssInspectorWindow : public QWidget
{
    Q_OBJECT

public:
    static HbCssInspectorWindow *instance();
    virtual ~HbCssInspectorWindow();

public slots:
    void updateFocusItem(const QGraphicsItem* item);
    void setVisible(bool visible);
    void refresh();

private:
    void removeFilters();
    void addFilters();
    static QString meshItemsToHtmlInfo(HbMeshLayout *mesh, const QString itemName, const QString layoutName);

private:
    explicit HbCssInspectorWindow(QWidget *parent = 0);
    QTextEdit *mLayoutWidgetMLBox;
    QTextEdit *mLayoutCssBox;
    QTextEdit *mColorsCssBox;
    QLabel *mPathLabel;
    QLabel *mSizeHintLabel;
    QCheckBox *mArrowsCheck;
    QCheckBox *mOutlinesCheck;
	QCheckBox *mHintOutlinesCheck;
    QCheckBox *mSpacersCheck;
    QCheckBox *mNameCheck;
    QCheckBox *mSizeHintCheck;
    QCheckBox *mGuideLinesCheck;
    QRadioButton *mLiveRadio;
    QRadioButton *mClickRadio;
    QRadioButton *mBlockRadio;
    QVector<HoveredWidgetFilter*> mInstalledFilters;
};

#endif

#endif // HBCSSINSPECTOR_P_H
