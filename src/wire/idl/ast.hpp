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
#include <boost/variant.hpp>

#include <wire/idl/qname.hpp>
#include <wire/idl/type_name.hpp>
#include <wire/idl/source_location.hpp>

#include <wire/idl/grammar/declarations.hpp>

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
using scope_weak_ptr                = weak_entity<scope>;

class namespace_;
using namespace_ptr                 = ::std::shared_ptr< namespace_ >;
using namespace_list                = ::std::map< ::std::string, namespace_ptr >;

class global_namespace;
using global_namespace_ptr          = ::std::shared_ptr< global_namespace >;

class structure;
using structure_ptr                 = shared_entity< structure >;

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

class generator;

//----------------------------------------------------------------------------
template < typename T, typename U >
shared_entity< T >
dynamic_entity_cast(::std::shared_ptr< U > const& e);

//----------------------------------------------------------------------------
struct compilation_unit {
    using entity_list = ::std::vector< entity_ptr >;

    ::std::string   name;
    entity_list     entities;

    compilation_unit( ::std::string const& n )
        : name{n} {}
    virtual ~compilation_unit() {}

    void
    generate(generator&) const;

    entity_const_set
    dependecies() const;

    entity_const_set
    external_dependencies() const;

    compilation_unit_const_set
    dependent_units() const;

    void
    collect_elements(entity_const_set&,
            entity_predicate pred = [](entity_const_ptr){ return true; }) const;

    bool
    has_classes() const;
    bool
    has_interfaces() const;
    bool
    has_exceptions() const;

    virtual bool
    is_builtin() const
    { return false; }

    template < typename T >
    bool
    has() const
    {
        entity_const_set ents;
        collect_elements(ents,
        [](entity_const_ptr e)
        {
            return (bool)dynamic_entity_cast<T>(e);
        });
        return !ents.empty();
    }

    ::std::int64_t
    get_hash() const noexcept;
};

//----------------------------------------------------------------------------
/**
 * Base AST entity class
 */
class entity : public ::std::enable_shared_from_this<entity> {
public:
    virtual ~entity() {}

    /**
     * Get position in input stream where the entity was first declared
     * @return
     */
    ::std::size_t
     decl_position() const
    { return decl_pos_; }
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

    virtual type_name
    get_type_name() const;

    ::std::int64_t
    get_name_hash() const noexcept;

    virtual ::std::int64_t
    get_hash() const noexcept;

    grammar::annotation_list const&
    get_annotations() const
    { return annotations_; }

    void
    add_annotations(grammar::annotation_list const& ann)
    {
        annotations_.insert(annotations_.end(), ann.begin(), ann.end());
    }

    compilation_unit_ptr
    unit() const
    { return compilation_unit_.lock(); }

    global_namespace_ptr
    get_global() const;

    entity_const_set
    depends_on() const
    {
        entity_const_set deps;
        collect_dependencies(deps);
        return deps;
    }
    void
    collect_dependencies(entity_const_set& deps) const
    {
        collect_dependencies(deps, [](entity_const_ptr) {return true;});
    }
    virtual void
    collect_dependencies(entity_const_set& deps, entity_predicate pred) const
    {}

    virtual void
    collect_elements(entity_const_set& elems, entity_predicate pred) const;
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
    entity() : owner_{}, name_{}, decl_pos_{0} {}
    /**
     * Constructor for built-in types
     * @param name
     */
    explicit
    entity(::std::string const& name);

    /**
     * Create an entity in the given scope
     * @param parent
     * @param pos Declaration position in the compilation unit
     * @param name
     */
    entity(scope_ptr parent, ::std::size_t pos, ::std::string const& name);

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
    scope_weak_ptr              owner_;
    ::std::string               name_;
    ::std::size_t               decl_pos_;
    grammar::annotation_list    annotations_;
    compilation_unit_weak_ptr   compilation_unit_;
};

//----------------------------------------------------------------------------
class entity_conflict : public ::std::runtime_error {
public:
    entity_conflict( entity_ptr prev, ::std::size_t pos )
        : runtime_error("Entity conflict"), previous{prev}, pos{pos} {}

    entity_ptr      previous;
    ::std::size_t   pos;
};

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
    type(scope_ptr parent, ::std::size_t pos, ::std::string const& name)
        : entity(parent, pos, name) {}
};
using type_ptr =  shared_entity< type >;
using type_const_ptr =  const_shared_entity< type >;
using type_list = ::std::vector< type_ptr >;

//----------------------------------------------------------------------------
class forward_declaration : public type {
public:
    enum forward_kind {
        unknown,
        structure,
        interface,
        class_,
        exception
    };

