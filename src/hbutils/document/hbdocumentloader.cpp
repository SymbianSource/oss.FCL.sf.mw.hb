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
#include "hbdocumentloadersyntax_p.h"

#include <QString>
#include <QGraphicsWidget>
#include <QFile>

/*!
    @stable
    @hbutils
    \class HbDocumentLoader
    \brief The HbDocumentLoader class loads UI definitions from DocML files and constructs a hierarchy of widgets in memory.
    
    HbDocumentLoader loads and parses UI definitions from DocML files.
    A DocML file usually contains a UI definition of a view or a dialog, and defines layouts, widgets and actions.
    Using HbDocumentLoader you can load an entire UI screen by calling the load() method, which returns an object list to the caller.
    The object list contains the root objects defined at the top level of the DocML file (child elements of <hbdocument>). Each root object
    may have child objects, so a hierarchy of objects is returned to the caller. Object ownership is passed to the caller.
    
    An HbDocumentLoader object keeps references to the loaded objects until the reset() method is called, but it is up to your code
    to manage the lifecycle of the returned objects. For widgets this is often easy, since they have either a containing view or a
    layout as a parent: Qt's object cleanup policy means that child objects are automatically deleted. 
    You must be careful to manage the lifecycle of any root objects returned by load() that are not owned by another object,
    such as HbAction objects that do not have a menu or toolbar role.  
            
    A DocML file contains only a UI definition. To connect your UI objects in code you must get pointers to them.
    Use the findObject() and findWidget() methods to get pointers to your UI objects, which you can then connect using their signals and slots. 
         
    HbDocumentLoader uses the createObject() method to create objects. By default, this method always creates new object instances
    based on the class name in the DocML. This method can create the public widgets of the Hb library and some basic objects that derive
    from QObject, such as HbAction. 
    In order to be able to load your own classes of objects from DocML, for example to load your own custom widgets, you can either:
    - create your own custom loader by subclassing from HbDocumentLoader and overriding the createObject() method.
    - create an  HbDocumentLoaderPlugin object in a separate library.
      
    HbDocumentLoader can load DocML in either text-based or binary format. Using a binary format reduces the time to load and parse a DocML file.
    You can create binary DocML files by specifying your DocML files in your project file. To declare a DocML file for conversion to binary format, add the following to the project file:
    \code
    DOCML += hello_docml_world.docml
    \endcode
    When the build process runs, files with the suffix docml.bin are created. You create entries in the resource file to load the binary DocML.
    The following shows a binary DocML file declared in the resource file. The binary file is given an alias ending in .docml which is used to reference it in code.
    \code
    <RCC>
    <qresource prefix="/docml">
        <file alias="hello_docml_world.docml">hello_docml_world.docml.bin</file>
    </qresource>
    </RCC>
    \endcode
    
    \section _usecases_hbdocumentloader Using the HbDocumentLoader class.
    
    \subsection _uc_001_hbdocumentloader Loading a simple DocML file containing a view definition.
        
    \code
    int main(int argc, char *argv[])
    {
        HbApplication app(argc, argv);
        HbMainWindow window;

        // Create a document loader object.
        HbDocumentLoader loader;
        bool ok = false;

        // Load the DocML file.
        loader.load(":/docml/hello_docml_world.docml", &ok);

        if (ok) {
            // Get the view and add it to the main window.
            HbView* view = qobject_cast<HbView*>(loader.findWidget("view"));
            window.addView(view);
        } 
        else {
            qFatal("Unable to read :/docml/hello_docml_world.docml");
        }

        window.show();
        return app.exec();
    }
    \endcode
    
    \subsection _uc_002_hbdocumentloader Getting pointers to loaded objects and taking ownership.
    Call findObject() and findWidget() to get pointers to and take ownership of loaded objects

    \code
    const QString LIST_VIEW = ":/resources/ui/listview.docml";
    
    HbDocumentLoader* loader;
    loader = new HbDocumentLoader();

    HbView* listView; 
    HbListView* notesList;
    HbAction* newNoteAction;
    HbAction* deleteNoteAction;
    
    // Load the DocML which defines the notes list view
    bool loaded = false;
    loader->reset();
    loader->load(LIST_VIEW, &loaded);
    Q_ASSERT_X(loaded, "Notes::load", "Unable to load view from DocML");
    if ( loaded ) {

        QGraphicsWidget *widget = loader->findWidget(QString("view"));
        if (widget) {

            // Widgets
            listView = qobject_cast<HbView*>(widget);
            notesList = qobject_cast<HbListView*>(loader->findWidget("notesList"));

            // Actions
            newNoteAction = qobject_cast<HbAction*>(loader->findObject("newNoteAction"));
            deleteNoteAction = qobject_cast<HbAction*>(loader->findObject("deleteNoteAction"));
          }
    \endcode
    
    
*/

/*!
    Default Constructor. 
    
    Use HbDocumentLoader(const HbMainWindow *window) constructor if an application 
    has an HbMainWindow. If the main window parameter is omitted 
    HbDeviceProfile::current() is used to access the device profile.
 */
HbDocumentLoader::HbDocumentLoader()
: d_ptr(new HbDocumentLoaderPrivate(0))
{
    Q_D(HbDocumentLoader);
    d->q_ptr = this;
}

