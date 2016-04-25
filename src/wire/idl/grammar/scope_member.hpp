/*
 * scope_member.hpp
 *
 *  Created on: 25 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_IDL_GRAMMAR_SCOPE_MEMBER_HPP_
#define WIRE_IDL_GRAMMAR_SCOPE_MEMBER_HPP_

#include <wire/idl/grammar/type_name.hpp>

namespace wire {
namespace idl {
namespace grammar {

//----------------------------------------------------------------------------
//    data member declaration
//----------------------------------------------------------------------------
using data_member_decl = ::std::pair< type_name, ::std::string >;

struct create_data_member_decl_func {
    using result = data_member_decl;

    template < typename TokenValue >
    data_member_decl
    operator()(type_name const& tn, TokenValue const& tok) const
    {
        return ::std::make_pair(tn, ::std::string{ tok.begin(), tok.end() });
    }
};

::boost::phoenix::function< create_data_member_decl_func > const create_data_member
      = create_data_member_decl_func{};

//----------------------------------------------------------------------------
template < typename InputIterator, typename Lexer >
struct data_member_grammar : parser_value_grammar< InputIterator, data_member_decl, Lexer > {
    using main_rule_type = parser_value_rule< InputIterator, data_member_decl, Lexer >;
    using type_name_rule = type_name_grammar< InputIterator, Lexer >;

    template < typename TokenDef >
    data_member_grammar(TokenDef const& tok)
        : data_member_grammar::base_type{ data_member },
          type_name_{tok}
    {
        namespace qi = ::boost::spirit::qi;
        using qi::_val;
        using qi::_1;
        using qi::_2;

        data_member = (type_name_ >> tok.identifier >> ';')
                [ _val = create_data_member(_1, _2) ];
    }

    main_rule_type     data_member;
    type_name_rule     type_name_;
};

//----------------------------------------------------------------------------
//    const data member declaration
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
template < typename InputIterator, typename Lexer >
struct const_member_grammar : parser_grammar< InputIterator, Lexer > {
    using main_rule_type = parser_rule< InputIterator, Lexer >;
    using type_name_rule = type_name_grammar< InputIterator, Lexer >;
    using rule_type = parser_rule< InputIterator, Lexer >;

    template < typename TokenDef >
    const_member_grammar(TokenDef const& tok)
        : const_member_grammar::base_type{ const_member },
          type_name_{tok}
    {
        const_member =
               -tok.const_ >> type_name_ >> -tok.const_ >> tok.identifier
            >> '='
            >> initializer
            >> ';'
        ;
        initializer = (tok.dec_literal | tok.oct_literal | tok.hex_literal
                | tok.string_literal | initializer_list);
        initializer_list =
            '{'
                >> -(initializer >> *(',' >> initializer))
            >> '}';
    }

    main_rule_type     const_member;
    type_name_rule     type_name_;
    rule_type          initializer;
    rule_type          initializer_list;
};

//----------------------------------------------------------------------------
struct function_decl {
    using param_type = ::std::pair< type_name, ::std::string >;
    using params_list = ::std::vector< param_type >;
    using type_list = ::std::vector< type_name >;

    type_name         return_type;
    ::std::string     name;
    params_list       params;
    bool              const_qualified;
    type_list         throw_spec;
};

//----------------------------------------------------------------------------
template < typename InputIterator, typename Lexer >
struct function_member_grammar : parser_value_grammar< InputIterator, function_decl, Lexer > {
    using main_rule_type = parser_value_rule< InputIterator, function_decl, Lexer >;
    using type_name_rule = type_name_grammar< InputIterator, Lexer >;
    using rule_type = parser_rule< InputIterator, Lexer >;
    template < typename T >
    using value_rule_type = parser_value_rule< InputIterator, T, Lexer >;

    template < typename TokenDef >
    function_member_grammar(TokenDef const& tok)
        : function_member_grammar::base_type{ func_member },
          type_name_{tok}
    {
        namespace phx = ::boost::phoenix;
        namespace qi = ::boost::spirit::qi;
        using qi::_val;
        using qi::_1;
        using qi::_2;

        param = (type_name_ >> tok.identifier)
                [ _val = create_data_member(_1, _2) ];
        param_list %= -(param) >> *(',' >> param);

        throw_list %= type_name_ >> *(',' >> type_name_);

        func_member = type_name_               [ phx::bind(&function_decl::return_type, _val) = _1 ]
            >> tok.identifier                  [ phx::bind(&function_decl::name, _val) = to_string(_1) ]
            >> '('
                >> param_list                  [ phx::bind(&function_decl::params, _val) = _1 ]
            >> ')'
            >> -tok.const_                     [ phx::bind(&function_decl::const_qualified, _val) = true ]
            >> -(tok.throw_ >> '('
                >> throw_list                  [ phx::bind(&function_decl::throw_spec, _val) = _1]
            >> ')')
            >> ';'
        ;
    }

    main_rule_type                                 func_member;
    type_name_rule                                 type_name_;
    value_rule_type< function_decl::param_type >   param;
    value_rule_type< function_decl::params_list >  param_list;
    value_rule_type< function_decl::type_list >    throw_list;
};

//----------------------------------------------------------------------------
//    type alias decl
//----------------------------------------------------------------------------
using type_alias_decl = ::std::pair< ::std::string, type_name >;
//----------------------------------------------------------------------------
template < typename InputIterator, typename Lexer >
struct type_alias_grammar : parser_value_grammar< InputIterator, type_alias_decl, Lexer > {
    using main_rule_type = parser_value_rule< InputIterator, type_alias_decl, Lexer >;
    using type_name_rule = type_name_grammar< InputIterator, Lexer >;

    template < typename TokenDef >
    type_alias_grammar(TokenDef const& tok)
        : type_alias_grammar::base_type{ type_alias },
          type_name_{tok}
    {
        namespace phx = ::boost::phoenix;
        namespace qi = ::boost::spirit::qi;
        using qi::_val;
        using qi::_1;

        type_alias = tok.using_
            >> tok.identifier      [ phx::bind(&type_alias_decl::first, _val) = to_string(_1) ]
            >> '=' >> type_name_   [ phx::bind(&type_alias_decl::second, _val) = _1 ]
            >> ';'
        ;
    }

    main_rule_type     type_alias;
    type_name_rule     type_name_;
};

}  /* namespace grammar */
}  /* namespace idl */
}  /* namespace wire */


#endif /* WIRE_IDL_GRAMMAR_SCOPE_MEMBER_HPP_ */
