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
struct forward_decl_grammar : parser_grammar< InputIterator, Lexer > {
    using main_rule_type = parser_rule< InputIterator, Lexer >;

    template < typename TokenDef >
    forward_decl_grammar(TokenDef const& tok)
        : forward_decl_grammar::base_type{fwd}
    {
        fwd = (tok.struct_ | tok.interface | tok.class_ | tok.exception)
            >> tok.identifier >> ';';
    }

    main_rule_type fwd;
};

template < typename InputIterator, typename Lexer >
struct enum_grammar : parser_grammar< InputIterator, Lexer > {
    using main_rule_type = parser_rule< InputIterator, Lexer >;
    using rule_type = parser_rule< InputIterator, Lexer >;

    template < typename TokenDef >
    enum_grammar(TokenDef const& tok)
        : enum_grammar::base_type{ enum_ }
    {
        namespace qi = ::boost::spirit::qi;
        using qi::char_;
        enum_ = tok.enum_ >> -tok.class_ >> tok.identifier
            >> '{'
                >> enumerator_list
            >> '}' >> ';';
        enumerator_list = -(enumerator >> *(',' >> enumerator));
        enumerator = tok.identifier
            >> -('=' >> enumerator_init)
        ;
        enumerator_init = tok.dec_literal | tok.oct_literal | tok.hex_literal | expression;
        expression = tok.identifier >> *(char_('|') >> tok.identifier);
    }

    main_rule_type enum_;
    rule_type    enumerator;
    rule_type    enumerator_init;
    rule_type    enumerator_list;
    rule_type    expression;
};

template < typename InputIterator, typename Lexer >
struct block_grammar : parser_grammar< InputIterator, Lexer > {
    using main_rule_type = parser_rule< InputIterator, Lexer >;
    using type_name_rule = type_name_grammar< InputIterator, Lexer >;
    using rule_type = parser_rule< InputIterator, Lexer >;
    using type_alias_rule = type_alias_grammar<InputIterator, Lexer>;
    using constant_rule = const_member_grammar<InputIterator, Lexer>;
    using forward_rule = forward_decl_grammar<InputIterator, Lexer>;
    using data_member_rule = data_member_grammar<InputIterator, Lexer>;
    using function_member_rule = function_member_grammar<InputIterator, Lexer>;
    using enum_rule = enum_grammar<InputIterator, Lexer>;

    template < typename TokenDef >
    block_grammar(TokenDef const& tok)
        : block_grammar::base_type{block},
          type_name_{tok},
          type_alias{tok},
          constant{tok},
          fwd_decl{tok},
          data_member{tok},
          function{tok},
          enum_{tok}
    {
        scope_member = type_alias | fwd_decl
            | struct_ | interface_ | class_ | exception_
            | enum_ | constant;
        struct_ = tok.struct_ >> tok.identifier
            >> '{'
                >> *(scope_member | data_member)
            >> '}' >> ';';

        interface_ = tok.interface >> tok.identifier
            >> -(':' >> type_list)
            >> '{'
                >> *(scope_member | function)
            >> '}' >> ';';

        class_ = tok.class_ >> tok.identifier
            >> -(':' >> type_list)
            >> '{'
                >> *(scope_member | function | data_member)
            >> '}' >> ';';

        exception_ = tok.exception >> tok.identifier
            >> -(':' >> type_name_)
            >> '{'
                >> *(scope_member | data_member)
            >> '}' >> ';';

        namespace_ = tok.namespace_ >> tok.identifier
            >> '{'
                >> *(scope_member | namespace_)
            >> '}';

        type_list = type_name_ >> -(',' >> type_name_);

        block = scope_member | namespace_;
    }

    main_rule_type         block;
    type_name_rule         type_name_;
    type_alias_rule        type_alias;
    constant_rule          constant;
    forward_rule           fwd_decl;
    data_member_rule       data_member;
    function_member_rule   function;
    enum_rule              enum_;
    rule_type              type_list;
    rule_type scope_member, struct_, interface_, class_, exception_, namespace_;
};

}  /* namespace grammar */
}  /* namespace idl */
}  /* namespace wire */

#endif /* WIRE_IDL_GRAMMAR_BLOCK_HPP_ */
