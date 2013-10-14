/*
 * Defines the class template boost::iostreams::detail::filter_adapter,
 * a convenience base class for filter adapters.
 *
 * File:        boost/iostreams/detail/adapter/filter_adapter.hpp
 * Date:        Mon Nov 26 14:35:48 MST 2007
 * Copyright:   2007-2008 CodeRage, LLC
 * Author:      Jonathan Turkanis
 * Contact:     turkanis at coderage dot com
 *
 * Distributed under the Boost Software License, Version 1.0.(See accompanying 
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)
 * 
 * See http://www.boost.org/libs/iostreams for documentation.
 */

#ifndef CPPCMS_BOOST_IOSTREAMS_DETAIL_FILTER_ADAPTER_HPP_INCLUDED
#define CPPCMS_BOOST_IOSTREAMS_DETAIL_FILTER_ADAPTER_HPP_INCLUDED

#include <cppcms_boost/iostreams/categories.hpp>
#include <cppcms_boost/iostreams/detail/call_traits.hpp>
#include <cppcms_boost/iostreams/detail/ios.hpp>
#include <cppcms_boost/iostreams/operations.hpp>
#include <cppcms_boost/iostreams/traits.hpp>
#include <cppcms_boost/static_assert.hpp>

namespace cppcms_boost { namespace iostreams { namespace detail {

template<typename T>
class filter_adapter {
private:
    typedef typename detail::value_type<T>::type value_type;
    typedef typename detail::param_type<T>::type param_type;
public:
    explicit filter_adapter(param_type t) : t_(t) { }
    T& component() { return t_; }

    template<typename Device>
    void close(Device& dev) 
    { 
        detail::close_all(t_, dev); 
    }

    template<typename Device>
    void close(Device& dev, CPPCMS_BOOST_IOS::openmode which) 
    { 
        iostreams::close(t_, dev, which); 
    }

    template<typename Device>
    void flush(Device& dev) 
    { 
        return iostreams::flush(t_, dev); 
    }

    template<typename Locale> // Avoid dependency on <locale>
    void imbue(const Locale& loc) { iostreams::imbue(t_, loc); }

    std::streamsize optimal_buffer_size() const 
    { return iostreams::optimal_buffer_size(t_); }
public:
    value_type t_;
};

//----------------------------------------------------------------------------//

} } } // End namespaces detail, iostreams, boost.

#endif // #ifndef BOOST_IOSTREAMS_DETAIL_FILTER_ADAPTER_HPP_INCLUDED
