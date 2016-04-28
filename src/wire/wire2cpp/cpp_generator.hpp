/*
 * cpp_generator.hpp
 *
 *  Created on: 27 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_WIRE2CPP_CPP_GENERATOR_HPP_
#define WIRE_WIRE2CPP_CPP_GENERATOR_HPP_

#include <wire/idl/ast.hpp>

#include <fstream>

namespace wire {
namespace idl {
namespace cpp {

struct generate_options {
    ::std::string    header_include_dir;

    ::std::string    header_output_dir;
    ::std::string    source_output_dir;
};

class generator {
public:
    using include_dir_list = ::std::vector< std::string >;
public:
    generator(generate_options const&, ast::global_namespace_ptr);
    ~generator();

    void
    write_dependencies();

    void
    generate_enum(ast::enumeration_ptr enum_);

    void
    generate_struct(ast::structure_ptr struct_);

    void
    generate_interface(ast::interface_ptr iface);

    void
    generate_class(ast::class_ptr class_);

    void
    generate_exception(ast::exception_ptr ex);
private:
    ast::global_namespace_ptr ns_;
    ast::compilation_unit_ptr unit_;
    ::std::ofstream header_;
    ::std::ofstream source_;

    ::std::string header_guard_;
};

}  /* namespace cpp */
}  /* namespace idl */
}  /* namespace wire */


#endif /* WIRE_WIRE2CPP_CPP_GENERATOR_HPP_ */
