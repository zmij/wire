/*
 * generator.hpp
 *
 *  Created on: Apr 28, 2016
 *      Author: zmij
 */

#ifndef WIRE_IDL_GENERATOR_HPP_
#define WIRE_IDL_GENERATOR_HPP_

#include <wire/idl/ast.hpp>

namespace wire {
namespace idl {
namespace ast {

::std::string const ARRAY       = "array";
::std::string const SEQUENCE    = "sequence";
::std::string const DICTONARY   = "dictionary";

::std::string const STRING      = "string";
::std::string const UUID        = "uuid";

namespace annotations {

::std::string const WEAK = "weak";

::std::string const SYNC = "sync";

}  /* namespace annotations */

class generator {
public:
    virtual ~generator() {}

    void
    generate_type_decl( type_ptr t );

    virtual void
    start_compilation_unit(compilation_unit const& u) {}
    virtual void
    finish_compilation_unit(compilation_unit const& u) {}

    virtual void
    generate_constant( constant_ptr c ) = 0;

    virtual void
    generate_type_alias( type_alias_ptr ta ) {}

    virtual void
    generate_forward_decl( forward_declaration_ptr fwd ) {}

    virtual void
    generate_enum( enumeration_ptr enum_ ) = 0;

    virtual void
    generate_struct( structure_ptr struct_ ) = 0;

    virtual void
    generate_exception( exception_ptr ex ) = 0;

    virtual void
    generate_interface(interface_ptr iface) = 0;

    virtual void
    generate_class(ast::class_ptr class_) = 0;
};

}  /* namespace ast */
}  /* namespace idl */
}  /* namespace wire */


#endif /* WIRE_IDL_GENERATOR_HPP_ */
