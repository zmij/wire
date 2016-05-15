/*
 * mapped_type.hpp
 *
 *  Created on: 15 мая 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_WIRE2CPP_MAPPED_TYPE_HPP_
#define WIRE_WIRE2CPP_MAPPED_TYPE_HPP_

#include <string>
#include <vector>
#include <map>

#include <wire/idl/ast.hpp>

namespace wire {
namespace idl {
namespace cpp {

struct type_mapping {
    ::std::string                     type_name;
    ::std::vector<::std::string>      headers;
};

using wire_type_map = ::std::map< ::std::string, type_mapping >;

extern wire_type_map const wire_to_cpp;

struct mapped_type {
    static grammar::annotation_list const empty_annotations;

    ast::type_const_ptr             type;
    grammar::annotation_list const& annotations = empty_annotations;
    bool                            is_arg      = false;

    mapped_type(ast::type_const_ptr t,
            grammar::annotation_list const& al = empty_annotations,
            bool is_arg_ = false)
        : type{t}, annotations{al}, is_arg{is_arg_} {}
    mapped_type(ast::type_const_ptr t, bool is_arg_)
        : type{t}, is_arg{is_arg_} {}
};

inline mapped_type
arg_type(ast::type_const_ptr t)
{
    return { t, true };
}

inline mapped_type
arg_type(ast::type_const_ptr t, grammar::annotation_list const& al)
{
    return { t, al, true };
}

}  /* namespace cpp */
}  /* namespace idl */
}  /* namespace wire */



#endif /* WIRE_WIRE2CPP_MAPPED_TYPE_HPP_ */