    forward_declaration(scope_ptr sc, ::std::size_t pos,
            ::std::string const& name, ::std::string const& what);

    bool
    is_resolved() const;

    void
    resolve( type_ptr t )
    {
        resolved_ = t;
    }

    type_ptr
    forwarded_type() const;

    bool
    is_compatible(entity_ptr en) const;

    forward_kind
    kind() const
    { return fw_; }

    static forward_kind
    parse_forward_type(::std::string const& what);
private:
    forward_kind        fw_;
    mutable type_ptr    resolved_;
};

using forward_declaration_ptr = shared_entity< forward_declaration >;
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
    type_alias(scope_ptr sc, ::std::size_t pos, ::std::string const& name, type_ptr aliased)
        : entity(sc, pos, name), type(sc, pos, name), type_{aliased}
    {
    }

    type_ptr
    alias() const
    { return type_; }

    ::std::int64_t
    get_hash() const noexcept override
    { return type_->get_hash(); }

    void
    collect_dependencies(entity_const_set& deps, entity_predicate pred) const override
    {
        if (pred(type_))
            deps.insert(type_);
        type_->collect_dependencies(deps, pred);
    }
private:
    type_ptr    type_;
};

using type_alias_ptr = shared_entity< type_alias >;

//----------------------------------------------------------------------------
struct template_param_type {
    enum param_kind {
        type,
        integral
    } kind;
    bool unbound;
};

using template_param_types = ::std::vector< template_param_type >;

class parametrized_type;
using parametrized_type_ptr = shared_entity< parametrized_type >;

//----------------------------------------------------------------------------
// Templated classes are predefined
class templated_type : public type {
public:
    parametrized_type_ptr
    create_parametrized_type(scope_ptr sc, ::std::size_t pos) const;

    template_param_types const&
    formal_parameters() const
    { return params; }
protected:
    templated_type(::std::string const& name, template_param_types&& args)
        : entity(name), type(name), params{ ::std::move(args) }
    {
    }
    template_param_types params;
};
using templated_type_ptr = shared_entity< templated_type >;
using templated_type_const_ptr = const_shared_entity< templated_type >;


//----------------------------------------------------------------------------
class parametrized_type : public type {
public:
    using parameter = ::boost::variant< type_ptr, ::std::string >;
    using parameters = ::std::vector< parameter >;
public:
    void
    add_parameter(parameter const&);

    ::std::int64_t
    get_hash() const noexcept override;
    void
    collect_dependencies(entity_const_set& deps, entity_predicate pred) const override;

    parameters const&
    params() const
    { return params_; }
private:
    friend parametrized_type_ptr
    templated_type::create_parametrized_type(scope_ptr sc, ::std::size_t pos) const;

    parametrized_type(scope_ptr sc, ::std::size_t pos, ::std::string const& name,
            templated_type_const_ptr tmpl)
        : entity(sc, pos, name), type(sc, pos, name),
          tmpl_type(tmpl),
          current{tmpl_type->formal_parameters().begin()}
        {}
private:
    templated_type_const_ptr                tmpl_type;
    template_param_types::const_iterator    current;
    parameters                              params_;
};

//----------------------------------------------------------------------------
/**
 * Wire IDL constant item
 */
class constant : public entity {
public:
    constant( scope_ptr sc, ::std::size_t pos, ::std::string const& name,
            type_ptr t, grammar::data_initializer const&);
    void
    collect_dependencies(entity_const_set& deps, entity_predicate pred) const override
    {
        if (pred(type_))
            deps.insert(type_);
        type_->collect_dependencies(deps, pred);
    }

    type_ptr
    get_type() const
    { return type_; }

    grammar::data_initializer
    get_init() const
    { return init_; }

    ::std::int64_t
    get_hash() const noexcept override;
private:
    type_ptr                    type_;
    grammar::data_initializer   init_;
};


using constant_ptr = ::std::shared_ptr<constant>;
using constant_list = ::std::vector< constant_ptr >;
//----------------------------------------------------------------------------
/**
 * Wire IDL variable item (for data members and function parameters)
 */
class variable : public entity {
public:
    variable(scope_ptr sc, ::std::size_t pos, ::std::string const& name, type_ptr t)
        : entity(sc, pos, name), type_(t) {/*TODO Check type is not void*/}

    void
    collect_dependencies(entity_const_set& deps, entity_predicate pred) const override
    {
        if (pred(type_))
            deps.insert(type_);
        type_->collect_dependencies(deps, pred);
    }

    type_ptr
    get_type() const
    { return type_; }

