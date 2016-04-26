/*
 * lexer.hpp
 *
 *  Created on: Apr 18, 2016
 *      Author: zmij
 */

#ifndef WIRE_IDL_LEXER_HPP_
#define WIRE_IDL_LEXER_HPP_

#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_algorithm.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <wire/idl/source_location.hpp>

#include <wire/idl/grammar/source_advance.hpp>

namespace wire {
namespace idl {
namespace lexer {

template <typename Lexer>
struct wire_tokens : ::boost::spirit::lex::lexer< Lexer > {
    using base = ::boost::spirit::lex::lexer< Lexer >;
    using base::self;
    template < typename ... T >
    using token_def = ::boost::spirit::lex::token_def<T...>;

    wire_tokens()
        : source_advance{"#line[ \t]+\\d+[ \t]+\\\"[^\\\"]+\\\"\n"},

          namespace_{"namespace"}, enum_{"enum"}, struct_{"struct"},
          interface{"interface"}, class_{"class"}, exception{"exception"},

          const_{"const"}, throw_{"throw"}, using_{"using"},

          identifier{"[a-zA-Z_][a-zA-Z0-9_]*"},
          dec_literal{"-?([1-9][0-9]*)|0"},
          oct_literal{"0[1-7][0-7]*"},
          hex_literal{"0[xX][0-9a-fA-F]+"},
          string_literal{R"~(\"((\\\")|(\\.)|[^\"])*\")~" },

          scope_resolution{"::"},
          annotation_start{"\\[\\["}, annotation_end{"\\]\\]"}
    {
        self = source_advance
            | namespace_ | enum_ | struct_ | interface | class_ | exception
            | const_ | throw_ | using_
            | identifier
            | dec_literal | oct_literal | hex_literal | string_literal
            | scope_resolution | annotation_start | annotation_end
            | ',' | '.' | ':' | ';'
            | '<' | '>' | '(' | ')' | '{' | '}'
            | '*'
            | '=' | '|' | '&' | '!' | '~'
        ;
        self("WS") = token_def<>("[ \\t\\n]+");
    }

    token_def<> source_advance;

    token_def<> namespace_, enum_, struct_, interface, class_, exception;
    token_def<> const_, throw_, using_;

    token_def<> identifier;
    token_def<> dec_literal,  oct_literal, hex_literal, string_literal;
    token_def<> scope_resolution, annotation_start, annotation_end;
};

}  /* namespace lexer */
}  // namespace idl
}  // namespace wire


#endif /* WIRE_IDL_LEXER_HPP_ */
