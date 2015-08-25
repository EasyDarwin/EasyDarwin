//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2006-2014. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/move for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_MOVE_MAKE_UNIQUE_HPP_INCLUDED
#define BOOST_MOVE_MAKE_UNIQUE_HPP_INCLUDED

#include <boost/move/detail/config_begin.hpp>
#include <boost/move/detail/workaround.hpp>
#include <boost/move/utility_core.hpp>
#include <boost/move/unique_ptr.hpp>
#include <cstddef>   //for std::size_t
#include <boost/move/detail/unique_ptr_meta_utils.hpp>

//!\file
//! Defines "make_unique" functions, which are factories to create instances
//! of unique_ptr depending on the passed arguments.
//!
//! This header can be a bit heavyweight in C++03 compilers due to the use of the
//! preprocessor library, that's why it's a a separate header from <tt>unique_ptr.hpp</tt>
 
#if !defined(BOOST_MOVE_DOXYGEN_INVOKED)

namespace std {   //no namespace versioning in clang+libc++

struct nothrow_t;

}  //namespace std {

namespace boost{
namespace move_upmu {

//Compile time switch between
//single element, unknown bound array
//and known bound array
template<class T>
struct unique_ptr_if
{
   typedef ::boost::movelib::unique_ptr<T> t_is_not_array;
};

template<class T>
struct unique_ptr_if<T[]>
{
   typedef ::boost::movelib::unique_ptr<T[]> t_is_array_of_unknown_bound;
};

template<class T, std::size_t N>
struct unique_ptr_if<T[N]>
{
   typedef void t_is_array_of_known_bound;
};

static std::nothrow_t *pnothrow;

}  //namespace move_upmu {
}  //namespace boost{

#endif   //!defined(BOOST_MOVE_DOXYGEN_INVOKED)

namespace boost{
namespace movelib {

#if defined(BOOST_MOVE_DOXYGEN_INVOKED) || !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

//! <b>Remarks</b>: This function shall not participate in overload resolution unless T is not an array.
//!
//! <b>Returns</b>: <tt>unique_ptr<T>(new T(std::forward<Args>(args)...))</tt>.
template<class T, class... Args>
inline BOOST_MOVE_DOC1ST(unique_ptr<T>, 
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array)
      make_unique(BOOST_FWD_REF(Args)... args)
{  return unique_ptr<T>(new T(::boost::forward<Args>(args)...));  }

//! <b>Remarks</b>: This function shall not participate in overload resolution unless T is not an array.
//!
//! <b>Returns</b>: <tt>unique_ptr<T>(new T(std::nothrow)(std::forward<Args>(args)...))</tt>.
template<class T, class... Args>
inline BOOST_MOVE_DOC1ST(unique_ptr<T>, 
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array)
      make_unique_nothrow(BOOST_FWD_REF(Args)... args)
{  return unique_ptr<T>(new (*boost::move_upmu::pnothrow)T(::boost::forward<Args>(args)...));  }

#else

   //0 arg
   template<class T>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique()
   {  return unique_ptr<T>(new T());  }

   template<class T>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique_nothrow()
   {  return unique_ptr<T>(new (*boost::move_upmu::pnothrow)T());  }

   //1 arg
   template<class T, class P0>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique( BOOST_FWD_REF(P0) p0
                 )
   {
      return unique_ptr<T>
         (  new T( ::boost::forward<P0>(p0)
                 )
         );
   }

   template<class T, class P0>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique_nothrow( BOOST_FWD_REF(P0) p0
                 )
   {
      return unique_ptr<T>
         (  new (*boost::move_upmu::pnothrow)T ( ::boost::forward<P0>(p0)
                              )
         );
   }
   //2 arg
   template<class T, class P0, class P1>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique( BOOST_FWD_REF(P0) p0
                 , BOOST_FWD_REF(P1) p1
                 )
   {
      return unique_ptr<T>
         (  new T( ::boost::forward<P0>(p0)
                 , ::boost::forward<P1>(p1)
                 )
         );
   }

   template<class T, class P0, class P1>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique_nothrow( BOOST_FWD_REF(P0) p0
                         , BOOST_FWD_REF(P1) p1
                         )
   {
      return unique_ptr<T>
         (  new (*boost::move_upmu::pnothrow)T ( ::boost::forward<P0>(p0)
                              , ::boost::forward<P1>(p1)
                              )
         );
   }
   //3 arg
   template<class T, class P0, class P1, class P2>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique( BOOST_FWD_REF(P0) p0
                 , BOOST_FWD_REF(P1) p1
                 , BOOST_FWD_REF(P2) p2
                 )
   {
      return unique_ptr<T>
         (  new T( ::boost::forward<P0>(p0)
                 , ::boost::forward<P1>(p1)
                 , ::boost::forward<P2>(p2)
                 )
         );
   }

