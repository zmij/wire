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

enum class date_and_time {
    std,
    boost
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

    static date_and_time
    datetime_type;
private:
    void
    adjust_namespace(ast::entity_ptr en);

    ::std::string
    constant_prefix( ast::qname const& ) const;

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

    void
    generate_enum_traits(ast::enumeration_const_ptr enum_);
private:
    generate_options                options_;
    ast::global_namespace_ptr       ns_;
    ast::compilation_unit_ptr       unit_;

    source_stream                   header_;
    source_stream                   source_;
};

::std::ostream&
operator <<(::std::ostream& os, date_and_time val);
::std::istream&
operator >>(::std::istream& is, date_and_time& val);

}  /* namespace cpp */
}  /* namespace idl */
}  /* namespace wire */


#endif /* WIRE_WIRE2CPP_CPP_GENERATOR_HPP_ */
