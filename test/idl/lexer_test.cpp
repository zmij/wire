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

GRAMMAR_TEST(grammar::location_grammar, PreprocessorLocation,
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
    using grammar_type = grammar::location_grammar<iterator_type>;

    iterator_type f = TEST_PREPROC_MESSAGE.begin();
    iterator_type l = TEST_PREPROC_MESSAGE.end();

    source_location loc;// {"", 100, 500};
    EXPECT_TRUE(qi::parse(f, l, grammar_type{}, loc));
    EXPECT_EQ(l, f);
    EXPECT_EQ(11, loc.line);
    EXPECT_EQ(0, loc.character);
    EXPECT_EQ("/wire/lexer_test.wire", loc.file);
}

}  // namespace test
}  /* namespace lexer */
}  // namespace idl
}  // namespace wire

