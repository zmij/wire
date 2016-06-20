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

    enum token_ids {
        id_string        = 1000,
        id_empty_string,
        id_integral,
        id_float,
        id_true,
        id_false,
        id_null
    };

    json_tokens()
        : string_literal{R"~(\"((\\\")|(\\.)|[^\"])+\")~", id_string },
          empty_string{R"~(\"\")~", id_empty_string},
          integral_literal{"-?([1-9][0-9]*)|0", id_integral},
          float_literal{"-?([1-9][0-9]*)?\\.[0-9]*([eE]-?[1-9][0-9]*)?", id_float},
          true_{"true", id_true},
          false_{"false", id_false},
          null{"null", id_null}
    {
        self = string_literal | empty_string
            | integral_literal | float_literal
            | true_ | false_ | null
            | '{' | '}' | '[' | ']' | ':' | ','
        ;
        self("WS") = token_def<>("[ \\t\\n]+")
            | R"~(\/\*[^*]*\*+([^/*][^*]*\*+)*\/)~" // C-style comments
            | R"~(\/\/.*?\n)~"                      // C++-style comments
        ;
    }

    token_def< ::std::string > string_literal, empty_string;
    token_def< long > integral_literal;
    token_def< long double > float_literal;
    token_def< bool > true_, false_;
    token_def<> null;
};

}  /* namespace lexer */
}  /* namespace json */
}  /* namespace wire */


#endif /* WIRE_JSON_DETAIL_JSON_LEXER_HPP_ */
