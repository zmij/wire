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

template < typename T, typename CharT, typename Traits = ::std::char_traits<CharT> >
struct basic_floating_parser : basic_parser_base<CharT, Traits> {
    using base_type     = basic_parser_base<CharT, Traits>;
    using string_type   = typename base_type::string_type;

    T& value;
    basic_floating_parser(T& val) : value{val} {}

    parse_result
    string_literal(string_type const& val) override
    {
        ::std::basic_istringstream<CharT, Traits> is(val);
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
using floating_parser   = basic_floating_parser<T, char>;
template < typename T >
using wfloating_parser  = basic_floating_parser<T, wchar_t>;

extern template struct basic_floating_parser<float, char>;
extern template struct basic_floating_parser<double, char>;
extern template struct basic_floating_parser<long double, char>;

extern template struct basic_floating_parser<float, wchar_t>;
extern template struct basic_floating_parser<double, wchar_t>;
extern template struct basic_floating_parser<long double, wchar_t>;

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */

#endif /* WIRE_JSON_DETAIL_FLOATING_PARSER_HPP_ */
