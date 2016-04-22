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

TEST(Parser, Exception)
{
    const std::string file_name = ::wire::test::DATA_SRC_ROOT + "/wire/exception.wire";

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

    EXPECT_TRUE(ast::namespace_::global()->find_type("test::test_error").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::severe_error").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::test_error::buffer").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::severe_error::buffer").get());

    EXPECT_TRUE(ast::namespace_::global()->find_entity("test::test_error::message").get());
    EXPECT_TRUE(ast::namespace_::global()->find_entity("test::severe_error::message").get());
    EXPECT_TRUE(ast::namespace_::global()->find_entity("test::severe_error::severity").get());
    EXPECT_TRUE(ast::namespace_::global()->find_entity("::test::test_error::buffer").get());
    EXPECT_TRUE(ast::namespace_::global()->find_entity("::test::severe_error::buffer").get());
}

TEST(Parser, Interface)
{
    const std::string file_name = ::wire::test::DATA_SRC_ROOT + "/wire/interface.wire";

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

    EXPECT_TRUE(ast::namespace_::global()->find_type("test::no_service").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::good_service").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::bad_service").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::controversial").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::good_service::good_list").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::bad_service::bad_list").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::controversial::good_list").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::controversial::bad_list").get());

}

TEST(Parser, Class)
{
    const std::string file_name = ::wire::test::DATA_SRC_ROOT + "/wire/class.wire";

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

    EXPECT_TRUE(ast::namespace_::global()->find_type("test::oops").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::shiny_one").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::backdoor").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::base_class").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::derived").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::shiny_one::magic").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::backdoor::key").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::base_class::magic").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::derived::magic").get());
    EXPECT_TRUE(ast::namespace_::global()->find_type("::test::derived::key").get());

    EXPECT_TRUE(ast::namespace_::global()->find_entity("::test::base_class::the_spell").get());
    EXPECT_TRUE(ast::namespace_::global()->find_entity("::test::derived::the_spell").get());
    EXPECT_TRUE(ast::namespace_::global()->find_entity("::test::derived::the_key").get());
}

}  // namespace test
}  // namespace parser
}  // namespace core
}  // namespace wire

