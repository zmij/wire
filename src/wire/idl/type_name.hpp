/*
 * type_name.hpp
 *
 *  Created on: 25 апр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_IDL_TYPE_NAME_HPP_
#define WIRE_IDL_TYPE_NAME_HPP_

#include <wire/idl/qname.hpp>
#include <memory>
#include <iosfwd>
#include <boost/variant.hpp>

namespace wire {
namespace idl {

struct type_name {
    using type_name_ptr = ::std::shared_ptr< type_name >;
    using template_param = ::boost::variant< type_name_ptr, ::std::string >;
    using template_params = ::std::vector< template_param >;

    ast::qname         name;
    template_params    params;
    bool               is_reference;
};

std::ostream&
operator << (std::ostream& os, type_name const& val);

}  /* namespace idl */
}  /* namespace wire */


#endif /* WIRE_IDL_TYPE_NAME_HPP_ */
