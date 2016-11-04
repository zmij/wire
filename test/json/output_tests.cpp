/*
 * output_tests.cpp
 *
 *  Created on: 4 нояб. 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>
#include <wire/json/json_stream.hpp>
#include <sstream>
#include <iostream>
#include "debug_parser.hpp"

namespace wire {
namespace json {
namespace test {

struct test_structure {
    ::std::string   str;
    int             ival;
    float           fval;
};

json::json_ostream&
json_write(json::json_ostream& os, test_structure const& v)
{
    os  << "str" << v.str
        << "ival" << v.ival
        << "fval" << v.fval;
    return os;
}

TEST(Json, SimpleDatatypesOut)
{
    ::std::string const k{R"~(bla"bla"bla)~"};
    test_structure ts{"foo", 100500, 3.14};
    test_structure* p_ts{nullptr};

    ::std::ostringstream os;
    json::json_ostream jos{os, true};
    jos << json::start_object
            << "foo" << "bar"
            << ::std::string{"str"} << 42
            << 100500 << -3.14
            << k << k
            << true << false
            << "struct" << test_structure{"foo", 100500, 3.14}
            << "array" << json::start_array
                << "foo" << "bar"
                << ::std::string{"str"} << 42
                << 100500 << -3.14
                << k << k
                << true << false
                << nullptr
                << test_structure{"foo", 100500, 3.14}
                << json::start_array
                    << "foo" << "bar"
                    << ::std::string{"str"} << 42
                    << 100500 << -3.14
                    << k << k
                    << true << false
                    << json::start_object
                    << json::end_object
                    << json::start_array
                    << json::end_array
                << json::end_array
                << &ts
                << p_ts
            << json::end_array
            << "by_ref" << ts
            << "by_ptr" << &ts
            << "by_null_ptr" << p_ts
        << json::end_object
    ;

    EXPECT_ANY_THROW(jos << "error");
    ::std::cerr << os.str() << "\n";

    ::std::istringstream is(os.str());
    json::test::debug_parser parser;
    bool r = detail::parse(parser, is);
    EXPECT_TRUE(r) << "Successful parse";
}

TEST(Json, VectorOut)
{
    ::std::vector<::std::string> strings{
        "one", "two", "three"
    };
    ::std::vector<int> ints {1, 2, 3};
    {
        ::std::ostringstream os;
        json::json_ostream jos{os};
        jos << strings;

        EXPECT_ANY_THROW(jos << "error");
        EXPECT_EQ(R"~(["one","two","three"])~", os.str()) << "Output vector of strings";

        ::std::istringstream is(os.str());
        json::test::debug_parser parser;
        bool r = detail::parse(parser, is);
        EXPECT_TRUE(r) << "Successful parse";
    }
    {
        ::std::ostringstream os;
        json::json_ostream jos{os};
        jos << ints;

        EXPECT_ANY_THROW(jos << "error");
        EXPECT_EQ("[1,2,3]", os.str()) << "Output vector of ints";

        ::std::istringstream is(os.str());
        json::test::debug_parser parser;
        bool r = detail::parse(parser, is);
        EXPECT_TRUE(r) << "Successful parse";
    }
    {
        ::std::ostringstream os;
        json::json_ostream jos{os};
        jos << json::start_array
            << strings << ints
            << json::end_array;
        EXPECT_ANY_THROW(jos << "error");
        EXPECT_EQ(R"~([["one","two","three"],[1,2,3]])~", os.str()) << "Correct output";

        ::std::istringstream is(os.str());
        json::test::debug_parser parser;
        bool r = detail::parse(parser, is);
        EXPECT_TRUE(r) << "Successful parse";
    }
}

}  /* namespace test */
}  /* namespace json */
}  /* namespace wire */
