/*
 * parse_tests.cpp
 *
 *  Created on: 13 июня 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>
#include <wire/json/parser.hpp>
#include <wire/json/json_istream.hpp>
#include <iterator>
#include "debug_parser.hpp"
#include "test_data_structure.hpp"

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
"sub-obj":{
    "foo" : "bar"
},
"empty-array": [],
"array": [ 123, "123" ]
})~";

}  /* namespace  */


TEST(Parser, CharIterator)
{
    debug_parser parser;
    bool r = detail::parse(parser, TEST_JSON_OBJECT.data(), TEST_JSON_OBJECT.size());
    EXPECT_TRUE(r);
}

TEST(Parser, StreamIterator)
{
    ::std::istringstream is{ TEST_JSON_OBJECT };

    debug_parser parser;
    bool r = detail::parse(parser, is);
    EXPECT_TRUE(r);
}

TEST(Parser, BoolParser)
{
    ::std::string json{"true"};

    bool val{false};
    parser<bool> parser{val};

    EXPECT_TRUE(detail::parse(parser, json));
    EXPECT_TRUE(val);

    json = "false";
    EXPECT_TRUE(detail::parse(parser, json));
    EXPECT_FALSE(val);
}

TEST(Parser, IntegralParser)
{
    ::std::string json{"42"};

    int val{0};
    detail::integral_parser< int > parser{val};
    EXPECT_TRUE(detail::parse(parser, json));
    EXPECT_EQ(42, val);

    json = "0";
    EXPECT_TRUE(detail::parse(parser, json));
    EXPECT_EQ(0, val);

    json = "-42";
    EXPECT_TRUE(detail::parse(parser, json));
    EXPECT_EQ(-42, val);
}

TEST(JIstream, Bool)
{
    {
        ::std::istringstream is{"true"};
        json_istream jis{is};
        bool val{false};
        EXPECT_NO_THROW(jis >> val);
        EXPECT_TRUE(val);
    }
    {
        ::std::istringstream is{"false"};
        json_istream jis{is};
        bool val{true};
        EXPECT_NO_THROW(jis >> val);
        EXPECT_FALSE(val);
    }
}

TEST(JIstream, Integral)
{
    {
        ::std::istringstream is{"100500"};
        json_istream jis{is};
        int val{0};
        EXPECT_NO_THROW(jis >> val);
        EXPECT_EQ(100500, val);
    }
}

TEST(JIstream, String)
{
    ::std::istringstream is{"\"foo\""};
    json_istream jis{is};
    ::std::string val{};
    EXPECT_NO_THROW(jis >> val);
    EXPECT_EQ("foo", val);
}

TEST(JIstream, Struct)
{
    ::std::istringstream is{R"~({"fval":2.13,"ival":42,"str":"bar"})~"};
    json_istream jis{is};
    test_structure ts;
    EXPECT_NO_THROW(jis >> ts);
}

}  /* namespace test */
}  /* namespace json */
}  /* namespace wire */
