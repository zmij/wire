/*
 * map_parse.hpp
 *
 *  Created on: 24 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_MAP_PARSER_HPP_
#define WIRE_JSON_DETAIL_MAP_PARSER_HPP_

#include <map>
#include <unordered_map>

#include <wire/json/traits.hpp>
#include <wire/json/detail/parser_base.hpp>

namespace wire {
namespace json {
namespace traits {

template < typename K, typename V, typename ... Rest >
struct json_type< ::std::map< K, V, Rest ... > > : json_type_constant< value_type::OBJECT >{};
template < typename K, typename V, typename ... Rest >
struct json_type< ::std::multimap< K, V, Rest ... > > : json_type_constant< value_type::OBJECT >{};
template < typename K, typename V, typename ... Rest >
struct json_type< ::std::unordered_map< K, V, Rest ... > > : json_type_constant< value_type::OBJECT >{};
template < typename K, typename V, typename ... Rest >
struct json_type< ::std::unordered_multimap< K, V, Rest ... > > : json_type_constant< value_type::OBJECT >{};

}
namespace detail {

template <typename Map >
struct map_parser_base : parser_base {
    using map_type      = Map;
    using key_type      = typename map_type::key_type;
    using mapped_type   = typename map_type::mapped_type;
    using value_type    = typename map_type::value_type;
    using key_parser    = parser< key_type >;
    using value_parser  = parser< value_type >;

    map_type& value;

};

template < bool StreamedKey, typename Map >
struct map_parser_impl;

template < template <typename, typename, typename ...> class Map,
    typename K, typename V, typename ... Rest >
struct map_parser_impl< true, Map< K, V, Rest... > >
        : map_parser_base< Map<K, V, Rest...> > {
    parser_base_ptr     key_parser_;

    map_parser_impl(map_type& value)
        : map_parser_base{value}
    {
    }
    virtual ~map_parser_impl() {}

    bool
    start_member( ::std::string const& name ) override
    {
        return false;
    }

    bool
    start_object() override
    {
        if (current_parser_) {
            if (current_parser_->start_object())
                current_parser_ = nullptr;
            return false;
        }
        if (!key_parser_) {
            key_parser_.reset(new key_parser{}); // TODO Pass a value
            return false;
        }
        throw ::std::runtime_error{"Unexpected object start"};
    }

    bool
    end_object() override
    {
        if (current_parser_) {
            if (current_parser_->start_object())
                current_parser_ = nullptr;
            return false;
        }
        // TODO Swap the value
        return true;
    }
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


#endif /* WIRE_JSON_DETAIL_MAP_PARSER_HPP_ */
