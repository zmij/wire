/*
 * ast_fwd.hpp
 *
 *  Created on: Jun 5, 2017
 *      Author: zmij
 */

#ifndef WIRE_IDL_AST_FWD_HPP_
#define WIRE_IDL_AST_FWD_HPP_

#include <memory>
#include <type_traits>
#include <functional>
#include <set>
#include <map>

namespace wire {
namespace idl {
namespace ast {

class entity;
using entity_ptr                    = ::std::shared_ptr< entity >;
using entity_const_ptr              = ::std::shared_ptr< entity const >;

using entity_set                    = ::std::set< entity_ptr >;
using entity_const_set              = ::std::set< entity_const_ptr >;
using entity_predicate              = ::std::function< bool( entity_const_ptr ) >;

template < typename T >
using shared_entity                 = ::std::shared_ptr< typename ::std::decay<T>::type >;
template < typename T >
using const_shared_entity           = ::std::shared_ptr< typename ::std::decay<T>::type const >;

template < typename T >
using weak_entity                   = ::std::weak_ptr< typename ::std::decay<T>::type >;
template < typename T >
using const_weak_entity             = ::std::weak_ptr< typename ::std::decay<T>::type const >;

class scope;
using scope_ptr                     = shared_entity<scope>;
using scope_const_ptr               = const_shared_entity<scope>;
using scope_weak_ptr                = weak_entity<scope>;

class namespace_;
using namespace_ptr                 = ::std::shared_ptr< namespace_ >;
using namespace_list                = ::std::map< ::std::string, namespace_ptr >;

class global_namespace;
using global_namespace_ptr          = ::std::shared_ptr< global_namespace >;

class structure;
using structure_ptr                 = shared_entity< structure >;
using structure_const_ptr           = const_shared_entity< structure >;

class interface;
using interface_ptr                 = shared_entity< interface >;
using interface_list                = ::std::vector< interface_ptr >;

class class_;

class exception;
using exception_ptr                 = shared_entity<exception>;
using exception_const_ptr           = const_shared_entity<exception>;
using exception_list                = ::std::vector< exception_ptr >;

struct compilation_unit;
using compilation_unit_ptr          = ::std::shared_ptr< compilation_unit >;
using compilation_unit_const_ptr    = ::std::shared_ptr< compilation_unit const >;
using compilation_unit_weak_ptr     = ::std::weak_ptr<compilation_unit>;
using compilation_unit_set          = ::std::set< compilation_unit_ptr >;
using compilation_unit_const_set    = ::std::set< compilation_unit_const_ptr >;

} /* namespace ast */
} /* namespace idl */
} /* namespace wire */



#endif /* WIRE_IDL_AST_FWD_HPP_ */
