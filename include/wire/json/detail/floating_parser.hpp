/*
 * floating_parser.hpp
 *
 *  Created on: 24 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_FLOATING_PARSER_HPP_
#define WIRE_JSON_DETAIL_FLOATING_PARSER_HPP_

#include <wire/json/detail/parser_base.hpp>

namespace wire {
namespace json {
namespace detail {

template < typename T >
struct floating_parser : parser_base {
    T& value;

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

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */

#endif /* WIRE_JSON_DETAIL_FLOATING_PARSER_HPP_ */
