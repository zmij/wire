/*
 * declarations.hpp
 *
 *  Created on: Apr 26, 2016
 *      Author: zmij
 */

#ifndef WIRE_IDL_GRAMMAR_DECLARATIONS_HPP_
#define WIRE_IDL_GRAMMAR_DECLARATIONS_HPP_

#include <wire/idl/type_name.hpp>
#include <wire/util/murmur_hash.hpp>
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

struct annotation_location {
    using iterator      = annotation_list::const_iterator;
    using pointer       = iterator::pointer;
    using reference     = iterator::reference;

    iterator    iter;
    bool        found;

    operator bool() const { return found; }
    bool
    operator !() const { return !found; }

    reference
    operator * () const { return *iter; }
    pointer
    operator -> () const { return iter.operator ->(); }
};

annotation_list::const_iterator
find(annotation_list const&, ::std::string const& name);

}  /* namespace grammar */
}  /* namespace idl */
namespace hash {

template <>
struct murmur_hash_calc< idl::grammar::data_initializer, 32 > {
    using base_type     = detail::hash_base< idl::grammar::data_initializer const&, 32 >;
    using result_type   = typename base_type::result_type;
    using argument_type = typename base_type::argument_type;

    result_type
    operator()(argument_type arg) const noexcept;
};

template <>
struct murmur_hash_calc< idl::grammar::data_initializer, 64 > {
    using base_type     = detail::hash_base< idl::grammar::data_initializer const&, 64 >;
    using result_type   = typename base_type::result_type;
    using argument_type = typename base_type::argument_type;

    result_type
    operator()(argument_type arg) const noexcept;
};

}  /* namespace hash */
}  /* namespace wire */


#endif /* WIRE_IDL_GRAMMAR_DECLARATIONS_HPP_ */
