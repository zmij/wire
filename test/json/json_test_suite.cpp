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

TEST(JTS, ExpectedPass)
{
    for (auto f : EXPECTED_PASS) {
        ::std::ifstream is{JSON_TEST_DATA_ROOT + f};
        debug_parser parser{false};
        bool r = false;
        ASSERT_NO_FATAL_FAILURE(detail::parse(parser, is));
        EXPECT_NO_THROW(r = detail::parse(parser, is))
            << "Exception when parsing " << f;
        EXPECT_TRUE(r) << f << " has been successfully parsed";
    }
}

TEST(JTS, ExpectedFail)
{
    for (auto f : EXPECTED_FAIL) {
        ::std::ifstream is{JSON_TEST_DATA_ROOT + f};
        debug_parser parser{false};
        bool r = false;
        ASSERT_NO_FATAL_FAILURE(detail::parse(parser, is));
        EXPECT_NO_THROW(r = detail::parse(parser, is))
            << "Exception when parsing " << f;
        EXPECT_FALSE(r) << f << " has failed to parse";
    }
}

}  /* namespace test */
}  /* namespace json */
}  /* namespace wire */


