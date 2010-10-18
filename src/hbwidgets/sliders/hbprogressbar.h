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


#ifndef HB_PROGRESSBAR_H
#define HB_PROGRESSBAR_H

#include <hbwidget.h>

class HbProgressBarPrivate;
class HbStyleOptionProgressBar;

class HB_WIDGETS_EXPORT HbProgressBar : public HbWidget
{
    Q_OBJECT
    Q_PROPERTY(int progressValue READ progressValue WRITE setProgressValue)
    Q_PROPERTY(bool invertedAppearance READ invertedAppearance WRITE setInvertedAppearance)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(QString minText READ minText WRITE setMinText)
    Q_PROPERTY(QString maxText READ maxText WRITE setMaxText)
    Q_PROPERTY(bool minMaxTextVisible READ isMinMaxTextVisible WRITE setMinMaxTextVisible)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(Qt::Alignment minMaxTextAlignment READ minMaxTextAlignment WRITE setMinMaxTextAlignment)

public:

    explicit HbProgressBar(QGraphicsItem *parent= 0);
    ~HbProgressBar();    

    bool invertedAppearance() const;
    void setInvertedAppearance(bool inverted);

    int maximum() const;
    int minimum() const;
    int progressValue() const;
    
    void setRange ( int minimum, int maximum );

    void setMinText(const QString &text);
    QString minText() const;
    
    void setMaxText(const QString &text);
    QString maxText() const;

    void setMinMaxTextVisible(bool visible);
    bool isMinMaxTextVisible() const;

    void setMinMaxTextAlignment(Qt::Alignment alignment);
    Qt::Alignment minMaxTextAlignment() const;

    Qt::Orientation orientation() const;

    QGraphicsItem *primitive(const QString &itemName) const;

    enum { Type = Hb::ItemType_ProgressBar };
    int type() const { return Type; }  

public slots:
    void setProgressValue(int value);
    void setMaximum(int maximum);
    void setMinimum(int minimum);
    void setOrientation(Qt::Orientation orientation);
    void updatePrimitives();

signals:
    void valueChanged(int value);

protected:
    HbProgressBar(HbProgressBarPrivate &dd, QGraphicsItem *parent = 0);
    void initStyleOption(HbStyleOptionProgressBar *option) const;
    void initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive);
    void closeEvent ( QCloseEvent * event );
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void changeEvent(QEvent *event);

private:    
    Q_DECLARE_PRIVATE_D( d_ptr, HbProgressBar )
    Q_DISABLE_COPY( HbProgressBar )

#ifdef HB_EFFECTS
    Q_PRIVATE_SLOT(d_func(), void _q_delayedHide(HbEffect::EffectStatus status))
    Q_PRIVATE_SLOT(d_func(), void _q_delayedShow(HbEffect::EffectStatus status))
#endif

};

#endif  //HB_PROGRESSBAR_H

