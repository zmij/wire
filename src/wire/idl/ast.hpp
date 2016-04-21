/*
 * ast.hpp
 *
 *  Created on: 25 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_IDL_AST_HPP_
#define WIRE_IDL_AST_HPP_

/**
 * @page idl_types Wire IDL Types
 *
 * @ref cpp_mapping [Mapping to C++ types]
 *
 * ## Built-in data types
 * ### Boolean
 *
 * ### Numeric types
 *
 * #### Integral types
 * #### Floating-point types
 *
 * ### Character type
 *
 * ### String
 *
 */


#include <memory>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <algorithm>

#include <boost/optional.hpp>

#include <wire/idl/qname.hpp>
#include <wire/idl/source_location.hpp>

namespace wire {
namespace idl {
namespace ast {

class entity;
using entity_ptr = ::std::shared_ptr< entity >;

template < typename T >
using shared_entity = ::std::shared_ptr< typename ::std::decay<T>::type >;
template < typename T >
using const_shared_entity = ::std::shared_ptr< typename ::std::decay<T>::type const >;

template < typename T >
using weak_entity = ::std::weak_ptr< typename ::std::decay<T>::type >;
template < typename T >
using const_weak_entity = ::std::weak_ptr< typename ::std::decay<T>::type const >;

class scope;
using scope_ptr = shared_entity<scope>;
using scope_weak_ptr = weak_entity<scope>;

class namespace_;
using namespace_ptr = ::std::shared_ptr< namespace_ >;
using namespace_list = ::std::map< ::std::string, namespace_ptr >;

class structure;
using structure_ptr = shared_entity< structure >;

class interface;
using interface_ptr = shared_entity< interface >;
using interface_list = ::std::vector< interface_ptr >;

struct literal_base {};
template < typename T >
struct literal : literal_base {
    using value_type = typename ::std::decay<T>::type;
    value_type value = T{};

    literal() {}
    literal(T const& v) : value{v} {}
};

template < typename T >
std::ostream&
operator << (std::ostream& os, literal<T> const& val)
{
    std::ostream::sentry s (os);
    if (s) {
        s << val.value;
    }
    return os;
}

using integral_literal = literal< ::std::int64_t >;
using string_literal = literal< ::std::string >;

//----------------------------------------------------------------------------
template < typename T, typename U >
shared_entity< T >
dynamic_entity_cast(shared_entity< U > const& e);
//----------------------------------------------------------------------------
/**
 * Base AST entity class
 */
class entity : public ::std::enable_shared_from_this<entity> {
public:
    virtual ~entity() {}

    /**
     * Get name of the entity
     * @return
     */
    ::std::string const&
    name() const;

    /**
     * Get pointer to surrounding scope
     * @return
     */
    virtual scope_ptr
    owner() const
    { return owner_.lock(); }

    /**
     * Get a fully qualified name for the entity
     * @return
     */
    virtual qname
    get_qualified_name() const;

    /**
     * Functor for comparing entities by name
     */
    struct name_compare {
        //@{
        /** @name Comparison operations */
        bool
        operator () (entity const& lhs, entity const& rhs) const
        {
            return lhs.name() < rhs.name();
        }

        bool
        operator () (entity_ptr const& lhs, entity_ptr const& rhs) const
        {
            return lhs->name() < rhs->name();
        }
        //@}
    };
protected:
    /**
     * Constructor for the global namespace only.
     */
    entity() : owner_{}, name_{} {}
    /**
     * Constructor for built-in types
     * @param name
     */
    explicit
    entity(::std::string const& name);

    /**
     * Create an entity in the given scope
     * @param parent
     * @param name
     */
    entity(scope_ptr parent, ::std::string const& name);

