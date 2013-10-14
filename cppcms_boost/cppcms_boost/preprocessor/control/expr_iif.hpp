# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2002.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef CPPCMS_BOOST_PREPROCESSOR_CONTROL_EXPR_IIF_HPP
# define CPPCMS_BOOST_PREPROCESSOR_CONTROL_EXPR_IIF_HPP
#
# include <cppcms_boost/preprocessor/config/config.hpp>
#
# /* BOOST_PP_EXPR_IIF */
#
# if ~CPPCMS_BOOST_PP_CONFIG_FLAGS() & CPPCMS_BOOST_PP_CONFIG_MWCC()
#    define CPPCMS_BOOST_PP_EXPR_IIF(bit, expr) CPPCMS_BOOST_PP_EXPR_IIF_I(bit, expr)
# else
#    define CPPCMS_BOOST_PP_EXPR_IIF(bit, expr) CPPCMS_BOOST_PP_EXPR_IIF_OO((bit, expr))
#    define CPPCMS_BOOST_PP_EXPR_IIF_OO(par) CPPCMS_BOOST_PP_EXPR_IIF_I ## par
# endif
#
# define CPPCMS_BOOST_PP_EXPR_IIF_I(bit, expr) CPPCMS_BOOST_PP_EXPR_IIF_ ## bit(expr)
#
# define CPPCMS_BOOST_PP_EXPR_IIF_0(expr)
# define CPPCMS_BOOST_PP_EXPR_IIF_1(expr) expr
#
# endif
