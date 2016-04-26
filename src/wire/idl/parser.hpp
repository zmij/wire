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

#include <deque>

namespace wire {
namespace idl {
namespace parser {

class parser_scope;

class parser_state {
public:
    using base_iterator = char const*;

    using optional_type = ::boost::optional< type_name >;
    using optional_type_list = ::boost::optional< grammar::type_name_list >;
public:
    parser_state( ::std::string const& contents );

    void
    update_location(base_iterator p, source_location const& loc);

    void
    declare_enum(grammar::enum_decl const& decl);
    void
    add_type_alias(grammar::type_alias_decl const& decl);
    void
    forward_declare(grammar::fwd_decl const& fwd);
    void
    add_constant(grammar::data_member_decl const& decl);
    void
    add_data_member(grammar::data_member_decl const& decl);
    void
    add_func_member(grammar::function_decl const& decl);
    void
    start_namespace(::std::string const& name);
    void
    start_structure(::std::string const& name);
    void
    start_interface(::std::string const& name, optional_type_list const&);
    void
    start_class(::std::string const& name, optional_type_list const&);
    void
    start_exception(::std::string const& name, optional_type const&);
    void
    end_scope();

    void
    add_annotations(grammar::annotation_list const&);
private:
    using scope_ptr = ::std::shared_ptr<parser_scope>;
    using scope_stack = ::std::deque<scope_ptr>;
    using location_jumps = ::std::map< ::std::size_t, source_location >;

    base_iterator   stream_begin;
    location_jumps  loc_jumps;
    scope_stack     scopes_;
};

struct parser_scope {
public:
    parser_scope(ast::scope_ptr sc);
protected:

};

struct namespace_scope {

};

}  // namespace parser
}  // namespace idl
}  // namespace wire

#endif /* WIRE_IDL_PARSER_HPP_ */
