/*
 * stream_feeder.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: zmij
 */

#ifndef WIRE_UTIL_CONCATENATE_HPP_
#define WIRE_UTIL_CONCATENATE_HPP_

#include <sstream>
#include <string>
#include <typeinfo>

namespace wire {
namespace util {
namespace detail {

struct __io_meta_function_helper {
    template <typename T> __io_meta_function_helper(T const&);
};

::std::false_type
operator << (::std::ostream const&, __io_meta_function_helper const&);

template <typename T>
struct has_output_operator {
private:
    static ::std::false_type test(::std::false_type);
    static ::std::true_type test(::std::ostream&);

    static ::std::ostream& os;
    static T const& val;
public:
    static constexpr bool value = ::std::is_same<
            decltype( test( os << val) ), ::std::true_type >::type::value;
};

template < typename T, bool >
struct output_impl {
    static void
    output(::std::ostream& os, T const& arg)
    {
        os << arg;
    }
};

template < typename T >
struct output_impl<T, false> {
    static void
    output(::std::ostream& os, T const&)
    {
        os << typeid(T).name();
    }
};

template < typename T >
void
concatenate( ::std::ostream& os, T const& arg)
{
    using output = output_impl<T, has_output_operator<T>::value>;
    output::output(os, arg);
}

template < typename T, typename ... Y >
void
concatenate( ::std::ostream& os, T const& arg, Y const& ... args )
{
    using output = output_impl<T, has_output_operator<T>::value>;
    output::output(os, arg);
    concatenate(os, args ...);
}

template < typename T >
void
delim_concatenate( ::std::ostream& os, ::std::string const& delim, T const& arg)
{
    using output = output_impl<T, has_output_operator<T>::value>;
    output::output(os, arg);
}

template < typename T, typename ... Y >
void
delim_concatenate( ::std::ostream& os, ::std::string const& delim, T const& arg, Y const& ... args )
{
    using output = output_impl<T, has_output_operator<T>::value>;
    output::output(os, arg);
    os << delim;
    delim_concatenate(os, delim, args ...);
}
}  // namespace detail

template < typename ... T >
::std::string
concatenate( T const& ... args)
{
    ::std::ostringstream os;
    detail::concatenate(os, args ...);
    return os.str();
}

template < typename ... T >
::std::string
delim_concatenate(::std::string const& delim, T const& ... args)
{
    ::std::ostringstream os;
    detail::delim_concatenate(os, delim, args ...);
    return os.str();
}

}  // namespace util
}  // namespace wire

#endif /* WIRE_UTIL_CONCATENATE_HPP_ */
