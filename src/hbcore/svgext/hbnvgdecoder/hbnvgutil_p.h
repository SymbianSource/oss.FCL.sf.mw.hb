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

#ifndef HB_NVGUTIL_P_H
#define HB_NVGUTIL_P_H

#include <VG/openvg.h>

#ifndef HB_NVG_DEBUG
#define NVG_DEBUGP1(x1)
#define NVG_DEBUGP2(x1, x2)
#define NVG_DEBUGP3(x1, x2, x3)
#define NVG_DEBUGP4(x1, x2, x3, x4)
#define NVG_DEBUGP5(x1, x2, x3, x4, x5)
#define NVG_DEBUGP6(x1, x2, x3, x4, x5, x6)
#define NVG_DEBUGP7(x1, x2, x3, x4, x5, x6, x7)
#define NVG_DEBUGP8(x1, x2, x3, x4, x5, x6, x7, x8)
#define NVG_DEBUGP9(x1, x2, x3, x4, x5, x6, x7, x8, x9)

#define NVG_DEBUGSTMT(x1)

#define VGCREATEPAINT()     vgCreatePaint()
#define VGDESTROYPAINT(h)   vgDestroyPaint(h)

#define VGCREATEPATH(a1, a2, a3, a4, a5, a6, a7)  vgCreatePath(a1, a2, a3, a4, a5, a6, a7)
#define VGDESTROYPATH(h)     vgDestroyPath(h)
    
#define VGCREATEIMAGE(a1, a2, a3, a4) vgCreateImage(a1, a2, a3, a4)
#define VGDESTROYIMAGE(h)     vgDestroyImage(h)

#define VGDESTROYPAINT(h)   vgDestroyPaint(h)
#define DECLARE_HANDLECHECKER_GLOBAL()
#define DECLARE_HANDLECHECKER()
#define INIT_HANDLECHECKER()
#define ASSERT_HANDLE_COUNT()
    
#else

#define NVG_DEBUGP1(x1)                                 qDebug(x1)
#define NVG_DEBUGP2(x1, x2)                             qDebug(x1, x2)
#define NVG_DEBUGP3(x1, x2, x3)                         qDebug(x1, x2, x3)
#define NVG_DEBUGP4(x1, x2, x3, x4)                     qDebug(x1, x2, x3, x4)
#define NVG_DEBUGP5(x1, x2, x3, x4, x5)                 qDebug(x1, x2, x3, x4, x5)
#define NVG_DEBUGP6(x1, x2, x3, x4, x5, x6)             qDebug(x1, x2, x3, x4, x5, x6)
#define NVG_DEBUGP7(x1, x2, x3, x4, x5, x6, x7)         qDebug(x1, x2, x3, x4, x5, x6, x7)
#define NVG_DEBUGP8(x1, x2, x3, x4, x5, x6, x7, x8)     qDebug(x1, x2, x3, x4, x5, x6, x7, x8)
#define NVG_DEBUGP9(x1, x2, x3, x4, x5, x6, x7, x8, x9) qDebug(x1, x2, x3, x4, x5, x6, x7, x8, x9)

#define NVG_DEBUGSTMT(x1)                             do { x1; } while (0)

struct HandleCounter
        {
        int    paintHC;
        int    pathHC;
        int    imageHC;
        HandleCounter() : paintHC(0), pathHC(0), imageHC(0) {}
        };

inline void incrementPaintHandleCountL(HandleCounter * handleCounter)
    {
    if (handleCounter){
        handleCounter->paintHC++;
    }
    }

inline void decrementPaintHandleCountL(HandleCounter * handleCounter)
    {
    if (handleCounter){
        handleCounter->paintHC--;
    }
    }

inline void decrementImageHandleCountL(HandleCounter * handleCounter)
    {
    if (handleCounter){
        handleCounter->imageHC--;
    }
    }

inline void decrementPathHandleCountL(HandleCounter * handleCounter)
    {
    if (handleCounter){
        handleCounter->pathHC--;
    }
    }

inline void incrementPathHandleCountL(HandleCounter * handleCounter)
    {
    if (handleCounter){
        handleCounter->pathHC++;
    }
    }

inline void incrementImageHandleCountL(HandleCounter * handleCounter)
    {
    if (handleCounter){
        handleCounter->imageHC++;
    }
    }

inline void checkHandleCout(HandleCounter * handleCounter)
    {
    if (handleCounter)
        {
        if (handleCounter->paintHC != 0){
			// @TODO, throw panic.
            //User::Panic(_L("Paint Handle Deallocation Not Matching"), 1);
        }
        
        if (handleCounter->pathHC != 0){
            // @TODO , throw panic
            //User::Panic(_L("Path Handle Deallocation Not Matching"), 1);
        }
        
        if (handleCounter->imageHC != 0){
            // @TODO , throw panic
            //User::Panic(_L("Image Handle Deallocation Not Matching"), 1);
        }
    }
    
    delete handleCounter;
    handleCounter = 0;
    }

