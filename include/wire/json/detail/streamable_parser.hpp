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
namespace detal {

template < typename T >
struct streamable_object_parser : parser_base {
    T& value;

    virtual ~streamable_object_parser() {}

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
        ::std::istringstream is(::std::to_string(val));
        T tmp;
        if ((bool)(is >> tmp)) {
            ::std::swap(value, tmp);
            return true;
        }
        throw ::std::runtime_error{"Incompatible integral value"};
    }
    bool
    float_literal(long double val) override
    {
        ::std::istringstream is(::std::to_string(val));
        T tmp;
        if ((bool)(is >> tmp)) {
            ::std::swap(value, tmp);
            return true;
        }
        throw ::std::runtime_error{"Incompatible float value"};
    }
    bool
    bool_literal(bool val) override
    {
        ::std::istringstream is(val ? "true" : "false");
        T tmp;
        if ((bool)(is >> tmp)) {
            ::std::swap(value, tmp);
            return true;
        }
        throw ::std::runtime_error{"Incompatible boolean value"};
    }
};

template<>
struct streamable_object_parser< ::std::string > {
    ::std::string& value;
    virtual ~streamable_object_parser() {}

    bool
    string_literal(::std::string const& val) override
    {
        value = val;
        return true;
    }
    bool
    integral_literal(::std::int64_t val) override
    {
        value = ::std::to_string(val);
        return true;
    }
    bool
    float_literal(long double val) override
    {
        value = ::std::to_string(val);
        return true;
    }
    bool
    bool_literal(bool val) override
    {
        value = val ? "true" : "false";
        return true;
    }
};

}  /* namespace detal */
}  /* namespace json */
}  /* namespace wire */


#endif /* WIRE_JSON_DETAIL_STREAMABLE_PARSER_HPP_ */
