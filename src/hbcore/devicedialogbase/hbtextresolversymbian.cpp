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

#include "hbtextresolversymbian.h"
#include <hbsymbianvariant.h>
#include <e32des16.h>
#include <QByteArray>
#include <QTranslator>
#include <QString>
#include <QDebug>
#include <QFile>
#include <QTextStream>

class HbTextResolverSymbianPrivate: public CBase
    {
public:
    HbTextResolverSymbianPrivate();    
    virtual ~HbTextResolverSymbianPrivate();
    static HbTextResolverSymbianPrivate* Instance();
    TBool LoadTranslationFile();
    TBool Initialize(const TDesC& aFilename, const TDesC& aPath);
    HBufC* LoadLC(const TDesC& aMessageId, TInt aPlural); // Simple string. + %n message(s). Plural and replaces all occurances of %n in the string.
    HBufC* LoadLC(const TDesC& aMessageId, const TDesC& aString, TInt aPlural); // One and %1. %n and %1. Plural and replaces all occurances of %n in the string.
    HBufC* LoadLC(const TDesC& aMessageId, const MDesCArray& aStrings, TInt aPlural);
    HBufC* LoadLC(const TDesC& aMessageId, TInt aInt, TInt aPlural);
    HBufC* LoadLC(const TDesC& aMessageId, const RArray<TInt>& aInts, TInt aPlural);    
        
    QString ConvertToQString(const TDesC& aMessageId, TInt aPlural = -1);
    QTranslator iTranslator;
    QByteArray iByteArray;
    TBool iLoaded; 
    QString fileName;
    QString path;        	      
    };

Q_GLOBAL_STATIC(HbTextResolverSymbianPrivate, hbTextResolverSymbianPrivate)

HbTextResolverSymbianPrivate* HbTextResolverSymbianPrivate::Instance()
    {
    HbTextResolverSymbianPrivate* resolver = hbTextResolverSymbianPrivate();
    return resolver;         
    }
    
static void replacePercentN(QString *result, int n)
{
    if (n >= 0) {
        int percentPos = 0;
        int len = 0;
        while ((percentPos = result->indexOf(QLatin1Char('%'), percentPos + len)) != -1) {
            len = 1;
            QString fmt;
            if (result->at(percentPos + len) == QLatin1Char('L')) {
                ++len;
                fmt = QLatin1String("%L1");
            } else {
                fmt = QLatin1String("%1");
            }
            if (result->at(percentPos + len) == QLatin1Char('n')) {
                fmt = fmt.arg(n);
                ++len;
                result->replace(percentPos, len, fmt);
                len = fmt.length();
            }
        }
    }
}

HbTextResolverSymbianPrivate::HbTextResolverSymbianPrivate()
    {        
    }
    
HbTextResolverSymbianPrivate::~HbTextResolverSymbianPrivate()
    {
    }

TBool HbTextResolverSymbianPrivate::LoadTranslationFile()
    {
    QString locale = QLocale::system().name();        
    return iTranslator.load(fileName + locale, path);                
    }     

TBool HbTextResolverSymbianPrivate::Initialize(const TDesC& aFilename, const TDesC& aPath)
    {
    fileName = QString::fromUtf16(aFilename.Ptr(), aFilename.Length());
    path = QString::fromUtf16(aPath.Ptr(), aPath.Length());
    return LoadTranslationFile();  
    }
       
HBufC* HbTextResolverSymbianPrivate::LoadLC(const TDesC& aMessageId, TInt aPlural)
    {
    QString string = ConvertToQString(aMessageId, aPlural);
        
    TPtrC descriptor(static_cast<const TUint16*>(string.utf16()),
                    string.length());
    return descriptor.AllocLC();
    }
 
