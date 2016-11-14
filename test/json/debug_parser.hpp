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
    debug_parser(bool blabla = true) : blabla_{blabla} {}
    virtual ~debug_parser() {}

    detail::parse_result
    string_literal(::std::string const& str) override
    {
        if (blabla_)
            ::std::cerr << "String literal '" << str << "'\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    integral_literal(::std::int64_t val) override
    {
        if (blabla_)
            ::std::cerr << "Int literal '" << val << "'\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    float_literal(long double val) override
    {
        if (blabla_)
            ::std::cerr << "Float literal '" << val << "'\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    bool_literal(bool val) override
    {
        if (blabla_)
            ::std::cerr << "Bool literal '" << val << "'\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    null_literal() override
    {
        if (blabla_)
            ::std::cerr << "Null literal\n";
        return detail::parse_result::need_more;
    }

    detail::parse_result
    start_array() override
    {
        if (blabla_)
            ::std::cerr << "Start array\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    end_array() override
    {
        if (blabla_)
            ::std::cerr << "End array\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    start_element() override
    {
        if (blabla_)
            ::std::cerr << "Start element\n";
        return detail::parse_result::need_more;
    }

    detail::parse_result
    start_object() override
    {
        if (blabla_)
            ::std::cerr << "Start object\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    end_object() override
    {
        if (blabla_)
            ::std::cerr << "End object\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    start_member(::std::string const& str) override
    {
        if (blabla_)
            ::std::cerr << "Start member '" << str << "'\n";
        return detail::parse_result::need_more;
    }
    bool blabla_;
};

}  /* namespace test */
}  /* namespace json */
}  /* namespace wire */


#endif /* JSON_DEBUG_PARSER_HPP_ */
