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

#ifndef HBSMARTPOINTER_P_H
#define HBSMARTPOINTER_P_H

#include "hbmemoryutils_p.h"

template <typename T>
class  smart_ptr
{
public:
    typedef T *               pointer;
    typedef T                 value_type;
    typedef T &               reference;
    typedef qptrdiff          difference_type;
public:
    
    /*
     * C'tor
    */
    HB_ALWAYS_INLINE smart_ptr( pointer ptr = 0, HbMemoryManager::MemoryType type = HbMemoryManager::InvalidMemory )
    { 
        mType = type;
        setOffset(ptr);
    }
    
    /*
     * Constructor from other pointer.
    */
    template <class U>
    HB_ALWAYS_INLINE smart_ptr( U *ptr, HbMemoryManager::MemoryType type )
    {  
        mType = type;
        pointer p(ptr);
        (void)p;
        setOffset(p);
    }

    /*
     * Copy C'tor
    */
    HB_ALWAYS_INLINE smart_ptr( const smart_ptr &other )
    { 
        mType = other.mType;
        setOffset(other.get());
    }

    /*
     *  Consttuction from other smart_ptr. If pointers of the pointee types are 
     *  convertible, smart_ptr will be convertibles.
    */
    template<class T2>
    HB_ALWAYS_INLINE smart_ptr( const smart_ptr<T2> &other )
    {  
        mType = other.memoryType();
        pointer p(other.get());
        setOffset(p);
    }

    HB_ALWAYS_INLINE smart_ptr operator+( difference_type offset )
    {
        return smart_ptr(get() + offset, mType);
    }

    HB_ALWAYS_INLINE reference operator*() const
    { 
        return *get();
    }

    HB_ALWAYS_INLINE reference operator[]( difference_type idx ) const
    {
        return get()[idx];
    }

    HB_ALWAYS_INLINE smart_ptr & operator++( void )
    {
        incOffset( sizeof(T) );
        return *this;
    }

    HB_ALWAYS_INLINE smart_ptr operator++(int)
    { 
        smart_ptr temp(*this);
        ++*this;
        return temp;
    }

    HB_ALWAYS_INLINE smart_ptr & operator-- (void)
    {  
        decOffset(sizeof(T));
        return *this;
    }

    HB_ALWAYS_INLINE smart_ptr operator-- (int)
    {  
        smart_ptr temp(*this);
        --*this;
        return temp;
    }

    HB_ALWAYS_INLINE bool operator == ( const smart_ptr<T> &other )
    { 
        return (get() == other.get() && mType == other.mType); 
    }

    HB_ALWAYS_INLINE bool operator != ( const smart_ptr<T> &other )
    { 
        return (get() != other.get() && mType == other.mType);
    }

    HB_ALWAYS_INLINE bool operator! () const
    {
        return get() == 0;
    }

    HB_ALWAYS_INLINE pointer get() const
    {
        return static_cast<pointer>( rawPointer() );
    }

    // this two functions are here as change of the smart pointer behaviour
    //of getting offset wrt shared memory baseaddress.
    HB_ALWAYS_INLINE qptrdiff offset()
    { 
        return mOffset;
    }

    HB_ALWAYS_INLINE void setOffset( difference_type offset )
    { 
       mOffset = offset;
    }

    // This function is here for being able to modify offset when
    // cells in shared memory are moved.
    HB_ALWAYS_INLINE difference_type *offsetPtr()
    {
        return &mOffset;
    }

    HB_ALWAYS_INLINE HbMemoryManager::MemoryType memoryType() const
    {
        return mType;
    }

    HB_ALWAYS_INLINE pointer operator->() const      
    {
        return get();
    }

    HB_ALWAYS_INLINE smart_ptr & operator = ( const smart_ptr &other )
    { 
        mType = other.mType;    
        this->setOffset(other.get()); 
        return *this;
    }

/*
* Assignment From other smart_ptr
*/
    template <class T2>
    HB_ALWAYS_INLINE smart_ptr & operator = ( const smart_ptr<T2> & other )
    {  
        mType = other.memoryType();
        setOffset(other.get());
        return *this;
    }