HBufC* HbTextResolverSymbianPrivate::LoadLC(const TDesC& aMessageId, const TDesC& aString, TInt aPlural)
    {
    QString string = ConvertToQString(aMessageId, aPlural);
    QString result = ConvertToQString(aString);
    
    if (result.isEmpty())
        {
        result = QString::fromUtf16(aString.Ptr(), aString.Length());
        }
    string = string.arg(result);     
    TPtrC descriptor(static_cast<const TUint16*>(string.utf16()),
                    string.length());
    return descriptor.AllocLC();       
    }

HBufC* HbTextResolverSymbianPrivate::LoadLC(const TDesC& aMessageId, const MDesCArray& aStrings, TInt aPlural)
    {
    QString string = ConvertToQString(aMessageId, aPlural);
    
    QString tmp;
    for (int i(0); i < aStrings.MdcaCount(); i++)
        {
        TPtrC ptr = aStrings.MdcaPoint(i);
        tmp = ConvertToQString(ptr);
        
        if (tmp.isEmpty() == 0)
            {
            tmp = QString::fromUtf16(ptr.Ptr(), ptr.Length());
            }
        
        bool ok(false);
        int conv = tmp.toInt(&ok); 

        if (ok) 
            {   
            string = string.arg(conv);	
            } 
        else
            {
            string = string.arg(tmp);
            }
        }            
    TPtrC descriptor(static_cast<const TUint16*>(string.utf16()),
                    string.length());
    return descriptor.AllocLC();
    }
       
HBufC* HbTextResolverSymbianPrivate::LoadLC(const TDesC& aMessageId, TInt aInt, TInt aPlural)
    {
    QString string = ConvertToQString(aMessageId, aPlural);       
    string = string.arg(aInt); 
    
    TPtrC descriptor(static_cast<const TUint16*>(string.utf16()),
                    string.length());
    return descriptor.AllocLC(); 
    }    
        
HBufC* HbTextResolverSymbianPrivate::LoadLC(const TDesC& aMessageId, const RArray<TInt>& aInts, TInt aPlural)
    {
    QString string = ConvertToQString(aMessageId, aPlural);
    
    for (int i(0); i < aInts.Count(); i++)
        {                              
        string = string.arg(aInts[i]);
        }            
             
    TPtrC descriptor(static_cast<const TUint16*>(string.utf16()),
                    string.length());
    return descriptor.AllocLC();
    }
        
QString HbTextResolverSymbianPrivate::ConvertToQString(const TDesC& aMessageId, TInt aPlural)
    {        
    QString result = QString::fromUtf16(aMessageId.Ptr(), aMessageId.Length());
    iByteArray = result.toLatin1();
    const char* tmp = iByteArray.data();        

    QString string;
    if (aPlural == -1)
        {        
        string = iTranslator.translate(0, tmp, 0);           
        }
    else
        {
        string = iTranslator.translate(0, tmp, 0, aPlural);
        }

    if (string.isEmpty()) 
        {
        string = result;
        }
    replacePercentN(&string, aPlural);
    return string;
    }