/*!
    Constructor. 
    
    HbMainWindow is needed to access the device profile of the application's main window.
    HbDeviceProfile::current() is used in case where the window parameter is omitted.
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
    Loads and processes a DocML file. 

    On successful execution, returns a list of root objects. This list contains 
    only the objects which do not have an owner already. For example the child of a parent 
    widget is never returned, and the parent is returned only if it does not 
    have its own parent or a specific role like a menu-role. The document 
    loader instance does not own any of the objects: the ones which do not 
    have any other owner are returned within the root object list. The caller 
    is resposible of handling the lifecycle of the returned objects.

    Usually this returns the view or the dialog which the loaded DocML files defines. 

    There are also situations when this function returns an empty list. These cases 
    usually include sections, which change the existing properties of loaded objects and do not 
    create any new root objects. Also, any objects passed to the document loader using the setObjectTree() 
    function are not returned. The success of calling load() cannot therefore be
    determined directly by the number of the returned objects. 
    Check the parameter \a ok to see if the document was loaded successfully.

    

    \param fileName The %file to be loaded.
    \param section Space separated path to the section to load.
    \param ok Indicates whether loading was successful.
    \return Root objects list. On failed execution, returns an empty list and deletes the created object hierarchy, which 
    may be incomplete.
*/
QObjectList HbDocumentLoader::load( const QString &fileName, const QString &section , bool *ok )
{
    QFile file( fileName );
    
    if( !file.open( QFile::ReadOnly ) ) {
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
    \param device The IO device to be processed.
    \param section Space separated route to the section that you want to load.
    \param ok Indicates whether loading was successful.
    \return Root objects list.
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
    This is an overloaded member function, provided for convenience. It loads the part of the DocML that is
    outside of any section.
    \param fileName The %file to be processed.
    \param ok Indicates whether loading was successful.
    \return Root objects list.
*/
QObjectList HbDocumentLoader::load( const QString &fileName, bool *ok )
{
    return load( fileName, QString(), ok );
}

/*!
    This is an overloaded member function, provided for convenience. It loads the part of the DocML that is
    outside of any section.
    \param device IO device to be processed.
    \param ok Indicates if loading was successful.
    \return Root objects list.
*/
QObjectList HbDocumentLoader::load( QIODevice *device, bool *ok ) 
{
    return load( device, QString(), ok );
}

/*!
    Converts a text-based DocML document to a binary document at runtime. 

    You can also convert DocML files to binary format at build time by listing the files in the "DOCML"
    variable in the .pro file. This will create a binary docml file called <file_name>.bin that
    can be included with the application resources (.qrc file). 
    
    Known issues: Currently the resource compiler gives warnings about missing binary files during
    qmake. You can safely ignore these warnings.

    \param srcDevice Source IO device to be processed.
    \param dstDevice Destination IO device to which to write the binary DocML.
    \return True if the conversion to binary was successful.
*/
bool HbDocumentLoader::createBinary( QIODevice *srcDevice, QIODevice *dstDevice )
{
    Q_D(HbDocumentLoader);
    return d->createBinary( srcDevice, dstDevice );
}

/*!
    Retrieves widget whose object name is \a name. 
    \param name The name of the widget in DocML. 
    \return The found widget or 0 if not found.
*/
QGraphicsWidget *HbDocumentLoader::findWidget(const QString &name) const
{
    const Q_D(HbDocumentLoader);
    return d->findWidget(name);
}


/*!
    Retrieves object whose object name is \a name.
    \param name The name of the widget in DocML. 
    \return The found object or 0 if not found.
*/
QObject *HbDocumentLoader::findObject(const QString &name) const
{
    const Q_D(HbDocumentLoader);
    return d->findObject(name);
}


/*!
    Inserts an object tree into the document loader object. 
    You can use the output of the load() method as the \a roots parameter. 

    The document loader does not take ownership of the passed objects.

    \param roots The list of root objects to add to the loader.
    \return True if the objects were successfully added to the document loader, false otherwise.
*/
bool HbDocumentLoader::setObjectTree( QObjectList roots )
{
    Q_D(HbDocumentLoader);
    return d->setObjectTree( roots );    
}

/*!
    Looks up an object which inherits class \a type and for which the 
    object name is \a name. If you decide to override this method in your own document loader class,
    call this base class method in case you are
    not able to create an instance of the specified object type.
    \param type The type of the object to create.
    \param name The object name of the object to create.
    \return The created object or 0 if it could not be created.
*/
QObject *HbDocumentLoader::createObject(const QString& type, const QString &name)
{
#ifdef HB_BOOTSTRAPPED
    Q_UNUSED(type);
    Q_UNUSED(name);
    return 0;
#else
    Q_D(HbDocumentLoader);
    return d->actions->createObjectWithFactory(type, name);
#endif
}


/*!
    Sets loader to an initial state without references to any loaded objects.
*/
void HbDocumentLoader::reset()
{
    Q_D(HbDocumentLoader);
    d->reset();
}


/*!
    Prints the current version of the document loader and the minimum version of supported DocML in brackets
    For example "3.2 (1.4)" means that current version is 3.2 and DocML versions from 1.4 to 3.2 are supported   
*/
QString HbDocumentLoader::version()
{
    return HbDocumentLoaderPrivate::version();
}
