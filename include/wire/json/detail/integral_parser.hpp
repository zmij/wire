/*
 * integral_parser.hpp
 *
 *  Created on: 24 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_INTEGRAL_PARSER_HPP_
#define WIRE_JSON_DETAIL_INTEGRAL_PARSER_HPP_

#include <wire/json/detail/parser_base.hpp>

namespace wire {
namespace json {
namespace detail {

template < bool IsEnum, typename T >
struct integral_parser_impl : parser_base {
    T& value;

    integral_parser_impl(T& v) : value{v} {}
    virtual ~integral_parser_impl() {}

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
        value = val;
        return parse_result::done;
    }
    parse_result
    float_literal(long double val) override
    {
        value = val;
        return parse_result::done;
    }
    parse_result
    bool_literal(bool val) override
    {
        value = val;
        return parse_result::done;
    }
};

template < typename T >
struct integral_parser_impl< true, T > : parser_base {
    T& value;

    integral_parser_impl(T& v) : value{v} {}
    virtual ~integral_parser_impl() {}

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
        value = static_cast<T>(val);
        return parse_result::done;
    }
    parse_result
    bool_literal(bool val) override
    {
        value = static_cast<T>(val);
        return parse_result::done;
    }
};

template < typename T >
struct integral_parser :
        integral_parser_impl< ::std::is_enum<T>::value, T > {};

struct boolean_parser : parser_base {
    bool& value;

    boolean_parser(bool& v) : value{v} {}
    virtual ~boolean_parser() {}

    parse_result
    string_literal(::std::string const& val) override
    {
        ::std::istringstream is(val);
        bool tmp;
        if ((bool)(is >> tmp)) {
            ::std::swap(value, tmp);
            return parse_result::done;
        }
        throw ::std::runtime_error{"Incompatible string value"};
    }
    parse_result
    integral_literal(::std::int64_t val) override
    {
        value = val;
        return parse_result::done;
    }
    parse_result
    float_literal(long double val) override
    {
        value = val;
        return parse_result::done;
    }
    parse_result
    bool_literal(bool val) override
    {
        value = val;
        return parse_result::done;
    }
};

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */

#endif /* WIRE_JSON_DETAIL_INTEGRAL_PARSER_HPP_ */
