/*
 * debug_parser.hpp
 *
 *  Created on: 5 нояб. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef JSON_DEBUG_PARSER_HPP_
#define JSON_DEBUG_PARSER_HPP_

#include <wire/json/parser.hpp>

namespace wire {
namespace json {
namespace test {

struct debug_parser : detail::parser_base {
    virtual ~debug_parser() {}

    detail::parse_result
    string_literal(::std::string const& str) override
    {
        ::std::cerr << "String literal '" << str << "'\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    integral_literal(::std::int64_t val) override
    {
        ::std::cerr << "Int literal '" << val << "'\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    float_literal(long double val) override
    {
        ::std::cerr << "Float literal '" << val << "'\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    bool_literal(bool val) override
    {
        ::std::cerr << "Bool literal '" << val << "'\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    null_literal() override
    {
        ::std::cerr << "Null literal\n";
        return detail::parse_result::need_more;
    }

    detail::parse_result
    start_array() override
    {
        ::std::cerr << "Start array\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    end_array() override
    {
        ::std::cerr << "End array\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    start_element() override
    {
        ::std::cerr << "Start element\n";
        return detail::parse_result::need_more;
    }

    detail::parse_result
    start_object() override
    {
        ::std::cerr << "Start object\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    end_object() override
    {
        ::std::cerr << "End object\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    start_member(::std::string const& str) override
    {
        ::std::cerr << "Start member '" << str << "'\n";
        return detail::parse_result::need_more;
    }
};

}  /* namespace test */
}  /* namespace json */
}  /* namespace wire */


#endif /* JSON_DEBUG_PARSER_HPP_ */
