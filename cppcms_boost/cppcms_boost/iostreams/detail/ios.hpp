// (C) Copyright 2008 CodeRage, LLC (turkanis at coderage dot com)
// (C) Copyright 2003-2007 Jonathan Turkanis
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// See http://www.boost.org/libs/iostreams for documentation.

#ifndef CPPCMS_BOOST_IOSTREAMS_DETAIL_IOS_HPP_INCLUDED
#define CPPCMS_BOOST_IOSTREAMS_DETAIL_IOS_HPP_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif              
                 
#include <cppcms_boost/config.hpp> // BOOST_MSVC.
#include <cppcms_boost/detail/workaround.hpp>
#include <cppcms_boost/iostreams/detail/config/wide_streams.hpp>
#ifndef CPPCMS_BOOST_IOSTREAMS_NO_STREAM_TEMPLATES
# if !CPPCMS_BOOST_WORKAROUND(__MWERKS__, <= 0x3003)
#  include <ios>
# else
#  include <istream>
#  include <ostream>
# endif
#else 
# include <exception>
# include <iosfwd>
#endif 

namespace cppcms_boost { namespace iostreams { namespace detail {

#ifndef CPPCMS_BOOST_IOSTREAMS_NO_STREAM_TEMPLATES //--------------------------------//
# define CPPCMS_BOOST_IOSTREAMS_BASIC_IOS(ch, tr)  std::basic_ios< ch, tr >
# if !CPPCMS_BOOST_WORKAROUND(__MWERKS__, <= 0x3003) && \
     !CPPCMS_BOOST_WORKAROUND(__BORLANDC__, < 0x600) && \
     !CPPCMS_BOOST_WORKAROUND(CPPCMS_BOOST_MSVC, < 1300) \
     /**/

#define CPPCMS_BOOST_IOS                std::ios
#define CPPCMS_BOOST_IOSTREAMS_FAILURE  std::ios::failure

# else

#define CPPCMS_BOOST_IOS                std::ios_base
#define CPPCMS_BOOST_IOSTREAMS_FAILURE  std::ios_base::failure

# endif
#else // #ifndef BOOST_IOSTREAMS_NO_STREAM_TEMPLATES //-----------------------//

#define CPPCMS_BOOST_IOS                          std::ios
#define CPPCMS_BOOST_IOSTREAMS_BASIC_IOS(ch, tr)  std::ios
#define CPPCMS_BOOST_IOSTREAMS_FAILURE            cppcms_boost::iostreams::detail::failure

class failure : std::exception {    
public:
    explicit failure(const std::string& what_arg) : what_(what_arg) { }
    const char* what() const { return what_.c_str(); }
private:
    std::string what_;
};

#endif // #ifndef BOOST_IOSTREAMS_NO_STREAM_TEMPLATES //----------------------//

} } } // End namespace failure, iostreams, boost.

#endif // #ifndef BOOST_IOSTREAMS_DETAIL_IOS_HPP_INCLUDED
