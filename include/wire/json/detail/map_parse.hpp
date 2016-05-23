/*
 * map_parse.hpp
 *
 *  Created on: 24 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_MAP_PARSE_HPP_
#define WIRE_JSON_DETAIL_MAP_PARSE_HPP_

#include <map>
#include <unordered_map>

#include <wire/json/traits.hpp>
#include <wire/json/detail/parser_base.hpp>

namespace wire {
namespace json {
namespace detail {

template <typename Map >
struct map_parser_base : parser_base {
    using map_type      = Map;
    using key_type      = typename map_type::key_type;
    using mapped_type   = typename map_type::mapped_type;
    using key_parser    = parser< key_type >;
    using value_parser  = parser< value_type >;
};

template < bool StreamedKey, typename Map >
struct map_parser_impl;

template < template <typename, typename, typename ...> class Map,
    typename K, typename V, typename ... Rest >
struct map_parser_impl< true, Map< K, V, Rest... > > : map_parser_base< Map<K, V, Rest...> > {
};

template < template <typename, typename, typename ...> class Map,
    typename K, typename V, typename ... Rest >
struct map_parser_impl< false, Map< K, V, Rest... > > : map_parser_base< Map<K, V, Rest...> > {
};


template < typename Map >
struct map_parser;
template < template <typename, typename, typename ...> class Map,
        typename K, typename V, typename Rest ... >
struct map_parser< Map<K, V, Rest...> > : map_parser_impl<
    util::has_iostream_operators< K >::value, Map<K, V, Rest...> > {};

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */


#endif /* WIRE_JSON_DETAIL_MAP_PARSE_HPP_ */
