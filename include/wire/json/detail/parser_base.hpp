/*
 * parser_fwd.hpp
 *
 *  Created on: 24 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_PARSER_BASE_HPP_
#define WIRE_JSON_DETAIL_PARSER_BASE_HPP_

#include <string>
#include <cstdint>
#include <exception>
#include <memory>
#include <sstream>

namespace wire {
namespace json {
namespace detail {

template < typename T >
struct parser;

struct parser_base;
using parser_base_ptr = ::std::shared_ptr<parser_base>;

struct parser_base {
    virtual ~parser_base() {}

    virtual bool
    string_literal(::std::string const&);
    virtual bool
    integral_literal(::std::int64_t val);
    virtual bool
    float_literal(long double val);
    virtual bool
    bool_literal(bool val);
    virtual bool
    null_literal();

    virtual bool
    start_member(::std::string const& name);

    virtual bool
    start_array();
    virtual bool
    end_array();
    virtual bool
    start_object();
    virtual bool
    end_object();
protected:
    parser_base_ptr     current_parser_ = nullptr;
};

struct ignore_parser : parser_base {
    virtual ~ignore_parser() {}

    bool
    string_literal(::std::string const&) override;
    bool
    integral_literal(::std::int64_t val) override;
    bool
    float_literal(long double val) override;
    bool
    bool_literal(bool val) override;
    bool
    null_literal() override;

    bool
    start_array() override;
    bool
    end_array() override;
    bool
    start_object() override;
    bool
    end_object() override;
};

struct ignore_array_parser : ignore_parser {
    virtual ~ignore_array_parser() {}

    bool
    end_array() override;
};

struct ignore_object_parser : ignore_parser {
    virtual ~ignore_object_parser() {}

    bool
    end_object() override;
};

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

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */


#endif /* WIRE_JSON_DETAIL_PARSER_BASE_HPP_ */
