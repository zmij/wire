/*
 * json_test_suite.cpp
 *
 *  Created on: Nov 8, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>

#include <wire/json/parser.hpp>
#include "json_test_config.hpp"
#include "debug_parser.hpp"
#include <fstream>

namespace wire {
namespace json {
namespace test {

class parse_test : public ::testing::TestWithParam< ::std::string > {
};

using pass_parse_test = parse_test;
using fail_parse_test = parse_test;
using unsure_test     = parse_test;

TEST_P(pass_parse_test, PASS)
{
    auto fname = GetParam();
    ::std::ifstream is{JSON_TEST_DATA_ROOT + fname};
    debug_parser parser{false};
    bool r = detail::parse(parser, is);
    EXPECT_TRUE(r) << fname << " has been successfully parsed";
}

TEST_P(fail_parse_test, FAIL)
{
    auto fname = GetParam();
    ::std::ifstream is{JSON_TEST_DATA_ROOT + fname};
    debug_parser parser{false};
    bool r = detail::parse(parser, is);
    EXPECT_FALSE(r) << fname << " has failed to parse";
}

TEST_P(unsure_test, PASS)
{
    auto fname = GetParam();
    ::std::ifstream is{JSON_TEST_DATA_ROOT + fname};
    debug_parser parser{false};
    bool r = false;
    EXPECT_NO_THROW(r = detail::parse(parser, is));
    if (r) {
        ::std::cerr << fname << " successfully parsed\n";
    } else {
        ::std::cerr << fname << " was not parsed\n";
    }
}

@INSTANTIATE_EXPECTED_PASS@
@INSTANTIATE_EXPECTED_FAIL@
@INSTANTIATE_UNSURE@

}  /* namespace test */
}  /* namespace json */
}  /* namespace wire */


