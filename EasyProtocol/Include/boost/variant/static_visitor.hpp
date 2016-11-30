//-----------------------------------------------------------------------------
// boost variant/static_visitor.hpp header file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2002-2003
// Eric Friedman
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_VARIANT_STATIC_VISITOR_HPP
#define BOOST_VARIANT_STATIC_VISITOR_HPP

#include "boost/config.hpp"
#include "boost/detail/workaround.hpp"

#include "boost/mpl/if.hpp"
#include "boost/type_traits/is_base_and_derived.hpp"

// should be the last #include
#include "boost/type_traits/detail/bool_trait_def.hpp"

namespace boost {

//////////////////////////////////////////////////////////////////////////
// class template static_visitor
//
// An empty base class that typedefs the return type of a deriving static
// visitor. The class is analogous to std::unary_function in this role.
//

namespace detail {

    struct is_static_visitor_tag { };

    typedef void static_visitor_default_return;

} // namespace detail

template <typename R = ::boost::detail::static_visitor_default_return>
class static_visitor
    : public detail::is_static_visitor_tag
{
public: // typedefs

    typedef R result_type;

protected: // for use as base class only
#if !defined(BOOST_NO_CXX11_DEFAULTED_FUNCTIONS) && !defined(BOOST_NO_CXX11_NON_PUBLIC_DEFAULTED_FUNCTIONS)
    static_visitor() = default;
    ~static_visitor() = default;
#else
    static_visitor()  BOOST_NOEXCEPT { }
    ~static_visitor()  BOOST_NOEXCEPT { }
#endif
};

//////////////////////////////////////////////////////////////////////////
// metafunction is_static_visitor
//
// Value metafunction indicates whether the specified type derives from
// static_visitor<...>.
//
// NOTE #1: This metafunction does NOT check whether the specified type
//  fulfills the requirements of the StaticVisitor concept.
//
// NOTE #2: This template never needs to be specialized!
//

namespace detail {

template <typename T>
struct is_static_visitor_impl
{
    BOOST_STATIC_CONSTANT(bool, value = 
        (::boost::is_base_and_derived< 
            detail::is_static_visitor_tag,
            T
        >::value));
};

} // namespace detail

BOOST_TT_AUX_BOOL_TRAIT_DEF1(
      is_static_visitor
    , T
    , (::boost::detail::is_static_visitor_impl<T>::value)
    )

} // namespace boost

#include "boost/type_traits/detail/bool_trait_undef.hpp"

#endif // BOOST_VARIANT_STATIC_VISITOR_HPP
