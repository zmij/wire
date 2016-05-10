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

struct create_identity_func {
    using result = identity;

    identity
    operator()(::std::string const& cat, identity::id_type const& id) const
    {
        return { cat, id };
    }
    identity
    operator()(identity::id_type const& id) const
    {
        return { id };
    }
};

::boost::phoenix::function<create_identity_func> const create_id = create_identity_func{};

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
        using qi::_2;

        identifier %= char_("a-zA-Z0-9_") >> *char_("a-zA-Z0-9._:-");
        id = (identifier >> "/" >> (uuid | identifier)) [ _val = create_id(_1, _2) ]
             | (uuid | identifier)[ _val = create_id(_1) ];
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
