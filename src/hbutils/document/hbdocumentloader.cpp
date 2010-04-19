/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbUtils module of the UI Extensions for Mobile.
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

#include "hbdocumentloader.h"
#include "hbdocumentloader_p.h"
#include "hbdocumentloaderactions_p.h"
#include <hbmainwindow.h>

#include <QString>
#include <QGraphicsWidget>
#include <QFile>

/*!
    @stable
    @hbutils
    \class HbDocumentLoader
    \brief HbDocumentLoader allows applications to construct widget hierarchies from DocML files.
    
    You can load a document using \c HbDocumentLoader::load method. Afther that you can access to
    loaded objects by processing returned objects roots tree, or by using \c findObject / \c findWidget
    methods (\c HbDocumentLoader always keeps references to loaded objects)
     
    \c HbDocumentLoader uses \c HbDocumentLoader::createObject method to retrieve objects. By default, 
    this method creates always new instances based on class name. In addition to basic \c QObject 
    classes provided by Qt, only public widgets of Hb library are supported. Thus, in order to be 
    able to create your own custom widgets, you have to derive from this class and override 
    \c createObject method.

    Use the \c HbDocumentLoaderPlugin to add tool support for custom widgets.
    
    Example code:
    \snippet{documentloadersample.cpp,1}

    For more information about DocML syntax, please refer to S60QtProgrammersGuide.
*/

/*!
    Constructor. 
    
    Use HbDocumentLoader(const HbMainWindow *window) constructor if an application 
    have HbMainWindow. If the main window parameter is omitted 
    HbDeviceProfile::current() is used to access device profile.
 */
HbDocumentLoader::HbDocumentLoader()
: d_ptr(new HbDocumentLoaderPrivate(0))
{
    Q_D(HbDocumentLoader);
    d->q_ptr = this;
}

/*!
    Constructor. 
    
    HbMainWindow is needed to access device profile of the application's main window.
    HbDeviceProfile::current() is used in case of the window parameter is omitted.
    \param window main window of the loaded layout.
 */

HbDocumentLoader::HbDocumentLoader(const HbMainWindow *window)
: d_ptr(new HbDocumentLoaderPrivate(window))
{
    Q_D(HbDocumentLoader);
    d->q_ptr = this;
}

/*!
    Destructor.
*/
HbDocumentLoader::~HbDocumentLoader()
{
    delete d_ptr;
}

/*!
    Loads and processes DocML file. If After processing of file was successful,
    it returnes root objects list as a return parameter. Otherwise returnes empty list. 
    Qwnership to loaded objects is passed to the caller.
    \param fileName file to be processed.
    \param section space separated route to section, that you want to load.
    \param ok indicates if loading was successful.
    \return root objects list.
*/
QObjectList HbDocumentLoader::load( const QString &fileName, const QString &section , bool *ok )
{
    QFile file( fileName );
    
    if( !file.open( QFile::ReadOnly | QFile::Text ) ) {
        qWarning( "Unable to open file" );
        if( ok ) {
            *ok = false;
        }
        return QList<QObject *>();
    }
    
    return load( &file, section, ok ) ;
}

/*!
    This is an overloaded member function, provided for convenience.
    \param device IO device to be processed.
    \param section space separated route to section, that you want to load.
    \param ok indicates if loading was successful.
    \return root objects list.
*/
QObjectList HbDocumentLoader::load( QIODevice *device, const QString &section, bool *ok )
{
    Q_D(HbDocumentLoader);
 
    if( ok ) {
        *ok = d->load( device, section );
    } else {
        d->load( device, section );
    }
    return d->takeAll();
}

/*!
    This is an overloaded member function, provided for convenience. Loads part of DocML that is
    outside of any section.
    \param fileName file to be processed.
    \param ok indicates if loading was successful.
    \return root objects list.
*/
QObjectList HbDocumentLoader::load( const QString &fileName, bool *ok )
{
    return load( fileName, QString(), ok );
}

/*!
    This is an overloaded member function, provided for convenience. Loads part of DocML that is
    outside of any section.
    \param device IO device to be processed.
    \param ok indicates if loading was successful.
    \return root objects list.
*/
QObjectList HbDocumentLoader::load( QIODevice *device, bool *ok ) 
{
    return load( device, QString(), ok );
}


/*!
    Retrieves widget of which object name equals to \a name.
    \param name object name of desired widget.
    \return widget or 0 if not found.
*/
QGraphicsWidget *HbDocumentLoader::findWidget(const QString &name) const
{
    const Q_D(HbDocumentLoader);
    return d->findWidget(name);
}


/*!
    Retrieves object of which object name equals to \a name.
    \param name object name of desired widget.
    \return object or 0 if not found.
*/
QObject *HbDocumentLoader::findObject(const QString &name) const
{
    const Q_D(HbDocumentLoader);
    return d->findObject(name);
}


/*!
    Inserts object tree to documentloader. You can pass as an input parameter 
    output of "load" mothod. 
    \param roots root objects list.
    \return true if success, false otherwise.
*/
bool HbDocumentLoader::setObjectTree( QObjectList roots )
{
    Q_D(HbDocumentLoader);
    return d->setObjectTree( roots );    
}

/*!
    Looks up an object which inherits class \a type and of which
    object name equals to \a name. If you decide override this method,
    you should call this base class method in case you are
    not able to handle the case.
    \param type type of the desired object.
    \param name object name of the desired object.
    \return object or 0 if not found.
*/
QObject *HbDocumentLoader::createObject(const QString& type, const QString &name)
{
    Q_D(HbDocumentLoader);
    return d->actions->createObjectWithFactory(type, name);
}


/*!
    Sets loader to initial state. 
*/
void HbDocumentLoader::reset()
{
    Q_D(HbDocumentLoader);
    d->reset();
}


/*!
    Prints current version of document loader and minimum version of supported DocML in brackets
    For example "3.2 (1.4)" means that current version is 3.2 and DocML versions from 1.4 to 3.2 are supported   
*/
QString HbDocumentLoader::version()
{
    return HbDocumentLoaderPrivate::version();
}