/*!
    \class HbTextResolverSymbian
    \brief HbTextResolverSymbian implements translation support for Symbian OS applications.
    
    HbTextResolverSymbian is class that provides convinience API for translation support and
    is meant to be used only by pure Symbian OS applications. Dynamic strings and plurals are
    also supported. 
    
    Before using the translation support HbTextResolverSymbian must be initialized. Note that 
    the filename must not have language specific suffix, because that is handled internally by
    HbTextResolverSymbian.
    
    \code
    _LIT(KSomeFile, "somefile_");
    _LIT(KPath, "c:\\resource");
    
    TBool result = HbTextResolverSymbian::Init(KSomeFile, KPath);
    \endcode
    
    Translating a string given the snippet from a translation file (.ts) below:
    ...
    \code
    <message id="text_test">
        <source>Test</source>
        <translation>Text test</translation>
    </message>
    

    _LIT(KTextTest, "text_test");
    // returns the string "Text test"
    HBufC* tmp = HbTextResolverSymbian::LoadL(KTextTest);
    \endcode
    
    Translating a string with replacement string given the snippet from a translation
    file below:
    
    \code
    <message id="text_test2">
        <source>Test2</source>
        <translation>Test2 %1</translation>
    </message>
    
    
    _LIT(KTextTest2, "text_test2");
    _LIT(KTextTestReplace,"passed.");
    // returns the string "Test2 passed." 
    HBufC* tmp = HbTextResolverSymbian::LoadL(KTextTest2, KTextTestReplace);
    \endcode
   
    Translating a string with replacement strings given the snippet from a translation
    file below:
    
    \code
    <message id="text_test3">
        <source>Test3</source>
        <translation>Test3 %1 %2</translation>
    </message>
    
    
    _LIT(KTextTest3, "text_test3");
    _LIT(KTextTest3Replace1,"is");
    _LIT(KTextTest3Replace2,"passed.");
    
    CDesC16ArrayFlat* array = new CDesC16ArrayFlat(8);
    TBufC<20> buffer;
    buffer = KTextTest3Replace1;
    array->AppendL(buffer);
    buffer = KTextTest3Replace2;
    array->AppendL(buffer);
    // returns the string "Test3 is passed."
    HBufC* tmp = HbTextResolverSymbian::LoadL(KTextTest3, *array);
   \endcode       
   
   Note: String e.g. "Moving %1 files of %2 from %3" where place markers %1 and %2 are
   for integer values and the %3 is for a string, requires that the integers are converted
   to descriptors and appended to MDesArray. Those are converted back to integers during 
   replacement.
   
   Translating a string with replacement integer given the snippet from a translation
   file below:
   \code
    </message>
        <message id="text_test4" numerus="yes">
        <source>Test4</source>
        <translation>
        <numerusform>%n message of %1</numerusform>
        <numerusform>%n messages of %1</numerusform>
        </translation>
    </message>
    
    
    _LIT(KTextTest4, "text_test4");
    // returns the string "1 message of 5"
    HBufC* tmp = HbTextResolverSymbian::LoadL(KTextTest4, 5, 1);
    // // returns the string "2 messages of 5"
    tmp = HbTextResolverSymbian::LoadL(KTextTest4, 5, 2);
    \endcode
    
    Translating a string with replacement integers given the snippet from a translation
    file below:
    \code
    <message id="text_test3">
        <source>Test3</source>
        <translation>Test3 %1 %2</translation>
    </message>
    
    
    RArray<TInt> intarray;
    intarray.Append(20);
    intarray.Append(30);
    
    HBufC* tmp = HbTextResolverSymbian::LoadL(KTextTest3, intarray, 1);
    \endcode
    
*/

/*!
    Initializes the translator. 
        
    \param aFilename specifies the used translation file. 
    \param aPath specifies path to translation file.
    
    Returns ETrue if initialization was succesful.     
*/
EXPORT_C TBool HbTextResolverSymbian::Init(const TDesC& aFilename, const TDesC& aPath)
    {
    HbTextResolverSymbianPrivate* resolver = HbTextResolverSymbianPrivate::Instance();
    return resolver->Initialize(aFilename, aPath);
    }    

/*!
    Translates the string. 
    If aPlural is equal to zero or greater, then the translation depending on the 
    value is returned. Returns a pointer to a heap descriptor containing the translated
    string.  
    If the string for passed \a aMessageId is not found from the translation file then 
    the source string is returned. 
        
    \param aMessageId Identifier of the string to be translated.
    \param aPlural Specifies the plural format of the translation.    
*/
EXPORT_C HBufC* HbTextResolverSymbian::LoadL(const TDesC& aMessageId, TInt aPlural)
    {
    HbTextResolverSymbianPrivate* resolver = HbTextResolverSymbianPrivate::Instance();    
    HBufC* buf = resolver->LoadLC(aMessageId, aPlural);
    CleanupStack::Pop(buf);        
    return buf;
    }