    ::std::int64_t
    get_hash() const noexcept override;
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
    using function_param = ::std::pair< type_ptr, ::std::string >;
    using function_params = ::std::vector< function_param >;
public:
    function(interface_ptr sc, ::std::size_t pos, ::std::string const& name,
        type_ptr ret, bool is_const,
        function_params const& params,
        exception_list const& t_spec);

    bool&
    is_const()
    { return is_const_; }

    bool
    is_const() const
    { return is_const_; }

    bool
    is_void() const;

    void
    collect_dependencies(entity_const_set& deps, entity_predicate pred) const override;

    type_ptr
    get_return_type() const
    { return ret_type_; }

    function_params const&
    get_params() const
    { return parameters_; }

    exception_list const&
    throw_specification() const
    { return throw_spec_; }

    ::std::int64_t
    get_hash() const noexcept override;
private:
    type_ptr         ret_type_;
    function_params  parameters_;
    bool             is_const_ = false;
    exception_list   throw_spec_;

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
    virtual entity_ptr
    local_entity_search(qname_search const& search) const;
    //@}
    //@{
    /** @name Type lookup */
    type_ptr
    find_type(type_name const& name, ::std::size_t pos) const;
    type_ptr
    find_type(qname const& name) const;
    type_ptr
    find_type(qname_search const& search) const;
    template < typename T >
    shared_entity< T >
    find_type(type_name const& name, ::std::size_t pos) const
    {
        static_assert(::std::is_base_of< entity, T >::value,
            "Cannot cast to non-type");
        type_ptr t = find_type(name, pos);
        return dynamic_entity_cast<T>(t);
    }
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
    add_type(::std::size_t pos, qname const& qn, Y&& ... args )
    {
        auto res = add_type_impl<T>(pos, qn, ::std::forward<Y>(args) ... );
        on_add_entity(res);
        return res;
    }
    /**
     * Access types in this scope
     * @return
     */
    type_list const&
    get_types() const
    { return types_; }

    constant_ptr
    add_constant(::std::size_t pos, ::std::string const& name, type_ptr t,
            grammar::data_initializer const& init);
    /**
     * Access constants in this scope
     * @return
     */
    constant_list const&
    get_constants() const
    { return constants_; }

    void
    collect_dependencies(entity_const_set& deps, entity_predicate pred) const override;
    void
    collect_elements(entity_const_set& elems, entity_predicate pred) const override;

    ::std::int64_t
    get_hash() const noexcept override;
protected:
    scope() : entity() {}
    /**
     * Create a scope with a parent and name
     * @param parent
     * @param name
     */
    scope(scope_ptr parent, ::std::size_t pos, ::std::string const& name)
        : entity(parent, pos, name) {}

