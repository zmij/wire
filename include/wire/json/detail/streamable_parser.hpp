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

template < typename T, typename CharT, typename Traits = ::std::char_traits<CharT> >
struct basic_streamable_object_parser : basic_parser_base<CharT, Traits> {
    using base_type     = basic_parser_base<CharT, Traits>;
    using string_type   = typename base_type::string_type;
    using json_io       = json_io_base<CharT, Traits>;
    using istringstream = ::std::basic_istringstream<CharT, Traits>;

    T& value;

    explicit
    basic_streamable_object_parser(T& v) : value{v} {}
    virtual ~basic_streamable_object_parser() {}

    parse_result
    string_literal(string_type const& val) override
    {
        istringstream is(val);
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
        istringstream is(json_io::to_string(val));
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
        istringstream is(json_io::to_string(val));
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
        istringstream is(val ? json_io::true_str : json_io::false_str);
        T tmp;
        if ((bool)(is >> tmp)) {
            ::std::swap(value, tmp);
            return parse_result::done;
        }
        throw ::std::runtime_error{"Incompatible boolean value"};
    }
};

template<>
struct basic_streamable_object_parser< ::std::string, char >
        : basic_parser_base<char> {
    using base_type     = basic_parser_base<char>;
    using string_type   = typename base_type::string_type;
    using json_io       = json_io_base<char>;

    ::std::string& value;

    explicit
    basic_streamable_object_parser(::std::string& v) : value{v} {}
    virtual ~basic_streamable_object_parser() {}

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
        value = val ? json_io::true_str : json_io::false_str;
        return parse_result::done;
    }
};

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */


#endif /* WIRE_JSON_DETAIL_STREAMABLE_PARSER_HPP_ */