    /**
     * @return Shared pointer to this cast to a specific type
     */
    template < typename T >
    shared_entity< T >
    shared_this()
    {
        static_assert(::std::is_base_of<entity, T>::value,
                "Cannot cast to non-entity");
        return ::std::dynamic_pointer_cast<T>(shared_from_this());
    }
    template < typename T >
    const_shared_entity< T >
    shared_this() const
    {
        static_assert(::std::is_base_of<entity, T>::value,
                "Cannot cast to non-entity");
        return ::std::dynamic_pointer_cast<T const>(shared_from_this());
    }
private:
    scope_weak_ptr    owner_;
    ::std::string     name_;
};

using entity_set = ::std::set<entity_ptr, entity::name_compare>;

//----------------------------------------------------------------------------
/**
 * Base class for type entities
 */
class type : public virtual entity {
public:
    /**
     * Check if name is built-in type
     * @param name
     * @return
     */
    static bool
    is_built_in(qname const& name);
protected:
    /**
     * Constructor for built-in types only
     * @param name
     */
    type(::std::string const& name) : entity(name) {}
    /**
     * Create type in a scope with a name
     * @param parent
     * @param name
     */
    type(scope_ptr parent, ::std::string const& name)
        : entity(parent, name) {}
};
using type_ptr =  ::std::shared_ptr< type >;
using type_list = ::std::vector< type_ptr >;

//----------------------------------------------------------------------------
class forward_declaration : public type {
public:
    enum forward_type {
        structure,
        interface,
        class_,
        exception
    };

    forward_declaration(scope_ptr sc, ::std::string const& name, forward_type fw)
        : entity(sc, name), type(sc, name), fw_(fw)
    {
    }
private:
    forward_type fw_;
};
//----------------------------------------------------------------------------
/**
 * Class for type alias
 */
class type_alias : public type {
public:
    /**
     * Construct a type alias to aliased type with name name in scope sc
     * @param sc
     * @param name
     * @param aliased
     */
    type_alias(scope_ptr sc, ::std::string const& name, type_ptr aliased)
        : entity(sc, name), type(sc, name), type_{aliased}
    {
    }

    type_ptr
    alias() const
    { return type_; }
private:
    type_ptr    type_;
};

//----------------------------------------------------------------------------
class parametrized_type : public type {
};


//----------------------------------------------------------------------------
/**
 * Wire IDL constant item
 */
class constant : public entity {
public:
    constant( scope_ptr sc, ::std::string const& type,
            ::std::string const& name,
            ::std::string const& literal);
private:
    explicit
    constant(::std::string const& name);
    static constant
    create_dummy(::std::string const& name);
    type_ptr        type_;
    ::std::string    literal_;
};


using constant_ptr = ::std::shared_ptr<constant>;
using constant_list = ::std::set< constant_ptr, entity::name_compare >;
//----------------------------------------------------------------------------
/**
 * Wire IDL variable item (for data members and function parameters)
 */
class variable : public entity {
public:
    variable(scope_ptr sc, ::std::string const& name, type_ptr t)
        : entity(sc, name), type_(t) {/*TODO Check type is not void*/}
private:
    type_ptr type_;
};
using variable_ptr = shared_entity< variable >;
using variable_list = ::std::vector< variable_ptr >;

//----------------------------------------------------------------------------
/**
 * Wire IDL function item (for member function)
 */

class function : public entity {
public:
    function(interface_ptr sc, ::std::string const& name, type_ptr ret = type_ptr{});

    variable_ptr
    add_parameter(::std::string const& name, type_ptr type);

    bool&
    is_const()
    { return is_const_; }

    bool
    is_const() const
    { return is_const_; }
private:
    type_ptr         ret_type_;
    variable_list    parameters_;
    bool             is_const_ = false;

};
using function_ptr = shared_entity<function>;
using function_list = ::std::vector< function_ptr >;

//----------------------------------------------------------------------------
/**
 * Types and constants container
 */
class scope : public virtual entity {
public:
    virtual ~scope() {}

    //@{
    /** @name Scope lookup */
    ::std::pair<scope_ptr, scope_ptr>
    find_scope(qname const& name) const;
    ::std::pair<scope_ptr, scope_ptr>
    find_scope(qname_search const& search) const;

