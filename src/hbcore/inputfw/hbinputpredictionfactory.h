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

#ifndef HB_INPUT_PREDICTION_FACTORY_H
#define HB_INPUT_PREDICTION_FACTORY_H

#include <QObject>
#include <QList>
#include <QString>

#include <hbinputlanguage.h>
#include <hbinputpredictionengine.h>

class HbPredictionFactoryPrivate;

class HbPredictionPlugin
{
public:
    virtual ~HbPredictionPlugin() {}
    virtual HbPredictionEngine *createInterface() = 0;
    virtual HbPredictionInterfaceType type() const = 0;
    virtual QList<HbInputLanguage> languages() const = 0;
    virtual QString vendorId() const = 0;
};

Q_DECLARE_INTERFACE(HbPredictionPlugin, "com.Nokia.HbPredictionPlugin");

class HB_CORE_EXPORT HbPredictionFactory : public QObject
{
    Q_OBJECT
public:
    static HbPredictionFactory *instance();

    HbPredictionEngine *predictionEngineForLanguage(const HbInputLanguage &language);
    QList<int> allPredictionEnginesForLanguage(const HbInputLanguage &language);
    QList<int> allPredictionEngines();
    HbPredictionEngine *createEngine(int handle);
    HbPredictionEngine *createEngine(const QString &vendorIdString);

    void initialize();
    void clearEngineData();

public slots:
    void shutDown();

private:
    ~HbPredictionFactory();
    HbPredictionFactory();

    // Prevent copying
    HbPredictionFactory(const HbPredictionFactory &);
    HbPredictionFactory &operator = (const HbPredictionFactory &other);

private:
    HbPredictionFactoryPrivate *d;
};

#endif // HB_INPUT_PREDICTION_FACTORY_H

// End of file
