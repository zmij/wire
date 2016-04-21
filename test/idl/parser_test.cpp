/*
 * parser_test.cpp
 *
 *  Created on: Apr 20, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include "config.hpp"

#include <wire/idl/preprocess.hpp>
#include <wire/idl/lexer.hpp>
#include <wire/idl/parser.hpp>

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>

namespace wire {
namespace idl {
namespace parser {
namespace test {

TEST(Parser, Namespace)
{
    const std::string file_name = ::wire::test::DATA_SRC_ROOT + "/wire/namespace.wire";

    preprocessor pp { file_name, {{ ::wire::test::IDL_ROOT }, true } };
    parser_state parser;
    lexer::wire_tokens<parser_state::lexer_type> tokens;

    std::string input_str = pp.to_string();
    auto sb     = input_str.data();
    auto se     = sb + input_str.size();

    auto end    = tokens.end();

    for (auto iter   = tokens.begin(sb, se);
            iter != end && token_is_valid(*iter); ++iter) {
        parser.process_token(tokens.current_location, *iter);
    }

    EXPECT_TRUE(ast::namespace_::global()->find_namespace("test").get());
    EXPECT_TRUE(ast::namespace_::global()->find_namespace("::test").get());
}

TEST(Parser, Structure)
{
    const std::string file_name = ::wire::test::DATA_SRC_ROOT + "/wire/structure.wire";

    preprocessor pp { file_name, {{ ::wire::test::IDL_ROOT }, true } };
    parser_state parser;
    lexer::wire_tokens<parser_state::lexer_type> tokens;

    std::string input_str = pp.to_string();
    auto sb     = input_str.data();
    auto se     = sb + input_str.size();

    auto end    = tokens.end();

    for (auto iter   = tokens.begin(sb, se);
            iter != end && token_is_valid(*iter); ++iter) {
        parser.process_token(tokens.current_location, *iter);
    }

    EXPECT_TRUE(ast::namespace_::global()->find_namespace("test").get());
    EXPECT_TRUE(ast::namespace_::global()->find_namespace("::test").get());

    EXPECT_TRUE(ast::namespace_::global()->find_type("test::buffer").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::buffer").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("test::test_struct").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::test_struct").get());

    EXPECT_TRUE(ast::namespace_::global()->find_entity("test::test_struct::value").get());
    EXPECT_TRUE(ast::namespace_::global()->find_entity("::test::test_struct::value").get());
    EXPECT_TRUE(ast::namespace_::global()->find_entity("test::test_struct::ids").get());
    EXPECT_TRUE(ast::namespace_::global()->find_entity("::test::test_struct::buffer").get());
}

}  // namespace test
}  // namespace parser
}  // namespace core
}  // namespace wire

