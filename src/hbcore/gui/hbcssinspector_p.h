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
#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QWidget>
#include <hbanchorlayout.h>
#include <hbwidgetbase.h>
QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QGraphicsItem)
QT_FORWARD_DECLARE_CLASS(QGraphicsScene)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QRadioButton)
QT_FORWARD_DECLARE_CLASS(QTreeView)
QT_FORWARD_DECLARE_CLASS(QTextEdit)
QT_FORWARD_DECLARE_CLASS(HbAnchorArrowDrawer)
QT_FORWARD_DECLARE_CLASS(HbMainWindow)


class HbCssInfoDrawer : public HbWidgetBase
{
    Q_OBJECT

public:
    HbCssInfoDrawer(QGraphicsItem *parent=0);
    virtual ~HbCssInfoDrawer();

public slots:
    void updateFocusItem(const QGraphicsItem* item);

    void setBoxVisible(bool visible) { mShowBox = visible; };
    void setItemTextVisible(bool visible) { mShowItemText = visible; };
    void setGuideLinesVisible(bool visible) { mDrawGuideLines = visible; };

    void setHintTextVisible(bool visible) { mShowHintText = visible; };
    void setMinHintBoxVisible(bool visible) { mShowMinHintBox = visible; };
    void setPrefHintBoxVisible(bool visible) { mShowPrefHintBox = visible; };
    void setMaxHintBoxVisible(bool visible) { mShowMaxHintBox = visible; };
    void setSizePrefsVisible(bool visible) { mShowSizePrefs = visible; };

protected:
    void changeEvent(QEvent *event);
    void updateColors();
    void paintRect(QPainter *painter, QRectF rect);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QColor mTextColor;
    QColor mBoxColor;
    QString mItemText;
    QString mHintText;
    QRectF mItemRect;
    QSizePolicy mItemPolicy;
    QRectF mMinHintRect;
    QRectF mPrefHintRect;
    QRectF mMaxHintRect;
    
    bool mShowBox;
    bool mShowItemText;
    bool mDrawGuideLines;

    bool mShowHintText;
    bool mShowMinHintBox;
    bool mShowPrefHintBox;
    bool mShowMaxHintBox;
    bool mShowSizePrefs;
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
    void setHoverMode(bool enabled) { mHoverMode = enabled; };
    void setBlockingMode(bool enabled) { mBlockingMode = enabled; };

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    QGraphicsScene *mScene;
    QGraphicsItem *mCurrentItem;
    HbAnchorArrowDrawer *mArrowDrawer;
    HbCssInfoDrawer *mCssInfoDrawer;
    bool mHoverMode;
    bool mBlockingMode;

friend class HbCssInspectorWindow;
};


class CodeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CodeWidget(const QString &title, QWidget *parent=0);
    virtual ~CodeWidget();
public slots:
    void setText(const QString &text);
    void setHtml(const QString &html);
    void setLayoutDirection(Qt::LayoutDirection dir);
private:
    QLabel *mLabel;
    QTextEdit *mTextBox;
};


class CssInspectorModelItem
{
public:
    CssInspectorModelItem(QGraphicsItem *item, int row,
        CssInspectorModelItem *parent=0);
    ~CssInspectorModelItem();
    CssInspectorModelItem *child(int i);
    CssInspectorModelItem *parent();
    QGraphicsItem *data();
    int row();
private:
    QGraphicsItem *mItem;
    QHash<int, CssInspectorModelItem*> mChildren;
    CssInspectorModelItem *mParent;
    int mRow;
};


class CssInspectorModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    CssInspectorModel(HbMainWindow *win=0, QObject *parent=0);
    ~CssInspectorModel();

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
    HbMainWindow *mWin;
    CssInspectorModelItem *mRootItem;
};


class ItemColorDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ItemColorDelegate(QObject *parent=0);
    ~ItemColorDelegate();
    void paint(QPainter *painter, const QStyleOptionViewItem &option, 
        const QModelIndex &index) const;
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
    static QString anchorsToHtmlInfo(HbAnchorLayout *anchorLayout, const QString itemName, const QString layoutName);
    static QString anchorItemName(QGraphicsLayoutItem* item, QGraphicsLayout* layout, bool& isIdBased);

private slots:
    void updateFromTreeView(const QModelIndex &index);
    void updateColumnSizes(const QModelIndex &index);

private:
    explicit HbCssInspectorWindow(QWidget *parent = 0);
    QVector<HoveredWidgetFilter*> mInstalledFilters;

    CodeWidget *mWidgetMLBox;
    CodeWidget *mLayoutCssBox;
    CodeWidget *mColorsCssBox;
    QTreeView *mTreeView;
    QLabel *mPathLabel;
    QLabel *mSizeHintLabel;
    QLabel *mSizePolicyHoriz;
    QLabel *mSizePolicyVert;

    QCheckBox *mObjectNameCheck;
    QCheckBox *mAnchorArrowsCheck;
    QCheckBox *mSubitemOutlinesCheck;
    QCheckBox *mSpacersCheck;
    QCheckBox *mGuideLinesCheck;

    QCheckBox *mSizeHintTextCheck;
    QCheckBox *mMinSizeHintCheck;
    QCheckBox *mPrefSizeHintCheck;
    QCheckBox *mMaxSizeHintCheck;
    QCheckBox *mSizePrefCheck;

    QRadioButton *mHoverRadio;
    QRadioButton *mClickRadio;
    QRadioButton *mBlockRadio;
};

#endif

#endif // HBCSSINSPECTOR_P_H
