/*
 * grammar_common.hpp
 *
 *  Created on: 25 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_IDL_GRAMMAR_COMMON_HPP_
#define WIRE_IDL_GRAMMAR_COMMON_HPP_

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_algorithm.hpp>
#include <boost/spirit/include/phoenix_core.hpp>

namespace wire {
namespace idl {
namespace grammar {

template < typename InputIterator, typename Lexer, typename ... Rest >
using parser_grammar = ::boost::spirit::qi::grammar<
        InputIterator,
        ::boost::spirit::qi::in_state_skipper<Lexer>,
        Rest ...
>;

template < typename InputIterator, typename Lexer, typename ... Rest >
using parser_rule = ::boost::spirit::qi::rule<
        InputIterator,
        ::boost::spirit::qi::in_state_skipper<Lexer>,
        Rest ...
>;

template < typename InputIterator, typename T, typename Lexer, typename ... Rest >
using parser_value_grammar = ::boost::spirit::qi::grammar<
        InputIterator,
        T(),
        ::boost::spirit::qi::in_state_skipper<Lexer>,
        Rest ...
>;

template < typename InputIterator, typename T, typename Lexer, typename ... Rest >
using parser_value_rule = ::boost::spirit::qi::rule<
        InputIterator,
        T(),
        ::boost::spirit::qi::in_state_skipper<Lexer>,
        Rest ...
>;

template < typename ... T >
using parser_locals = ::boost::spirit::qi::locals< T ... >;

struct token_to_string_func {
    using result = ::std::string;

    template < typename TokenValue >
    result
    operator()(TokenValue const& tok) const
    {
        return result{ tok.begin(), tok.end() };
    }
};

::boost::phoenix::function< token_to_string_func > const to_string
      = token_to_string_func{};

struct append_token_to_string_func {
    using result = void;

    template< typename TokenValue >
    result
    operator()(::std::string& val, TokenValue const& tok) const
    {
        val.insert(val.end(), tok.begin(), tok.end());
    }
};

::boost::phoenix::function< append_token_to_string_func > const append
     = append_token_to_string_func{};

}  /* namespace grammar */
}  /* namespace idl */
}  /* namespace wire */


#endif /* WIRE_IDL_GRAMMAR_COMMON_HPP_ */
