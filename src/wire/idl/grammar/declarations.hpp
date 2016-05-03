/*
 * declarations.hpp
 *
 *  Created on: Apr 26, 2016
 *      Author: zmij
 */

#ifndef WIRE_IDL_GRAMMAR_DECLARATIONS_HPP_
#define WIRE_IDL_GRAMMAR_DECLARATIONS_HPP_

#include <wire/idl/type_name.hpp>
#include <boost/variant.hpp>
#include <boost/optional.hpp>

namespace wire {
namespace idl {
namespace grammar {

using type_alias_decl  = ::std::pair< ::std::string, type_name >; // TODO declaration location
using fwd_decl = ::std::pair< ::std::string, ::std::string >;
using type_name_list = ::std::vector< type_name >;

//----------------------------------------------------------------------------
struct data_initializer {
    using initializer_ptr   = ::std::shared_ptr< data_initializer >;
    using initializer_list  = ::std::vector< initializer_ptr >;
    using initializer_value = ::boost::variant< ::std::string, initializer_list >;

    initializer_value       value;
};

//----------------------------------------------------------------------------
struct data_member_decl {
    using optional_init = ::boost::optional< data_initializer >;

    type_name               type;
    ::std::string           name;
    optional_init           init;
};

//----------------------------------------------------------------------------
struct function_decl {
    using param_type = ::std::pair< type_name, ::std::string >;
    using params_list = ::std::vector< param_type >;
    using type_list = ::std::vector< type_name >;

    type_name               return_type;
    ::std::string           name;
    params_list             params;
    bool                    const_qualified;
    type_list               throw_spec;
};

//----------------------------------------------------------------------------
struct enumerator_decl {
    using optional_initializer = ::boost::optional< ::std::string >;
    ::std::string           name;
    optional_initializer    init;
};

//----------------------------------------------------------------------------
struct enum_decl {
    using enumerator_list = ::std::vector< enumerator_decl >;
    ::std::string           name;
    bool                    constrained;
    enumerator_list         enumerators;
};

//----------------------------------------------------------------------------
struct annotation {
    using annotation_ptr = ::std::shared_ptr< annotation >;
    using argument_list = ::std::vector< annotation_ptr >;

    ::std::string           name;
    argument_list           arguments;
};

using annotation_list = ::std::vector< annotation >;

annotation_list::const_iterator
find(annotation_list const&, ::std::string const& name);

}  /* namespace grammar */
}  /* namespace idl */
}  /* namespace wire */


#endif /* WIRE_IDL_GRAMMAR_DECLARATIONS_HPP_ */
