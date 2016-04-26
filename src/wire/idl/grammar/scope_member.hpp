/*
 * scope_member.hpp
 *
 *  Created on: 25 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_IDL_GRAMMAR_SCOPE_MEMBER_HPP_
#define WIRE_IDL_GRAMMAR_SCOPE_MEMBER_HPP_

#include <wire/idl/grammar/type_name.hpp>
#include <wire/idl/grammar/declarations.hpp>

#include <boost/spirit/include/phoenix.hpp>

namespace wire {
namespace idl {
namespace grammar {

//----------------------------------------------------------------------------
//    data member declaration
//----------------------------------------------------------------------------
struct create_data_member_func {
    using result = data_member_decl;

    template < typename TokenValue >
    result
    operator()(type_name const& tn, TokenValue const& tok) const
    {
        return result{ tn, ::std::string{ tok.begin(), tok.end() } };
    }

    result
    operator()(type_name const& tn, ::std::string const& name, data_initializer const& init) const
    {
        return result{ tn, name, init };
    }
};

::boost::phoenix::function<create_data_member_func> const create_dm
     = create_data_member_func{};

struct create_data_initializer_func {
    using result = data_initializer;

    template < typename TokenValue >
    result
    operator()(TokenValue const& tok) const
    {
        return result{ ::std::string{ tok.begin(), tok.end() } };
    }

    result
    operator()(data_initializer::initializer_list const& list) const
    {
        return result{ list };
    }
};

::boost::phoenix::function< create_data_initializer_func > const create_di
      = create_data_initializer_func{};

struct add_initializer_to_list_func {
    using result = void;

    void
    operator()(data_initializer::initializer_list& list, data_initializer const& val) const
    {
        list.push_back(::std::make_shared< data_initializer >(val));
    }
};

::boost::phoenix::function< add_initializer_to_list_func > const add_di
      = add_initializer_to_list_func{};

//----------------------------------------------------------------------------
template < typename InputIterator, typename Lexer >
struct data_initializer_grammar : parser_value_grammar< InputIterator, data_initializer, Lexer > {
    using main_rule_type = parser_value_rule< InputIterator, data_initializer, Lexer >;
    template< typename T >
    using value_rule_type = parser_value_rule< InputIterator, T, Lexer >;

    template< typename TokenDef >
    data_initializer_grammar(TokenDef const& tok)
        : data_initializer_grammar::base_type{ initializer }
    {
        namespace qi = ::boost::spirit::qi;
        namespace phx = ::boost::phoenix;
        using qi::_val;
        using qi::_1;
        using phx::ref;

        initializer =
              (tok.dec_literal | tok.oct_literal
                      | tok.hex_literal | tok.string_literal)   [ _val = create_di(_1) ]
            | initializer_list                                  [ _val = create_di(_1) ]
        ;

        initializer_list =
            '{'
                >> -(initializer                [ add_di(ref(_val), _1) ]
                        >> *(',' >> initializer [ add_di(ref(_val), _1) ]))
            >> '}';
    }

    main_rule_type                                          initializer;
    value_rule_type< data_initializer::initializer_list >   initializer_list;
};

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
        namespace phx = ::boost::phoenix;
        using qi::_val;
        using qi::_1;
        using qi::_2;

        // TODO Optional initializer
        data_member = (type_name_ >> tok.identifier >> ';')
                [ _val = create_dm(_1, _2) ];
    }

    main_rule_type     data_member;
    type_name_rule     type_name_;
};

//----------------------------------------------------------------------------
//    const data member declaration
//----------------------------------------------------------------------------
template < typename InputIterator, typename Lexer >
struct const_member_grammar : parser_value_grammar< InputIterator, data_member_decl, Lexer > {
    using main_rule_type = parser_value_rule< InputIterator, data_member_decl, Lexer >;
    using main_rule_locals_type =
            parser_value_rule<
                InputIterator, data_member_decl, Lexer,
                parser_locals< type_name, ::std::string >
            >;
    using type_name_rule = type_name_grammar< InputIterator, Lexer >;
    using data_init_rule = data_initializer_grammar<InputIterator, Lexer>;

    template < typename TokenDef >
    const_member_grammar(TokenDef const& tok)
        : const_member_grammar::base_type{ main },
          type_name_{tok},
          initializer{tok}
    {
        namespace qi = ::boost::spirit::qi;
        namespace phx = ::boost::phoenix;
        using qi::_val;
        using qi::_1;
        using qi::_2;
        using qi::_a;
        using qi::_b;
        using phx::ref;

        const_member =
               ((tok.const_ >> type_name_)      [ _a = _2 ]
               | (type_name_ >> tok.const_)     [ _a = _1 ])
            >> tok.identifier                   [ _b = to_string(_1) ]
            >> '='
            >> initializer                      [ _val = create_dm(_a, _b, _1) ]
            >> ';'
        ;

        main = const_member;
    }

    main_rule_type          main;
    main_rule_locals_type   const_member;
    type_name_rule          type_name_;
    data_init_rule          initializer;
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
                [ _val = phx::construct< function_decl::param_type >(_1, to_string(_2)) ];
        param_list %= -(param >> *(',' >> param));

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
        using qi::_2;

        type_alias = tok.using_
            >> (tok.identifier >> '=' >> type_name_)
                    [ _val = phx::construct< type_alias_decl >(to_string(_1), _2) ]
            >> ';'
        ;
    }

    main_rule_type     type_alias;
    type_name_rule     type_name_;
};

//----------------------------------------------------------------------------
//    forward decl
//----------------------------------------------------------------------------
template < typename InputIterator, typename Lexer >
struct forward_decl_grammar : parser_value_grammar< InputIterator, fwd_decl, Lexer > {
    using main_rule_type = parser_value_rule< InputIterator, fwd_decl, Lexer >;

    template < typename TokenDef >
    forward_decl_grammar(TokenDef const& tok)
        : forward_decl_grammar::base_type{fwd}
    {
        namespace qi = ::boost::spirit::qi;
        namespace phx = ::boost::phoenix;
        using qi::_val;
        using qi::_1;
        using qi::_2;

        fwd = ((tok.struct_ | tok.interface | tok.class_ | tok.exception)
            >> tok.identifier
            >> ';')
                    [ _val = phx::construct<fwd_decl>( to_string(_1), to_string(_2) ) ];
    }

    main_rule_type fwd;
};

//----------------------------------------------------------------------------
//    enum decl
//----------------------------------------------------------------------------
struct create_enum_decl_func {
    using result = enum_decl;

    template< typename TokenValue >
    result
    operator()(TokenValue const& tok, bool constrained, enum_decl::enumerator_list const& enumerators) const
    {
        return result{ ::std::string{ tok.begin(), tok.end() }, constrained, enumerators };
    }
};

::boost::phoenix::function< create_enum_decl_func > const create_enum
     = create_enum_decl_func{};

//----------------------------------------------------------------------------
template < typename InputIterator, typename Lexer >
struct enum_grammar : parser_value_grammar< InputIterator, enum_decl, Lexer > {
    using main_rule_type = parser_value_rule< InputIterator, enum_decl, Lexer >;
    using main_rule_locals_type = parser_value_rule<
            InputIterator, enum_decl, Lexer, parser_locals< bool > >;
    using rule_type = parser_rule< InputIterator, Lexer >;
    template < typename T >
    using value_rule_type = parser_value_rule< InputIterator, T, Lexer >;

    template < typename TokenDef >
    enum_grammar(TokenDef const& tok)
        : enum_grammar::base_type{ main }
    {
        namespace qi = ::boost::spirit::qi;
        namespace phx = ::boost::phoenix;
        using qi::char_;
        using qi::_val;
        using qi::_1;
        using qi::_2;
        using qi::_a;
        using qi::eps;

        enum_ = eps                     [ _a = false ]
            >> tok.enum_
            >> -tok.class_              [ _a = true ]
            >> (tok.identifier
            >> '{'
                >> enumerator_list)     [ _val = create_enum( _1, _a, _2 ) ]
            >> '}'
            >> ';';

        enumerator_list %= -(enumerator >> *(',' >> enumerator));

        enumerator = tok.identifier         [ phx::bind(&enumerator_decl::name, _val) = to_string(_1) ]
            >> -('=' >> enumerator_init     [ phx::bind(&enumerator_decl::init, _val) = _1 ])
        ;
        enumerator_init = (tok.dec_literal | tok.oct_literal | tok.hex_literal) [ _val = to_string(_1) ]
            | expression                                                        [ _val = _1 ]
        ;

        unary_operator = char_("~!");
        binary_operator = char_("|&");
        expression = (-unary_operator               [ phx::push_back(_val, _1) ]
                       >> tok.identifier            [ append(_val, _1) ])
                >> *(binary_operator                [ phx::push_back(_val, _1) ]
                        >> -unary_operator          [ phx::push_back(_val, _1) ]
                        >> tok.identifier           [ append(_val, _1) ]);

        main = enum_;
    }

    main_rule_type                                  main;
    main_rule_locals_type                           enum_;
    value_rule_type< enumerator_decl >              enumerator;
    value_rule_type< ::std::string >                enumerator_init;
    value_rule_type< enum_decl::enumerator_list >   enumerator_list;
    value_rule_type< char >                         unary_operator;
    value_rule_type< char >                         binary_operator;
    value_rule_type< ::std::string >                expression;
};

//----------------------------------------------------------------------------
struct create_annotation_ptr_func {
    using result = annotation::annotation_ptr;

    result
    operator()(annotation const& ann) const
    {
        return ::std::make_shared< annotation >(ann);
    }

    template < typename TokenValue >
    result
    operator()(TokenValue const& tok) const
    {
        annotation ann{::std::string{ tok.begin(), tok.end() }};
        return ::std::make_shared< annotation >(::std::move(ann));
    }
};

::boost::phoenix::function< create_annotation_ptr_func > const ann_ptr
     = create_annotation_ptr_func{};

//----------------------------------------------------------------------------
struct create_annotation_func {
    using result = annotation;

    template < typename TokenValue >
    result
    operator()(TokenValue const& tok,
            ::boost::optional< annotation::argument_list > const& args) const
    {
        if (args.is_initialized()) {
            return result { ::std::string{ tok.begin(), tok.end() }, *args };
        }
        return result{ ::std::string{ tok.begin(), tok.end() } };
    }
};

::boost::phoenix::function< create_annotation_func > const create_annotation
     = create_annotation_func{};

//----------------------------------------------------------------------------
template < typename InputIterator, typename Lexer >
struct annotation_grammar : parser_value_grammar< InputIterator, annotation_list, Lexer > {
    using main_rule_type = parser_value_rule< InputIterator, annotation_list, Lexer >;
    template< typename T >
    using value_rule_type = parser_value_rule<InputIterator, T, Lexer>;

    template < typename TokenDef >
    annotation_grammar(TokenDef const& tok)
        : annotation_grammar::base_type(main)
    {
        namespace qi = ::boost::spirit::qi;
        using qi::_val;
        using qi::_1;
        using qi::_2;

        main = tok.annotation_start
            >> annotation_list_ [ _val = _1 ]
            >> tok.annotation_end;

        annotation_list_ %= annotation_ >> *(',' >> annotation_);
        annotation_ = (tok.identifier >> -( '(' >> args >> ')' ))
                [ _val = create_annotation(_1, _2) ];
        args %= -(arg >> *(',' >> arg));
        arg = annotation_ [ _val = ann_ptr(_1) ]
            | (tok.dec_literal | tok.oct_literal | tok.hex_literal | tok.string_literal)
                    [ _val = ann_ptr(_1) ];
    }

    main_rule_type                                  main;
    main_rule_type                                  annotation_list_;
    value_rule_type< annotation >                   annotation_;
    value_rule_type< annotation::argument_list >    args;
    value_rule_type< annotation::annotation_ptr >   arg;
};
}  /* namespace grammar */
}  /* namespace idl */
}  /* namespace wire */


#endif /* WIRE_IDL_GRAMMAR_SCOPE_MEMBER_HPP_ */
