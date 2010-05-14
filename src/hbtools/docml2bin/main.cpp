/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbTools module of the UI Extensions for Mobile.
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

#include <hbdocumentloader.h>
#include <QtGui>
#include <assert.h>
#include <iostream>


void showHelp() {
    std::cout << "docml2bin.exe usage:\n\n";

    std::cout << "docml2bin -s sourceFile [-t targetFile]\n\n";

    std::cout << "    Converts plain text docml file to a binary file.\n\n";

    std::cout << "    If target file name is not given creates file named\n";
    std::cout << "    <source file> + \".bin\".\n\n";

    std::cout << "options:\n\n";

    std::cout << " -s \t\tname of the source file (\"<filename.docml>\").\n";
    std::cout << " -t \t\tname of the targe file (\"<binaryfile.bin>\").\n\n";

    std::cout << "Example:\n";
    std::cout << "docml2bin.exe -s myfile.docml -t c:/resouces/bin/mybinary.docml\n\n";
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv, false); // GUIenabled=false

    if (argc <= 2) {
        showHelp();
    } else {
        //bool backup = false;
        QString source, target;
        //QStringList restore;
        QStringList args(app.arguments());

        for (int n = 0; n < args.count(); n++) {
            if (args[n].toLower() == "-s") {
                source = args[n+1];
                n++;
            } else if (args[n].toLower() == "-t") {
                target = args[n+1];
                n++;
            /*
            } else if (args[n].toLower() == "--backup-and-replace") {
                backup = true;
            } else if (args[n].toLower() == "-restore") {
                n++;
                while (n < args.count()) {
                    restore.append( args[n] );
                    n++;
                }
            */
            }
        }
        /*
        if (restore.count()) {
            for (int i = 0; i < restore.count(); i++) {
                QString orig = restore.at(i) + ".orig";
                if ( QFile::exists(orig) ) {
                    QFile::remove(restore.at(i));
                    QFile origFile(orig);
                    origFile.rename(restore.at(i));
                }
            }
            return 0;
        }
        */

        if (source.length() > 0) {
            /*
            if (backup) {
                QString orig = source + ".orig";
                if ( QFile::exists(orig) ) {
                    QFile::remove(orig);
                }
                QFile origFile(source);
                origFile.rename(orig);
                target = source;
                source = orig;
            }
            */
            if (!QFile::exists(source)) {
                std::cout << "Error: file " << source.toStdString() << " does not exist.\n";
            } else {
                // Open file and parse lines. Each line should have three value separated with:
                QFile sourceFile(source);
                if (sourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    if (!target.length()) {
                        target = source + ".bin";
                    }
                    QFile targetFile(target);
                    if (targetFile.open(QIODevice::WriteOnly)) {
                        HbDocumentLoader loader;
                        loader.createBinary( &sourceFile, &targetFile );
                        targetFile.close();
                    }
                    sourceFile.close();
                }
            }
        }
    }

    return 0;
}