inline VGHandle vgCreatePaintWrapper(HandleCounter * handleCounter)
    {
    VGPaint pH = vgCreatePaint();
    if (pH){
        incrementPaintHandleCountL(handleCounter);
    }
    return pH;
    }
                                    
inline void vgDestroyPaintWrapper(HandleCounter * handleCounter, VGHandle h)
    {
    if (h){
        vgDestroyPaint(h);
        decrementPaintHandleCountL(handleCounter);
    }
    }
    
    
inline VGHandle vgCreatePathWrapper(HandleCounter * handleCounter, VGint a1,
        VGPathDatatype a2,
        VGfloat a3, VGfloat a4,
        VGint a5,
        VGint a6,
        VGbitfield a7)
    {
    VGPath pH = vgCreatePath(a1, a2, a3, a4, a5, a6, a7);
    if (pH){
        incrementPathHandleCountL(handleCounter);
    }
    return pH;
    }
   
inline void vgDestroyPathWrapper(HandleCounter * handleCounter, VGHandle h)
    {
    if (h){
        vgDestroyPath(h);
        decrementPathHandleCountL(handleCounter);
    }
    }
    
inline VGHandle vgCreateImageWrapper(HandleCounter * handleCounter, VGImageFormat a1,
        VGint a2, VGint a3,
        VGbitfield a4)
    {
    VGImage iH = vgCreateImage(a1, a2, a3, a4);
    if (iH){
        incrementImageHandleCountL(handleCounter);
    }
    return iH;
    }

inline void vgDestroyImageWrapper(HandleCounter * handleCounter, VGHandle h)
    {
    if (h){
        vgDestroyImage(h);
        decrementImageHandleCountL(handleCounter);
    }
    }
   
    #define DECLARE_HANDLECHECKER_GLOBAL()  HandleCounter * __handleCounter;
    #define DECLARE_HANDLECHECKER()  private:  HandleCounter * __handleCounter;
    #define INIT_HANDLECHECKER()  __handleCounter = new HandleCounter;
    #define ASSERT_HANDLE_COUNT() checkHandleCout(__handleCounter);

    #define VGCREATEPAINT()     (NVG_DEBUGP3("/*vgCreatePaint() %s:%d*/", __FILE__, __LINE__), vgCreatePaintWrapper(__handleCounter))
    #define VGDESTROYPAINT(h)   (NVG_DEBUGP3("/*vgDestroyPaint() %s:%d*/", __FILE__, __LINE__), vgDestroyPaintWrapper(__handleCounter, h))
    
    #define VGCREATEPATH(a1, a2, a3, a4, a5, a6, a7)  (NVG_DEBUGP3("/*vgCreatePath() %s:%d*/", __FILE__, __LINE__), vgCreatePathWrapper(__handleCounter, a1, a2, a3, a4, a5, a6, a7))
	#define VGDESTROYPATH(h)     (NVG_DEBUGP3("/*vgDestroyPath() %s:%d*/", __FILE__, __LINE__), vgDestroyPathWrapper(__handleCounter, h))

    #define VGCREATEIMAGE(a1, a2, a3, a4) (NVG_DEBUGP3("/*vgCreateImage() %s:%d*/", __FILE__, __LINE__), vgCreateImageWrapper(__handleCounter, a1, a2, a3, a4))
    #define VGDESTROYIMAGE(h)     (NVG_DEBUGP3("/*vgDestroyImage() %s:%d*/", __FILE__, __LINE__), vgDestroyImageWrapper(__handleCounter, h))

#endif

#ifdef OPENVG_OBJECT_CACHING
// first one is for openVG object caching, the macro is used for testing purpose
#define COND_COM_OC(obj, s1, s2)       if (obj) { s1; } else { s2; }

// if object caching is enabled take the first one, which is for object caching,
// else the second one
#define COND_COM_OC_NOC(s1, s2)        s1

// if object caching is enabled take the first one, which is for object caching
// else don't do anything
#define COND_COM_OC_OOC(s1)            s1

#else

// first one is for openVG object caching, the macro is used for testing purpose
#define COND_COM_OC(obj, s1, s2)       s2

// if object caching is enabled take the first one, which is for object caching,
// else the second one
#define COND_COM_OC_NOC(s1, s2)        s2

// if object caching is enabled take the first one, which is for object caching
// else don't do anything
// if object caching is enabled take the first one, which is for object caching
// else don't do anything
#define COND_COM_OC_OOC(s1)

// if object caching is enabled take the first one, which is for object caching
// else don't do anything
#endif

/*
 * There is an Align4 function in symbian which does the alignement
 * this is just to check whether the given pointer is aligned or not
 */
template <class T>
inline bool isAligned4(T aValue)
    {return !((quint32)aValue & (sizeof(quint32) - 1));}

/*
 * There is an Align2 function which does the alignement
 * this is just to check whether the given pointer is aligned or not
 */
template <class T>
inline bool isAligned2(T aValue)
    {return !((quint16)aValue & (sizeof(quint16) - 1));}
    
#endif /* HBNVGUTIL_H*/
