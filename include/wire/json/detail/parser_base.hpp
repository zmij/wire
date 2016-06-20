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

namespace wire {
namespace json {
namespace detail {

template < typename T >
struct parser;

struct parser_base;
using parser_base_ptr = ::std::shared_ptr<parser_base>;

enum class parse_result {
    need_more,
    done
};

struct parser_base {
    virtual ~parser_base() {}

    ::std::size_t
    stack_size() const;

    virtual parse_result
    string_literal(::std::string const&);
    virtual parse_result
    integral_literal(::std::int64_t val);
    virtual parse_result
    float_literal(long double val);
    virtual parse_result
    bool_literal(bool val);
    virtual parse_result
    null_literal();

    virtual parse_result
    start_array();
    virtual parse_result
    end_array();
    virtual parse_result
    start_element();

    virtual parse_result
    start_object();
    virtual parse_result
    end_object();
    virtual parse_result
    start_member(::std::string const& name);
protected:
    parser_base_ptr     current_parser_ = nullptr;
};

struct ignore_parser : parser_base {
    virtual ~ignore_parser() {}

    parse_result
    string_literal(::std::string const&) override;
    parse_result
    integral_literal(::std::int64_t val) override;
    parse_result
    float_literal(long double val) override;
    parse_result
    bool_literal(bool val) override;
    parse_result
    null_literal() override;

    parse_result
    start_array() override;
    parse_result
    end_array() override;
    parse_result
    start_object() override;
    parse_result
    end_object() override;
};

struct ignore_array_parser : ignore_parser {
    virtual ~ignore_array_parser() {}

    parse_result
    end_array() override;
    parse_result
    start_element() override;
};

struct ignore_object_parser : ignore_parser {
    virtual ~ignore_object_parser() {}

    parse_result
    end_object() override;
    parse_result
    start_member(::std::string const&) override;
};

struct delegate_parser : parser_base {
    virtual ~delegate_parser() {}

    parse_result
    string_literal(::std::string const&) override;
    parse_result
    integral_literal(::std::int64_t val) override;
    parse_result
    float_literal(long double val) override;
    parse_result
    bool_literal(bool val) override;
    parse_result
    null_literal() override;

    parse_result
    start_array() override;
    parse_result
    end_array() override;
    parse_result
    start_element() override;

    parse_result
    start_object() override;
    parse_result
    end_object() override;
    parse_result
    start_member(::std::string const&) override;
};

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */


#endif /* WIRE_JSON_DETAIL_PARSER_BASE_HPP_ */
