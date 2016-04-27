/*
 * parser.hpp
 *
 *  Created on: Apr 20, 2016
 *      Author: zmij
 */

#ifndef WIRE_IDL_PARSER_HPP_
#define WIRE_IDL_PARSER_HPP_

#include <wire/idl/ast.hpp>
#include <wire/idl/syntax_error.hpp>
#include <wire/idl/grammar/declarations.hpp>

#include <wire/idl/grammar/idl_file.hpp>
#include <wire/idl/lexer.hpp>

#include <deque>

namespace wire {
namespace idl {
namespace parser {

struct parser_scope;

class parser_state {
public:
    using base_iterator = char const*;

    using optional_type = ::boost::optional< type_name >;
    using optional_type_list = ::boost::optional< grammar::type_name_list >;
public:
    parser_state( ::std::string const& contents );

    ast::namespace_ptr
    get_tree() const
    { return global_; }

    void
    update_location(base_iterator p, source_location const& loc);

    void
    start_namespace(::std::size_t pos, ::std::string const& name);
    void
    start_structure(::std::size_t pos, ::std::string const& name);
    void
    start_interface(::std::size_t pos, ::std::string const& name, optional_type_list const&);
    void
    start_class(::std::size_t pos, ::std::string const& name, optional_type_list const&);
    void
    start_exception(::std::size_t pos, ::std::string const& name, optional_type const&);
    void
    end_scope(::std::size_t pos);

    void
    declare_enum(::std::size_t pos, grammar::enum_decl const& decl);
    void
    add_type_alias(::std::size_t pos, grammar::type_alias_decl const& decl);
    void
    forward_declare(::std::size_t pos, grammar::fwd_decl const& fwd);
    void
    add_constant(::std::size_t pos, grammar::data_member_decl const& decl);
    void
    add_data_member(::std::size_t pos, grammar::data_member_decl const& decl);
    void
    add_func_member(::std::size_t pos, grammar::function_decl const& decl);

    void
    add_annotations(::std::size_t pos, grammar::annotation_list const&);

    source_location
    get_location(::std::size_t pos) const;
private:
    parser_scope&
    current();
    ast::scope_ptr
    ast_scope();
private:
    using scope_ptr = ::std::shared_ptr<parser_scope>;
    using scope_stack = ::std::deque<scope_ptr>;
    using location_jumps = ::std::map< ::std::size_t, source_location >;

    base_iterator               stream_begin;
    location_jumps              loc_jumps;
    ast::namespace_ptr          global_;
    scope_stack                 scopes_;
    grammar::annotation_list    current_annotations_;
};

//----------------------------------------------------------------------------
struct parser_scope {
    using scope_ptr = ::std::shared_ptr<parser_scope>;

    explicit
    parser_scope(ast::scope_ptr sc)
        : scope_(sc)
    {
    }
    virtual ~parser_scope() {}

    ast::scope_ptr
    scope()
    {
        return scope_;
    }
    template < typename T >
    ast::shared_entity< T >
    scope()
    {
        return ast::dynamic_entity_cast< T >(scope_);
    }

    //@{
    /** @name Functions where behavior differs depending on scope */
    scope_ptr
    start_namespace(::std::size_t pos, ::std::string const& name)
    {
        return start_namespace_impl(pos, name);
    }
    ast::variable_ptr
    add_data_member(::std::size_t pos, grammar::data_member_decl const& decl)
    {
        return add_data_member_impl(pos, decl);
    }
    ast::function_ptr
    add_func_member(::std::size_t pos, grammar::function_decl const& decl)
    {
        return add_func_member_impl(pos, decl);
    }
    //@}
private:
    virtual scope_ptr
    start_namespace_impl(::std::size_t pos, ::std::string const& name);

    virtual ast::variable_ptr
    add_data_member_impl(::std::size_t pos, grammar::data_member_decl const& decl);

    virtual ast::function_ptr
    add_func_member_impl(::std::size_t pos, grammar::function_decl const& decl);
private:
    ast::scope_ptr scope_;
};

//----------------------------------------------------------------------------
struct namespace_scope : parser_scope {
    explicit
    namespace_scope(ast::namespace_ptr ns)
        : parser_scope(ns) {}
private:
    scope_ptr
    start_namespace_impl(::std::size_t pos, ::std::string const& name) override;
};

//----------------------------------------------------------------------------
struct structure_scope : virtual parser_scope {
    explicit
    structure_scope(ast::structure_ptr st)
        : parser_scope(st) {}
private:
    ast::variable_ptr
    add_data_member_impl(::std::size_t pos, grammar::data_member_decl const& decl) override;
};

//----------------------------------------------------------------------------
struct interface_scope : virtual parser_scope {
    explicit
    interface_scope(ast::interface_ptr iface)
        : parser_scope(iface) {}
private:
    ast::function_ptr
    add_func_member_impl(::std::size_t pos, grammar::function_decl const& decl) override;
};

//----------------------------------------------------------------------------
struct class_scope : structure_scope, interface_scope {
    explicit
    class_scope(ast::class_ptr cl)
        : parser_scope(cl), structure_scope(cl), interface_scope(cl) {}
};
//----------------------------------------------------------------------------
struct exception_scope : structure_scope {
    explicit
    exception_scope(ast::exception_ptr ex)
        : parser_scope(ex), structure_scope(ex) {}
};

//----------------------------------------------------------------------------
class parser {
public:
    using base_iterator     = char const*;
    using attribs           = ::boost::mpl::vector0<>;
    using token_type        = ::boost::spirit::lex::lexertl::token<base_iterator, attribs, ::boost::mpl::true_>;
    using lexer_type        = ::boost::spirit::lex::lexertl::lexer<token_type>;
    using tokens_type       = lexer::wire_tokens< lexer_type >;
    using token_iterator    = tokens_type::iterator_type;
    using grammar_type      = grammar::idl_file_grammar< token_iterator, tokens_type::lexer_def >;
public:
    parser( ::std::string const& contents );

    void
    parse();
private:
    ::std::string const&  contents;
};

}  // namespace parser
}  // namespace idl
}  // namespace wire

#endif /* WIRE_IDL_PARSER_HPP_ */
