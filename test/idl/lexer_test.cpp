/*
 * lexer_test.cpp
 *
 *  Created on: Apr 18, 2016
 *      Author: zmij
 */

#include <gtest/gtest.h>
#include "config.hpp"

#include <wire/idl/preprocess.hpp>
#include <wire/idl/lexer.hpp>

#include <iostream>
#include <fstream>
#include <iterator>

#include <grammar/grammar_parse_test.hpp>

namespace wire {
namespace idl {
namespace lexer {
namespace test {

GRAMMAR_TEST(location_grammar, PreprocessorLocation,
    ::testing::Values(
        R"~(#line 11 "/wire/lexer_test.wire"
)~",
        R"~(#line 100500 "/wire/sugar.wire"
)~"
    ),
    ::testing::Values(
        R"~(#line 11 ")~",
        R"~(#line 11 "")~",
        R"~(#line  "/wire/compilation_unit_track.wire")~",
        R"~(#line 11 "/wire/compilation_unit_track.wire")~"
    )
);

namespace {

::std::string const TEST_PREPROC_MESSAGE =
R"~(#line 11 "/wire/lexer_test.wire"
)~";

}  // namespace

TEST(Lexer, LocationGrammar)
{
    namespace qi = ::boost::spirit::qi;
    using iterator_type = ::std::string::const_iterator;
    using grammar_type = location_grammar<iterator_type>;

    iterator_type f = TEST_PREPROC_MESSAGE.begin();
    iterator_type l = TEST_PREPROC_MESSAGE.end();

    source_location loc;// {"", 100, 500};
    EXPECT_TRUE(qi::parse(f, l, grammar_type{}, loc));
    EXPECT_EQ(l, f);
    EXPECT_EQ(11, loc.line);
    EXPECT_EQ(0, loc.character);
    EXPECT_EQ("/wire/lexer_test.wire", loc.file);
}

TEST(Lexer, UpdateLocation)
{
    source_location loc {"", 100, 500};
    EXPECT_NO_THROW(file_location_func{}(loc, TEST_PREPROC_MESSAGE.begin(), TEST_PREPROC_MESSAGE.end()));
    EXPECT_EQ(11, loc.line);
    EXPECT_EQ(0, loc.character);
    EXPECT_EQ("/wire/lexer_test.wire", loc.file);
}

TEST(Lexer, TrackCompilationUnit)
{
    namespace lex = ::boost::spirit::lex;
    using input_stream_iterator = ::std::istream_iterator<char>;
    using output_stream_iterator = ::std::ostream_iterator<char>;
    using iterator_type = ::std::string::const_iterator;
    using token_value_type = token<>;
    using lexer_type = lex::lexertl::actor_lexer< token_value_type >;

    const std::string file_name = ::wire::test::DATA_SRC_ROOT + "/wire/lexer_test.wire";
    preprocessor pp{ file_name, {{ ::wire::test::IDL_ROOT }, true} };

    ::std::size_t line_count = 0;
    {
        ::std::ifstream file(file_name);
        if (file) {
            char c;
            while(file.get(c)) {
                if (c == '\n') {
                    ++line_count;
                }
            }
        }
    }

    ::std::ostringstream os;
    ::std::copy( input_stream_iterator{ pp.stream() },
            input_stream_iterator{},
            output_stream_iterator{ os } );

    ::std::string input_str = os.str();
    wire_tokens<lexer_type> lexer;
    auto sb = input_str.data();
    auto se = sb + input_str.size();
    auto iter = lexer.begin(sb, se);
    auto end = lexer.end();

    while (iter != end && token_is_valid(*iter)) {
        switch (iter->id()) {
            case token_locn:
                ::std::cerr << "Update file location\n"
                     << lexer.current_location.file << "\n"
                     << ::std::setw(3) << lexer.current_location.line << ": ";
                break;
            case token_ns:
                ::std::cerr << "Namespace";
                break;
            case token_struct:
                ::std::cerr << "Structure";
                break;
            case token_class:
                ::std::cerr << "Class";
                break;
            case token_interface:
                ::std::cerr << "Interface";
                break;
            case token_exception:
                ::std::cerr << "Exception";
                break;
            case token_enum:
                ::std::cerr << "Enumeration";
                break;

            case token_const:
                ::std::cerr << "const";
                break;
            case token_using:
                ::std::cerr << "using";
                break;
            case token_throw:
                ::std::cerr << "throw";
                break;

            case token_comma:
                ::std::cerr << ",";
                break;
            case token_colon:
                ::std::cerr << ":";
                break;
            case token_scope_resolution:
                ::std::cerr << ":SCOPE:";
                 break;
            case token_semicolon:
                ::std::cerr << ";";
                break;
            case token_assign:
                ::std::cerr << "=";
                break;
            case token_asterisk:
                ::std::cerr << "*";
                break;
            case token_brace_open:
                ::std::cerr << "(";
                break;
            case token_brace_close:
                ::std::cerr << ")";
                break;
            case token_block_start:
                ::std::cerr << "{";
                break;
            case token_block_end:
                ::std::cerr << "}";
                break;
            case token_angled_open:
                ::std::cerr << "<";
                break;
            case token_angled_close:
                ::std::cerr << ">";
                break;
            case token_attrib_start:
                ::std::cerr << "[[";
                break;
            case token_attrib_end:
                ::std::cerr << "]]";
                break;

            case token_identifier:
                ::std::cerr << "Identifier("
                      << ::std::string{ iter->value().begin(), iter->value().end() } << ")";
                break;
            case token_number:
                ::std::cerr << "NUMBER";
                break;
            case token_oct_number:
                ::std::cerr << "OCTAL";
                break;
            case token_hex_number:
                ::std::cerr << "HEX";
                break;
            case token_float_literal:
                ::std::cerr << "FLOAT";
                break;
            case token_quoted_string:
                ::std::cerr << "\"string literal\"";
                break;

            case token_c_comment:
                ::std::cerr << "C Comment " << ::std::string{ iter->value().begin(), iter->value().end() } << "\n"
                    << ::std::setw(3) << lexer.current_location.line << ": ";
                break;
            case token_cpp_comment:
                ::std::cerr << "CPP Comment: " << ::std::string{ iter->value().begin(), iter->value().end() }
                    << ::std::setw(3) << lexer.current_location.line << ": ";
                break;
            case token_eol:
                ::std::cerr << "\n"
                     << ::std::setw(3) << lexer.current_location.line << ": ";
                break;
            case token_whitespace:
                ::std::cerr << *iter->value().begin();
                break;
            default:
                ::std::cerr << "{_|_}";
                break;
        }
        ++iter;
    }

    EXPECT_EQ(end, iter);
    EXPECT_EQ(file_name, lexer.current_location.file);
    EXPECT_EQ(0, lexer.current_location.character);

}

}  // namespace test
}  /* namespace lexer */
}  // namespace idl
}  // namespace wire