    scope_ptr
    find_scope_of(qname const& name) const;
    scope_ptr
    find_scope_of(qname_search const& search) const;
    //@}
    //@{
    /** Entity lookup */
    entity_ptr
    find_entity(qname const& name) const;
    entity_ptr
    find_entity(qname_search const& search) const;

    template< typename T >
    shared_entity< T >
    find_entity(qname const& name) const
    {
        static_assert(::std::is_base_of<entity, T>::value,
            "Cannot cast to non-entity");
        return ::std::dynamic_pointer_cast<T>(find_entity(name));
    }
    //@}
    //@{
    /** @name Type lookup */
    type_ptr
    find_type(qname const& name) const;
    type_ptr
    find_type(qname_search const& search) const;
    template < typename T >
    shared_entity< T >
    find_type(qname const& name) const
    {
        static_assert(::std::is_base_of< entity, T >::value,
            "Cannot cast to non-type");
        type_ptr t = find_type(name);
        return dynamic_entity_cast<T>(t);
    }
    template < typename T >
    shared_entity< T >
    find_type(qname_search const& search) const
    {
        static_assert(::std::is_base_of< entity, T >::value,
            "Cannot cast to non-type");
        type_ptr t = find_type(search);
        return dynamic_entity_cast<T>(t);
    }
    //@}

    template < typename T, typename ... Y >
    shared_entity< T >
    add_type(qname const& qn, Y&& ... args )
    {
        static_assert( ::std::is_base_of< type, T >::value,
            "Cannot add a non-type");
        if (qn.size() > 1)
            throw ::std::runtime_error("Wrong place to add type");
        if (qn.empty())
            throw ::std::runtime_error("Name is empty");
        shared_entity< T > t = ::std::make_shared< T >(
                shared_this< scope >(), qn.name(),
                ::std::forward< Y >(args) ... );
        types_.push_back( t );
        return t;
    }
    /**
     * Access types in this scope
     * @return
     */
    type_list const&
    types() const
    { return types_; }

    /**
     * Access constants in this scope
     * @return
     */
    constant_list const&
    constants() const
    { return constants_; }
protected:
    scope() : entity() {}
    /**
     * Create a scope with a parent and name
     * @param parent
     * @param name
     */
    scope(scope_ptr parent, ::std::string const& name)
        : entity(parent, name) {}

    virtual type_ptr
    local_type_search(qname_search const& search) const;
    virtual scope_ptr
    local_scope_search(qname_search const& search) const;
    virtual entity_ptr
    local_entity_search(qname_search const& search) const;
protected:
    type_list        types_;
    constant_list    constants_;
};

//----------------------------------------------------------------------------
/**
 * IDL Namespace. Can contain types, constants and nested namespaces
 */
class namespace_ : public scope {
public:
    static namespace_ptr
    global();

    static void
    clear_global();
public:
    namespace_(scope_ptr parent, ::std::string const& name)
        : entity(parent, name), scope(parent, name) {}

    namespace_ptr
    find_namespace(qname const& qn) const;
    namespace_ptr
    find_namespace(qname_search const& search) const;

    /**
     * Check if this namespace is global
     * @return
     */
    bool
    is_global() const;

    /**
     * Create or find existing namespace specified by qualified name.
     * @param qn Qualified name of the namespace
     * @return
     */
    namespace_ptr
    add_namespace(qname const& qn);
    /**
     * Create or fine namespace specified by qname_search object.
     * @param qn
     * @return
     */
    namespace_ptr
    add_namespace(qname_search const& qn);

    /**
     * Access nested namespaces list
     * @return
     */
    namespace_list const&
    nested() const
    { return nested_; }
protected:
    namespace_() : entity(), scope() {}
private:
    scope_ptr
    local_scope_search(qname_search const& search) const override;
    entity_ptr
    local_entity_search(qname_search const& search) const override;
private:
    namespace_list    nested_;
};

//----------------------------------------------------------------------------
/**
 * Wire IDL enum item
 */
class enumeration : public type {
public:
    // TODO Expression for value, can contain integral constants
    // and already defined enumerators
    using optional_value = ::boost::optional< integral_literal >;
    using enumerator = ::std::pair< ::std::string, optional_value >;
    using enumerator_list = ::std::vector<enumerator>;
public:
    enumeration(scope_ptr owner, ::std::string const& name)
        : entity(owner, name), type(owner, name) {}