   template<class T, class P0, class P1, class P2>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique_nothrow  ( BOOST_FWD_REF(P0) p0
                           , BOOST_FWD_REF(P1) p1
                           , BOOST_FWD_REF(P2) p2
                           )
   {
      return unique_ptr<T>
         (  new (*boost::move_upmu::pnothrow)T ( ::boost::forward<P0>(p0)
                              , ::boost::forward<P1>(p1)
                              , ::boost::forward<P2>(p2)
                              )
         );
   }
   //4 arg
   template<class T, class P0, class P1, class P2, class P3>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique( BOOST_FWD_REF(P0) p0
                 , BOOST_FWD_REF(P1) p1
                 , BOOST_FWD_REF(P2) p2
                 , BOOST_FWD_REF(P3) p3
                 )
   {
      return unique_ptr<T>
         (  new T( ::boost::forward<P0>(p0)
                 , ::boost::forward<P1>(p1)
                 , ::boost::forward<P2>(p2)
                 , ::boost::forward<P3>(p3)
                 )
         );
   }

   template<class T, class P0, class P1, class P2, class P3>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique_nothrow  ( BOOST_FWD_REF(P0) p0
                           , BOOST_FWD_REF(P1) p1
                           , BOOST_FWD_REF(P2) p2
                           , BOOST_FWD_REF(P3) p3
                           )
   {
      return unique_ptr<T>
         (  new (*boost::move_upmu::pnothrow)T ( ::boost::forward<P0>(p0)
                              , ::boost::forward<P1>(p1)
                              , ::boost::forward<P2>(p2)
                              , ::boost::forward<P3>(p3)
                              )
         );
   }
   //5 arg
   template<class T, class P0, class P1, class P2, class P3, class P4>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique( BOOST_FWD_REF(P0) p0
                 , BOOST_FWD_REF(P1) p1
                 , BOOST_FWD_REF(P2) p2
                 , BOOST_FWD_REF(P3) p3
                 , BOOST_FWD_REF(P4) p4
                 )
   {
      return unique_ptr<T>
         (  new T( ::boost::forward<P0>(p0)
                 , ::boost::forward<P1>(p1)
                 , ::boost::forward<P2>(p2)
                 , ::boost::forward<P3>(p3)
                 , ::boost::forward<P4>(p4)
                 )
         );
   }

   template<class T, class P0, class P1, class P2, class P3, class P4>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique_nothrow  ( BOOST_FWD_REF(P0) p0
                           , BOOST_FWD_REF(P1) p1
                           , BOOST_FWD_REF(P2) p2
                           , BOOST_FWD_REF(P3) p3
                           , BOOST_FWD_REF(P4) p4
                           )
   {
      return unique_ptr<T>
         (  new (*boost::move_upmu::pnothrow)T ( ::boost::forward<P0>(p0)
                              , ::boost::forward<P1>(p1)
                              , ::boost::forward<P2>(p2)
                              , ::boost::forward<P3>(p3)
                              , ::boost::forward<P4>(p4)
                              )
         );
   }
   //6 arg
   template<class T, class P0, class P1, class P2, class P3, class P4, class P5>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique( BOOST_FWD_REF(P0) p0
                 , BOOST_FWD_REF(P1) p1
                 , BOOST_FWD_REF(P2) p2
                 , BOOST_FWD_REF(P3) p3
                 , BOOST_FWD_REF(P4) p4
                 , BOOST_FWD_REF(P5) p5
                 )
   {
      return unique_ptr<T>
         (  new T( ::boost::forward<P0>(p0)
                 , ::boost::forward<P1>(p1)
                 , ::boost::forward<P2>(p2)
                 , ::boost::forward<P3>(p3)
                 , ::boost::forward<P4>(p4)
                 , ::boost::forward<P5>(p5)
                 )
         );
   }

