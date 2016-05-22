/*
 * has_io_operator.hpp
 *
 *  Created on: 22 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_UTIL_HAS_IO_OPERATOR_HPP_
#define WIRE_UTIL_HAS_IO_OPERATOR_HPP_

#include <iosfwd>
#include <type_traits>

namespace wire {
namespace util {

struct __io_meta_function_helper {
    template <typename T> __io_meta_function_helper(T const&);
};

::std::false_type
operator << (::std::ostream const&, __io_meta_function_helper const&);
::std::false_type
operator >> (::std::istream const&, __io_meta_function_helper const&);

template <typename T>
struct has_input_operator {
private:
    static ::std::false_type test(::std::false_type);
    static ::std::true_type test(::std::istream&);

    static ::std::istream& is;
    static T& val;
public:
    static constexpr bool value = ::std::is_same<
            decltype( test( is >> val ) ), ::std::true_type >::type::value;
};

template <typename T>
struct has_output_operator {
private:
    static ::std::false_type test(std::false_type);
    static ::std::true_type test(std::ostream&);

    static ::std::ostream& os;
    static T const& val;
public:
    static constexpr bool value = ::std::is_same<
            decltype( test( os << val) ), ::std::true_type >::type::value;
};

template < typename T >
struct has_iostream_operators {
    static constexpr bool value = has_input_operator<T>::value
            && has_output_operator<T>::value;
};

}  /* namespace util */
}  /* namespace wire */


#endif /* WIRE_UTIL_HAS_IO_OPERATOR_HPP_ */
