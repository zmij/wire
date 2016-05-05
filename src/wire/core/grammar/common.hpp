/*
 * common.hpp
 *
 *  Created on: May 5, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_GRAMMAR_COMMON_HPP_
#define WIRE_CORE_GRAMMAR_COMMON_HPP_

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_algorithm.hpp>
#include <boost/spirit/include/phoenix_core.hpp>

namespace wire {
namespace core {
namespace grammar {

template < typename InputIterator, typename ... Rest >
using parser_grammar = ::boost::spirit::qi::grammar< InputIterator, Rest ... >;
template < typename InputIterator, typename ... Rest >
using parser_rule = ::boost::spirit::qi::rule< InputIterator, Rest ... >;

template < typename InputIterator, typename T, typename ... Rest >
using parser_value_grammar = ::boost::spirit::qi::grammar< InputIterator, T(), Rest ... >;
template < typename InputIterator, typename T, typename ... Rest >
using parser_value_rule = ::boost::spirit::qi::rule< InputIterator, T(), Rest ... >;

template < typename ... T >
using parser_locals = ::boost::spirit::qi::locals< T ... >;

}  /* namespace grammar */
}  /* namespace core */
}  /* namespace wire */


#endif /* WIRE_CORE_GRAMMAR_COMMON_HPP_ */
