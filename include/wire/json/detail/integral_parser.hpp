/*
 * integral_parser.hpp
 *
 *  Created on: 24 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_INTEGRAL_PARSER_HPP_
#define WIRE_JSON_DETAIL_INTEGRAL_PARSER_HPP_

#include <wire/json/detail/parser_base.hpp>
#include <sstream>

namespace wire {
namespace json {
namespace detail {

template < bool IsEnum, typename T, typename CharT, typename Traits = ::std::char_traits<CharT> >
struct basic_integral_parser_impl : basic_parser_base<CharT, Traits> {
    using base_type     = basic_parser_base<CharT, Traits>;
    using string_type   = typename base_type::string_type;

    T& value;

    basic_integral_parser_impl(T& v) : value{v} {}
    virtual ~basic_integral_parser_impl() {}

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

template < typename T, typename CharT, typename Traits >
struct basic_integral_parser_impl< true, T, CharT, Traits > : basic_parser_base<CharT, Traits> {
    using base_type     = basic_parser_base<CharT, Traits>;
    using string_type   = typename base_type::string_type;

    T& value;

    basic_integral_parser_impl(T& v) : value{v} {}
    virtual ~basic_integral_parser_impl() {}

    parse_result
    string_literal(::std::string const& val) override
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

template < typename T, typename CharT, typename Traits = ::std::char_traits<CharT> >
struct basic_integral_parser :
        basic_integral_parser_impl< ::std::is_enum<T>::value, T, CharT, Traits > {
    using base_type = basic_integral_parser_impl< ::std::is_enum<T>::value, T, CharT, Traits >;
    basic_integral_parser(T& value) : base_type{value} {}
};

template < typename CharT, typename Traits = ::std::char_traits<CharT> >
struct basic_boolean_parser : basic_parser_base<CharT, Traits> {
    using base_type     = basic_parser_base<CharT, Traits>;
    using string_type   = typename base_type::string_type;

    bool& value;

    basic_boolean_parser(bool& v) : value{v} {}
    virtual ~basic_boolean_parser() {}

    parse_result
    string_literal(string_type const& val) override
    {
        ::std::basic_istringstream<CharT, Traits> is(val);
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

template < typename T >
using integral_parser = basic_integral_parser<T, char>;
template < typename T >
using wintegral_parser = basic_integral_parser<T, wchar_t>;

using boolean_parser    = basic_boolean_parser<char>;
using wboolean_parser   = basic_boolean_parser<wchar_t>;

extern template struct basic_integral_parser<char, char>;
extern template struct basic_integral_parser<unsigned char, char>;
extern template struct basic_integral_parser<short, char>;
extern template struct basic_integral_parser<unsigned short, char>;
extern template struct basic_integral_parser<int, char>;
extern template struct basic_integral_parser<unsigned int, char>;
extern template struct basic_integral_parser<long, char>;
extern template struct basic_integral_parser<unsigned long, char>;
extern template struct basic_integral_parser<long long, char>;
extern template struct basic_integral_parser<unsigned long long, char>;

extern template struct basic_integral_parser<short, wchar_t>;
extern template struct basic_integral_parser<unsigned short, wchar_t>;
extern template struct basic_integral_parser<int, wchar_t>;
extern template struct basic_integral_parser<unsigned int, wchar_t>;
extern template struct basic_integral_parser<long, wchar_t>;
extern template struct basic_integral_parser<unsigned long, wchar_t>;
extern template struct basic_integral_parser<long long, wchar_t>;
extern template struct basic_integral_parser<unsigned long long, wchar_t>;

extern template struct basic_boolean_parser<char>;
extern template struct basic_boolean_parser<wchar_t>;

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */

#endif /* WIRE_JSON_DETAIL_INTEGRAL_PARSER_HPP_ */