    virtual type_ptr
    local_type_search(qname_search const& search) const;
    virtual scope_ptr
    local_scope_search(qname_search const& search) const;
private:
    /**
     * Add type implementation for type alias
     * @param pos
     * @param qn
     * @param args
     * @return
     */
    template < typename T, typename ... Y >
    typename ::std::enable_if<
         ::std::is_same<type_alias, T>::value, shared_entity< T >>::type
    add_type_impl(::std::size_t pos, qname const& qn, type_ptr aliased, Y&& ... args)
    {
        static_assert( ::std::is_base_of< type, T >::value,
            "Cannot add a non-type");
        if (qn.size() > 1)
            throw ::std::runtime_error("Wrong place to add type");
        if (qn.empty())
            throw ::std::runtime_error("Name is empty");

        entity_ptr en = local_entity_search(qn.search());
        if (en) {
            if (auto ta = dynamic_entity_cast< type_alias >(en)) {
                // Check if aliased types are the same
                if (aliased != ta->alias())
                    throw entity_conflict(en, pos);
            } else {
                throw entity_conflict(en, pos);
            }
        }

        shared_entity< T > t = ::std::make_shared< T >(
                shared_this< scope >(), pos, qn.name(), aliased,
                ::std::forward< Y >(args) ... );
        types_.push_back( t );
        return t;
    }
    /**
     * Add type implementation for forward declarations
     * @param pos
     * @param qn
     * @param args
     * @return
     */
    template < typename T, typename ... Y >
    typename ::std::enable_if<
            ::std::is_same< forward_declaration, T >::value,
            shared_entity< T >>::type
    add_type_impl(::std::size_t pos, qname const& qn, Y&& ... args)
    {
        if (qn.size() > 1)
            throw ::std::runtime_error("Wrong place to add type");
        if (qn.empty())
            throw ::std::runtime_error("Name is empty");

        shared_entity< T > t = ::std::make_shared< T >(
                shared_this< scope >(), pos, qn.name(),
                ::std::forward< Y >(args) ... );
        entity_ptr en = local_entity_search(qn.search());
        if (en) {
            // Check if the entity is a forward or a forward declared type
            if (!t->is_compatible(en)) {
                throw entity_conflict(en, pos);
            }
        }

        forwards_.push_back(t);
        return t;
    }
    /**
     * Add type implementation for all other types
     * @param pos
     * @param qn
     * @param args
     * @return
     */
    template < typename T, typename ... Y >
    typename ::std::enable_if<
         !(::std::is_same<type_alias, T>::value ||
                 ::std::is_same< forward_declaration, T >::value),
         shared_entity< T >>::type
    add_type_impl(::std::size_t pos, qname const& qn, Y&& ... args)
    {
        static_assert( ::std::is_base_of< type, T >::value,
            "Cannot add a non-type");
        if (qn.size() > 1)
            throw ::std::runtime_error("Wrong place to add type");
        if (qn.empty())
            throw ::std::runtime_error("Name is empty");

        shared_entity< T > t = ::std::make_shared< T >(
                shared_this< scope >(), pos, qn.name(),
                ::std::forward< Y >(args) ... );

        entity_ptr en = local_entity_search(qn.search());
        forward_declaration_ptr fwd;
        if (en) {
            // Check if the entity is a forward
            fwd = dynamic_entity_cast< forward_declaration >(en);
            if (!fwd) {
                throw entity_conflict(en, pos);
            } else if (!fwd->is_compatible(t)) {
                throw entity_conflict(en, pos);
            }
        }

        types_.push_back( t );

        // Resolve forward declaration if any
        if (fwd) {
            fwd->resolve(t);
        }

        return t;
    }
private:
    virtual void
    on_add_entity(entity_ptr) {}
protected:
    type_list        types_;
    type_list        forwards_;
    constant_list    constants_;
};

//----------------------------------------------------------------------------
/**
 * IDL Namespace. Can contain types, constants and nested namespaces
 */
class namespace_ : public scope {
public:
    namespace_(scope_ptr parent, ::std::size_t pos, ::std::string const& name)
        : entity(parent, pos, name), scope(parent, pos, name) {}

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
    add_namespace(::std::size_t pos, qname const& qn);
    /**
     * Create or fine namespace specified by qname_search object.
     * @param qn
     * @return
     */
    namespace_ptr
    add_namespace(::std::size_t pos, qname_search const& qn);

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
    void
    on_add_entity(entity_ptr en) override;
private:
    namespace_list    nested_;
};

//----------------------------------------------------------------------------
class global_namespace : public namespace_ {
public:
    static global_namespace_ptr
    create();

    compilation_unit_ptr
    current_compilation_unit();
    void
    set_current_compilation_unit(::std::string const& name);

    void
    on_add_entity(entity_ptr en) override;
private:
    global_namespace()
        : namespace_()
    {
    }
private:
    using unit_list = ::std::map< ::std::string, compilation_unit_ptr >;
    compilation_unit_ptr    current_;
    unit_list               units_;
};


//----------------------------------------------------------------------------
/**
 * Wire IDL enum item
 */
class enumeration : public type {
public:
    using optional_value = ::boost::optional< ::std::string >;
    using enumerator = ::std::pair< ::std::string, optional_value >;
    using enumerator_list = ::std::vector<enumerator>;
public:
    enumeration(scope_ptr owner, ::std::size_t pos, ::std::string const& name,
            bool constrained)
        : entity(owner, pos, name), type(owner, pos, name), constrained_(constrained) {}

    void
    add_enumerator(::std::size_t pos, ::std::string const& name,
        optional_value = optional_value{});

    bool
    constrained() const
    { return constrained_; }

    enumerator_list const&
    get_enumerators() const
    { return enumerators_; }

    ::std::int64_t
    get_hash() const noexcept override;
private:
    bool            constrained_;
    enumerator_list enumerators_;

};

using enumeration_ptr = shared_entity< enumeration >;
//----------------------------------------------------------------------------
/**
 * Wire IDL struct item
 */
class structure : public virtual type, public virtual scope {
public:
    structure(scope_ptr sc, ::std::size_t pos, ::std::string const& name)
        : entity(sc, pos, name), type(sc, pos, name), scope(sc, pos, name) {}

    variable_ptr
    add_data_member(::std::size_t pos, ::std::string const& name, type_ptr t );

    variable_list const&
    get_data_members() const

