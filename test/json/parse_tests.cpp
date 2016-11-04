/*
 * parse_tests.cpp
 *
 *  Created on: 13 июня 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>
#include <wire/json/parser.hpp>
#include <iterator>
#include "debug_parser.hpp"

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
    detail::boolean_parser parser{val};

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

}  /* namespace test */
}  /* namespace json */
}  /* namespace wire */
