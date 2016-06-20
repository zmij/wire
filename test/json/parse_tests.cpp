/*
 * parse_tests.cpp
 *
 *  Created on: 13 июня 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>
#include <wire/json/parser.hpp>
#include <wire/json/detail/parser_traits.hpp>
#include <wire/json/detail/parser_base.hpp>

namespace wire {
namespace json {
namespace test {

namespace {

::std::string const TEST_JSON_OBJECT =
R"~({
"string":"string",
"int":123,
"obj": {},
"obj2": null,
"array": [ 123, "123" ],
"empty-array": [],
"sub-obj":{
    "foo" : "bar"
}
})~";

}  /* namespace  */

struct debug_parser : detail::parser_base {
    virtual ~debug_parser() {}

    detail::parse_result
    string_literal(::std::string const& str) override
    {
        ::std::cerr << "String literal " << str << "\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    integral_literal(::std::int64_t val) override
    {
        ::std::cerr << "Int literal " << val << "\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    float_literal(long double val) override
    {
        ::std::cerr << "Float literal " << val << "\n";
        return detail::parse_result::need_more;
    }
    detail::parse_result
    bool_literal(bool val) override
    {
        ::std::cerr << "Bool literal " << val << "\n";
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
        ::std::cerr << "Start member " << str << "\n";
        return detail::parse_result::need_more;
    }
};

TEST(Parser, LexerConcept)
{
    namespace qi = ::boost::spirit::qi;
    using parser_traits = detail::parser_traits< char const* >;

    auto sb = TEST_JSON_OBJECT.data();
    auto se = sb + TEST_JSON_OBJECT.size();

    parser_traits::tokenizer_type tokens;
    parser_traits::token_iterator iter = tokens.begin(sb, se);
    parser_traits::token_iterator end = tokens.end();

    debug_parser parser;
    parser_traits::grammar_type grammar(tokens, parser);
    bool r = qi::phrase_parse(iter, end, grammar, qi::in_state("WS")[tokens.self]);
    EXPECT_TRUE(r);
}

}  /* namespace test */
}  /* namespace json */
}  /* namespace wire */
