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

template < traits::value_type, typename T >
struct parser_impl;

template <>
struct parser_impl< traits::value_type::BOOL, bool > : boolean_parser {};

template < typename T >
struct parser_impl<traits::value_type::INTEGRAL, T> : integral_parser< T > {};

template < typename T >
struct parser_impl<traits::value_type::FLOATING, T> : floating_parser< T > {};

template < typename T >
struct parser_impl< traits::value_type::STRING, T > : streamable_object_parser< T >{};

template < typename T >
struct parser_impl< traits::value_type::OBJECT, T > : map_parser< T >{};

}  /* namespace detail */

template < typename T >
struct parser : detail::parser_impl< traits::json_type< T >::value, T > {};



}  /* namespace json */
}  /* namespace wire */

#endif /* WIRE_JSON_PARSER_HPP_ */