   template<class T, class P0, class P1, class P2, class P3, class P4, class P5>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique_nothrow  ( BOOST_FWD_REF(P0) p0
                           , BOOST_FWD_REF(P1) p1
                           , BOOST_FWD_REF(P2) p2
                           , BOOST_FWD_REF(P3) p3
                           , BOOST_FWD_REF(P4) p4
                           , BOOST_FWD_REF(P5) p5
                           )
   {
      return unique_ptr<T>
         (  new (*boost::move_upmu::pnothrow)T ( ::boost::forward<P0>(p0)
                              , ::boost::forward<P1>(p1)
                              , ::boost::forward<P2>(p2)
                              , ::boost::forward<P3>(p3)
                              , ::boost::forward<P4>(p4)
                              , ::boost::forward<P5>(p5)
                              )
         );
   }
   //7 arg
   template<class T, class P0, class P1, class P2, class P3, class P4, class P5, class P6>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique( BOOST_FWD_REF(P0) p0
                 , BOOST_FWD_REF(P1) p1
                 , BOOST_FWD_REF(P2) p2
                 , BOOST_FWD_REF(P3) p3
                 , BOOST_FWD_REF(P4) p4
                 , BOOST_FWD_REF(P5) p5
                 , BOOST_FWD_REF(P6) p6
                 )
   {
      return unique_ptr<T>
         (  new T( ::boost::forward<P0>(p0)
                 , ::boost::forward<P1>(p1)
                 , ::boost::forward<P2>(p2)
                 , ::boost::forward<P3>(p3)
                 , ::boost::forward<P4>(p4)
                 , ::boost::forward<P5>(p5)
                 , ::boost::forward<P6>(p6)
                 )
         );
   }

   template<class T, class P0, class P1, class P2, class P3, class P4, class P5, class P6>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique_nothrow  ( BOOST_FWD_REF(P0) p0
                           , BOOST_FWD_REF(P1) p1
                           , BOOST_FWD_REF(P2) p2
                           , BOOST_FWD_REF(P3) p3
                           , BOOST_FWD_REF(P4) p4
                           , BOOST_FWD_REF(P5) p5
                           , BOOST_FWD_REF(P6) p6
                           )
   {
      return unique_ptr<T>
         (  new (*boost::move_upmu::pnothrow)T ( ::boost::forward<P0>(p0)
                              , ::boost::forward<P1>(p1)
                              , ::boost::forward<P2>(p2)
                              , ::boost::forward<P3>(p3)
                              , ::boost::forward<P4>(p4)
                              , ::boost::forward<P5>(p5)
                              , ::boost::forward<P6>(p6)
                              )
         );
   }

   //8 arg
   template<class T, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique( BOOST_FWD_REF(P0) p0
                 , BOOST_FWD_REF(P1) p1
                 , BOOST_FWD_REF(P2) p2
                 , BOOST_FWD_REF(P3) p3
                 , BOOST_FWD_REF(P4) p4
                 , BOOST_FWD_REF(P5) p5
                 , BOOST_FWD_REF(P6) p6
                 , BOOST_FWD_REF(P7) p7
                 )
   {
      return unique_ptr<T>
         (  new T( ::boost::forward<P0>(p0)
                 , ::boost::forward<P1>(p1)
                 , ::boost::forward<P2>(p2)
                 , ::boost::forward<P3>(p3)
                 , ::boost::forward<P4>(p4)
                 , ::boost::forward<P5>(p5)
                 , ::boost::forward<P6>(p6)
                 , ::boost::forward<P7>(p7)
                 )
         );
   }

   template<class T, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique_nothrow  ( BOOST_FWD_REF(P0) p0
                           , BOOST_FWD_REF(P1) p1
                           , BOOST_FWD_REF(P2) p2
                           , BOOST_FWD_REF(P3) p3
                           , BOOST_FWD_REF(P4) p4
                           , BOOST_FWD_REF(P5) p5
                           , BOOST_FWD_REF(P6) p6
                           , BOOST_FWD_REF(P7) p7
                           )
   {
      return unique_ptr<T>
         (  new (*boost::move_upmu::pnothrow)T ( ::boost::forward<P0>(p0)
                              , ::boost::forward<P1>(p1)
                              , ::boost::forward<P2>(p2)
                              , ::boost::forward<P3>(p3)
                              , ::boost::forward<P4>(p4)
                              , ::boost::forward<P5>(p5)
                              , ::boost::forward<P6>(p6)
                              , ::boost::forward<P7>(p7)
                              )
         );
   }
   //9 arg
   template<class T, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique( BOOST_FWD_REF(P0) p0
                 , BOOST_FWD_REF(P1) p1
                 , BOOST_FWD_REF(P2) p2
                 , BOOST_FWD_REF(P3) p3
                 , BOOST_FWD_REF(P4) p4
                 , BOOST_FWD_REF(P5) p5
                 , BOOST_FWD_REF(P6) p6
                 , BOOST_FWD_REF(P7) p7
                 , BOOST_FWD_REF(P8) p8
                 )
   {
      return unique_ptr<T>
         (  new T( ::boost::forward<P0>(p0)
                 , ::boost::forward<P1>(p1)
                 , ::boost::forward<P2>(p2)
                 , ::boost::forward<P3>(p3)
                 , ::boost::forward<P4>(p4)
                 , ::boost::forward<P5>(p5)
                 , ::boost::forward<P6>(p6)
                 , ::boost::forward<P7>(p7)
                 , ::boost::forward<P8>(p8)
                 )
         );
   }

