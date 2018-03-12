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

type_mapping const&
wire_to_cpp(::std::string const&);

struct mapped_type {
    static grammar::annotation_list const empty_annotations;

    ast::type_const_ptr             type;
    grammar::annotation_list const& annotations         = empty_annotations;
    bool                            is_arg              = false;
    bool                            movable     = false;

    mapped_type(ast::type_const_ptr t,
            grammar::annotation_list const& al = empty_annotations,
            bool is_arg_        = false,
            bool is_movable     = false)
        : type{t}, annotations{al}, is_arg{is_arg_}, movable{ is_movable } {}
    mapped_type(ast::type_const_ptr t, bool is_arg_, bool is_movable = false)
        : type{t}, is_arg{is_arg_}, movable{is_movable} {}
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

inline mapped_type
movable_arg_type(ast::type_const_ptr t)
{
    return { t, false, true };
}

inline mapped_type
movable_arg_type(ast::type_const_ptr t, grammar::annotation_list const& al)
{
    return { t, al, false, true };
}

struct invoke_param {
    ast::function::function_param const& param;
    invoke_param( ast::function::function_param const& p )
        : param{ p } {}
};

struct return_value {
    ast::function::function_param value;
    return_value( ast::function::function_param const& p )
        : value{ p } {}
    return_value(ast::type_ptr t, ::std::string const& name)
        : value{ ::std::make_pair(t, name) } {}
};

}  /* namespace cpp */
}  /* namespace idl */
}  /* namespace wire */



#endif /* WIRE_WIRE2CPP_MAPPED_TYPE_HPP_ */
