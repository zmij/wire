/*
 * type_name.hpp
 *
 *  Created on: 25 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_IDL_GRAMMAR_TYPE_NAME_HPP_
#define WIRE_IDL_GRAMMAR_TYPE_NAME_HPP_

#include <wire/idl/source_location.hpp>
#include <wire/idl/qname.hpp>
#include <wire/idl/type_name.hpp>
#include <wire/idl/syntax_error.hpp>

#include <wire/idl/grammar/common.hpp>

namespace wire {
namespace idl {
namespace grammar {

//----------------------------------------------------------------------------
struct qname_add_component_func {
    using result = void;

    template <typename Token>
    void
    operator()(qname& qn, Token const& tok) const
    {
        qn.components.emplace_back(tok.begin(), tok.end());
    }
};

::boost::phoenix::function< qname_add_component_func > const qname_add_component = qname_add_component_func{};

//----------------------------------------------------------------------------
struct qname_reset_func {
    using result = void;
    void
    operator()(qname& qn) const
    {
        qn.fully = false;
        qn.components.clear();
    }
};

::boost::phoenix::function< qname_reset_func > const qname_reset = qname_reset_func{};

//----------------------------------------------------------------------------
template <typename InputIterator, typename Lexer >
struct qname_grammar : parser_value_grammar< InputIterator, qname, Lexer > {
    using main_rule_type = parser_value_rule< InputIterator, qname, Lexer >;
    using rule_type = parser_rule< InputIterator, Lexer >;

    template < typename TokenDef >
    qname_grammar(TokenDef const& tok)
        : qname_grammar::base_type{main}
    {
        namespace qi = ::boost::spirit::qi;
        namespace phx = ::boost::phoenix;
        using phx::ref;
        using qi::eps;
        using qi::_val;
        using qi::_1;
        using qi::_2;

        main = eps[ qname_reset(_val) ]
            >> -tok.scope_resolution[ phx::bind(&qname::fully, _val) = true ]
                    >> tok.identifier[ qname_add_component(_val, _1) ]
            >> *(tok.scope_resolution >> tok.identifier[ qname_add_component(_val, _1) ]);
    }

    main_rule_type        main;
};

//----------------------------------------------------------------------------
struct create_template_param_func {
    using result = type_name::template_param;

    result
    operator()(type_name const& tn) const
    {
        return result{ ::std::make_shared<type_name>(tn) };
    }

    template < typename TokenValue >
    result
    operator()(TokenValue const& tok) const
    {
        return result{ ::std::string{ tok.begin(), tok.end() } };
    }
};

::boost::phoenix::function< create_template_param_func > const tmpl_param
      = create_template_param_func{};

//----------------------------------------------------------------------------
struct add_template_param {
    using result = void;

    void
    operator()(type_name::template_params& params, type_name::template_param const& p) const
    {
        params.push_back(p);
    }
};

::boost::phoenix::function< add_template_param > const add_t_param
      = add_template_param{};

//----------------------------------------------------------------------------
template <typename InputIterator, typename Lexer >
struct type_name_grammar : parser_value_grammar< InputIterator, type_name, Lexer > {
    using main_rule_type  = parser_value_rule< InputIterator, type_name, Lexer >;
    using qname_rule_type = qname_grammar< InputIterator, Lexer >;
    using rule_type = parser_rule< InputIterator, Lexer >;
    template < typename T >
    using value_rule_type = parser_value_rule< InputIterator, T, Lexer >;

    template< typename TokenDef >
    type_name_grammar(TokenDef const& tok)
        : type_name_grammar::base_type{type_name_},
          qname_{tok}
    {
        namespace qi = ::boost::spirit::qi;
        namespace phx = ::boost::phoenix;
        using phx::ref;
        using qi::eps;
        using qi::_val;
        using qi::_1;
        using qi::_2;
        using qi::char_;

        template_param = type_name_[ _val = tmpl_param(_1) ]
            | (tok.dec_literal | tok.oct_literal | tok.hex_literal)[ _val = tmpl_param(_1) ];

        template_params %=
                template_param
                >> *(',' >> template_param)
        ;

        type_name_ =
               qname_                            [ phx::bind(&type_name::name, _val) = _1 ]
            >> -('<'
                >> template_params               [ phx::bind(&type_name::params, _val) = _1 ]
                >> '>')
            >> -char_("*")                       [ phx::bind(&type_name::is_reference, _val) = true ]
        ;
    }

    main_rule_type                                 type_name_;
    qname_rule_type                                qname_;
    value_rule_type< type_name::template_param >   template_param;
    value_rule_type< type_name::template_params >  template_params;
};

}  /* namespace grammar */
}  /* namespace idl */
}  /* namespace wire */



#endif /* WIRE_IDL_GRAMMAR_TYPE_NAME_HPP_ */