   template<class T, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique_nothrow  ( BOOST_FWD_REF(P0) p0
                           , BOOST_FWD_REF(P1) p1
                           , BOOST_FWD_REF(P2) p2
                           , BOOST_FWD_REF(P3) p3
                           , BOOST_FWD_REF(P4) p4
                           , BOOST_FWD_REF(P5) p5
                           , BOOST_FWD_REF(P6) p6
                           , BOOST_FWD_REF(P7) p7
                           , BOOST_FWD_REF(P8) p8
                           )
   {
      return unique_ptr<T>
         (  new (*boost::move_upmu::pnothrow)T ( ::boost::forward<P0>(p0)
                              , ::boost::forward<P1>(p1)
                              , ::boost::forward<P2>(p2)
                              , ::boost::forward<P3>(p3)
                              , ::boost::forward<P4>(p4)
                              , ::boost::forward<P5>(p5)
                              , ::boost::forward<P6>(p6)
                              , ::boost::forward<P7>(p7)
                              , ::boost::forward<P8>(p8)
                              )
         );
   }
   //10 arg
   template<class T, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique( BOOST_FWD_REF(P0) p0
                 , BOOST_FWD_REF(P1) p1
                 , BOOST_FWD_REF(P2) p2
                 , BOOST_FWD_REF(P3) p3
                 , BOOST_FWD_REF(P4) p4
                 , BOOST_FWD_REF(P5) p5
                 , BOOST_FWD_REF(P6) p6
                 , BOOST_FWD_REF(P7) p7
                 , BOOST_FWD_REF(P8) p8
                 , BOOST_FWD_REF(P9) p9
                 )
   {
      return unique_ptr<T>
         (  new T( ::boost::forward<P0>(p0)
                 , ::boost::forward<P1>(p1)
                 , ::boost::forward<P2>(p2)
                 , ::boost::forward<P3>(p3)
                 , ::boost::forward<P4>(p4)
                 , ::boost::forward<P5>(p5)
                 , ::boost::forward<P6>(p6)
                 , ::boost::forward<P7>(p7)
                 , ::boost::forward<P8>(p8)
                 , ::boost::forward<P9>(p9)
                 )
         );
   }

