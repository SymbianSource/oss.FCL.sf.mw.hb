/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbServers module of the UI Extensions for Mobile.
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

#ifndef HBDOUBLELINKEDLIST_P_H
#define HBDOUBLELINKEDLIST_P_H


template<class ElemType>
class HbDLinkList;

/*
 * Template classes for linkage in a doubly-linked list.
 * ObDLink is a class for linkage in a doubly-linked list.  For each
 * list that an element is in, it needs to declare a member of type
 * ObDLink.
 */

template<class ElemType>
class HbDLink
{
    friend class HbDLinkList<ElemType>;
public:

    inline HbDLink();

    inline ElemType *next() const;

    inline ElemType *prev() const;

    inline void setNext(ElemType *element);

    inline void setPrev(ElemType *element);

private:

    ElemType *mNext;
    ElemType *mPrev;

};


/**
 * HbDLinkList is a doubly-linked list of ElemTypes.  ElemType must
 * declare at least one member variable of type HbDLink<ElemType> for
 * linkage in a linked list.Each variable of type HbDLink that
 * ElemType has represents a potential HbDLinkList that ElemType can be
 * in.  One of the arguments to the HbDLinkList constructor is a pointer
 * to the HbDLink member within ElemType of the HbDLink to use for
 * that list, so an ElemType can be in multiple independent HbDLinkList
 * at a time.
 *
 * The reason for using this class instead of a templated container
 * like QLinkedList which does not place any requirements on the element type is to
 * provide O(1) deletion and no ownership of the Elements.
 *
 *
 * HbDLinkList does not own the elements in the list means,
 * it does not destroy them when it is destroyed.
 *
 */

template<class ElemType>
class HbDLinkList
{
public:
    inline explicit HbDLinkList(HbDLink<ElemType> ElemType:: *dLink);
    inline ~HbDLinkList();
    inline void insertBack(ElemType *item);
    inline ElemType* removeFront();
    inline void removeNode(ElemType *item);
    inline void removeAll();
    inline bool isEmpty() const;
    inline ElemType *front() const;
    inline ElemType *back() const;

private:
    inline void insertFrontIfListEmpty(ElemType *item);
    inline void insertAfter(ElemType *newItem, ElemType *lastItem);

private:
    ElemType *listFront;
    ElemType *listBack;
    HbDLink<ElemType> ElemType:: *mDLink;
};

#include "hbdoublelinkedlistinline_p.h"

#endif // HBDOUBLELINKEDLIST_P_H

