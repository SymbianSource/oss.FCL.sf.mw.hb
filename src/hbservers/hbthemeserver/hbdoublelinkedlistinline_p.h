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

#ifndef HBDOUBLELINKEDLIST_INLINE_P_H
#define HBDOUBLELINKEDLIST_INLINE_P_H

/*!
    @hbserver
    \class HbDLink
    \brief HbDLink implements a linkage linkage in a doubly-linked list

*/

template<class ElemType> inline
HbDLink<ElemType>::HbDLink()
        : mNext(0), mPrev(0)
{
}

template<class ElemType> inline
ElemType*
HbDLink<ElemType>::next() const
{
    return mNext;
}

template<class ElemType> inline
ElemType*
HbDLink<ElemType>::prev() const
{
    return mPrev;
}

template<class ElemType> inline
void
HbDLink<ElemType>::setNext(ElemType *element)
{
    mNext = element;
}

template<class ElemType> inline
void
HbDLink<ElemType>::setPrev(ElemType *element)
{
    mPrev = element;
}

/*!
    @hbserver
    \class HbDLinkList
    \brief HbDLinkList implements a generic doubly link list of ElemType.

    HbDlinkList class use for List Recently Used policy implementation.
*/


/*!
    \fn HbDLinkList::HbDLinkList()
    Constructor
    Constructs a HbDLinkList obect
    \a dLink ia a pointer to HbDLink member of ElemType to use for this list.
 */
template<class ElemType> inline
HbDLinkList<ElemType>::HbDLinkList(HbDLink<ElemType> ElemType:: *dLink)
        : listFront(0), listBack(0), mDLink(dLink)
{

}

/*!
    \fn HbDLinkList::~HbDLinkList()
    Destructor

 */
template<class ElemType> inline
HbDLinkList<ElemType>::~HbDLinkList()
{
    // remove links before remove.
    if ((listFront != 0) || (listBack != 0)) {
        removeAll();
    }
}

/*!
    \fn HbDLinkList::insertBack()
    insert a node after the last node
    \a newItem denotes the item to be inserted after the last node
 */
template<class ElemType> inline
void
HbDLinkList<ElemType>::insertBack(ElemType* newItem)
{
    if (newItem == 0) {
        return;
    }
    if (listBack == 0) {
        insertFrontIfListEmpty(newItem);
    } else {
        insertAfter(newItem, listBack);
    }
}

/*!
    \fn HbDLinkList::removeFront()
    remove the front node
 */
template<class ElemType> inline
ElemType*
HbDLinkList<ElemType>::removeFront()
{
    ElemType* itemToRemove = listFront;
    if (itemToRemove != 0) {
        removeNode(itemToRemove);
    } else {
        return 0;
    }
    return itemToRemove;
}

/*!
    \fn HbDLinkList::removeNode()
    remove a particular node
    /a itemToRemove denotesthe cache item to be removed from the list
 */
template<class ElemType> inline
void
HbDLinkList<ElemType>::removeNode(ElemType* itemToRemove)
{
    if (!itemToRemove) {
        return;
    }

    if (itemToRemove == listFront) {
        listFront = (listFront->*mDLink).next();
        if (listFront) {
            (listFront->*mDLink).mPrev = 0;
        } else {
            listBack = 0;
        }
    } else if (itemToRemove == listBack) {
        listBack = (listBack->*mDLink).prev();
        if (listBack) {
            (listBack->*mDLink).mNext = 0;
        }
    } else {
        ((itemToRemove->*mDLink).prev()->*mDLink).mNext = (itemToRemove->*mDLink).next();
        ((itemToRemove->*mDLink).next()->*mDLink).mPrev = (itemToRemove->*mDLink).prev();
    }
    (itemToRemove->*mDLink).mNext = 0;
    (itemToRemove->*mDLink).mPrev = 0;
}

/*!
    \fn HbDLinkList::removeAll()
    remove all the items in the list.
 */
template<class ElemType> inline
void
HbDLinkList<ElemType>::removeAll()
{
    while (listFront) {
        removeFront();
    }
}

/*!
    \fn HbDLinkList::isEmpty()
    Check if list is empty
 */
template<class ElemType> inline
bool
HbDLinkList<ElemType>::isEmpty() const
{
    if (listFront != 0)
        return false;
    else
        return true;
}

/*!
    \fn HbDLinkList::front()
    Return the node at the front of list
 */
template<class ElemType> inline
ElemType*
HbDLinkList<ElemType>:: front() const
{
    return listFront;
}

/*!
    \fn HbDLinkList::front()
    Return the last node in the list
 */
template<class ElemType> inline
ElemType*
HbDLinkList<ElemType>:: back() const
{
    return listBack;
}

/*!
    \fn HbDLinkList::insertFrontIfListEmpty()
    This is a private function. We are only interested in inserting at the back of list.
    However, when a list is empty both front and back point to the same item.
    \a newItem denotes the cache item to be inserted at the front of the list
 */
template<class ElemType> inline
void
HbDLinkList<ElemType>::insertFrontIfListEmpty(ElemType* newItem)
{
    if (listFront == 0) {
        listFront = newItem;
        listBack = newItem;
    }
}

/*!
    \fn HbDLinkList::insertAfter()
    insert a node after a given node
    \a newItem denotes the new cache item to be inserted
    \a lastItem denotes the cache item after which the new item is to be inserted
 */
template<class ElemType> inline
void
HbDLinkList<ElemType>::insertAfter(ElemType *newItem, ElemType *lastItem)
{
    (newItem->*mDLink).mNext = (lastItem->*mDLink).next() ;
    (newItem->*mDLink).mPrev = lastItem;

    if ((lastItem->*mDLink).next() == 0) {
        listBack = newItem;
    } else {
        ((lastItem->*mDLink).mNext->*mDLink).mPrev = newItem;
    }
    (lastItem->*mDLink).mNext = newItem;
}

#endif

