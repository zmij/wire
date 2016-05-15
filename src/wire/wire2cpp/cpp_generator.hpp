/*
 * cpp_generator.hpp
 *
 *  Created on: 27 апр. 2016 г.
 *      Author: sergey.fedorov
 */

/**
 * @page wire2cpp Wire to C++ mapping
 *
 *  ## Annotations
 *
 *  ### cpp_container
 *
 *  Applicable to sequence and dictionary
 *
 */

#ifndef WIRE_WIRE2CPP_CPP_GENERATOR_HPP_
#define WIRE_WIRE2CPP_CPP_GENERATOR_HPP_

#include <wire/idl/ast.hpp>
#include <wire/idl/preprocess.hpp>
#include <wire/idl/generator.hpp>
#include <wire/idl/grammar/declarations.hpp>

#include <wire/wire2cpp/generate_options.hpp>
#include <wire/wire2cpp/cpp_source_stream.hpp>

#include <fstream>
#include <deque>
#include <functional>

namespace wire {
namespace idl {
namespace cpp {

namespace annotations {

::std::string const CPP_CONTAINER = "cpp_container";
::std::string const GENERATE_CMP = "cpp_cmp";
::std::string const GENERATE_IO = "cpp_io";

}  /* namespace annotations */

struct relative_name {
    qname_search    current;
    qname           qn;
};

struct type_name_rules {
    static grammar::annotation_list const empty_annotations;

    qname_search                    current;
    ast::type_const_ptr             type;
    grammar::annotation_list const& annotations = empty_annotations;
    bool                            is_arg = false;

    type_name_rules( qname_search const& scope, ast::type_const_ptr t,
            grammar::annotation_list const& al = empty_annotations,
            bool argument = false)
        : current{scope}, type{t}, annotations{al}, is_arg{argument}
    {}
    type_name_rules( qname const& scope, ast::type_const_ptr t,
            grammar::annotation_list const& al = empty_annotations,
            bool argument = false)
        : current{scope.search()}, type{t}, annotations{al}, is_arg{argument}
    {}

    type_name_rules(qname_search const scope, ast::type_const_ptr t, bool arg)
        : current{ scope }, type{t}, is_arg{arg}
    {}
    type_name_rules(qname const& scope, ast::type_const_ptr t, bool arg)
        : current{ scope.search() }, type{t}, is_arg{arg}
    {}
};

class generator : public ast::generator {
public:
    using include_dir_list = ::std::vector< std::string >;
public:
    generator(generate_options const&, preprocess_options const&,
            ast::global_namespace_ptr);
    virtual ~generator();

    void
    finish_compilation_unit(ast::compilation_unit const& u) override;

    void
    generate_forward_decl( ast::forward_declaration_ptr fwd ) override;

    void
    generate_constant( ast::constant_ptr c) override;

    void
    generate_type_alias( ast::type_alias_ptr ta ) override;

    void
    generate_enum(ast::enumeration_ptr enum_) override;

    void
    generate_struct(ast::structure_ptr struct_) override;

    void
    generate_exception(ast::exception_ptr exc) override;

    void
    generate_interface(ast::interface_ptr iface) override;

    void
    generate_class(ast::class_ptr class_) override;
private:
    void
    adjust_scope(ast::entity_ptr en);
    void
    pop_scope();

    ::std::string
    constant_prefix( qname const& ) const;

    type_name_rules
    arg_type(ast::type_const_ptr t,
            grammar::annotation_list const& al = grammar::annotation_list{})
    { return { current_scope_.search(), t, al, true }; }

    type_name_rules
    type_name( ast::type_const_ptr t,
            grammar::annotation_list const& al = type_name_rules::empty_annotations )
    { return { current_scope_.search(), t, al }; };

    qname
    rel_name(ast::entity_const_ptr en)
    { return en->get_qualified_name(); }

    source_stream&
    write_data_member(source_stream&, ast::variable_ptr var);

    void
    generate_read_write(source_stream& s, ast::structure_ptr struct_);
    void
    generate_member_read_write( ast::structure_ptr struct_,
            ast::structure_const_ptr parent, bool ovrde = true );
    void
    generate_comparison(source_stream& s, ast::structure_ptr struct_);
    void
    generate_io(ast::structure_ptr struct_);

    ::std::string
    generate_type_id_funcs(ast::entity_ptr elem);
    void
    generate_wire_functions(ast::interface_ptr iface);
    void
    generate_dispatch_function_member(ast::function_ptr func);
    void
    generate_invocation_function_member(ast::function_ptr func);

    void
    generate_dispatch_interface(ast::interface_ptr iface);
    void
    generate_proxy_interface(ast::interface_ptr iface);
private:
    using free_function = ::std::function< void() >;
private:
    generate_options                options_;
    ast::global_namespace_ptr       ns_;
    ast::compilation_unit_ptr       unit_;

    source_stream                   header_;
    source_stream                   source_;

    qname                           current_scope_;

    ::std::deque<ast::type_ptr>     scope_stack_;
    ::std::vector< free_function >  free_functions_;
};

}  /* namespace cpp */
}  /* namespace idl */
}  /* namespace wire */


#endif /* WIRE_WIRE2CPP_CPP_GENERATOR_HPP_ */
