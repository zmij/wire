/*
 * json_grammar.hpp
 *
 *  Created on: 23 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_JSON_GRAMMAR_HPP_
#define WIRE_JSON_DETAIL_JSON_GRAMMAR_HPP_

#include <wire/idl/grammar/common.hpp>
#include <wire/json/detail/parser_state_adapter.hpp>

namespace wire {
namespace json {
namespace grammar {

struct dequote_func {
    using result = ::std::string;

    template < typename TokenValue >
    result
    operator()(TokenValue const& tok) const
    {
        // TODO Grammar for unescaping
        auto sz = ::std::distance(tok.begin(), tok.end());
        auto b = tok.begin();
        ++b;
        auto e = b;
        ::std::advance(e, sz - 2);
        return {b, e};
    }
};

::boost::phoenix::function< dequote_func > const dequote { dequote_func{} };

template < typename InputIterator, typename Lexer >
struct json_grammar : idl::grammar::parser_grammar< InputIterator, Lexer > {
    using main_rule_type    = idl::grammar::parser_rule< InputIterator, Lexer >;
    using rule_type         = idl::grammar::parser_rule< InputIterator, Lexer >;

    template < typename TokenDef, typename ParserState >
    json_grammar(TokenDef const& tok, ParserState& state)
        : json_grammar::base_type{json}
    {
        using state_adapter = parser_state_adapter< ParserState >;
        namespace qi = ::boost::spirit::qi;
        using qi::_1;
        using qi::eps;

        state_adapter parser{state};

        json = value;
        value = object | array
            | tok.string_literal        [ parser.string_literal( dequote(_1) ) ]
            | tok.empty_string          [ parser.string_literal("") ]
            | tok.integral_literal      [ parser.integral_literal(_1) ]
            | tok.float_literal         [ parser.float_literal(_1) ]
            | tok.true_                 [ parser.bool_literal(true) ]
            | tok.false_                [ parser.bool_literal(false) ]
            | tok.null                  [ parser.null_literal() ]
        ;

        array   = '['  >> eps           [ parser.start_array() ]
                >> -(value >> *(',' >> value))
                >> ']' >> eps           [ parser.end_array() ]
        ;
        object  = '{'  >> eps           [ parser.start_object() ]
                >> -(member >> *(',' >> member))
                >> '}' >> eps           [ parser.end_object() ]
        ;
        member  = tok.string_literal    [ parser.start_member( dequote(_1) ) ]
                >> ':' >> value;
    }

    main_rule_type          json;
    rule_type               member;
    rule_type               value, object, array;
};

}  /* namespace grammar */
}  /* namespace json */
}  /* namespace wire */

#endif /* WIRE_JSON_DETAIL_JSON_GRAMMAR_HPP_ */
