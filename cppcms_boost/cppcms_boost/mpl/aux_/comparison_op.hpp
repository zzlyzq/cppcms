
// NO INCLUDE GUARDS, THE HEADER IS INTENDED FOR MULTIPLE INCLUSION

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: comparison_op.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-11 02:19:02 -0400 (Sat, 11 Oct 2008) $
// $Revision: 49267 $

#if !defined(CPPCMS_BOOST_MPL_PREPROCESSING_MODE)
#   include <cppcms_boost/mpl/bool.hpp>
#   include <cppcms_boost/mpl/aux_/value_wknd.hpp>
#endif

#if !defined(AUX778076_OP_PREFIX)
#   define AUX778076_OP_PREFIX AUX778076_OP_NAME
#endif

#define AUX778076_OP_ARITY 2

#include <cppcms_boost/mpl/aux_/numeric_op.hpp>
#include <cppcms_boost/mpl/aux_/config/static_constant.hpp>
#include <cppcms_boost/mpl/aux_/config/use_preprocessed.hpp>

#if !defined(CPPCMS_BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS) \
    && !defined(CPPCMS_BOOST_MPL_PREPROCESSING_MODE)

#   define CPPCMS_BOOST_MPL_PREPROCESSED_HEADER AUX778076_OP_PREFIX.hpp
#   include <cppcms_boost/mpl/aux_/include_preprocessed.hpp>

#else

#   include <cppcms_boost/mpl/aux_/config/integral.hpp>
#   include <cppcms_boost/preprocessor/cat.hpp>

namespace cppcms_boost { namespace mpl {

// MSVC workaround: implement less in terms of greater
#if 0 AUX778076_OP_TOKEN 1 && !(1 AUX778076_OP_TOKEN 0) && !(0 AUX778076_OP_TOKEN 0)
#   define AUX778076_OP(N1, N2) \
    ( CPPCMS_BOOST_MPL_AUX_VALUE_WKND(N2)::value > CPPCMS_BOOST_MPL_AUX_VALUE_WKND(N1)::value ) \
/**/
#else
#   define AUX778076_OP(N1, N2) \
    ( CPPCMS_BOOST_MPL_AUX_VALUE_WKND(N1)::value \
          AUX778076_OP_TOKEN CPPCMS_BOOST_MPL_AUX_VALUE_WKND(N2)::value \
        ) \
/**/
#endif

template<>
struct AUX778076_OP_IMPL_NAME<integral_c_tag,integral_c_tag>
{
    template< typename N1, typename N2 > struct apply
#if !defined(CPPCMS_BOOST_MPL_CFG_NO_NESTED_VALUE_ARITHMETIC)
        : bool_< AUX778076_OP(N1, N2) >
    {
#else
    {
        CPPCMS_BOOST_STATIC_CONSTANT(bool, value = AUX778076_OP(N1, N2));
        typedef bool_<value> type;
#endif
    };
};

#undef AUX778076_OP

}}

#endif // BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS

#undef AUX778076_OP_TAG_NAME
#undef AUX778076_OP_IMPL_NAME
#undef AUX778076_OP_ARITY
#undef AUX778076_OP_PREFIX
#undef AUX778076_OP_NAME
#undef AUX778076_OP_TOKEN
