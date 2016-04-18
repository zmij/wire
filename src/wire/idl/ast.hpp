/*
 * ast.hpp
 *
 *  Created on: 25 февр. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef WIRE_IDL_AST_HPP_
#define WIRE_IDL_AST_HPP_

#include <memory>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iosfwd>

#include <wire/idl/source_location.hpp>

namespace wire {
namespace idl {

class entity;
typedef ::std::shared_ptr< entity > entity_ptr;

class scope;
typedef ::std::shared_ptr<scope> scope_ptr;
typedef ::std::weak_ptr<scope> scope_weak_ptr;

class namespace_;
using namespace_ptr = ::std::shared_ptr< namespace_ >;
using namespace_list = ::std::map< ::std::string, namespace_ptr >;

struct qname;
struct qname_search;

class entity : public ::std::enable_shared_from_this<entity> {
public:
    virtual ~entity() {}
    ::std::string const&
    name() const
    { return name_; }

    scope_ptr
    parent() const
    { return scope_.lock(); }

    virtual qname
    get_qualified_name() const;

    struct name_compare {
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
    };
    template < typename T >
    ::std::shared_ptr<T>
    shared_this()
    {
        static_assert(::std::is_base_of<entity, T>::value,
                "Cannot cast to non-entity");
        return ::std::dynamic_pointer_cast<T>(shared_from_this());
    }
protected:
    /**
     * Constructor for the global namespace only.
     */
    entity();
    /**
     * Constructor for built-in types
     * @param name
     */
    explicit
    entity(::std::string const& name);

    /**
     * Create an entity in the given scope
     * @param sc
     * @param name
     */
    entity(scope_ptr sc, ::std::string const& name);

private:
    scope_weak_ptr    scope_;
    ::std::string     name_;
};

typedef ::std::set<entity_ptr, entity::name_compare> entity_set;

//----------------------------------------------------------------------------
class type : public virtual entity {
public:
    static bool
    is_biult_in(qname const& name);
protected:
    /**
     * Constructor for built-in types only
     * @param name
     */
    type(::std::string const& name) : entity(name) {}
};
using type_ptr =  ::std::shared_ptr< type >;
using type_list = ::std::map< ::std::string, type_ptr >;

//----------------------------------------------------------------------------
class type_alias : public type {
public:
    type_alias(scope_ptr sc, ::std::string const& name, type_ptr aliased);
private:
    type_ptr    type_;
};

//----------------------------------------------------------------------------
class parametrized_type : public type {
};


//----------------------------------------------------------------------------
/**
 * Constant declaration
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
 * Types and constants container
 */
class scope : public virtual entity {
public:
    virtual ~scope() {}
    void
    add_type(type_ptr);

    entity_ptr
    find_name(qname const& name) const;
    virtual entity_ptr
    find_name(qname_search const& search) const;

    template< typename T >
    ::std::shared_ptr< T >
    find_name(qname const& name) const
    {
        static_assert(::std::is_base_of<entity, T>::value,
            "Cannot cast to non-entity");
        auto ptr = ::std::dynamic_pointer_cast<T>(find_name(name));
        if (!ptr)
            throw ::std::runtime_error("Incorrect type cast");
        return ptr;
    }
    void
    add_constant(constant&& c);

    type_list const&
    types() const
    { return types_; }

    constant_list const&
    constants() const
    { return constants_; }
protected:
    scope() : entity{} {}
    scope(scope_ptr parent, ::std::string const& name)
        : entity(parent, name) {}
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

    using scope::find_name;
    entity_ptr
    find_name(qname_search const& search) const override;

    bool
    is_global() const;

    namespace_ptr
    add_namespace(qname const& qn);
    namespace_ptr
    add_namespace(qname_search const& qn);

    namespace_list const&
    nested() const
    { return nested_; }
protected:
    namespace_() : scope{} {}
private:
    namespace_list    nested_;
};



}  // namespace ast
}  // namespace wire



#endif /* WIRE_IDL_AST_HPP_ */
