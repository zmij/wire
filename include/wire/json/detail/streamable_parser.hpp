/*
 * streamable_parser.hpp
 *
 *  Created on: 24 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_STREAMABLE_PARSER_HPP_
#define WIRE_JSON_DETAIL_STREAMABLE_PARSER_HPP_

#include <wire/json/detail/parser_base.hpp>
#include <sstream>

namespace wire {
namespace json {
namespace detail {

template < typename T >
struct streamable_object_parser : parser_base {
    T& value;

    explicit
    streamable_object_parser(T& v) : value{v} {}
    virtual ~streamable_object_parser() {}

    parse_result
    string_literal(::std::string const& val) override
    {
        ::std::istringstream is(val);
        T tmp;
        if ((bool)(is >> tmp)) {
            ::std::swap(value, tmp);
            return parse_result::done;
        }
        throw ::std::runtime_error{"Incompatible string value"};
    }
    parse_result
    integral_literal(::std::int64_t val) override
    {
        ::std::istringstream is(::std::to_string(val));
        T tmp;
        if ((bool)(is >> tmp)) {
            ::std::swap(value, tmp);
            return parse_result::done;
        }
        throw ::std::runtime_error{"Incompatible integral value"};
    }
    parse_result
    float_literal(long double val) override
    {
        ::std::istringstream is(::std::to_string(val));
        T tmp;
        if ((bool)(is >> tmp)) {
            ::std::swap(value, tmp);
            return parse_result::done;
        }
        throw ::std::runtime_error{"Incompatible float value"};
    }
    parse_result
    bool_literal(bool val) override
    {
        ::std::istringstream is(val ? "true" : "false");
        T tmp;
        if ((bool)(is >> tmp)) {
            ::std::swap(value, tmp);
            return parse_result::done;
        }
        throw ::std::runtime_error{"Incompatible boolean value"};
    }
};

template<>
struct streamable_object_parser< ::std::string > : parser_base {
    ::std::string& value;

    explicit
    streamable_object_parser(::std::string& v) : value{v} {}
    virtual ~streamable_object_parser() {}

    parse_result
    string_literal(::std::string const& val) override
    {
        value = val;
        return parse_result::done;
    }
    parse_result
    integral_literal(::std::int64_t val) override
    {
        value = ::std::to_string(val);
        return parse_result::done;
    }
    parse_result
    float_literal(long double val) override
    {
        value = ::std::to_string(val);
        return parse_result::done;
    }
    parse_result
    bool_literal(bool val) override
    {
        value = val ? "true" : "false";
        return parse_result::done;
    }
};

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */


#endif /* WIRE_JSON_DETAIL_STREAMABLE_PARSER_HPP_ */