   template<class T, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array
      make_unique_nothrow  ( BOOST_FWD_REF(P0) p0
                           , BOOST_FWD_REF(P1) p1
                           , BOOST_FWD_REF(P2) p2
                           , BOOST_FWD_REF(P3) p3
                           , BOOST_FWD_REF(P4) p4
                           , BOOST_FWD_REF(P5) p5
                           , BOOST_FWD_REF(P6) p6
                           , BOOST_FWD_REF(P7) p7
                           , BOOST_FWD_REF(P8) p8
                           , BOOST_FWD_REF(P9) p9
                           )
   {
      return unique_ptr<T>
         (  new (*boost::move_upmu::pnothrow)T ( ::boost::forward<P0>(p0)
                              , ::boost::forward<P1>(p1)
                              , ::boost::forward<P2>(p2)
                              , ::boost::forward<P3>(p3)
                              , ::boost::forward<P4>(p4)
                              , ::boost::forward<P5>(p5)
                              , ::boost::forward<P6>(p6)
                              , ::boost::forward<P7>(p7)
                              , ::boost::forward<P8>(p8)
                              , ::boost::forward<P9>(p9)
                              )
         );
   }

#endif

//! <b>Remarks</b>: This function shall not participate in overload resolution unless T is not an array.
//!
//! <b>Returns</b>: <tt>unique_ptr<T>(new T)</tt> (default initialization)
template<class T>
inline BOOST_MOVE_DOC1ST(unique_ptr<T>, 
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array)
      make_unique_definit()
{
    return unique_ptr<T>(new T);
}

//! <b>Remarks</b>: This function shall not participate in overload resolution unless T is not an array.
//!
//! <b>Returns</b>: <tt>unique_ptr<T>(new T(std::nothrow)</tt> (default initialization)
template<class T>
inline BOOST_MOVE_DOC1ST(unique_ptr<T>, 
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_not_array)
      make_unique_nothrow_definit()
{
    return unique_ptr<T>(new (*boost::move_upmu::pnothrow)T);
}

//! <b>Remarks</b>: This function shall not participate in overload resolution unless T is an array of 
//!   unknown bound.
//!
//! <b>Returns</b>: <tt>unique_ptr<T>(new remove_extent_t<T>[n]())</tt> (value initialization)
template<class T>
inline BOOST_MOVE_DOC1ST(unique_ptr<T>, 
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_array_of_unknown_bound)
      make_unique(std::size_t n)
{
    typedef typename ::boost::move_upmu::remove_extent<T>::type U;
    return unique_ptr<T>(new U[n]());
}

//! <b>Remarks</b>: This function shall not participate in overload resolution unless T is an array of 
//!   unknown bound.
//!
//! <b>Returns</b>: <tt>unique_ptr<T>(new (std::nothrow)remove_extent_t<T>[n]())</tt> (value initialization)
template<class T>
inline BOOST_MOVE_DOC1ST(unique_ptr<T>, 
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_array_of_unknown_bound)
      make_unique_nothrow(std::size_t n)
{
    typedef typename ::boost::move_upmu::remove_extent<T>::type U;
    return unique_ptr<T>(new (*boost::move_upmu::pnothrow)U[n]());
}

//! <b>Remarks</b>: This function shall not participate in overload resolution unless T is an array of 
//!   unknown bound.
//!
//! <b>Returns</b>: <tt>unique_ptr<T>(new remove_extent_t<T>[n])</tt> (default initialization)
template<class T>
inline BOOST_MOVE_DOC1ST(unique_ptr<T>, 
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_array_of_unknown_bound)
      make_unique_definit(std::size_t n)
{
    typedef typename ::boost::move_upmu::remove_extent<T>::type U;
    return unique_ptr<T>(new U[n]);
}

//! <b>Remarks</b>: This function shall not participate in overload resolution unless T is an array of 
//!   unknown bound.
//!
//! <b>Returns</b>: <tt>unique_ptr<T>(new (std::nothrow)remove_extent_t<T>[n])</tt> (default initialization)
template<class T>
inline BOOST_MOVE_DOC1ST(unique_ptr<T>, 
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_array_of_unknown_bound)
      make_unique_nothrow_definit(std::size_t n)
{
    typedef typename ::boost::move_upmu::remove_extent<T>::type U;
    return unique_ptr<T>(new (*boost::move_upmu::pnothrow) U[n]);
}

#if !defined(BOOST_NO_CXX11_DELETED_FUNCTIONS)

//! <b>Remarks</b>: This function shall not participate in overload resolution unless T is
//!   an array of known bound.
template<class T, class... Args>
inline BOOST_MOVE_DOC1ST(unspecified, 
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_array_of_known_bound)
      make_unique(BOOST_FWD_REF(Args) ...) = delete;

//! <b>Remarks</b>: This function shall not participate in overload resolution unless T is
//!   an array of known bound.
template<class T, class... Args>
inline BOOST_MOVE_DOC1ST(unspecified, 
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_array_of_known_bound)
      make_unique_definit(BOOST_FWD_REF(Args) ...) = delete;

//! <b>Remarks</b>: This function shall not participate in overload resolution unless T is
//!   an array of known bound.
template<class T, class... Args>
inline BOOST_MOVE_DOC1ST(unspecified, 
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_array_of_known_bound)
      make_unique_nothrow(BOOST_FWD_REF(Args) ...) = delete;

//! <b>Remarks</b>: This function shall not participate in overload resolution unless T is
//!   an array of known bound.
template<class T, class... Args>
inline BOOST_MOVE_DOC1ST(unspecified, 
   typename ::boost::move_upmu::unique_ptr_if<T>::t_is_array_of_known_bound)
      make_unique_nothrow_definit(BOOST_FWD_REF(Args) ...) = delete;

#endif

}  //namespace movelib {

}  //namespace boost{

#include <boost/move/detail/config_end.hpp>

#endif   //#ifndef BOOST_MOVE_MAKE_UNIQUE_HPP_INCLUDED
