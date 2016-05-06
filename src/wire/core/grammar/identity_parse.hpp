/*
 * identity_parse.hpp
 *
 *  Created on: May 5, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_GRAMMAR_IDENTITY_PARSE_HPP_
#define WIRE_CORE_GRAMMAR_IDENTITY_PARSE_HPP_

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include <wire/core/grammar/uuid_parse.hpp>
#include <wire/core/identity.hpp>

namespace wire {
namespace core {
namespace grammar {
namespace parse {

template < typename InputIterator >
struct identity_grammar : parser_value_grammar< InputIterator, identity > {
    using value_type = identity;

    identity_grammar() : identity_grammar::base_type(id)
    {
        namespace qi = ::boost::spirit::qi;
        namespace phx = ::boost::phoenix;
        using qi::char_;
        using qi::_val;
        using qi::_1;

        identifier %= char_("a-zA-Z0-9_") >> *char_("a-zA-Z0-9._:-");
        id = -(identifier [ phx::bind(&identity::category, _val) = _1 ] >> "/")
            >> ((uuid  [ phx::bind(&identity::id, _val) = _1 ])
             | (identifier [ phx::bind(&identity::id, _val) = _1 ]))
        ;
    }

    parser_value_rule< InputIterator, identity >        id;
    parser_value_rule< InputIterator, ::std::string >   identifier;
    uuid_grammar< InputIterator >                       uuid;
};

}  /* namespace parse */
}  /* namespace grammar */
}  /* namespace core */
}  /* namespace wire */

#endif /* WIRE_CORE_GRAMMAR_IDENTITY_PARSE_HPP_ */
