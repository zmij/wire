/*
 * lexer.hpp
 *
 *  Created on: 22 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_JSON_LEXER_HPP_
#define WIRE_JSON_DETAIL_JSON_LEXER_HPP_

#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_algorithm.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <wire/idl/source_location.hpp>

namespace wire {
namespace json {
namespace lexer {

template < typename Lexer >
struct json_tokens : ::boost::spirit::lex::lexer< Lexer > {
    using base = ::boost::spirit::lex::lexer< Lexer >;
    using base::self;

    template < typename ... T >
    using token_def = ::boost::spirit::lex::token_def<T...>;

    json_tokens()
        : string_literal{R"~(\"((\\\")|(\\.)|[^\"])+\")~" },
          empty_string{R"~("")~"},
          integral_literal{"-?([1-9][0-9]*)|0"},
          float_literal{"-?([1-9][0-9]*)?\\.[0-9]*([eE]-?[1-9][0-9]*)?"},
          true_{"true"}, false_{"false"}, null{"null"}
    {
        self = string_literal | integral_literal | float_literal
            | true_ | false_ | null
            | '{' | '}' | '[' | ']' | ':' | ','
        ;
        self("WS") = token_def<>("[ \\t\\n]+")
            | R"~(\/\*[^*]*\*+([^/*][^*]*\*+)*\/)~" // C-style comments
            | R"~(\/\/.*?\n)~"                      // C++-style comments
        ;
    }

    token_def<> string_literal, empty_string, integral_literal, float_literal;
    token_def<> true_, false_, null;
};

}  /* namespace lexer */
}  /* namespace json */
}  /* namespace wire */


#endif /* WIRE_JSON_DETAIL_JSON_LEXER_HPP_ */