    HB_ALWAYS_INLINE smart_ptr & operator = ( pointer from )
    {
        setOffset(from);
        return *this;
    }

    HB_ALWAYS_INLINE smart_ptr operator+ ( difference_type offset ) const
    {  
        return smart_ptr(get() + offset, mType);
    }

    HB_ALWAYS_INLINE smart_ptr operator- ( difference_type offset ) const 
    {  
        return smart_ptr(get() - offset, mType);
    }

    HB_ALWAYS_INLINE smart_ptr &operator += ( difference_type offset )
    {  
        incOffset(offset * sizeof(T));
        return *this; 
    }

    HB_ALWAYS_INLINE smart_ptr &operator-= ( difference_type offset )
    {  
        decOffset(offset * sizeof(T));
        return *this;
    }

    HB_ALWAYS_INLINE operator void *()
    {
        return (void*)get();
    }

private:

   typedef smart_ptr<T> self_t;

   void unspecified_bool_type_func() const {}
   typedef void ( self_t::*unspecified_bool_type )() const;

   HB_ALWAYS_INLINE void setOffset( const void *ptr )
   {
      const char *p = reinterpret_cast<const char*>(ptr);
      // mOffset -1 is Null pointer.    
      if( !ptr ){
         mOffset = -1;
      }
      else{
          GET_MEMORY_MANAGER(mType);
          mOffset = p - (char*)manager->base();
          Q_ASSERT( mOffset != -1 );
      }
   }
    
   HB_ALWAYS_INLINE void * rawPointer() const
   { 
       GET_MEMORY_MANAGER(mType);
       // mOffset -1 is Null pointer.
       return ( mOffset == -1 ) ? 0: ( (char*)manager->base() + mOffset );
   }

   HB_ALWAYS_INLINE void incOffset( difference_type bytes )
   { mOffset += bytes; }

   HB_ALWAYS_INLINE void decOffset( difference_type bytes )
   { mOffset -= bytes; }

private:
    difference_type mOffset;
    HbMemoryManager::MemoryType mType;
   
};

#if defined(__arm__) // this only works with arm compiler
template<typename T1, typename T2>
inline bool operator == ( const smart_ptr<T1> &pt1,
                         const smart_ptr<T2> &pt2 ) HB_ALWAYS_INLINE
{  return pt1.get() == pt2.get(); }

template<class T1, class T2>
inline bool operator!= ( const smart_ptr<T1> &pt1, 
                        const smart_ptr<T2> &pt2 ) HB_ALWAYS_INLINE
{  return pt1.get() != pt2.get();  }

template<class T1, class T2>
inline bool operator< ( const smart_ptr<T1> &pt1, 
                       const smart_ptr<T2> &pt2 ) HB_ALWAYS_INLINE
{  return pt1.get() < pt2.get();  }

template<class T1, class T2>
inline bool operator<= ( const smart_ptr<T1> &pt1, 
                        const smart_ptr<T2> &pt2 ) HB_ALWAYS_INLINE
{  return pt1.get() <= pt2.get();  }

template<class T1, class T2>
inline bool operator> ( const smart_ptr<T1> &pt1, 
                       const smart_ptr<T2> &pt2 ) HB_ALWAYS_INLINE
{  return pt1.get() > pt2.get();  }

template<class T1, class T2>
inline bool operator>= ( const smart_ptr<T1> &pt1, 
                        const smart_ptr<T2> &pt2 ) HB_ALWAYS_INLINE
{  return pt1.get() >= pt2.get();  }

template<class T, class T2>
inline int operator- ( const smart_ptr<T> &pt, const smart_ptr<T2> &pt2 ) HB_ALWAYS_INLINE
{  return pt.get() - pt2.get();   }

#else

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
#endif

#endif // HBSMARTPOINTER_P_H
