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

template < typename K, typename V>
struct key_value_parser_base : delegate_parser {
    using key_type      = K;
    using value_type    = V;

    using key_parser    = parser< key_type >;
    using value_parser  = parser< value_type >;

    key_type            key;
    value_type          value;
};
/**
 * Key-value pair with a key type serializable to a string
 * will serialize to a "<key>" : <value> pair
 */
template < bool Serializable, typename K, typename V >
struct key_value_parser_impl : key_value_parser_base< K, V > {
    using base_type     = key_value_parser_base< K, V >;
    using key_parser    = typename base_type::key_parser;
    using value_parser  = typename base_type::value_parser;

    parse_result
    start_member(::std::string const& val) override
    {
        if (current_parser_) {
            return current_parser_->start_member(val);
        }
        key_parser p{key};
        p.string_literal(val);
        current_parser_.reset(new value_parser{value});
        return parse_result::need_more;
    }
protected:
    using parser_base::current_parser_;
    using base_type::key;
    using base_type::value;
};
/**
 * Key-value pair with a key that doesn't have streaming
 * operators will serialize to an object in form
 * @code{.js}
 * {
 *      "key"   : <serialized key>,
 *      "value" : <serialized value>
 * }
 * @endcode
 */
template < typename K, typename V >
struct key_value_parser_impl<false, K, V> : key_value_parser_base< K, V >{
    using base_type         = key_value_parser_base< K, V >;
    using key_parser        = typename base_type::key_parser;
    using value_parser      = typename base_type::value_parser;
    using key_parser_ptr    = ::std::shared_ptr<key_parser>;
    using value_parser_ptr  = ::std::shared_ptr<value_parser>;

    parse_result
    start_object() override
    {
        if (current_parser_) {
            if (current_parser_->start_object() == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        if (!opened_) {
            opened_ = true;
            return parse_result::need_more;
        }
        throw ::std::runtime_error{"Unexpected object start"};
    }
    parse_result
    end_object() override
    {
        if (current_parser_) {
            if (current_parser_->start_object() == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        return parse_result::done;
    }
    parse_result
    start_member(::std::string const& name) override
    {
        if (current_parser_) {
            if (current_parser_->start_member(name) == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        if (name == "key") {

        } else if (name == "value") {

        } else {
            current_parser_.reset(new ignore_parser{});
        }
        return parse_result::need_more;
    }
protected:
    using parser_base::current_parser_;
    using base_type::key;
    using base_type::value;
private:
    bool    opened_ = false;

};

template < typename K, typename V >
struct key_value_parser :
        key_value_parser_impl< util::has_iostream_operators<K>::value, K, V > {};

template <typename Map >
struct map_parser_base : parser_base {
    using map_type          = Map;
    using key_type          = typename map_type::key_type;
    using mapped_type       = typename map_type::mapped_type;
    using value_type        = typename map_type::value_type;
    using kv_parser         = key_value_parser<key_type, value_type>;
    using kv_parser_ptr     = ::std::shared_ptr< kv_parser >;

    map_type&   value;
    map_type    tmp;
};

template < bool StreamedKey, typename Map >
struct map_parser_impl;

template < template <typename, typename, typename ...> class Map,
    typename K, typename V, typename ... Rest >
struct map_parser_impl< true, Map< K, V, Rest... > >
        : map_parser_base< Map<K, V, Rest...> > {
    using base_type     = map_parser_base< Map<K, V, Rest...> >;
    using map_type      = typename base_type::map_type;
    using kv_parser     = typename base_type::kv_parser;
    using kv_parser_ptr = typename base_type::kv_parser_ptr;

    map_parser_impl(map_type& value)
        : base_type{value}
    {
    }
    virtual ~map_parser_impl() {}

    parse_result
    start_object() override
    {
        if (current_parser_) {
            if (current_parser_->start_object() == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        if (!opened_) {
            opened_ = true;
            return parse_result::need_more;
        }
        throw ::std::runtime_error{"Unexpected object start"};
    }

    parse_result
    end_object() override
    {
        if (current_parser_) {
            if (current_parser_->start_object() == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        ::std::swap(value, tmp);
        return parse_result::done;
    }

    parse_result
    start_member(::std::string const& name)
    {
        if (!current_parser_) {
            // parser either done it's work or was never called
            if (kv_parser_) {
                tmp.emplace(kv_parser_->key, kv_parser_->value);
            } else {
                kv_parser_.reset(new kv_parser{});
            }
            current_parser_ = kv_parser_;
        }
        if (current_parser_) {
            if (current_parser_->start_member(name) == parse_result::done)
                current_parser_ = nullptr;
            return parse_result::need_more;
        }
        return parse_result::need_more;
    }
protected:
    using parser_base::current_parser_;
    using base_type::value;
    using base_type::tmp;
private:
    bool    opened_     = false;
    kv_parser_ptr    kv_parser_;
};

template < template <typename, typename, typename ...> class Map,
    typename K, typename V, typename ... Rest >
struct map_parser_impl< false, Map< K, V, Rest... > > : map_parser_base< Map<K, V, Rest...> > {
};


template < typename Map >
struct map_parser;
template < template <typename, typename, typename ...> class Map,
        typename K, typename V, typename ... Rest >
struct map_parser< Map<K, V, Rest...> > : map_parser_impl<
    util::has_iostream_operators< K >::value, Map<K, V, Rest...> > {};

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */


#endif /* WIRE_JSON_DETAIL_MAP_PARSER_HPP_ */
