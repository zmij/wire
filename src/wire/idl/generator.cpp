/*
 * generator.cpp
 *
 *  Created on: Apr 28, 2016
 *      Author: zmij
 */

#include <wire/idl/generator.hpp>
#include <wire/idl/syntax_error.hpp>

namespace wire {
namespace idl {
namespace ast {

void
generator::generate_type_decl(type_ptr t)
{
    if ( auto cl = dynamic_entity_cast< class_ >(t) ) {

    } else if (auto iface = dynamic_entity_cast< interface >(t)) {

    } else if (auto exc = dynamic_entity_cast< exception >(t)) {

    } else if (auto st = dynamic_entity_cast< structure >(t)) {
        generate_struct(st);
    } else if (auto enum_ = dynamic_entity_cast< enumeration >(t)) {
        generate_enum(enum_);
    } else if (auto ref = dynamic_entity_cast< reference >(t)) {

    } else if (auto ta = dynamic_entity_cast< type_alias >(t)) {
        generate_type_alias(ta);
    } else if (auto fwd = dynamic_entity_cast< forward_declaration >(t)) {
        generate_forward_decl(fwd);
    }
}

}  /* namespace ast */
}  /* namespace idl */
}  /* namespace wire */


