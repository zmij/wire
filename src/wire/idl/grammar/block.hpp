/*
 * block.hpp
 *
 *  Created on: 25 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_IDL_GRAMMAR_BLOCK_HPP_
#define WIRE_IDL_GRAMMAR_BLOCK_HPP_

#include <wire/idl/grammar/scope_member.hpp>

namespace wire {
namespace idl {
namespace grammar {

template < typename InputIterator, typename Lexer >
struct block_grammar : parser_grammar< InputIterator, Lexer > {
    using main_rule_type        = parser_rule< InputIterator, Lexer >;
    using type_name_rule        = type_name_grammar< InputIterator, Lexer >;
    using rule_type             = parser_rule< InputIterator, Lexer >;
    using type_alias_rule       = type_alias_grammar<InputIterator, Lexer>;
    using constant_rule         = const_member_grammar<InputIterator, Lexer>;
    using forward_rule          = forward_decl_grammar<InputIterator, Lexer>;
    using data_member_rule      = data_member_grammar<InputIterator, Lexer>;
    using function_member_rule  = function_member_grammar<InputIterator, Lexer>;
    using enum_rule             = enum_grammar<InputIterator, Lexer>;
    using annotation_rule       = annotation_grammar<InputIterator, Lexer>;

    template < typename T, typename ... Rest >
    using value_rule_type = parser_value_rule<InputIterator, T, Lexer, Rest ...>;

    template < typename TokenDef, typename ParserState >
    block_grammar(TokenDef const& tok, ParserState& st)
        : block_grammar::base_type{block},
          type_name_{tok},
          annotation_{tok},
          type_alias{tok},
          fwd_decl{tok},
          enum_{tok},
          constant{tok},
          data_member_{tok},
          function_{tok}
    {
        using update_parser_state = parser_state_update< ParserState >;
        namespace qi = ::boost::spirit::qi;
        using qi::_1;
        using qi::_2;
        using qi::char_;

        update_parser_state scope{ st };

        scope_member = annotation_      [ scope.add_annotations(_1) ]
            | type_alias                [ scope.add_type_alias(_1) ]
            | fwd_decl                  [ scope.forward_declare(_1) ]
            | enum_                     [ scope.declare_enum(_1) ]
            | constant                  [ scope.add_constant(_1) ]
            | struct_ | interface_ | class_ | exception_
        ;
        struct_ = tok.struct_ >> tok.identifier         [ scope.start_structure(to_string(_1)) ]
            >> '{'
                >> *(scope_member | data_member)
            >> '}' >> char_(';')                        [ scope.end_scope() ]
        ;

        interface_ = tok.interface
            >> (tok.identifier >> -(':' >> type_list))  [ scope.start_interface(to_string(_1), _2) ]
            >> '{'
                >> *(scope_member | function)
            >> '}' >> char_(';')                        [ scope.end_scope() ]
        ;

        class_ = tok.class_
            >> (tok.identifier >> -(':' >> type_list))  [ scope.start_class(to_string(_1), _2) ]
            >> '{'
                >> *(scope_member | function | data_member)
            >> '}' >> char_(';')                        [ scope.end_scope() ]
        ;

        exception_ = tok.exception
            >> (tok.identifier >> -(':' >> type_name_)) [ scope.start_exception(to_string(_1), _2) ]
            >> '{'
                >> *(scope_member | data_member)
            >> '}' >> char_(';')                        [ scope.end_scope() ]
        ;

        namespace_ = tok.namespace_ >> tok.identifier   [ scope.start_namespace(to_string(_1)) ]
            >> '{'
                >> *(scope_member | namespace_)
            >> char_('}')                               [ scope.end_scope() ]
        ;

        type_list %= type_name_ >> -(',' >> type_name_);

        block = scope_member | namespace_;

        data_member = data_member_                      [ scope.add_data_member(_1) ];
        function = function_                            [ scope.add_func_member(_1) ];
    }

    main_rule_type                      block;
    type_name_rule                      type_name_;
    annotation_rule                     annotation_;
    type_alias_rule                     type_alias;
    forward_rule                        fwd_decl;
    enum_rule                           enum_;
    constant_rule                       constant;

    data_member_rule                    data_member_;
    function_member_rule                function_;

    rule_type                           data_member, function; // Semantic action attached

    value_rule_type< type_name_list >   type_list;
    rule_type scope_member, struct_, interface_, class_, exception_, namespace_;
};

}  /* namespace grammar */
}  /* namespace idl */
}  /* namespace wire */

#endif /* WIRE_IDL_GRAMMAR_BLOCK_HPP_ */