/*!
    Translates the string and replaces the lowest place marker with the replacement string. 
    If \a aPlural is equal to zero or greater, then translation depending on the value
    is returned. Returns a pointer to a heap descriptor containing the formatted string.
    If the string for passed \a aMessageId or \a aString is not found from the translation
    file then the source string is used. 
        
    \param aMessageId Identifier of the string to be translated.
    \param aString Replacement string for the lowest %N.
    \param aPlural Specifies the plural format of the translation.   
*/       
EXPORT_C HBufC* HbTextResolverSymbian::LoadL(const TDesC& aMessageId, const TDesC& aString, TInt aPlural)
    {
    HbTextResolverSymbianPrivate* resolver = HbTextResolverSymbianPrivate::Instance();
    HBufC* buf = resolver->LoadLC(aMessageId, aString, aPlural);
    CleanupStack::Pop(buf);    
    return buf;    
    } 

/*!
    Translates the string and replaces place markers from the lowest place marker with the 
    replacement strings provided in the array. 
    If \a aPlural is equal to zero or greater, then translation depending on the value
    is returned. Returns a pointer to a heap descriptor containing the formatted string.
    If the string for passed \a aMessageId or \a aStrings are not found from the translation
    file then the source string is used.      
        
    \param aMessageId Identifier of the string to be translated.
    \param aStrings Replacement strings for the place markers.
    \param aPlural Specifies the plural format of the translation.   
*/          
EXPORT_C HBufC* HbTextResolverSymbian::LoadL(const TDesC& aMessageId, const MDesCArray& aStrings, TInt aPlural)
    {
    HbTextResolverSymbianPrivate* resolver = HbTextResolverSymbianPrivate::Instance();
    HBufC* buf = resolver->LoadLC(aMessageId, aStrings, aPlural);
    CleanupStack::Pop(buf);
    return buf; 
    }

/*!
    Translates the string and replaces the lowest place marker with the replacement integer. 
    If \a aPlural is equal to zero or greater, then translation depending on the value
    is returned. Returns a pointer to a heap descriptor containing the formatted string.
    If the string for passed \a aMessageId is not found from the translation
    file then the source string is used. 
        
    \param aMessageId Identifier of the string to be translated.
    \param aInt Replacement integer for the lowest place marker %(index).
    \param aPlural Specifies the plural format of the translation.   
*/    
EXPORT_C HBufC* HbTextResolverSymbian::LoadL(const TDesC& aMessageId, TInt aInt, TInt aPlural)
    {
    HbTextResolverSymbianPrivate* resolver = HbTextResolverSymbianPrivate::Instance();
    HBufC* buf = resolver->LoadLC(aMessageId, aInt, aPlural);      
    CleanupStack::Pop(buf);
    return buf;     
    } 

/*!
    Translates the string and replaces place markers from the lowest place marker with the 
    replacement integers provided in the array. 
    If \a aPlural is equal to zero or greater, then translation depending on the value
    is returned. Returns a pointer to a heap descriptor containing the formatted string.
    If the string for passed \a aMessageId is not found from the translation
    file then the source string is used.
           
    \param aMessageId Identifier of the string to be translated.
    \param aInts Replacement integers for the place markers.
    \param aPlural Specifies the plural format of the translation.   
*/         
EXPORT_C HBufC* HbTextResolverSymbian::LoadL(const TDesC& aMessageId, const RArray<TInt>& aInts, TInt aPlural)
    {
    HbTextResolverSymbianPrivate* resolver = HbTextResolverSymbianPrivate::Instance();
    HBufC* buf = resolver->LoadLC(aMessageId, aInts, aPlural);
    CleanupStack::Pop(buf);    
    return buf;
    }