    void
    add_enumerator(::std::string const& name,
        optional_value = optional_value{});
private:
    enumerator_list enumerators_;
};
//----------------------------------------------------------------------------
/**
 * Wire IDL struct item
 */
class structure : public virtual type, public virtual scope {
public:
    structure(scope_ptr sc, ::std::string const& name)
        : entity(sc, name), type(sc, name), scope(sc, name) {}

    variable_ptr
    add_data_member( ::std::string const& name, type_ptr t );
    variable_ptr
    find_member( ::std::string const& name) const;
protected:
    entity_ptr
    local_entity_search(qname_search const& search) const override;
protected:
    variable_list data_members_;
};

//----------------------------------------------------------------------------
/**
 * Wire IDL interface item
 */
class interface : public virtual type, public virtual scope {
public:
    interface(scope_ptr sc, ::std::string const& name,
            interface_list const& ancestors = interface_list{})
        : entity(sc, name), type(sc, name), scope(sc, name),
          ancestors_{ ancestors } {}
    // TODO function members
    function_ptr
    add_function(::std::string const& name, type_ptr t = type_ptr{});
    function_ptr
    find_function(::std::string const& name) const;
protected:
    entity_ptr
    ancestors_entity_search(qname_search const& search) const;
    type_ptr
    ancestors_type_search(qname_search const& search) const;

    entity_ptr
    local_entity_search(qname_search const& search) const override;
    type_ptr
    local_type_search(qname_search const& search) const override;
protected:
    interface_list ancestors_;
    function_list functions_;
};

//----------------------------------------------------------------------------
/**
 * Wire IDL class item
 */
class class_;
using class_ptr = shared_entity<class_>;

class class_ : public structure, public interface {
public:
    class_(scope_ptr sc, ::std::string const& name,
            class_ptr parent = class_ptr{},
            interface_list const& implements = interface_list{})
        : entity(sc, name),
          type(sc, name),
          scope(sc, name),
          structure(sc, name),
          interface(sc, name, implements),
          parent_(parent)
    {}
private:
    entity_ptr
    local_entity_search(qname_search const& search) const override;
    type_ptr
    local_type_search(qname_search const& search) const override;
private:
    class_ptr parent_;
};

//----------------------------------------------------------------------------
/**
 * Wire IDL class item
 */
class exception;
using exception_ptr = shared_entity<exception>;

class exception : public structure {
public:
    exception(scope_ptr sc, ::std::string const& name,
            exception_ptr parent = exception_ptr{})
        : entity(sc, name),
          type(sc, name),
          scope(sc, name),
          structure(sc, name),
          parent_(parent)
    {}
private:
    entity_ptr
    local_entity_search(qname_search const& search) const override;
    type_ptr
    local_type_search(qname_search const& search) const override;
private:
    exception_ptr parent_;
};

//----------------------------------------------------------------------------
template < typename T, typename U >
shared_entity< T >
dynamic_entity_cast(::std::shared_ptr< U > const& e)
{
    static_assert(::std::is_base_of< entity, U >::value,
        "Cast should be used for idl::ast::entity objects only");
    static_assert(::std::is_base_of< entity, T >::value,
        "Cast should be used for idl::ast::entity objects only");

    return ::std::dynamic_pointer_cast<T>(e);
}

template < typename T  >
shared_entity< T >
dynamic_type_cast(type_ptr t)
{
    static_assert(::std::is_base_of< entity, T >::value,
        "Cast should be used for idl::ast::entity objects only");

    shared_entity< type_alias > ta = dynamic_entity_cast< type_alias >(t);
    if (ta) {
        return dynamic_type_cast<T>(ta->alias());
    }

    return dynamic_entity_cast<T>(t);
}

}  // namespace ast
}  // namespace idl
}  // namespace wire



#endif /* WIRE_IDL_AST_HPP_ */