    { return data_members_; }
    variable_ptr
    find_member( ::std::string const& name) const;
    void
    collect_dependencies(entity_const_set& deps, entity_predicate pred) const override;
    void
    collect_elements(entity_const_set& elems, entity_predicate pred) const override;

    ::std::int64_t
    get_hash() const noexcept override;
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
    using function_param = function::function_param;
    using function_params = function::function_params;
public:
    interface(scope_ptr sc, ::std::size_t pos, ::std::string const& name,
            interface_list const& ancestors = interface_list{})
        : entity(sc, pos, name), type(sc, pos, name), scope(sc, pos, name),
          ancestors_{ ancestors } {}

    function_ptr
    add_function(::std::size_t pos, ::std::string const& name, type_ptr t = type_ptr{},
        bool is_const = false,
        function_params const& params = function_params{},
        exception_list const& t_spec = exception_list{});
    function_ptr
    find_function(::std::string const& name) const;

    void
    collect_dependencies(entity_const_set& deps, entity_predicate pred) const override;
    void
    collect_elements(entity_const_set& elems, entity_predicate pred) const override;

    interface_list const&
    get_ancestors() const
    { return ancestors_; }

    /**
     * Collect all ancestors in depth-first search
     * @param ifaces
     * @param pred
     */
    void
    collect_ancestors(interface_list& ifaces,
            entity_predicate pred = [](entity_const_ptr){ return true; }) const;

    function_list const&
    get_functions() const
    { return functions_; }

    ::std::int64_t
    get_hash() const noexcept override;
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
 * Wire IDL reference to interface item
 */
class reference : public type {
public:
    reference(type_ptr iface);
    void
    collect_dependencies(entity_const_set& deps, entity_predicate pred) const override;

    ::std::int64_t
    get_hash() const noexcept override;
};

//----------------------------------------------------------------------------
/**
 * Wire IDL class item
 */
class class_;
using class_ptr = shared_entity<class_>;
using class_const_ptr = const_shared_entity< class_ >;

class class_ : public structure, public interface {
public:
    class_(scope_ptr sc, ::std::size_t pos, ::std::string const& name,
            class_const_ptr parent = class_const_ptr{},
            interface_list const& implements = interface_list{})
        : entity(sc, pos, name),
          type(sc, pos, name),
          scope(sc, pos, name),
          structure(sc, pos, name),
          interface(sc, pos, name, implements),
          parent_(parent)
    {}
    void
    collect_dependencies(entity_const_set& deps, entity_predicate pred) const override;
    void
    collect_elements(entity_const_set& elems, entity_predicate pred) const override;
    ::std::int64_t
    get_hash() const noexcept override;
private:
    entity_ptr
    local_entity_search(qname_search const& search) const override;
    type_ptr
    local_type_search(qname_search const& search) const override;
private:
    class_const_ptr parent_;
};

//----------------------------------------------------------------------------
/**
 * Wire IDL class item
 */
class exception : public structure {
public:
    exception(scope_ptr sc, ::std::size_t pos, ::std::string const& name,
            exception_const_ptr parent = exception_const_ptr{})
        : entity(sc, pos, name),
          type(sc, pos, name),
          scope(sc, pos, name),
          structure(sc, pos, name),
          parent_(parent)
    {}
    void
    collect_dependencies(entity_const_set& deps, entity_predicate pred) const override;
    exception_const_ptr
    get_parent() const
    { return parent_; }
    ::std::int64_t
    get_hash() const noexcept override;
private:
    entity_ptr
    local_entity_search(qname_search const& search) const override;
    type_ptr
    local_type_search(qname_search const& search) const override;
private:
    exception_const_ptr parent_;
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

template < typename T, typename U >
const_shared_entity< T >
dynamic_entity_cast(::std::shared_ptr< U const > const& e)
{
    static_assert(::std::is_base_of< entity, U >::value,
        "Cast should be used for idl::ast::entity objects only");
    static_assert(::std::is_base_of< entity, T >::value,
        "Cast should be used for idl::ast::entity objects only");

    return ::std::dynamic_pointer_cast<T const>(e);
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

template < typename T >
const_shared_entity< T >
dynamic_type_cast(type_const_ptr t)
{
    static_assert(::std::is_base_of< entity, T >::value,
        "Cast should be used for idl::ast::entity objects only");

    const_shared_entity< type_alias > ta = dynamic_entity_cast< type_alias >(t);
    if (ta) {
        return dynamic_type_cast<T>(ta->alias());
    }
    return dynamic_entity_cast<T>(t);
}

}  // namespace ast
}  // namespace idl
}  // namespace wire



#endif /* WIRE_IDL_AST_HPP_ */