/*!
    Translates the string. 
    If aPlural is equal to zero or greater, then the translation depending on the 
    value is returned. Returns a pointer to a heap descriptor containing the translated
    string. Pointer is pushed to cleanup stack.  
    If the string for passed \a aMessageId is not found from the translation file then 
    the source string is returned. 
        
    \param aMessageId Identifier of the string to be translated.
    \param aPlural Specifies the plural format of the translation.    
*/     
EXPORT_C HBufC* HbTextResolverSymbian::LoadLC(const TDesC& aMessageId, TInt aPlural)
    {
    HbTextResolverSymbianPrivate* resolver = HbTextResolverSymbianPrivate::Instance();
    return resolver->LoadLC(aMessageId, aPlural);       
    }

/*!
    Translates the string and replaces the lowest place marker with the replacement string. 
    If \a aPlural is equal to zero or greater, then translation depending on the value
    is returned. Returns a pointer to a heap descriptor containing the formatted string. 
    Pointer is pushed to cleanup stack.
    If the string for passed \a aMessageId or \a aString is not found from the translation
    file then the source string is used. 
        
    \param aMessageId Identifier of the string to be translated.
    \param aString Replacement string for the lowest %N.
    \param aPlural Specifies the plural format of the translation.   
*/         
EXPORT_C HBufC* HbTextResolverSymbian::LoadLC(const TDesC& aMessageId, const TDesC& aString, TInt aPlural)
    {
    HbTextResolverSymbianPrivate* resolver = HbTextResolverSymbianPrivate::Instance();
    return resolver->LoadLC(aMessageId, aString, aPlural);
    }

/*!
    Translates the string and replaces place markers from the lowest place marker with the 
    replacement strings provided in the array. 
    If \a aPlural is equal to zero or greater, then translation depending on the value
    is returned. Returns a pointer to a heap descriptor containing the formatted string.
    Pointer is pushed to cleanup stack.
    If the string for passed \a aMessageId or \a aStrings are not found from the translation
    file then the source string is used.
        
    \param aMessageId Identifier of the string to be translated.
    \param aStrings Replacement strings for the place markers.
    \param aPlural Specifies the plural format of the translation.   
*/
EXPORT_C HBufC* HbTextResolverSymbian::LoadLC(const TDesC& aMessageId, const MDesCArray& aStrings, TInt aPlural)
    {
    HbTextResolverSymbianPrivate* resolver = HbTextResolverSymbianPrivate::Instance();
    return resolver->LoadLC(aMessageId, aStrings, aPlural);
    }

/*!
    Translates the string and replaces the lowest place marker with the replacement integer. 
    If \a aPlural is equal to zero or greater, then translation depending on the value
    is returned. Returns a pointer to a heap descriptor containing the formatted string.
    If the string for passed \a aMessageId is not found from the translation
    file then the source string is used. 
        
    \param aMessageId Identifier of the string to be translated.
    \param aInt Replacement integer for the lowest place marker %(index).
    \param aPlural Specifies the plural format of the translation.   
*/ 
EXPORT_C HBufC* HbTextResolverSymbian::LoadLC(const TDesC& aMessageId, TInt aInt, TInt aPlural)
    {
    HbTextResolverSymbianPrivate* resolver = HbTextResolverSymbianPrivate::Instance();   
    return resolver->LoadLC(aMessageId, aInt, aPlural);
    }

/*!
    Translates the string and replaces place markers from the lowest place marker with the 
    replacement integers provided in the array. 
    If \a aPlural is equal to zero or greater, then translation depending on the value
    is returned. Returns a pointer to a heap descriptor containing the formatted string.
    If the string for passed \a aMessageId is not found from the translation
    file then the source string is used.
           
    \param aMessageId Identifier of the string to be translated.
    \param aInts Replacement integers for the place markers.
    \param aPlural Specifies the plural format of the translation.   
*/            
EXPORT_C HBufC* HbTextResolverSymbian::LoadLC(const TDesC& aMessageId, const RArray<TInt>& aInts, TInt aPlural)
    {
    HbTextResolverSymbianPrivate* resolver = HbTextResolverSymbianPrivate::Instance();
    return resolver->LoadLC(aMessageId, aInts, aPlural);
    }

