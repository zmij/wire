/*
 * mapped_type.hpp
 *
 *  Created on: 15 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_WIRE2LUA_MAPPED_TYPE_HPP_
#define WIRE_WIRE2LUA_MAPPED_TYPE_HPP_

#include <string>
#include <vector>
#include <map>

#include <wire/idl/ast.hpp>

namespace wire {
namespace idl {
namespace lua {

struct mapped_type {
    static grammar::annotation_list const empty_annotations;

    ast::type_const_ptr             type;
    grammar::annotation_list const& annotations = empty_annotations;

    mapped_type(ast::type_const_ptr t,
            grammar::annotation_list const& al = empty_annotations)
        : type{t}, annotations{al} {}
};

inline mapped_type
arg_type(ast::type_const_ptr t)
{
    return { t };
}

inline mapped_type
arg_type(ast::type_const_ptr t, grammar::annotation_list const& al)
{
    return { t, al };
}

}  /* namespace lua */
}  /* namespace idl */
}  /* namespace wire */



#endif /* WIRE_WIRE2LUA_MAPPED_TYPE_HPP_ */
