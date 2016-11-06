/*
 * parser_traits.hpp
 *
 *  Created on: 20 июня 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_PARSER_TRAITS_HPP_
#define WIRE_JSON_DETAIL_PARSER_TRAITS_HPP_

#include <wire/json/detail/json_lexer.hpp>
#include <wire/json/detail/json_grammar.hpp>

namespace wire {
namespace json {
namespace detail {

template < typename BaseIterator, typename CharT,
        typename Traits = ::std::char_traits<CharT> >
struct parser_traits {
    using base_iterator     = BaseIterator;
    using string_type       = ::std::basic_string<CharT, Traits>;
    using attribs           = ::boost::mpl::vector< string_type, long, long double, bool >;
    using token_type        = ::boost::spirit::lex::lexertl::token< base_iterator, attribs >;
    using lexer_type        = ::boost::spirit::lex::lexertl::lexer< token_type >;
    using tokenizer_type    = lexer::basic_json_tokens< lexer_type, CharT, Traits >;
    using token_iterator    = typename tokenizer_type::iterator_type;
    using grammar_type      = grammar::json_grammar< token_iterator,
                                    typename tokenizer_type::lexer_def, CharT, Traits >;
};

}  /* namespace detail */
}  /* namespace json */
}  /* namespace wire */


#endif /* WIRE_JSON_DETAIL_PARSER_TRAITS_HPP_ */
