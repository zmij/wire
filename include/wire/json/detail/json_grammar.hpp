/*
 * json_grammar.hpp
 *
 *  Created on: 23 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_JSON_DETAIL_JSON_GRAMMAR_HPP_
#define WIRE_JSON_DETAIL_JSON_GRAMMAR_HPP_

#include <wire/json/detail/parser_state_adapter.hpp>

namespace wire {
namespace json {
namespace grammar {

template < typename InputIterator, typename Lexer, typename CharT >
using parser_grammar = ::boost::spirit::qi::grammar<
        InputIterator,
        ::boost::spirit::qi::in_state_skipper< Lexer, CharT const* >
>;

template < typename InputIterator, typename Lexer, typename CharT >
using parser_rule = ::boost::spirit::qi::rule <
        InputIterator,
        ::boost::spirit::qi::in_state_skipper< Lexer, CharT const* >
>;

template < typename CharT, typename Traits = ::std::char_traits< CharT > >
struct dequote_func {
    using result = ::std::basic_string<CharT, Traits>;

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

template < typename InputIterator, typename Lexer, typename CharT,
        typename Traits = ::std::char_traits<CharT> >
struct json_grammar : parser_grammar< InputIterator, Lexer, CharT > {
    using main_rule_type    = parser_rule< InputIterator, Lexer, CharT >;
    using rule_type         = parser_rule< InputIterator, Lexer, CharT >;
    using json_io_type      = json_io_base<CharT, Traits>;
    using char_type         = typename json_io_type::char_type;
    using string_type       = typename json_io_type::string_type;
    using chars             = typename json_io_type::chars;

    constexpr static char_type
    cvt(chars v)
    { return static_cast<char_type>(v); }

    template < typename TokenDef, typename ParserState >
    json_grammar(TokenDef const& tok, ParserState& state)
        : json_grammar::base_type{json}
    {
        using state_adapter = basic_parser_state_adapter< ParserState, CharT, Traits >;
        using dequote_type  = dequote_func<CharT, Traits>;
        ::boost::phoenix::function<dequote_type> const dequote{dequote_type{}};

        namespace qi = ::boost::spirit::qi;
        using qi::_1;
        using qi::eps;
        using qi::lit;

        state_adapter parser{state};

        value = object | array
            | tok.string_literal                                            [ parser.string_literal( dequote(_1) ) ]
            | tok.empty_string                                              [ parser.string_literal(string_type{}) ]
            | tok.integral_literal                                          [ parser.integral_literal(_1) ]
            | tok.float_literal                                             [ parser.float_literal(_1) ]
            | tok.true_                                                     [ parser.bool_literal(true) ]
            | tok.false_                                                    [ parser.bool_literal(false) ]
            | tok.null                                                      [ parser.null_literal() ]
        ;

        array   = (cvt(chars::start_array) >> lit(cvt(chars::end_array))    [ parser.start_array(), parser.end_array() ])
                | (lit(cvt(chars::start_array)) >> eps                      [ parser.start_array() ]
                >> -(element >> *(cvt(chars::comma) >> element))
                >> lit(cvt(chars::end_array)) >> eps                        [ parser.end_array() ])
        ;
        object  = cvt(chars::start_object)  >> eps                          [ parser.start_object() ]
                >> -(member >> *(cvt(chars::comma) >> member))
                >> cvt(chars::end_object) >> eps                            [ parser.end_object() ]
        ;
        member  = tok.string_literal                                        [ parser.start_member( dequote(_1) ) ]
                >> cvt(chars::colon) >> value;

        element = eps                                                       [ parser.start_element() ]
                >> value;

        json = value.alias();
    }

    main_rule_type          json;
    rule_type               member, element;
    rule_type               value, object, array;
};

}  /* namespace grammar */
}  /* namespace json */
}  /* namespace wire */

#endif /* WIRE_JSON_DETAIL_JSON_GRAMMAR_HPP_ */
