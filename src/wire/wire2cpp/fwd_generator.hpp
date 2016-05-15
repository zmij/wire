/*
 * fwd_generator.hpp
 *
 *  Created on: 16 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_WIRE2CPP_FWD_GENERATOR_HPP_
#define WIRE_WIRE2CPP_FWD_GENERATOR_HPP_

#include <wire/idl/ast.hpp>
#include <wire/idl/preprocess.hpp>
#include <wire/idl/generator.hpp>

#include <wire/wire2cpp/generate_options.hpp>
#include <wire/wire2cpp/cpp_source_stream.hpp>

namespace wire {
namespace idl {
namespace cpp {

class fwd_generator : public ast::generator {
public:
    using include_dir_list = ::std::vector< std::string >;
public:
    fwd_generator(generate_options const&, preprocess_options const&,
            ast::global_namespace_ptr);
    virtual ~fwd_generator(){}

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
    ast::global_namespace_ptr       ns_;
    ast::compilation_unit_ptr       unit_;

    source_stream                   header_;
};

}  /* namespace cpp */
}  /* namespace idl */
}  /* namespace wire */

#endif /* WIRE_WIRE2CPP_FWD_GENERATOR_HPP_ */
