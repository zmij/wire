/*
 * reference_parse.hpp
 *
 *  Created on: May 5, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_GRAMMAR_REFERENCE_PARSE_HPP_
#define WIRE_CORE_GRAMMAR_REFERENCE_PARSE_HPP_

#include <wire/core/grammar/identity_parse.hpp>
#include <wire/core/grammar/endpoint_parse.hpp>
#include <wire/core/reference.hpp>

namespace wire {
namespace core {
namespace grammar {
namespace parse {

template < typename InputIterator >
struct reference_grammar : parser_value_grammar< InputIterator, reference_data > {
    using value_type = reference_data;

    reference_grammar() : reference_grammar::base_type(ref)
    {
        namespace qi = ::boost::spirit::qi;
        namespace phx = ::boost::phoenix;
        using qi::char_;
        using qi::space;
        using qi::_val;
        using qi::_1;
        using qi::_2;

        //space = char_(" \t");
        facet %= *char_("a-zA-Z0-9_");
        ref = id                                [ phx::bind(&reference_data::object_id, _val) = _1 ]
            >> -('['
                    >> facet                    [ phx::bind(&reference_data::facet, _val) = _1 ]
            >> ']')
            >> (('@' >> id)                     [ phx::bind(&reference_data::adapter, _val) = _1 ]
             | ( (+space | ':') >> endpoints )  [ phx::bind(&reference_data::endpoints, _val) = _2 ])
        ;

    }

    parser_value_rule< InputIterator, reference_data >  ref;
    identity_grammar<InputIterator>                     id;
    parser_value_rule< InputIterator, ::std::string >   facet;
    endpoints_grammar<InputIterator, endpoint_list>     endpoints;
};

}  /* namespace parse */
}  /* namespace grammar */
}  /* namespace core */
}  /* namespace wire */


#endif /* WIRE_CORE_GRAMMAR_REFERENCE_PARSE_HPP_ */
