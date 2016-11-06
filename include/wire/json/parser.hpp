/*
 * parser.hpp
 *
 *  Created on: 24 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_PARSER_HPP_
#define WIRE_JSON_PARSER_HPP_

#include <wire/json/detail/parser_base.hpp>
#include <wire/json/detail/streamable_parser.hpp>
#include <wire/json/detail/integral_parser.hpp>
#include <wire/json/detail/floating_parser.hpp>

#include <wire/json/detail/map_parser.hpp>

namespace wire {
namespace json {

namespace detail {

template < traits::value_type, typename T, typename CharT,
    typename Traits = ::std::char_traits<CharT> >
struct basic_parser_impl;

template < typename CharT, typename Traits >
struct basic_parser_impl< traits::value_type::BOOL, bool, CharT, Traits >
        : basic_boolean_parser<CharT, Traits> {
    using base_type = basic_boolean_parser<CharT, Traits>;
    basic_parser_impl(bool& val) : base_type{val} {}
};

template < typename T, typename CharT, typename Traits >
struct basic_parser_impl<traits::value_type::INTEGRAL, T, CharT, Traits>
        : basic_integral_parser< T, CharT, Traits > {
    using base_type = basic_integral_parser< T, CharT, Traits >;
    basic_parser_impl(T& val) : base_type{val} {}
};

template < typename T, typename CharT, typename Traits >
struct basic_parser_impl<traits::value_type::FLOATING, T, CharT, Traits>
        : basic_floating_parser< T, CharT, Traits > {
    using base_type = basic_floating_parser< T, CharT, Traits >;
    basic_parser_impl(T& val) : base_type{val} {}
};

template < typename T, typename CharT, typename Traits >
struct basic_parser_impl< traits::value_type::STRING, T, CharT, Traits >
        : basic_streamable_object_parser< T, CharT, Traits > {
    using base_type = basic_streamable_object_parser< T, CharT, Traits >;
    basic_parser_impl(T& val) : base_type{val} {}
};

//// FIXME Use a generic object parser
//template < typename T >
//struct parser_impl< traits::value_type::OBJECT, T > : map_parser< T > {
//    using base_type = map_parser< T >;
//    parser_impl(T& val) : base_type{val} {}
//};

}  /* namespace detail */

template < typename T, typename CharT, typename Traits >
struct basic_parser
        : detail::basic_parser_impl< traits::json_type< T >::value, T, CharT, Traits > {
    using base_type = detail::basic_parser_impl< traits::json_type< T >::value, T, CharT, Traits >;
    basic_parser(T& val) : base_type{val} {}
};

}  /* namespace json */
}  /* namespace wire */

#endif /* WIRE_JSON_PARSER_HPP_ */
