/*
 * json_grammar.hpp
 *
 *  Created on: 23 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_JSON_GRAMMAR_HPP_
#define WIRE_JSON_DETAIL_JSON_GRAMMAR_HPP_

#include <wire/idl/grammar/common.hpp>

namespace wire {
namespace json {
namespace grammar {

template < typename InputIterator, typename Lexer >
struct json_grammar : idl::grammar::parser_grammar< InputIterator, Lexer > {
    using main_rule_type    = idl::grammar::parser_rule< InputIterator, Lexer >;
    using rule_type         = idl::grammar::parser_rule< InputIterator, Lexer >;

    template < typename TokenDef >
    json_grammar(TokenDef const& tok)
        : json_grammar::base_type{json}
    {
        json = value;
        value = object | array
            | tok.string_literal | tok.empty_string
            | tok.decimal_literal | tok.float_literal
            | tok.true_ | tok.false_ | tok.null;

        array   = '[' >> -(value >> *(',' >> value)) >> ']';
        object  = '{' >> -(member >> *(',' >> member)) >> '}';
        member  = tok.string_literal >> ':' >> value;
    }

    main_rule_type          json;
    rule_type               member;
    rule_type               value, object, array;
};

}  /* namespace grammar */
}  /* namespace json */
}  /* namespace wire */

#endif /* WIRE_JSON_DETAIL_JSON_GRAMMAR_HPP_ */
