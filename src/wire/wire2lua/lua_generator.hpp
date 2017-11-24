/*
 * lua_generator.hpp
 *
 *  Created on: Nov 23, 2017
 *      Author: zmij
 */

#ifndef WIRE_WIRE2LUA_LUA_GENERATOR_HPP_
#define WIRE_WIRE2LUA_LUA_GENERATOR_HPP_

#include <wire/idl/ast.hpp>
#include <wire/idl/preprocess.hpp>
#include <wire/idl/generator.hpp>
#include <wire/idl/grammar/declarations.hpp>

#include <wire/wire2lua/generate_options.hpp>
#include <wire/wire2lua/lua_source_stream.hpp>

namespace wire {
namespace idl {
namespace lua {

class generator: public ast::generator {
public:
    generator(ast::global_namespace_ptr, source_stream&);
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
    write_fields(ast::structure_ptr struct_);

    void
    write_functions(ast::interface_ptr iface);
private:
    source_stream&                  out_;
    ast::global_namespace_ptr       ns_;
    ast::compilation_unit_ptr       unit_;
};

} /* namespace lua */
} /* namespace idl */
} /* namespace wire */

#endif /* WIRE_WIRE2LUA_LUA_GENERATOR_HPP_ */
