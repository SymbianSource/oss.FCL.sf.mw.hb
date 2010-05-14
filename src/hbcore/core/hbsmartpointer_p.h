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

#ifndef SMARTPOINTER_P_H
#define SMARTPOINTER_P_H

#include "hbmemoryutils_p.h"

template <typename T>
class  smart_ptr
{
public:
    typedef T *                pointer;
    typedef T                 value_type;
    typedef T &               reference;
    typedef int               difference_type;
public:
    
    /*
     * C'tor
    */
    smart_ptr( pointer ptr = 0, HbMemoryManager::MemoryType type = HbMemoryManager::InvalidMemory )
    { 
        mType = type;
        this->setOffset(ptr);
    }
    
    /*
     * Constructor from other pointer.
    */
    template <class U>
    smart_ptr( U *ptr, HbMemoryManager::MemoryType type ) 
    {  
        mType = type;
        pointer p (ptr);
        (void)p;
        this->setOffset(p);
    }

    /*
     * Copy C'tor
    */
    smart_ptr( const smart_ptr &other )
    { 
        mType = other.mType;
        this->setOffset( other.get() );
    }

    /*
     *  Consttuction from other smart_ptr. If pointers of the pointee types are 
     *  convertible, smart_ptr will be convertibles.
    */
    template<class T2>
    smart_ptr( const smart_ptr<T2> &other ) 
    {  
        this->mType = other.memoryType();
        pointer p( other.get() );
        (void)p;
        this->setOffset(p);
    }

    smart_ptr operator+( difference_type offset )
    {
        return smart_ptr( get() + offset, this->mType );
    }

    reference operator*() const
    { 
        return *get();
    }

    reference operator[]( difference_type idx ) const
    {
        return this->get()[idx];
    }

    smart_ptr & operator++( void )
    {
        incOffset( sizeof(T) );
        return *this;
    }

    smart_ptr operator++(int)
    { 
        smart_ptr temp(*this);
        ++*this;
        return temp;
    }

    smart_ptr & operator-- (void) 
    {  
        this->decOffset(sizeof (T));
        return *this;
    }

    smart_ptr operator-- (int)
    {  
        smart_ptr temp(*this);
        --*this;
        return temp;
    }

    bool operator == ( const smart_ptr<T> &other )
    { 
        return ( get() == other.get() && mType == other.mType );
    }

    bool operator != ( const smart_ptr<T> &other )
    { 
        return ( get() != other.get() && mType == other.mType );
    }

    bool operator! () const
    {
        return this->get() == 0;   
    }

    pointer get() const
    {
        return static_cast<pointer>( rawPointer() );
    }

    // this two functions are here as change of the smart pointer behaviour
    //of getting offset wrt shared memory baseaddress.
    int offset()
    { 
        return mOffset;
    }

    void setOffset( difference_type offset )
    { 
       mOffset = offset;
    }

	// This function is here for being able to modify offset when
	// cells in shared memory are moved.
	difference_type *offsetPtr()
	{
		return &mOffset;
	}

    HbMemoryManager::MemoryType memoryType() const
    {
        return mType;
    }

    pointer operator->() const           
    {
        return this->get();
    }

    smart_ptr & operator= ( const smart_ptr &other )
    { 
        mType = other.mType;    
        pointer p( other.get() );
        (void)p; 
        this->setOffset(p); 
        return *this;
    }

/*
* Assignment From other smart_ptr
*/
    template <class T2>
    smart_ptr & operator= ( const smart_ptr<T2> & other )
    {  
        mType = other.memoryType();
        pointer p( other.get() );
        this->setOffset(p);
        return *this;
    }

    smart_ptr & operator= ( pointer from )
    {
        this->setOffset(from); 
        return *this;
    }

    smart_ptr operator+ ( difference_type offset ) const   
    {  
        return smart_ptr( this->get() + offset, this->mType );
    }

    smart_ptr operator- ( difference_type offset ) const   
    {  
        return smart_ptr( this->get() - offset, this->mType );
    }

    smart_ptr &operator+= ( difference_type offset )
    {  
        this->incOffset( offset * sizeof (T) );
        return *this; 
    }

    smart_ptr &operator-= ( difference_type offset )
    {  
        this->decOffset( offset * sizeof (T) );
        return *this;
    }

    operator void *()
    {
        return (void*)this->get();
    }

private:

   typedef smart_ptr<T>           self_t;

   void unspecified_bool_type_func() const {}
   typedef void ( self_t::*unspecified_bool_type )() const;

   void setOffset( const void *ptr )
   {
      const char *p = static_cast<const char*>(ptr);
      // mOffset -1 is Null pointer.    
      if( !ptr ){
         mOffset = -1;
      }
      else{
          GET_MEMORY_MANAGER(mType);
          Q_ASSERT( HbMemoryManager::InvalidMemory  !=  mType );
          mOffset = p - (char*)manager->base();
          Q_ASSERT( mOffset != -1 );
      }
   }
    
   void * rawPointer() const
   { 
       Q_ASSERT( HbMemoryManager::InvalidMemory  !=  mType );
       GET_MEMORY_MANAGER(mType);
       // mOffset -1 is Null pointer.
       return ( mOffset == -1 ) ? 0: ( (char*)manager->base() + mOffset );
   }

   void incOffset( difference_type bytes )
   { mOffset += bytes; }

   void decOffset( difference_type bytes )
   { mOffset -= bytes; }

private:
    difference_type mOffset;
    HbMemoryManager::MemoryType mType;
   
};

template<typename T1, typename T2>
inline bool operator == ( const smart_ptr<T1> &pt1,
                         const smart_ptr<T2> &pt2 )
{  return pt1.get() == pt2.get(); }

template<class T1, class T2>
inline bool operator!= ( const smart_ptr<T1> &pt1, 
                        const smart_ptr<T2> &pt2 )
{  return pt1.get() != pt2.get();  }

template<class T1, class T2>
inline bool operator< ( const smart_ptr<T1> &pt1, 
                       const smart_ptr<T2> &pt2 )
{  return pt1.get() < pt2.get();  }

template<class T1, class T2>
inline bool operator<= ( const smart_ptr<T1> &pt1, 
                        const smart_ptr<T2> &pt2 )
{  return pt1.get() <= pt2.get();  }

template<class T1, class T2>
inline bool operator> ( const smart_ptr<T1> &pt1, 
                       const smart_ptr<T2> &pt2 )
{  return pt1.get() > pt2.get();  }

template<class T1, class T2>
inline bool operator>= ( const smart_ptr<T1> &pt1, 
                        const smart_ptr<T2> &pt2 )
{  return pt1.get() >= pt2.get();  }

template<class T, class T2>
inline int operator- ( const smart_ptr<T> &pt, const smart_ptr<T2> &pt2 )
{  return pt.get() - pt2.get();   }

#endif // SMARTPOINTER_P_H
