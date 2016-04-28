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

class generator {
public:
    virtual ~generator() {}

    void
    generate_type_decl( type_ptr t );

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
};

}  /* namespace ast */
}  /* namespace idl */
}  /* namespace wire */


#endif /* WIRE_IDL_GENERATOR_HPP_ */
