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

    bool
    string_literal(::std::string const& val) override
    {
        ::std::istringstream is(val);
        T tmp;
        if ((bool)(is >> tmp)) {
            ::std::swap(value, tmp);
            return true;
        }
        throw ::std::runtime_error{"Incompatible string value"};
    }
    bool
    integral_literal(::std::int64_t val) override
    {
        value = val;
        return true;
    }
    bool
    float_literal(long double val) override
    {
        value = val;
        return true;
    }
    bool
    bool_literal(bool val) override
    {
        value = val;
        return true;
    }
};

template < typename T >
struct integral_parser_impl< true, T > {
    T& value;

    bool
    string_literal(::std::string const& val) override
    {
        ::std::istringstream is(val);
        T tmp;
        if ((bool)(is >> tmp)) {
            ::std::swap(value, tmp);
            return true;
        }
        throw ::std::runtime_error{"Incompatible string value"};
    }
    bool
    integral_literal(::std::int64_t val) override
    {
        value = static_cast<T>(val);
        return true;
    }
    bool
    bool_literal(bool val) override
    {
        value = static_cast<T>(val);
        return true;
    }
};

template < typename T >
struct integral_parser :
        integral_parser_impl< ::std::is_enum<T>::value, T > {};

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */

#endif /* WIRE_JSON_DETAIL_INTEGRAL_PARSER_HPP_ */
