/*
 * ast_grammar.hpp
 *
 *  Created on: 18 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_IDL_QNAME_GRAMMAR_HPP_
#define WIRE_IDL_QNAME_GRAMMAR_HPP_

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_algorithm.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

#include <wire/idl/qname.hpp>

namespace wire {
namespace idl {
namespace ast {
namespace parse {

template < typename InputIterator >
struct qualified_name_grammar : ::boost::spirit::qi::grammar< InputIterator, qname() > {
    using string_seq = qname::string_seq;

    qualified_name_grammar() : qualified_name_grammar::base_type(root)
    {
        namespace qi = boost::spirit::qi;
        namespace phx = boost::phoenix;
        using qi::char_;
        using qi::lit;
        using qi::eps;
        using qi::_val;
        using qi::_1;

        identifier %= char_("a-zA-Z_") >> *char_("a-zA-Z0-9_");
        components %= identifier >> *("::" >> identifier);
        root = eps[ phx::bind(&qname::fully, _val) = false ]
            >> -(qi::lit("::")[ phx::bind(&qname::fully, _val) = true ])
            >> components[ phx::bind(&qname::components, _val) = _1 ]
        ;
    }

    ::boost::spirit::qi::rule< InputIterator, qname() >           root;
    ::boost::spirit::qi::rule< InputIterator, ::std::string() >   identifier;
    ::boost::spirit::qi::rule< InputIterator, string_seq() >      components;
};

}  /* namespace parse */
}  /* namespace ast */
}  /* namespace idl */
}  /* namespace wire */

#endif /* WIRE_IDL_QNAME_GRAMMAR_HPP_ */
